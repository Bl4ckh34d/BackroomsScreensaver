        auto loadPbrMaterial = [&](int material, const wchar_t* stem) {
            ImageRGBA img;
            std::wstring base(stem);
            if (base.empty()) return;
            if (loadPbrImage(base, {
                L"_color_4k.jpg", L"_color_4k.png",
                L"_Color.jpg", L"_Color.png",
                L"_BaseColor.jpg", L"_BaseColor.png",
                L"_Albedo.jpg", L"_Albedo.png",
                L"_Diffuse.jpg", L"_Diffuse.png"
            }, img)) {
                applyAlbedo(material, img);
            }
            if (loadPbrImage(base, {
                L"_height_4k.png", L"_height_4k.jpg",
                L"_Displacement.jpg", L"_Displacement.png",
                L"_Height.jpg", L"_Height.png"
            }, img)) {
                applyHeight(material, img);
            }
