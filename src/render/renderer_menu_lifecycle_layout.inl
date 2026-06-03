    void SetMenuHoverButtonIndex(int index) {
        menuRuntime_.hoverButtonIndex = index;
        menuRuntime_.buttonHover = index >= 0;
        menuRuntime_.singlePlayerHover = index >= 0 &&
            index < menuRuntime_.buttonCount &&
            menuRuntime_.buttonLabelRows[static_cast<size_t>(index)] == 0;
    }

    void SetMenuResumeLabel(bool resume) {
        menuRuntime_.resumeLabel = resume;
    }

    void SetMenuButtonLayout(bool canResumeCurrent, bool canResumeSaved) {
        menuRuntime_.buttonCount = 0;
        auto add = [&](int labelRow) {
            if (menuRuntime_.buttonCount >= static_cast<int>(menuRuntime_.buttonLabelRows.size())) return;
            menuRuntime_.buttonLabelRows[static_cast<size_t>(menuRuntime_.buttonCount++)] = labelRow;
        };
        if (canResumeCurrent) add(1);
        if (canResumeSaved) add(2);
        add(0);
        add(3);
        add(4);
        add(5);
        menuRuntime_.resumeLabel = canResumeCurrent;
    }
