#include "loading_overlay_internal.h"

const ImageRGBA& LoadingOverlayLogo() {
    static ImageRGBA logo;
    static bool loaded = false;
    if (!loaded || !logo.Valid()) {
        ScopedCom com;
        if (!com.Ok()) return logo;
        const std::array<std::filesystem::path, 3> candidates = {
            ModuleDirectory() / L"assets" / L"branding" / L"NeuralForge_Solutions.png",
            std::filesystem::current_path() / L"assets" / L"branding" / L"NeuralForge_Solutions.png",
            std::filesystem::current_path().parent_path() / L"assets" / L"branding" / L"NeuralForge_Solutions.png"
        };
        for (const auto& candidate : candidates) {
            std::error_code ec;
            if (std::filesystem::exists(candidate, ec) && LoadImageWic(candidate, 0, 0, logo)) break;
        }
        loaded = logo.Valid();
    }
    return logo;
}

const ImageRGBA& LoadingOverlayTitleLogo() {
    static ImageRGBA logo;
    static bool loaded = false;
    if (!loaded || !logo.Valid()) {
        ScopedCom com;
        if (!com.Ok()) return logo;
        const std::array<std::filesystem::path, 3> candidates = {
            ModuleDirectory() / L"assets" / L"images" / L"title.png",
            std::filesystem::current_path() / L"assets" / L"images" / L"title.png",
            std::filesystem::current_path().parent_path() / L"assets" / L"images" / L"title.png"
        };
        for (const auto& candidate : candidates) {
            std::error_code ec;
            if (std::filesystem::exists(candidate, ec) && LoadImageWic(candidate, 0, 0, logo)) break;
        }
        loaded = logo.Valid();
    }
    return logo;
}
