    void SetPresentSyncInterval(UINT syncInterval) {
        presentRuntime_.syncInterval = syncInterval;
    }

    void SetPresentFlags(UINT flags) {
        presentRuntime_.flags = flags;
    }

    void SetPresentEnabled(bool enabled) {
        presentRuntime_.enabled = enabled;
    }

    bool LastPresentCompleted() const {
        return presentRuntime_.lastCompleted;
    }
