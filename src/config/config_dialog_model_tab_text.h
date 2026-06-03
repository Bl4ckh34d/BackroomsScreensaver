#pragma once

const wchar_t* kConfigTabs[kConfigMaxTabCount] = {
    L"Renderer",
    L"Maze",
    L"Textures",
    L"Lighting",
    L"Camera AI",
    L"Camera FX",
    L"Atmosphere",
    L"Monster"
};

const wchar_t* kConfigNotes[kConfigMaxTabCount] = {
    L"Renderer device policy. Keep WARP disabled to require hardware GPU rendering.",
    L"Maze dimensions are forced odd. Per-run variation jitters Camera AI, Flashlight Motion, and non-blood/flesh Atmosphere values by up to 15%; room +/- fields are explicit integer ranges.",
    L"PBR stems accept Backrooms names like <stem>_color_4k.jpg and Substance names like <stem>_Color.jpg. Empty floor/ceiling stems use built-in textures.",
    L"Flashlight shadows, lamp population, black haze, and procedural corner ambient occlusion.",
    L"Camera pathing, head movement, room scanning, and look-back behavior.",
    L"Independent flashlight motion, entrance fades, and the exit door transition.",
    L"Paper, props, water damage, in-ceiling lamp failures, and spark particles.",
    L"Monster scale, chase logic, dread response, and mesh orientation. On this tab, right-drag the preview to orbit and use the wheel to zoom."
};
