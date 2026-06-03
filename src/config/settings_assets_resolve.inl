std::filesystem::path ResolveAsset(const Settings& settings, const std::wstring& filename) {
    std::vector<std::filesystem::path> roots;
    auto add = [&](std::filesystem::path p) {
        p = p.lexically_normal();
        if (std::find(roots.begin(), roots.end(), p) == roots.end()) roots.push_back(std::move(p));
    };
    std::filesystem::path folder(settings.assetFolder);
    if (!folder.empty()) {
        if (folder.is_absolute()) add(folder);
        else {
            add(ModuleDirectory() / folder);
            add(ModuleDirectory().parent_path() / folder);
            add(ModuleDirectory().parent_path().parent_path() / folder);
            add(std::filesystem::current_path() / folder);
        }
    }
    for (const auto& root : CandidateAssetRoots()) add(root);
    for (const auto& root : roots) {
        std::filesystem::path p = root / filename;
        std::error_code ec;
        if (std::filesystem::exists(p, ec)) return p;
    }
    return {};
}
