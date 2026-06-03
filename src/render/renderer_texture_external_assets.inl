        {
            ReportStartupSubStep(L"Loading textures", L"Loading external PBR textures.", 2);
            ScopedCom com;
            if (com.Ok()) {
                loadPbrMaterial(0, settingsRuntime_.live.wallStem.c_str());
                loadPbrMaterial(1, settingsRuntime_.live.floorStem.c_str());
                loadPbrMaterial(2, settingsRuntime_.live.ceilingStem.c_str());
                loadPbrMaterial(15, settingsRuntime_.live.fleshStem.c_str());
                loadRuntimeAlbedo(7, L"assets\\models\\runtime\\textures\\emergency_exit_sign_diffuse.jpeg");
                loadRuntimeAlbedo(16, L"assets\\models\\runtime\\textures\\office_chair_modern_diffuse.jpg");
                loadRuntimeAlbedo(19, L"assets\\models\\runtime\\textures\\office_chair_classic_2209.jpg");
                loadRuntimeAlbedo(20, L"assets\\models\\runtime\\textures\\office_chair_classic_textiles.png");
                loadRuntimeAlbedo(22, L"assets\\models\\runtime\\textures\\office_chair_task_diffuse.png");
                loadRuntimeAlbedo(26, L"assets\\models\\monster_face_mask\\horror_mask_baseColor.png");
                loadRuntimeNormal(26, L"assets\\models\\monster_face_mask\\horror_mask_normal.png");
                loadRuntimeRoughnessFromGreen(26, L"assets\\models\\monster_face_mask\\horror_mask_metallicRoughness.png");
                for (int page = 0; page < kCollectiblePageMaterialCount; ++page) {
                    std::wstring path = L"assets\\images\\8pages\\page" + std::to_wstring(page + 1) + L".jpg";
                    loadRuntimeAlbedo(kCollectiblePageMaterialFirst + page, path.c_str());
                }
                loadRandomLoosePageAtlas();
                darkenWhiteChairPlastic(16);
                darkenWhiteChairPlastic(20);
                toneTaskChairFabric(22);
            }
        }
        profile.Mark(L"LoadExternalPBRs");
