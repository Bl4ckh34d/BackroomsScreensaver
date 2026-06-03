std::filesystem::path FindAssetFile(const wchar_t* filename) {
    for (const auto& root : CandidateAssetRoots()) {
        std::filesystem::path p = root / filename;
        std::error_code ec;
        if (std::filesystem::exists(p, ec)) return p;
    }
    return {};
}
