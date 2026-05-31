enum class AudioBus {
    Effects,
    Ambience,
    Monster
};

enum class GameSound {
    CarpetStep,
    SoakedCarpetStep,
    NeonHumQuiet,
    NeonHumLoud,
    NeonHumLoud2,
    MonsterGrowl,
    MonsterSpottedScream,
    ElectricCrackle,
    NeonFlickerStarterClick,
    AirVentDustPuff,
    WetCarpetCeilingDrip,
    DoorOpenCreak,
    DoorCloseCreak,
    DoorCloseLock,
    LightBulbBreak,
    VisionFlash,
    FlashlightStutter,
    PaperFlutter
};

enum class AudioToneProfile {
    Normal,
    MetallicVent
};

struct AudioSample {
    WAVEFORMATEX format{};
    std::vector<uint8_t> formatExtra;
    std::vector<uint8_t> data;
};

struct AudioVoiceInstance {
    IXAudio2SourceVoice* voice = nullptr;
    size_t sampleIndex = 0;
    AudioBus bus = AudioBus::Effects;
    GameSound sound = GameSound::CarpetStep;
    XMFLOAT3 pos{};
    float baseVolume = 1.0f;
    float frequencyRatio = 1.0f;
    float occlusion = 0.0f;
    float occlusionRefresh = 0.0f;
    AudioToneProfile toneProfile = AudioToneProfile::Normal;
    int tag = -1;
    bool loop = false;
    bool spatial = true;
};

class AudioEngine {
public:
    bool Initialize(const Settings& settings) {
        if (initialized_) {
            ApplySettings(settings);
            return true;
        }
        if (!comInitialized_) {
            comHr_ = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
            comInitialized_ = SUCCEEDED(comHr_);
        }
        HRESULT hr = XAudio2Create(&xaudio_, 0, XAUDIO2_DEFAULT_PROCESSOR);
        if (FAILED(hr) || !xaudio_) {
            OutputDebugStringW(L"Backrooms audio: XAudio2Create failed.\n");
            return false;
        }
        hr = xaudio_->CreateMasteringVoice(&masterVoice_);
        if (FAILED(hr)) {
            OutputDebugStringW(L"Backrooms audio: CreateMasteringVoice failed.\n");
            xaudio_.Reset();
            return false;
        }
        XAUDIO2_VOICE_DETAILS details{};
        masterVoice_->GetVoiceDetails(&details);
        outputChannels_ = std::max<UINT32>(1, details.InputChannels);
        DWORD channelMask = 0;
        masterVoice_->GetChannelMask(&channelMask);
        if (channelMask == 0) {
            channelMask = outputChannels_ == 1 ? SPEAKER_FRONT_CENTER :
                (outputChannels_ == 2 ? SPEAKER_STEREO : SPEAKER_5POINT1);
        }
        X3DAudioInitialize(channelMask, X3DAUDIO_SPEED_OF_SOUND, x3dHandle_);
        initialized_ = true;
        ApplySettings(settings);
        return true;
    }

    void Shutdown() {
        StopAll();
        if (masterVoice_) {
            masterVoice_->DestroyVoice();
            masterVoice_ = nullptr;
        }
        xaudio_.Reset();
        initialized_ = false;
        if (comInitialized_) {
            CoUninitialize();
            comInitialized_ = false;
            comHr_ = E_FAIL;
        }
    }

    void ApplySettings(const Settings& settings) {
        muted_ = settings.audioMuted;
        masterVolume_ = std::clamp(settings.audioMasterVolume, 0.0f, 1.0f);
        effectsVolume_ = std::clamp(settings.audioEffectsVolume, 0.0f, 1.0f);
        ambienceVolume_ = std::clamp(settings.audioAmbienceVolume, 0.0f, 1.0f);
        monsterVolume_ = std::clamp(settings.audioMonsterVolume, 0.0f, 1.0f);
    }

