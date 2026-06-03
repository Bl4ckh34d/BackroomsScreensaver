// Playable HUD and dread meter overlay helpers. 
// Included inside Renderer's private section from renderer_overlays.inl.

    void DrawDreadMeterOverlay() {
        if (!settingsRuntime_.live.dreadEnabled || !settingsRuntime_.live.dreadDebugMeter || !renderBuffers_.overlayBuffer ||
            !shaders_.overlayVertexShader || !shaders_.overlayPixelShader || hostRuntime_.width <= 0 || hostRuntime_.height <= 0) {
            return;
        }

        std::vector<OverlayVertex> verts;
        verts.reserve(48);
        auto ndcX = [&](float px) { return px / static_cast<float>(hostRuntime_.width) * 2.0f - 1.0f; };
        auto ndcY = [&](float py) { return 1.0f - py / static_cast<float>(hostRuntime_.height) * 2.0f; };
        auto pushRect = [&](float x, float y, float w, float h, XMFLOAT4 color) {
            float l = ndcX(x);
            float r = ndcX(x + w);
            float t = ndcY(y);
            float b = ndcY(y + h);
            verts.push_back({{l, b}, color});
            verts.push_back({{l, t}, color});
            verts.push_back({{r, t}, color});
            verts.push_back({{l, b}, color});
            verts.push_back({{r, t}, color});
            verts.push_back({{r, b}, color});
        };

        float x = 18.0f;
        float y = 18.0f;
        float w = 170.0f;
        float h = 13.0f;
        float fill = std::round((w - 4.0f) * Clamp01(viewRuntime_.dreadMeterLevel));
        pushRect(x - 2.0f, y - 2.0f, w + 4.0f, h + 4.0f, {0.07f, 0.025f, 0.025f, 0.74f});
        pushRect(x, y, w, h, {0.015f, 0.008f, 0.008f, 0.86f});
        if (fill > 0.5f) {
            pushRect(x + 2.0f, y + 2.0f, fill, h - 4.0f, {0.72f, 0.055f, 0.042f, 0.93f});
            pushRect(x + 2.0f, y + 2.0f, fill, 2.0f, {1.0f, 0.20f, 0.13f, 0.38f});
        }
        pushRect(x - 2.0f, y - 2.0f, w + 4.0f, 1.0f, {0.40f, 0.08f, 0.07f, 0.86f});
        pushRect(x - 2.0f, y + h + 1.0f, w + 4.0f, 1.0f, {0.23f, 0.035f, 0.035f, 0.86f});
        pushRect(x - 2.0f, y - 2.0f, 1.0f, h + 4.0f, {0.33f, 0.055f, 0.052f, 0.86f});
        pushRect(x + w + 1.0f, y - 2.0f, 1.0f, h + 4.0f, {0.33f, 0.055f, 0.052f, 0.86f});

        DrawOverlayVertices(verts);
    }

    void DrawGameHudOverlay() {
        if (!IsPlayableSimulationMode(sessionRuntime_.mode) || !renderBuffers_.overlayBuffer ||
            !shaders_.overlayVertexShader || !shaders_.overlayPixelShader || hostRuntime_.width <= 0 || hostRuntime_.height <= 0) {
            return;
        }

        std::vector<OverlayVertex> verts;
        verts.reserve(96);
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        auto ndcX = [&](float px) { return px / static_cast<float>(hostRuntime_.width) * 2.0f - 1.0f; };
        auto ndcY = [&](float py) { return 1.0f - py / static_cast<float>(hostRuntime_.height) * 2.0f; };
        auto pushRect = [&](float x, float y, float w, float h, XMFLOAT4 color) {
            float l = ndcX(x);
            float r = ndcX(x + w);
            float t = ndcY(y);
            float b = ndcY(y + h);
            verts.push_back({{l, b}, color});
            verts.push_back({{l, t}, color});
            verts.push_back({{r, t}, color});
            verts.push_back({{l, b}, color});
            verts.push_back({{r, t}, color});
            verts.push_back({{r, b}, color});
        };
        auto pushBar = [&](float x, float y, float w, float h, float fill, XMFLOAT4 fillColor) {
            fill = Clamp01(fill);
            pushRect(x - 2.0f, y - 2.0f, w + 4.0f, h + 4.0f, {0.015f, 0.014f, 0.012f, 0.78f});
            pushRect(x, y, w, h, {0.03f, 0.028f, 0.024f, 0.84f});
            float fillW = std::round(w * fill);
            if (fillW > 0.5f) {
                pushRect(x, y, fillW, h, fillColor);
                pushRect(x, y, fillW, std::max(1.0f, h * 0.22f), {1.0f, 1.0f, 1.0f, 0.16f});
            }
        };

        if (sessionRuntime_.mode == RendererRuntimeMode::PlayableGame && gameWorld_.PlayableScoreScreenActive()) {
            pushRect(0.0f, 0.0f, static_cast<float>(hostRuntime_.width), static_cast<float>(hostRuntime_.height), {0.0f, 0.0f, 0.0f, 0.34f});
        }

        float x = 24.0f;
        float y = static_cast<float>(hostRuntime_.height) - 58.0f;
        float w = std::clamp(static_cast<float>(hostRuntime_.width) * 0.22f, 180.0f, 290.0f);
        pushBar(x, y, w, 14.0f, world.playerHealth / 100.0f, {0.72f, 0.05f, 0.045f, 0.92f});
        float staminaFill = world.playerStamina / 100.0f;
        float fatigue = 1.0f - SmoothStep(0.06f, 0.34f, staminaFill);
        XMFLOAT4 staminaColor{
            Lerp(0.88f, 0.92f, fatigue),
            Lerp(0.72f, 0.36f, fatigue),
            Lerp(0.23f, 0.08f, fatigue),
            0.88f
        };
        pushBar(x, y + 24.0f, w, 10.0f, staminaFill, staminaColor);
        float notificationAge = timeRuntime_.time - hudNotification_.startTime;
        float notificationAlpha = 0.0f;
        float notificationX = 0.0f;
        float notificationY = 0.0f;
        float notificationW = 0.0f;
        float notificationH = 0.0f;
        if (!hudNotification_.text.empty() && notificationAge >= 0.0f && notificationAge < hudNotification_.duration) {
            float inT = SmoothStep(0.0f, 0.22f, notificationAge);
            float outT = 1.0f - SmoothStep(std::max(0.0f, hudNotification_.duration - 0.65f), hudNotification_.duration, notificationAge);
            notificationAlpha = Clamp01(inT * outT);
            float uiScale = std::clamp(static_cast<float>(hostRuntime_.height) / 900.0f, 0.86f, 1.16f);
            notificationW = std::min(static_cast<float>(hudNotification_.textureWidth) * uiScale, static_cast<float>(hostRuntime_.width) - 84.0f);
            notificationH = static_cast<float>(hudNotification_.textureHeight) * uiScale;
            notificationX = (static_cast<float>(hostRuntime_.width) - notificationW) * 0.5f;
            notificationY = std::clamp(static_cast<float>(hostRuntime_.height) * 0.145f, 64.0f, 150.0f);
            pushRect(notificationX - 14.0f, notificationY - 7.0f, notificationW + 28.0f, notificationH + 14.0f,
                {0.010f, 0.012f, 0.011f, 0.58f * notificationAlpha});
            pushRect(notificationX - 14.0f, notificationY - 7.0f, notificationW + 28.0f, 1.0f,
                {0.64f, 0.58f, 0.38f, 0.34f * notificationAlpha});
            pushRect(notificationX - 14.0f, notificationY + notificationH + 6.0f, notificationW + 28.0f, 1.0f,
                {0.64f, 0.58f, 0.38f, 0.22f * notificationAlpha});
        }
        DrawOverlayVertices(verts);
        if (notificationAlpha > 0.002f) {
            DrawHudNotificationText(notificationX, notificationY, notificationW, notificationH, notificationAlpha);
        }
    }
