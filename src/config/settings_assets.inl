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

std::filesystem::path FindAssetFile(const wchar_t* filename) {
    for (const auto& root : CandidateAssetRoots()) {
        std::filesystem::path p = root / filename;
        std::error_code ec;
        if (std::filesystem::exists(p, ec)) return p;
    }
    return {};
}

bool LoadImageWic(const std::filesystem::path& path, int targetW, int targetH, ImageRGBA& out) {
    out = {};
    ComPtr<IWICImagingFactory> factory;
    HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory));
    if (FAILED(hr)) return false;

    ComPtr<IWICBitmapDecoder> decoder;
    hr = factory->CreateDecoderFromFilename(path.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &decoder);
    if (FAILED(hr)) return false;

    ComPtr<IWICBitmapFrameDecode> frame;
    hr = decoder->GetFrame(0, &frame);
    if (FAILED(hr)) return false;

    UINT sourceW = 0;
    UINT sourceH = 0;
    hr = frame->GetSize(&sourceW, &sourceH);
    if (FAILED(hr) || sourceW == 0 || sourceH == 0) return false;
    if (targetW <= 0) targetW = static_cast<int>(sourceW);
    if (targetH <= 0) targetH = static_cast<int>(sourceH);
    if (targetW <= 0 || targetH <= 0) return false;

    ComPtr<IWICBitmapSource> source = frame;
    ComPtr<IWICBitmapScaler> scaler;
    if (targetW != static_cast<int>(sourceW) || targetH != static_cast<int>(sourceH)) {
        hr = factory->CreateBitmapScaler(&scaler);
        if (SUCCEEDED(hr)) {
            hr = scaler->Initialize(frame.Get(), targetW, targetH, WICBitmapInterpolationModeFant);
            if (SUCCEEDED(hr)) source = scaler;
        }
    }

    ComPtr<IWICFormatConverter> converter;
    hr = factory->CreateFormatConverter(&converter);
    if (FAILED(hr)) return false;
    hr = converter->Initialize(source.Get(), GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom);
    if (FAILED(hr)) return false;

    out.width = targetW;
    out.height = targetH;
    out.pixels.resize(static_cast<size_t>(targetW * targetH * 4));
    hr = converter->CopyPixels(nullptr, targetW * 4, static_cast<UINT>(out.pixels.size()), out.pixels.data());
    return SUCCEEDED(hr);
}


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

