        const wchar_t* runtimeTextures[] = {
            L"assets\\models\\runtime\\textures\\emergency_exit_sign_diffuse.jpeg",
            L"assets\\models\\runtime\\textures\\office_chair_modern_diffuse.jpg",
            L"assets\\models\\runtime\\textures\\office_chair_classic_2209.jpg",
            L"assets\\models\\runtime\\textures\\office_chair_classic_textiles.png",
            L"assets\\models\\runtime\\textures\\office_chair_task_diffuse.png",
            L"assets\\models\\monster_face_mask\\horror_mask_baseColor.png",
            L"assets\\models\\monster_face_mask\\horror_mask_normal.png",
            L"assets\\models\\monster_face_mask\\horror_mask_metallicRoughness.png",
            L"assets\\images\\8pages\\page1.jpg",
            L"assets\\images\\8pages\\page2.jpg",
            L"assets\\images\\8pages\\page3.jpg",
            L"assets\\images\\8pages\\page4.jpg",
            L"assets\\images\\8pages\\page5.jpg",
            L"assets\\images\\8pages\\page6.jpg",
            L"assets\\images\\8pages\\page7.jpg",
            L"assets\\images\\8pages\\page8.jpg"
        };
        for (const wchar_t* texture : runtimeTextures) {
            addResolvedAsset(texture, ResolveConfiguredAssetPath(texture));
        }
