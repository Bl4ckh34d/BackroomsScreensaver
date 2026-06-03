#include "audio_engine.h"

namespace {
float Clamp01(float value) {
    return std::clamp(value, 0.0f, 1.0f);
}

float SmoothStep(float edge0, float edge1, float x) {
    if (edge1 <= edge0) return x >= edge1 ? 1.0f : 0.0f;
    float t = Clamp01((x - edge0) / (edge1 - edge0));
    return t * t * (3.0f - 2.0f * t);
}

float Lerp(float a, float b, float t) {
    return a + (b - a) * t;
}
}

void AudioEngine::AddFolder(const AudioEngineAssets& assets, GameSound sound, const std::wstring& folder,
                            const std::wstring& pattern) {
    std::filesystem::path root = ResolveSoundFolder(assets, folder);
    if (root.empty()) return;
    WIN32_FIND_DATAW data{};
    std::filesystem::path query = root / pattern;
    HANDLE find = FindFirstFileW(query.c_str(), &data);
    if (find == INVALID_HANDLE_VALUE) return;
    std::vector<std::filesystem::path> paths;
    do {
        if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) continue;
        paths.push_back(root / data.cFileName);
    } while (FindNextFileW(find, &data));
    FindClose(find);

    std::sort(paths.begin(), paths.end(), [](const std::filesystem::path& a, const std::filesystem::path& b) {
        return a.filename().wstring() < b.filename().wstring();
    });
    for (const std::filesystem::path& path : paths) {
        AudioSample sample{};
        if (LoadWav(path, sample)) {
            groups_[static_cast<size_t>(sound)].push_back(samples_.size());
            samples_.push_back(std::move(sample));
        }
    }
}

bool AudioEngine::LoadWav(const std::filesystem::path& path, AudioSample& out) const {
    std::ifstream in(path, std::ios::binary);
    if (!in) return false;
    auto readU32 = [&]() {
        uint32_t v = 0;
        in.read(reinterpret_cast<char*>(&v), sizeof(v));
        return v;
    };
    auto readU16 = [&]() {
        uint16_t v = 0;
        in.read(reinterpret_cast<char*>(&v), sizeof(v));
        return v;
    };
    uint32_t riff = readU32();
    uint32_t riffSize = readU32();
    (void)riffSize;
    uint32_t wave = readU32();
    if (riff != FourCC('R', 'I', 'F', 'F') || wave != FourCC('W', 'A', 'V', 'E')) return false;

    bool gotFmt = false;
    bool gotData = false;
    while (in && (!gotFmt || !gotData)) {
        uint32_t id = readU32();
        uint32_t size = readU32();
        if (!in) break;
        std::streampos next = in.tellg();
        next += static_cast<std::streamoff>(size + (size & 1u));
        if (id == FourCC('f', 'm', 't', ' ')) {
            if (size < sizeof(WAVEFORMATEX) - sizeof(WORD)) return false;
            WAVEFORMATEX fmt{};
            fmt.wFormatTag = readU16();
            fmt.nChannels = readU16();
            fmt.nSamplesPerSec = readU32();
            fmt.nAvgBytesPerSec = readU32();
            fmt.nBlockAlign = readU16();
            fmt.wBitsPerSample = readU16();
            fmt.cbSize = 0;
            if (size > 16) {
                fmt.cbSize = readU16();
                uint32_t extraBytes = std::min<uint32_t>(fmt.cbSize, size - 18);
                out.formatExtra.resize(extraBytes);
                if (extraBytes > 0) in.read(reinterpret_cast<char*>(out.formatExtra.data()), extraBytes);
                fmt.cbSize = static_cast<WORD>(out.formatExtra.size());
            }
            if (fmt.wFormatTag != WAVE_FORMAT_PCM && fmt.wFormatTag != WAVE_FORMAT_IEEE_FLOAT && fmt.wFormatTag != WAVE_FORMAT_EXTENSIBLE) {
                return false;
            }
            out.format = fmt;
            gotFmt = true;
        } else if (id == FourCC('d', 'a', 't', 'a')) {
            out.data.resize(size);
            if (size > 0) in.read(reinterpret_cast<char*>(out.data.data()), size);
            gotData = !out.data.empty();
        }
        in.seekg(next, std::ios::beg);
    }
    return gotFmt && gotData && out.format.nChannels > 0 && out.format.nSamplesPerSec > 0;
}
