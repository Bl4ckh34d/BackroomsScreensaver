#include "../platform/platform_headers.h"

#include "settings.h"

int NormalizeAntiAliasingMode(int value) {
    switch (value) {
    case 0:
    case 1:
    case 2:
    case 4:
    case 8:
    case 16:
        return value;
    default:
        return value <= 0 ? 0 : 1;
    }
}

int NormalizeTextureAnisotropy(int value) {
    if (value <= 1) return 1;
    if (value <= 2) return 2;
    if (value <= 4) return 4;
    if (value <= 8) return 8;
    return 16;
}

int AntiAliasingMsaaSamples(int value) {
    value = NormalizeAntiAliasingMode(value);
    return value == 2 || value == 4 || value == 8 || value == 16 ? value : 1;
}

bool AntiAliasingUsesFxaa(int value) {
    return NormalizeAntiAliasingMode(value) == 1;
}
