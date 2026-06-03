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
