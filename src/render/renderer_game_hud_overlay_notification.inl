        float notificationAge = timeRuntime_.time - hudNotification_.startTime;
        float notificationAlpha = 0.0f;
        float notificationX = 0.0f;
        float notificationY = 0.0f;
        float notificationW = 0.0f;
        float notificationH = 0.0f;
        if (!hudNotification_.text.empty() && notificationAge >= 0.0f && notificationAge < hudNotification_.duration) {
            float inT = SmoothStep(0.0f, std::max(0.01f, hudNotification_.fadeInSeconds), notificationAge);
            float outT = hudNotification_.persistent
                ? 1.0f
                : 1.0f - SmoothStep(std::max(0.0f, hudNotification_.duration - 0.65f), hudNotification_.duration, notificationAge);
            notificationAlpha = Clamp01(inT * outT);
            float uiScale = std::clamp(static_cast<float>(hostRuntime_.height) / 900.0f, 0.86f, 1.16f);
            notificationW = std::min(static_cast<float>(hudNotification_.textureWidth) * uiScale, static_cast<float>(hostRuntime_.width) - 84.0f);
            notificationH = static_cast<float>(hudNotification_.textureHeight) * uiScale;
            notificationX = (static_cast<float>(hostRuntime_.width) - notificationW) * 0.5f;
            notificationY = hudNotification_.centered
                ? (static_cast<float>(hostRuntime_.height) - notificationH) * 0.5f
                : std::clamp(static_cast<float>(hostRuntime_.height) * 0.145f, 64.0f, 150.0f);
            if (!hudNotification_.centered) {
                pushRect(notificationX - 14.0f, notificationY - 7.0f, notificationW + 28.0f, notificationH + 14.0f,
                    {0.010f, 0.012f, 0.011f, 0.58f * notificationAlpha});
                pushRect(notificationX - 14.0f, notificationY - 7.0f, notificationW + 28.0f, 1.0f,
                    {0.64f, 0.58f, 0.38f, 0.34f * notificationAlpha});
                pushRect(notificationX - 14.0f, notificationY + notificationH + 6.0f, notificationW + 28.0f, 1.0f,
                    {0.64f, 0.58f, 0.38f, 0.22f * notificationAlpha});
            }
        }
