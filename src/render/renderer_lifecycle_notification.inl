    void ShowGameNotification(const std::wstring& text, float durationSeconds = 4.2f) {
        hudNotification_.text = text;
        hudNotification_.startTime = timeRuntime_.time;
        hudNotification_.duration = std::max(0.25f, durationSeconds);
        hudNotification_.fadeInSeconds = 0.22f;
        hudNotification_.centered = false;
        hudNotification_.persistent = false;
        hudNotification_.textureDirty = true;
    }

    void ShowScoreNotification(const std::wstring& text, float durationSeconds = 3600.0f) {
        hudNotification_.text = text;
        hudNotification_.startTime = timeRuntime_.time;
        hudNotification_.duration = std::max(0.25f, durationSeconds);
        hudNotification_.fadeInSeconds = 1.25f;
        hudNotification_.centered = true;
        hudNotification_.persistent = true;
        hudNotification_.textureDirty = true;
    }
