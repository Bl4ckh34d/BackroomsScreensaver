    s.monsterScale = std::clamp(IniFloat(L"Monster", L"MonsterScale", s.monsterScale), 0.25f, 4.0f);
    s.monsterSpeed = std::clamp(IniFloat(L"Monster", L"MonsterSpeed", s.monsterSpeed), 0.1f, 4.0f);
    s.monsterSprintSpeed = std::clamp(IniFloat(L"Monster", L"MonsterSprintSpeed", s.monsterSprintSpeed), 0.1f, 4.0f);
    s.monsterIgnorePlayer = IniInt(L"Monster", L"MonsterIgnorePlayer", s.monsterIgnorePlayer ? 1 : 0) != 0;
    s.monsterKillDistance = std::clamp(IniFloat(L"Monster", L"MonsterKillDistance", s.monsterKillDistance), 0.2f, 4.0f);
    s.monsterVisibleDistance = std::clamp(IniFloat(L"Monster", L"MonsterVisibleDistance", s.monsterVisibleDistance), 1.0f, 60.0f);
    s.monsterSkullMesh = IniString(L"Monster", L"SkullMesh", s.monsterSkullMesh.c_str());
    s.monsterAltSkullMesh = IniString(L"Monster", L"AlternateSkullMesh", s.monsterAltSkullMesh.c_str());
    s.monsterAltSkullChance = std::clamp(IniFloat(L"Monster", L"AlternateSkullChance", s.monsterAltSkullChance), 0.0f, 1.0f);
    s.monsterSkullMaxTriangles = std::clamp(IniInt(L"Monster", L"SkullMaxTriangles", s.monsterSkullMaxTriangles), 0, 90000);
    auto normalizeLegacyMonsterMesh = [](std::wstring& meshPath) {
        std::wstring lowered = meshPath;
        std::transform(lowered.begin(), lowered.end(), lowered.begin(), [](wchar_t c) {
            return static_cast<wchar_t>(towlower(c));
        });
        if (lowered.find(L"white-tailed deer skull") != std::wstring::npos ||
            lowered.find(L"ram_skull") != std::wstring::npos) {
            meshPath = L"assets\\models\\monster_face_mask\\horror_mask.obj";
        }
    };
    normalizeLegacyMonsterMesh(s.monsterSkullMesh);
    normalizeLegacyMonsterMesh(s.monsterAltSkullMesh);
    if (s.monsterSkullMesh.empty()) {
        s.monsterSkullMesh = L"assets\\models\\monster_face_mask\\horror_mask.obj";
    }
    {
        std::wstring loweredSkull = s.monsterSkullMesh;
        std::transform(loweredSkull.begin(), loweredSkull.end(), loweredSkull.begin(), [](wchar_t c) {
            return static_cast<wchar_t>(towlower(c));
        });
        if (loweredSkull.find(L"horror_mask") != std::wstring::npos ||
            loweredSkull.find(L"monster_face_mask") != std::wstring::npos) {
            s.monsterSkullMaxTriangles = std::max(s.monsterSkullMaxTriangles, 16000);
        }
    }
    if (s.monsterAltSkullMesh.empty()) {
        s.monsterAltSkullChance = 0.0f;
    }
    s.monsterSkullYawDegrees = std::clamp(IniFloat(L"Monster", L"SkullYawDegrees", s.monsterSkullYawDegrees), -180.0f, 180.0f);
    s.monsterSkullPitchDegrees = std::clamp(IniFloat(L"Monster", L"SkullPitchDegrees", s.monsterSkullPitchDegrees), -180.0f, 180.0f);
    s.monsterSkullRollDegrees = std::clamp(IniFloat(L"Monster", L"SkullRollDegrees", s.monsterSkullRollDegrees), -180.0f, 180.0f);
    s.monsterAltSkullYawDegrees = std::clamp(IniFloat(L"Monster", L"AlternateSkullYawDegrees", s.monsterAltSkullYawDegrees), -180.0f, 180.0f);
    s.monsterAltSkullPitchDegrees = std::clamp(IniFloat(L"Monster", L"AlternateSkullPitchDegrees", s.monsterAltSkullPitchDegrees), -180.0f, 180.0f);
    s.monsterAltSkullRollDegrees = std::clamp(IniFloat(L"Monster", L"AlternateSkullRollDegrees", s.monsterAltSkullRollDegrees), -180.0f, 180.0f);
