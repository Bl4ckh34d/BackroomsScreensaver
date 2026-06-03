    s.effectBloodLoopSeconds = std::clamp(IniFloat(L"EffectTuning", L"BloodLoopSeconds", s.effectBloodLoopSeconds), 1.0f, 180.0f);
    s.effectBloodFullSpreadAge = std::clamp(IniFloat(L"EffectTuning", L"BloodFullSpreadAge", s.effectBloodFullSpreadAge), 0.1f, s.effectBloodLoopSeconds);
    s.effectWaterLoopSeconds = std::clamp(IniFloat(L"EffectTuning", L"WaterLoopSeconds", s.effectWaterLoopSeconds), 0.5f, 60.0f);
    s.effectAirVentLoopSeconds = std::clamp(IniFloat(L"EffectTuning", L"AirVentLoopSeconds", s.effectAirVentLoopSeconds), 0.5f, 60.0f);
    s.effectBrokenLampLoopSeconds = std::clamp(IniFloat(L"EffectTuning", L"BrokenLampLoopSeconds", s.effectBrokenLampLoopSeconds), 0.5f, 60.0f);
    s.effectStaticLoopSeconds = std::clamp(IniFloat(L"EffectTuning", L"StaticLoopSeconds", s.effectStaticLoopSeconds), 0.5f, 60.0f);
    s.effectBrokenLampSparkIntensityMin = std::clamp(IniFloat(L"EffectTuning", L"BrokenLampSparkIntensityMin", s.effectBrokenLampSparkIntensityMin), 0.1f, 12.0f);
    s.effectBrokenLampSparkIntensityMax = std::max(s.effectBrokenLampSparkIntensityMin,
        std::clamp(IniFloat(L"EffectTuning", L"BrokenLampSparkIntensityMax", s.effectBrokenLampSparkIntensityMax), 0.1f, 16.0f));
    s.effectBrokenLampChainIntensityScale = std::clamp(IniFloat(L"EffectTuning", L"BrokenLampChainIntensityScale", s.effectBrokenLampChainIntensityScale), 0.0f, 4.0f);
    s.effectBrokenLampChainBurstsMin = std::clamp(IniInt(L"EffectTuning", L"BrokenLampChainBurstsMin", s.effectBrokenLampChainBurstsMin), 0, 16);
    s.effectBrokenLampChainBurstsMax = std::max(s.effectBrokenLampChainBurstsMin,
        std::clamp(IniInt(L"EffectTuning", L"BrokenLampChainBurstsMax", s.effectBrokenLampChainBurstsMax), 0, 24));
    s.effectAirVentSteamIntensityMin = std::clamp(IniFloat(L"EffectTuning", L"AirVentSteamIntensityMin", s.effectAirVentSteamIntensityMin), 0.1f, 12.0f);
    s.effectAirVentSteamIntensityMax = std::max(s.effectAirVentSteamIntensityMin,
        std::clamp(IniFloat(L"EffectTuning", L"AirVentSteamIntensityMax", s.effectAirVentSteamIntensityMax), 0.1f, 16.0f));
    s.effectAirVentPanelDropEvery = std::clamp(IniInt(L"EffectTuning", L"AirVentPanelDropEvery", s.effectAirVentPanelDropEvery), 1, 32);
    s.effectAirVentPanelDropChance = std::clamp(IniFloat(L"EffectTuning", L"AirVentPanelDropChance", s.effectAirVentPanelDropChance), 0.0f, 1.0f);