    void LoadAll(const Settings& settings) {
        if (!initialized_) return;
        samples_.clear();
        groups_.clear();
        groups_.resize(static_cast<size_t>(GameSound::PaperFlutter) + 1);
        AddFolder(settings, GameSound::CarpetStep, L"assets\\sounds\\carpet_steps", L"carpet_step_*.wav");
        AddFolder(settings, GameSound::SoakedCarpetStep, L"assets\\sounds\\soaked_carpet_steps", L"soaked_step_*.wav");
        AddFolder(settings, GameSound::NeonHumQuiet, L"assets\\sounds\\neon_light_hum", L"*quiet*.wav");
        AddFolder(settings, GameSound::NeonHumLoud, L"assets\\sounds\\neon_light_hum", L"neon_light_hum_loud.wav");
        AddFolder(settings, GameSound::NeonHumLoud2, L"assets\\sounds\\neon_light_hum", L"neon_light_hum_loud_2.wav");
        AddFolder(settings, GameSound::MonsterGrowl, L"assets\\sounds\\monster_growls");
        AddFolder(settings, GameSound::MonsterSpottedScream, L"assets\\sounds\\monster_spotted_scream");
        AddFolder(settings, GameSound::ElectricCrackle, L"assets\\sounds\\electric_crackling");
        AddFolder(settings, GameSound::NeonFlickerStarterClick, L"assets\\sounds\\neon_light_flicker_start");
        AddFolder(settings, GameSound::AirVentDustPuff, L"assets\\sounds\\air_vent_dust_air_puff");
        AddFolder(settings, GameSound::WetCarpetCeilingDrip, L"assets\\sounds\\wet_carpet_ceiling_drips", L"muted_cardboard_drip_tap_*.wav");
        AddFolder(settings, GameSound::DoorOpenCreak, L"assets\\sounds\\door_open_creak");
        AddFolder(settings, GameSound::DoorCloseCreak, L"assets\\sounds\\door_close_creak");
        AddFolder(settings, GameSound::DoorCloseLock, L"assets\\sounds\\door_close_lock");
        AddFolder(settings, GameSound::LightBulbBreak, L"assets\\sounds\\light_bulb_break", L"light_bulb_break_1.wav");
        AddFolder(settings, GameSound::VisionFlash, L"assets\\sounds\\vision_flash", L"vision_flash_*.wav");
        AddFolder(settings, GameSound::FlashlightStutter, L"assets\\sounds\\flashlight_contact_click", L"flashlight_stutter.wav");
        AddFolder(settings, GameSound::PaperFlutter, L"assets\\sounds\\paper_flutter", L"paper_flutter.wav");
        if (samples_.empty()) {
            OutputDebugStringW(L"Backrooms audio: no WAV samples were loaded.\n");
        }
    }

    void SetListener(XMFLOAT3 pos, XMFLOAT3 forward) {
        listener_.Position = {pos.x, pos.y, pos.z};
        float len = std::sqrt(forward.x * forward.x + forward.y * forward.y + forward.z * forward.z);
        XMFLOAT3 f = len > 0.0001f
            ? XMFLOAT3{forward.x / len, forward.y / len, forward.z / len}
            : XMFLOAT3{0.0f, 0.0f, 1.0f};
        listener_.OrientFront = {f.x, f.y, f.z};
        listener_.OrientTop = {0.0f, 1.0f, 0.0f};
    }

    template <typename OcclusionFn>
    void Update(float dt, OcclusionFn occlusionForPosition) {
        if (!initialized_) return;
        dt = std::max(0.0f, dt);
        for (size_t i = 0; i < voices_.size();) {
            AudioVoiceInstance& instance = voices_[i];
            XAUDIO2_VOICE_STATE state{};
            instance.voice->GetState(&state);
            if (!instance.loop && state.BuffersQueued == 0) {
                instance.voice->DestroyVoice();
                voices_.erase(voices_.begin() + static_cast<std::ptrdiff_t>(i));
                continue;
            }
            float distanceGain = SpatialDistanceGain(instance);
            if (instance.spatial && distanceGain <= 0.0001f) {
                instance.voice->SetVolume(0.0f);
                instance.occlusionRefresh = std::min(instance.occlusionRefresh, 0.10f);
                ++i;
                continue;
            }
            instance.occlusionRefresh -= dt;
            if (instance.occlusionRefresh <= 0.0f) {
                instance.occlusion = std::clamp(occlusionForPosition(instance.pos), 0.0f, 8.0f);
                instance.occlusionRefresh = instance.bus == AudioBus::Ambience ? 0.32f : 0.14f;
            }
            Apply3D(instance);
            ++i;
        }
    }

