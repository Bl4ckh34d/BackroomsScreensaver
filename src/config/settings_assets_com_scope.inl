// Image loading and configured asset lookup helpers.

ScopedCom::ScopedCom() {
    hr_ = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
}

ScopedCom::~ScopedCom() {
    if (SUCCEEDED(hr_)) CoUninitialize();
}

bool ScopedCom::Ok() const {
    return SUCCEEDED(hr_) || hr_ == RPC_E_CHANGED_MODE;
}
