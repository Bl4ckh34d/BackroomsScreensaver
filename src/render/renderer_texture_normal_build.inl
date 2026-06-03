        ReportStartupSubStep(L"Loading textures", L"Building normal and material property maps.", 3);
        for (int m = 0; m < kMaterialCount; ++m) {
            for (int y = 0; y < kTextureSize; ++y) {
                for (int x = 0; x < kTextureSize; ++x) {
                    auto hAt = [&](int sx, int sy) {
                        sx = (sx + kTextureSize) % kTextureSize;
                        sy = (sy + kTextureSize) % kTextureSize;
                        return heights[static_cast<size_t>((m * kTextureSize + sy) * width + sx)];
                    };
                    float hl = hAt(x - 1, y);
                    float hr = hAt(x + 1, y);
                    float hu = hAt(x, y - 1);
                    float hd = hAt(x, y + 1);
                    XMVECTOR n = XMVector3Normalize(XMVectorSet((hl - hr) * 3.1f, (hu - hd) * 3.1f, 1.0f, 0.0f));
                    XMFLOAT3 nf;
                    XMStoreFloat3(&nf, n);
                    int gy = m * kTextureSize + y;
                    size_t i = static_cast<size_t>((gy * width + x) * 4);
                    if (hasExternalNormal[static_cast<size_t>(gy * width + x)]) {
                        normal[i + 0] = externalNormal[i + 0];
                        normal[i + 1] = externalNormal[i + 1];
                        normal[i + 2] = externalNormal[i + 2];
                    } else {
                        normal[i + 0] = Byte(nf.x * 0.5f + 0.5f);
                        normal[i + 1] = Byte(nf.y * 0.5f + 0.5f);
                        normal[i + 2] = Byte(nf.z * 0.5f + 0.5f);
                    }
                    normal[i + 3] = Byte(hAt(x, y));
                }
            }
        }
        profile.Mark(L"BuildNormalHeightArray");
        SaveTextureCache(textureHash, albedo, normal, props);
        profile.Mark(L"SaveTextureCache");
