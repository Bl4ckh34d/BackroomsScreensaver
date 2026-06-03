#include "platform_headers.h"

#include "command_line_mode.h"

namespace {
std::wstring Lower(std::wstring s) {
    std::transform(s.begin(), s.end(), s.begin(), [](wchar_t c) { return static_cast<wchar_t>(towlower(c)); });
    return s;
}

uintptr_t ParseHandle(const wchar_t* s) {
    if (!s) return 0;
    while (*s == L':' || *s == L' ') ++s;
    return static_cast<uintptr_t>(_wcstoui64(s, nullptr, 0));
}
}

ParsedRunMode ParseCommandLineMode(int argc, wchar_t** argv) {
    ParsedRunMode parsed{};
    for (int i = 1; i < argc; ++i) {
        std::wstring arg = Lower(argv[i]);
        if (arg.rfind(L"/c", 0) == 0 || arg.rfind(L"-c", 0) == 0) {
            parsed.mode = RunMode::Configure;
            return parsed;
        }
        if (arg == L"/selftest" || arg == L"-selftest") {
            parsed.mode = RunMode::SelfTest;
            return parsed;
        }
        if (arg == L"/makeini" || arg == L"-makeini") {
            parsed.mode = RunMode::GenerateIni;
            return parsed;
        }
        if (arg == L"/monsterpreviewfront" || arg == L"-monsterpreviewfront") {
            parsed.mode = RunMode::MonsterPreviewFront;
            return parsed;
        }
        if (arg == L"/monsterpreviewside" || arg == L"-monsterpreviewside") {
            parsed.mode = RunMode::MonsterPreviewSide;
            return parsed;
        }
        if (arg == L"/monsterpreviewleft" || arg == L"-monsterpreviewleft") {
            parsed.mode = RunMode::MonsterPreviewLeftSide;
            return parsed;
        }
        if (arg == L"/monsterpreviewtop" || arg == L"-monsterpreviewtop") {
            parsed.mode = RunMode::MonsterPreviewTop;
            return parsed;
        }
        if (arg == L"/monsterpreview" || arg == L"-monsterpreview") {
            parsed.mode = RunMode::MonsterPreview;
            return parsed;
        }
        if (arg == L"/blooddebug" || arg == L"-blooddebug" ||
            arg == L"/effectdebug" || arg == L"-effectdebug") {
            parsed.mode = RunMode::BloodDebug;
            return parsed;
        }
        if (arg == L"/floorwaterdebug" || arg == L"-floorwaterdebug") {
            parsed.startupDebugSliceEffect = DebugSliceEffect::FloorWater;
            parsed.hideDebugMonster = true;
            parsed.mode = RunMode::BloodDebug;
            return parsed;
        }
        if (arg == L"/ceilingwaterdebug" || arg == L"-ceilingwaterdebug") {
            parsed.startupDebugSliceEffect = DebugSliceEffect::CeilingWater;
            parsed.hideDebugMonster = true;
            parsed.mode = RunMode::BloodDebug;
            return parsed;
        }
        if (arg == L"/wallwaterdebug" || arg == L"-wallwaterdebug") {
            parsed.startupDebugSliceEffect = DebugSliceEffect::WallWater;
            parsed.hideDebugMonster = true;
            parsed.mode = RunMode::BloodDebug;
            return parsed;
        }
        if (arg == L"/nodebugmonster" || arg == L"-nodebugmonster") {
            parsed.hideDebugMonster = true;
            continue;
        }
        if (arg.rfind(L"/p", 0) == 0 || arg.rfind(L"-p", 0) == 0) {
            uintptr_t handle = 0;
            if (arg.size() > 2) handle = ParseHandle(arg.c_str() + 2);
            if (!handle && i + 1 < argc) handle = ParseHandle(argv[++i]);
            parsed.previewParent = reinterpret_cast<HWND>(handle);
            parsed.mode = RunMode::Preview;
            return parsed;
        }
        if (arg.rfind(L"/s", 0) == 0 || arg.rfind(L"-s", 0) == 0) {
            parsed.mode = RunMode::Fullscreen;
            return parsed;
        }
    }
    return parsed;
}
