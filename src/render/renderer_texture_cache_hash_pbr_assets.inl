        const wchar_t* pbrSuffixes[] = {
            L"_color_4k.jpg", L"_color_4k.png",
            L"_Color.jpg", L"_Color.png",
            L"_BaseColor.jpg", L"_BaseColor.png",
            L"_Albedo.jpg", L"_Albedo.png",
            L"_Diffuse.jpg", L"_Diffuse.png",
            L"_height_4k.png", L"_height_4k.jpg",
            L"_Displacement.jpg", L"_Displacement.png",
            L"_Height.jpg", L"_Height.png",
            L"_normal_directx_4k.png", L"_normal_directx_4k.jpg",
            L"_normal_dx_4k.png", L"_normal_dx_4k.jpg",
            L"_NormalDX.jpg", L"_NormalDX.png",
            L"_NormalDirectX.jpg", L"_NormalDirectX.png",
            L"_normal_opengl_4k.png", L"_normal_opengl_4k.jpg",
            L"_normal_gl_4k.png", L"_normal_gl_4k.jpg",
            L"_NormalGL.jpg", L"_NormalGL.png",
            L"_NormalOpenGL.jpg", L"_NormalOpenGL.png",
            L"_ao_4k.jpg", L"_ao_4k.png",
            L"_AO.jpg", L"_AO.png",
            L"_AmbientOcclusion.jpg", L"_AmbientOcclusion.png",
            L"_roughness_4k.jpg", L"_roughness_4k.png",
            L"_Roughness.jpg", L"_Roughness.png",
            L"_metallic_4k.jpg", L"_metallic_4k.png",
            L"_metalness_4k.jpg", L"_metalness_4k.png",
            L"_Metallic.jpg", L"_Metallic.png",
            L"_Metalness.jpg", L"_Metalness.png"
        };

        const std::wstring stems[] = {settingsRuntime_.live.wallStem, settingsRuntime_.live.floorStem, settingsRuntime_.live.ceilingStem, settingsRuntime_.live.fleshStem};
        for (const std::wstring& stem : stems) {
            for (const wchar_t* suffix : pbrSuffixes) {
                addPbrAsset(stem, suffix);
            }
        }
