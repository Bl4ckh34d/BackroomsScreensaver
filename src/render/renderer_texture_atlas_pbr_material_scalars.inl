            if (loadPbrImage(base, {
                L"_ao_4k.jpg", L"_ao_4k.png",
                L"_AO.jpg", L"_AO.png",
                L"_AmbientOcclusion.jpg", L"_AmbientOcclusion.png"
            }, img)) {
                applyScalarProp(material, img, 0);
            }
            if (loadPbrImage(base, {
                L"_roughness_4k.jpg", L"_roughness_4k.png",
                L"_Roughness.jpg", L"_Roughness.png"
            }, img)) {
                applyScalarProp(material, img, 1);
            }
            if (loadPbrImage(base, {
                L"_metallic_4k.jpg", L"_metallic_4k.png",
                L"_metalness_4k.jpg", L"_metalness_4k.png",
                L"_Metallic.jpg", L"_Metallic.png",
                L"_Metalness.jpg", L"_Metalness.png"
            }, img)) {
                applyScalarProp(material, img, 2);
            }
        };