    template <typename OcclusionFn>
    void Update(OcclusionFn occlusionForPosition) {
        Update(0.0f, occlusionForPosition);
    }

    void Update() {
        Update(0.0f, [](XMFLOAT3) { return 0.0f; });
    }

    void StopAll() {
        for (AudioVoiceInstance& instance : voices_) {
            if (instance.voice) instance.voice->DestroyVoice();
        }
        voices_.clear();
    }

    void PlayRandom(GameSound sound, AudioBus bus, XMFLOAT3 pos, float volume = 1.0f, bool spatial = true,
                    float initialOcclusion = 0.0f) {
        size_t sampleIndex = PickRandomSample(sound);
        if (sampleIndex == kInvalidSample) return;
        StartVoice(sound, sampleIndex, bus, pos, volume, false, spatial, 1.0f, -1, initialOcclusion);
    }

    void StartLoop(GameSound sound, AudioBus bus, XMFLOAT3 pos, float volume = 1.0f, bool spatial = true,
                   float initialOcclusion = 0.0f) {
        size_t sampleIndex = PickRandomSample(sound);
        if (sampleIndex == kInvalidSample) return;
        StartVoice(sound, sampleIndex, bus, pos, volume, true, spatial, 1.0f, -1, initialOcclusion);
    }

    void StartLoopTagged(GameSound sound, AudioBus bus, XMFLOAT3 pos, float volume, bool spatial, int tag,
                         float initialOcclusion = 0.0f) {
        if (tag >= 0 && HasTaggedVoice(tag)) return;
        size_t sampleIndex = PickRandomSample(sound);
        if (sampleIndex == kInvalidSample) return;
        StartVoice(sound, sampleIndex, bus, pos, volume, true, spatial, 1.0f, tag, initialOcclusion);
    }

    void StartLoopTaggedSample(GameSound sound, size_t sampleIndex, AudioBus bus, XMFLOAT3 pos, float volume,
                               bool spatial, int tag, float initialOcclusion = 0.0f) {
        if (tag >= 0 && HasTaggedVoice(tag)) return;
        if (sampleIndex == kInvalidSample) return;
        StartVoice(sound, sampleIndex, bus, pos, volume, true, spatial, 1.0f, tag, initialOcclusion);
    }

    bool HasTaggedVoice(int tag) const {
        if (tag < 0) return false;
        return std::any_of(voices_.begin(), voices_.end(), [tag](const AudioVoiceInstance& instance) {
            return instance.tag == tag;
        });
    }

    void StopTaggedVoice(int tag) {
        if (tag < 0) return;
        for (size_t i = 0; i < voices_.size();) {
            if (voices_[i].tag == tag) {
                if (voices_[i].voice) voices_[i].voice->DestroyVoice();
                voices_.erase(voices_.begin() + static_cast<std::ptrdiff_t>(i));
                continue;
            }
            ++i;
        }
    }

    size_t PickRandomSample(GameSound sound) {
        return PickSample(sound);
    }

    size_t PickStableSample(GameSound sound, uint32_t stableId) const {
        if (!initialized_) return kInvalidSample;
        const std::vector<size_t>& group = groups_[static_cast<size_t>(sound)];
        if (group.empty()) return kInvalidSample;
        return group[static_cast<size_t>(stableId) % group.size()];
    }

    size_t PickRandomSampleExcept(GameSound sound, size_t excludedSampleIndex) {
        if (!initialized_) return kInvalidSample;
        const std::vector<size_t>& group = groups_[static_cast<size_t>(sound)];
        if (group.empty()) return kInvalidSample;
        if (group.size() == 1) return group.front() == excludedSampleIndex ? kInvalidSample : group.front();
        for (size_t attempt = 0; attempt < 6; ++attempt) {
            size_t index = group[static_cast<size_t>(rng_() % group.size())];
            if (index != excludedSampleIndex) return index;
        }
        for (size_t index : group) {
            if (index != excludedSampleIndex) return index;
        }
        return kInvalidSample;
    }

