StartupProfile::StartupProfile(const wchar_t* name) : name_(name), enabled_(StartupProfileEnabled()) {
    if (!enabled_) return;
    start_ = ProfileNowMs();
    last_ = start_;
    StartupProfileLine(L"[" + name_ + L"]");
}

void StartupProfile::Mark(const wchar_t* label) {
    if (!enabled_) return;
    double now = ProfileNowMs();
    std::wostringstream line;
    line << std::fixed << std::setprecision(3);
    line << name_ << L" " << label
         << L": +" << (now - last_) << L" ms"
         << L", total " << (now - start_) << L" ms";
    StartupProfileLine(line.str());
    last_ = now;
}
