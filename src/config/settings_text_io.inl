// Settings file IO, INI readers, and LoadSettings.

bool WriteTextFile(const std::filesystem::path& path, const std::wstring& text) {
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out) return false;
    std::string utf8;
    int needed = WideCharToMultiByte(CP_UTF8, 0, text.c_str(), static_cast<int>(text.size()), nullptr, 0, nullptr, nullptr);
    if (needed > 0) {
        utf8.resize(static_cast<size_t>(needed));
        WideCharToMultiByte(CP_UTF8, 0, text.c_str(), static_cast<int>(text.size()), utf8.data(), needed, nullptr, nullptr);
    }
    out.write(utf8.data(), static_cast<std::streamsize>(utf8.size()));
    return out.good();
}

std::wstring ReadTextFile(const std::filesystem::path& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) return {};
    std::string bytes((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    if (bytes.empty()) return {};
    int needed = MultiByteToWideChar(CP_UTF8, 0, bytes.data(), static_cast<int>(bytes.size()), nullptr, 0);
    if (needed <= 0) return {};
    std::wstring text(static_cast<size_t>(needed), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, bytes.data(), static_cast<int>(bytes.size()), text.data(), needed);
    return text;
}