    void PlaySample(size_t sampleIndex, AudioBus bus, XMFLOAT3 pos, float volume = 1.0f, bool spatial = true,
                    float frequencyRatio = 1.0f, GameSound sound = GameSound::MonsterGrowl,
                    float initialOcclusion = 0.0f, AudioToneProfile toneProfile = AudioToneProfile::Normal) {
        if (sampleIndex == kInvalidSample) return;
        StartVoice(sound, sampleIndex, bus, pos, volume, false, spatial, frequencyRatio, -1, initialOcclusion, toneProfile);
    }

private:
    static constexpr size_t kInvalidSample = static_cast<size_t>(-1);
    static constexpr uint32_t FourCC(char a, char b, char c, char d) {
        return static_cast<uint32_t>(static_cast<uint8_t>(a)) |
            (static_cast<uint32_t>(static_cast<uint8_t>(b)) << 8) |
            (static_cast<uint32_t>(static_cast<uint8_t>(c)) << 16) |
            (static_cast<uint32_t>(static_cast<uint8_t>(d)) << 24);
    }

    std::filesystem::path ResolveSoundFolder(const Settings& settings, const std::wstring& folder) const {
        std::vector<std::filesystem::path> roots;
        auto add = [&](std::filesystem::path p) {
            p = p.lexically_normal();
            if (std::find(roots.begin(), roots.end(), p) == roots.end()) roots.push_back(std::move(p));
        };
        add(ModuleDirectory() / folder);
        add(ModuleDirectory().parent_path() / folder);
        add(ModuleDirectory().parent_path().parent_path() / folder);
        add(std::filesystem::current_path() / folder);
        if (!settings.assetFolder.empty()) {
            std::filesystem::path configured(settings.assetFolder);
            std::filesystem::path soundRelative = folder;
            if (soundRelative.is_relative()) {
                add((configured.is_absolute() ? configured : ModuleDirectory() / configured) / soundRelative);
            }
        }
        for (const auto& root : roots) {
            std::error_code ec;
            if (std::filesystem::exists(root, ec)) return root;
        }
        return {};
    }

