        bool drawNewPoster = false;
        bool drawCustomPoster = false;
        bool drawSettingsToolbox = false;
        bool hoverNewPoster = false;
        bool hoverCustomPoster = false;
        bool hoverSettingsToolbox = false;
        int newPosterIndex = -1;
        int customPosterIndex = -1;
        int settingsIndex = -1;

        for (int i = 0; i < menuRuntime_.buttonCount; ++i) {
            int labelIndex = std::clamp(menuRuntime_.buttonLabelRows[static_cast<size_t>(i)], 0, 5);
            bool hover = menuRuntime_.hoverButtonIndex == i;
            if (labelIndex == 3) {
                drawCustomPoster = true;
                hoverCustomPoster = hoverCustomPoster || hover;
                if (customPosterIndex < 0) customPosterIndex = i;
            } else if (labelIndex == 4) {
                drawSettingsToolbox = true;
                hoverSettingsToolbox = hoverSettingsToolbox || hover;
                if (settingsIndex < 0) settingsIndex = i;
            } else {
                drawNewPoster = true;
                hoverNewPoster = hoverNewPoster || hover;
                if (newPosterIndex < 0) newPosterIndex = i;
            }
        }

        auto appendCrookedPoster = [&](int index, bool customPoster, bool hover) {
            if (index < 0) return;
            MenuPlaquePlacement plaque = MenuButtonPlacement(index);
            float roll = customPoster ? -0.035f : 0.045f;
            float cr = std::cos(roll);
            float sr = std::sin(roll);
            XMFLOAT3 posterRight = Normalize3(Add3(Scale3(plaque.right, cr), Scale3(up, sr)), plaque.right);
            XMFLOAT3 posterUp = Normalize3(Add3(Scale3(up, cr), Scale3(plaque.right, -sr)), up);
            XMFLOAT3 faceCenter = Add3(plaque.center, Scale3(plaque.inward, 0.004f));
            XMFLOAT3 hw = Scale3(posterRight, plaque.halfW);
            XMFLOAT3 hh = Scale3(posterUp, plaque.halfH);
            float u0 = customPoster ? 0.50f : 0.0f;
            float u1 = customPoster ? 1.0f : 0.50f;
            AppendDynamicQuadUV(transparentVerts,
                Add3(faceCenter, Add3(Scale3(hw, -1.0f), Scale3(hh, -1.0f))),
                Add3(faceCenter, Add3(hw, Scale3(hh, -1.0f))),
                Add3(faceCenter, Add3(hw, hh)),
                Add3(faceCenter, Add3(Scale3(hw, -1.0f), hh)),
                plaque.inward, posterRight, {u0, 1.0f}, {u1, 1.0f}, {u1, 0.0f}, {u0, 0.0f}, 18.98f);
        };

        if (drawNewPoster) appendCrookedPoster(newPosterIndex, false, hoverNewPoster);
        if (drawCustomPoster) appendCrookedPoster(customPosterIndex, true, hoverCustomPoster);
