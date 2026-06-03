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
        bool loaded = false;
        std::wstring extension = path.extension().wstring();
        std::transform(extension.begin(), extension.end(), extension.begin(), [](wchar_t c) {
            return static_cast<wchar_t>(std::towlower(c));
        });
        if (extension == L".mp3") {
            loaded = LoadMp3(path, sample);
        } else {
            loaded = LoadWav(path, sample);
        }
        if (loaded) {
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

bool AudioEngine::LoadMp3(const std::filesystem::path& path, AudioSample& out) const {
    if (!mediaFoundationStarted_) return false;

    Microsoft::WRL::ComPtr<IMFSourceReader> reader;
    HRESULT hr = MFCreateSourceReaderFromURL(path.c_str(), nullptr, &reader);
    if (FAILED(hr) || !reader) return false;

    Microsoft::WRL::ComPtr<IMFMediaType> outputType;
    hr = MFCreateMediaType(&outputType);
    if (FAILED(hr) || !outputType) return false;
    outputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
    outputType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
    hr = reader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, nullptr, outputType.Get());
    if (FAILED(hr)) return false;

    Microsoft::WRL::ComPtr<IMFMediaType> currentType;
    hr = reader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, &currentType);
    if (FAILED(hr) || !currentType) return false;

    WAVEFORMATEX* waveFormat = nullptr;
    UINT32 waveFormatSize = 0;
    hr = MFCreateWaveFormatExFromMFMediaType(currentType.Get(), &waveFormat, &waveFormatSize);
    if (FAILED(hr) || !waveFormat || waveFormatSize < sizeof(WAVEFORMATEX)) return false;

    out.format = *waveFormat;
    out.formatExtra.clear();
    const UINT32 baseSize = static_cast<UINT32>(sizeof(WAVEFORMATEX));
    if (waveFormatSize > baseSize) {
        const uint8_t* extraBegin = reinterpret_cast<const uint8_t*>(waveFormat) + baseSize;
        out.formatExtra.assign(extraBegin, extraBegin + (waveFormatSize - baseSize));
        out.format.cbSize = static_cast<WORD>(out.formatExtra.size());
    }
    CoTaskMemFree(waveFormat);

    while (true) {
        DWORD streamFlags = 0;
        LONGLONG timestamp = 0;
        Microsoft::WRL::ComPtr<IMFSample> sample;
        hr = reader->ReadSample(MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, nullptr, &streamFlags, &timestamp, &sample);
        if (FAILED(hr)) return false;
        if ((streamFlags & MF_SOURCE_READERF_ENDOFSTREAM) != 0) break;
        if (!sample) continue;

        Microsoft::WRL::ComPtr<IMFMediaBuffer> buffer;
        hr = sample->ConvertToContiguousBuffer(&buffer);
        if (FAILED(hr) || !buffer) return false;

        BYTE* audioBytes = nullptr;
        DWORD maxLength = 0;
        DWORD currentLength = 0;
        hr = buffer->Lock(&audioBytes, &maxLength, &currentLength);
        if (FAILED(hr)) return false;
        (void)maxLength;
        if (audioBytes && currentLength > 0) {
            size_t offset = out.data.size();
            out.data.resize(offset + currentLength);
            std::memcpy(out.data.data() + offset, audioBytes, currentLength);
        }
        buffer->Unlock();
    }

    return !out.data.empty() && out.format.nChannels > 0 && out.format.nSamplesPerSec > 0;
}
