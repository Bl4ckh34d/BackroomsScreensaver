    s.audioMuted = IniInt(L"Audio", L"Muted", s.audioMuted ? 1 : 0) != 0;
    s.audioMasterVolume = std::clamp(IniFloat(L"Audio", L"MasterVolume", s.audioMasterVolume), 0.0f, 1.0f);
    s.audioMusicVolume = std::clamp(IniFloat(L"Audio", L"MusicVolume", s.audioMusicVolume), 0.0f, 1.0f);
    s.audioEffectsVolume = std::clamp(IniFloat(L"Audio", L"EffectsVolume", s.audioEffectsVolume), 0.0f, 1.0f);
    s.audioAmbienceVolume = std::clamp(IniFloat(L"Audio", L"AmbienceVolume", s.audioAmbienceVolume), 0.0f, 1.0f);
    s.audioMonsterVolume = std::clamp(IniFloat(L"Audio", L"MonsterVolume", s.audioMonsterVolume), 0.0f, 1.0f);