    void AddFolder(const Settings& settings, GameSound sound, const std::wstring& folder, const std::wstring& pattern = L"*.wav") {
        std::filesystem::path root = ResolveSoundFolder(settings, folder);
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

    bool LoadWav(const std::filesystem::path& path, AudioSample& out) const {
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

    size_t PickSample(GameSound sound) {
        if (!initialized_) return kInvalidSample;
        const std::vector<size_t>& group = groups_[static_cast<size_t>(sound)];
        if (group.empty()) return kInvalidSample;
        size_t index = static_cast<size_t>(rng_() % group.size());
        return group[index];
    }

    float BusVolume(AudioBus bus) const {
        if (muted_) return 0.0f;
        float busVolume = effectsVolume_;
        if (bus == AudioBus::Ambience) busVolume = ambienceVolume_;
        if (bus == AudioBus::Monster) busVolume = monsterVolume_;
        return masterVolume_ * busVolume;
    }

    void StartVoice(GameSound sound, size_t sampleIndex, AudioBus bus, XMFLOAT3 pos, float volume, bool loop, bool spatial,
                    float frequencyRatio = 1.0f, int tag = -1, float initialOcclusion = 0.0f,
                    AudioToneProfile toneProfile = AudioToneProfile::Normal) {
        if (!initialized_ || sampleIndex >= samples_.size()) return;
        AudioSample& sample = samples_[sampleIndex];
        WAVEFORMATEX format = sample.format;
        std::vector<uint8_t> formatStorage;
        WAVEFORMATEX* formatPtr = &format;
        if (!sample.formatExtra.empty()) {
            formatStorage.resize(sizeof(WAVEFORMATEX) + sample.formatExtra.size());
            std::memcpy(formatStorage.data(), &sample.format, sizeof(WAVEFORMATEX));
            std::memcpy(formatStorage.data() + sizeof(WAVEFORMATEX), sample.formatExtra.data(), sample.formatExtra.size());
            formatPtr = reinterpret_cast<WAVEFORMATEX*>(formatStorage.data());
        }

        IXAudio2SourceVoice* voice = nullptr;
        HRESULT hr = xaudio_->CreateSourceVoice(&voice, formatPtr, XAUDIO2_VOICE_USEFILTER, XAUDIO2_DEFAULT_FREQ_RATIO, nullptr, nullptr, nullptr);
        if (FAILED(hr) || !voice) return;

        XAUDIO2_BUFFER buffer{};
        buffer.AudioBytes = static_cast<UINT32>(sample.data.size());
        buffer.pAudioData = sample.data.data();
        buffer.Flags = loop ? 0 : XAUDIO2_END_OF_STREAM;
        buffer.PlayBegin = 0;
        buffer.PlayLength = 0;
        buffer.LoopBegin = 0;
        buffer.LoopLength = 0;
        buffer.LoopCount = loop ? XAUDIO2_LOOP_INFINITE : 0;
        hr = voice->SubmitSourceBuffer(&buffer);
        if (FAILED(hr)) {
            voice->DestroyVoice();
            return;
        }

        AudioVoiceInstance instance{};
        instance.voice = voice;
        instance.sampleIndex = sampleIndex;
        instance.bus = bus;
        instance.sound = sound;
        instance.pos = pos;
        instance.baseVolume = std::clamp(volume, 0.0f, 6.0f);
        instance.frequencyRatio = std::clamp(frequencyRatio, 0.50f, 2.0f);
        instance.occlusion = spatial ? std::clamp(initialOcclusion, 0.0f, 8.0f) : 0.0f;
        instance.occlusionRefresh = spatial ? (bus == AudioBus::Ambience ? 0.32f : 0.14f) : 0.0f;
        instance.toneProfile = toneProfile;
        instance.tag = tag;
        instance.loop = loop;
        instance.spatial = spatial;
        if (instance.spatial && SpatialDistanceGain(instance) <= 0.0001f) {
            voice->DestroyVoice();
            return;
        }
        Apply3D(instance);
        voice->Start(0);
        voices_.push_back(instance);
    }

    void Apply3D(AudioVoiceInstance& instance) {
        if (!instance.voice || instance.sampleIndex >= samples_.size()) return;
        AudioSample& sample = samples_[instance.sampleIndex];
        float occlusion = std::clamp(instance.occlusion, 0.0f, 8.0f);
        float occlusionGain = OcclusionGain(instance.bus, occlusion);
        float distanceGain = SpatialDistanceGain(instance);
        float toneGain = instance.toneProfile == AudioToneProfile::MetallicVent ? 0.42f : 1.0f;
        float volume = instance.baseVolume * BusVolume(instance.bus) * occlusionGain * distanceGain * toneGain;
        instance.voice->SetVolume(volume);
        if (instance.spatial && distanceGain <= 0.0001f) return;
        XAUDIO2_FILTER_PARAMETERS filter{};
        float cutoffHz = 18000.0f;
        if (instance.toneProfile == AudioToneProfile::MetallicVent) {
            filter.Type = BandPassFilter;
            cutoffHz = Lerp(1450.0f, 780.0f, SmoothStep(0.0f, 4.0f, occlusion));
            filter.OneOverQ = 0.22f;
        } else {
            filter.Type = LowPassFilter;
            float occludedCutoff = instance.bus == AudioBus::Monster ? 220.0f :
                (instance.bus == AudioBus::Ambience ? 420.0f : 520.0f);
            cutoffHz = Lerp(18000.0f, occludedCutoff, SmoothStep(0.0f, 4.0f, occlusion));
            filter.OneOverQ = 1.0f;
        }
        float nyquistSafe = std::max(10.0f, static_cast<float>(sample.format.nSamplesPerSec) * 0.45f);
        cutoffHz = std::clamp(cutoffHz, 10.0f, nyquistSafe);
        filter.Frequency = std::clamp(2.0f * std::sin(3.14159265358979323846f * cutoffHz / static_cast<float>(sample.format.nSamplesPerSec)),
            0.001f, 1.0f);
        instance.voice->SetFilterParameters(&filter);
        if (!instance.spatial || sample.format.nChannels != 1 || outputChannels_ == 0) {
            instance.voice->SetFrequencyRatio(std::clamp(instance.frequencyRatio, XAUDIO2_MIN_FREQ_RATIO, XAUDIO2_MAX_FREQ_RATIO));
            return;
        }

        X3DAUDIO_EMITTER emitter{};
        X3DAUDIO_CONE cone{};
        X3DAUDIO_DISTANCE_CURVE_POINT flatVolumePoints[2] = {{0.0f, 1.0f}, {1.0f, 1.0f}};
        X3DAUDIO_DISTANCE_CURVE flatVolumeCurve{flatVolumePoints, 2};
        emitter.pCone = &cone;
        emitter.pVolumeCurve = &flatVolumeCurve;
        emitter.ChannelCount = 1;
        emitter.CurveDistanceScaler = 1.0f;
        emitter.Position = {instance.pos.x, instance.pos.y, instance.pos.z};
        X3DAUDIO_DSP_SETTINGS dsp{};
        std::array<float, XAUDIO2_MAX_AUDIO_CHANNELS> matrix{};
        dsp.SrcChannelCount = 1;
        dsp.DstChannelCount = outputChannels_;
        dsp.pMatrixCoefficients = matrix.data();
        X3DAudioCalculate(x3dHandle_, &listener_, &emitter,
            X3DAUDIO_CALCULATE_MATRIX | X3DAUDIO_CALCULATE_DOPPLER, &dsp);
        instance.voice->SetOutputMatrix(masterVoice_, 1, outputChannels_, matrix.data());
        instance.voice->SetFrequencyRatio(std::clamp(dsp.DopplerFactor * instance.frequencyRatio, XAUDIO2_MIN_FREQ_RATIO, XAUDIO2_MAX_FREQ_RATIO));
    }

    float SpatialDistanceGain(const AudioVoiceInstance& instance) const {
        if (!instance.spatial) return 1.0f;
        float dx = instance.pos.x - listener_.Position.x;
        float dy = instance.pos.y - listener_.Position.y;
        float dz = instance.pos.z - listener_.Position.z;
        float dist = std::sqrt(dx * dx + dy * dy + dz * dz);

        float inner = 0.65f;
        float maxDist = 18.0f;
        float power = 1.18f;
        if (instance.bus == AudioBus::Effects && instance.sampleIndex < samples_.size()) {
            const AudioSample& sample = samples_[instance.sampleIndex];
            if (!sample.data.empty() && sample.data.size() < 65536 && sample.format.nSamplesPerSec >= 22050) {
                inner = 1.15f;
                maxDist = 24.0f;
                power = 1.08f;
            }
            if (instance.sound == GameSound::LightBulbBreak) {
                inner = 1.35f;
                maxDist = 30.0f;
                power = 1.02f;
            }
        }
        if (instance.bus == AudioBus::Ambience) {
            inner = 0.85f;
            maxDist = 16.0f;
            power = 1.25f;
        } else if (instance.bus == AudioBus::Monster) {
            inner = 1.15f;
            maxDist = 22.0f;
            power = 1.35f;
        }

        if (dist <= inner) return 1.0f;
        float t = Clamp01((dist - inner) / std::max(0.1f, maxDist - inner));
        float shaped = std::pow(1.0f - t, power);
        float tail = 1.0f - SmoothStep(0.88f, 1.0f, t);
        return shaped * tail;
    }

    float OcclusionGain(AudioBus bus, float occlusion) const {
        occlusion = std::clamp(occlusion, 0.0f, 8.0f);
        float wallGain = std::pow(0.25f, occlusion);
        float floor = bus == AudioBus::Monster ? 0.0010f : (bus == AudioBus::Ambience ? 0.006f : 0.008f);
        return std::max(floor, wallGain);
    }

    bool initialized_ = false;
    bool comInitialized_ = false;
    HRESULT comHr_ = E_FAIL;
    Microsoft::WRL::ComPtr<IXAudio2> xaudio_;
    IXAudio2MasteringVoice* masterVoice_ = nullptr;
    UINT32 outputChannels_ = 2;
    X3DAUDIO_HANDLE x3dHandle_{};
    X3DAUDIO_LISTENER listener_{};
    std::vector<AudioSample> samples_;
    std::vector<std::vector<size_t>> groups_;
    std::vector<AudioVoiceInstance> voices_;
    std::mt19937 rng_{0xA7710u};
    bool muted_ = false;
    float masterVolume_ = 1.0f;
    float effectsVolume_ = 1.0f;
    float ambienceVolume_ = 1.0f;
    float monsterVolume_ = 1.0f;
};
