    RendererRuntimeMode RuntimeMode() const {
        return sessionRuntime_.mode;
    }

    void SetGameInput(const GameInputSnapshot& input) {
        sessionRuntime_.input = input;
    }
