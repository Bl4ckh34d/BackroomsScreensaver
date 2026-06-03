std::vector<std::filesystem::path> CandidateAssetRoots() {
    std::vector<std::filesystem::path> roots;
    auto add = [&](std::filesystem::path p) {
        p = p.lexically_normal();
        if (std::find(roots.begin(), roots.end(), p) == roots.end()) roots.push_back(std::move(p));
    };

    std::filesystem::path module = ModuleDirectory();
    add(module / L"assets" / L"PBRs");
    add(module.parent_path() / L"assets" / L"PBRs");
    add(module.parent_path().parent_path() / L"assets" / L"PBRs");
    add(std::filesystem::current_path() / L"assets" / L"PBRs");
    add(std::filesystem::current_path());
    return roots;
}
