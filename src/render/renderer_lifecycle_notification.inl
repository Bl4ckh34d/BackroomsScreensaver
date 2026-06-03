    void ShowGameNotification(const std::wstring& text, float durationSeconds = 4.2f) {
        hudNotification_.text = text;
        hudNotification_.startTime = timeRuntime_.time;
        hudNotification_.duration = std::max(0.25f, durationSeconds);
        hudNotification_.textureDirty = true;
    }
