        HFONT font = reinterpret_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
        ConfigState* state = new ConfigState();
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        auto* params = cs ? reinterpret_cast<ConfigCreateParams*>(cs->lpCreateParams) : nullptr;
        if (params) {
            state->mode = params->mode;
            state->embedded = params->embedded;
        }
        state->hwnd = hwnd;
        BuildConfigModel(state);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(state));
