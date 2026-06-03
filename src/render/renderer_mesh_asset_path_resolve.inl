    std::filesystem::path ResolveConfiguredAssetPath(const std::wstring& configuredPath) const {
        if (configuredPath.empty()) return {};
        std::filesystem::path p(configuredPath);
        std::vector<std::filesystem::path> candidates;
        auto add = [&](std::filesystem::path c) {
            c = c.lexically_normal();
            if (std::find(candidates.begin(), candidates.end(), c) == candidates.end()) candidates.push_back(std::move(c));
        };
        if (p.is_absolute()) {
            add(p);
        } else {
            std::filesystem::path module = ModuleDirectory();
            add(module / p);
            add(module.parent_path() / p);
            add(module.parent_path().parent_path() / p);
            add(std::filesystem::current_path() / p);
            std::filesystem::path resolved = ResolveAsset(settingsRuntime_.live, configuredPath);
            if (!resolved.empty()) add(resolved);
        }
        for (const auto& candidate : candidates) {
            std::error_code ec;
            if (std::filesystem::exists(candidate, ec)) return candidate;
        }
        return {};
    }
