        std::vector<std::filesystem::path> files = RandomLoosePageFiles();
        ScopedCom com;
        bool canLoadImages = com.Ok();
        std::vector<uint8_t> pixels;
        pixels.reserve(static_cast<size_t>(pageW) * pageH * 4);
        for (int slot = 0; slot < pageCount; ++slot) {
            makeFallback(pixels, slot);
            if (canLoadImages && slot < static_cast<int>(files.size())) {
                ImageRGBA img;
                if (LoadImageWic(files[static_cast<size_t>(slot)], 0, 0, img)) {
                    blitFit(img, pixels);
                }
            }
            UINT subresource = D3D11CalcSubresource(0, static_cast<UINT>(slot), mipLevels);
            d3dRuntime_.context->UpdateSubresource(tex.Get(), subresource, nullptr, pixels.data(), pageW * 4, 0);
        }
