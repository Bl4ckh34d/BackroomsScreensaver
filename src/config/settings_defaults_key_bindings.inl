    for (const GameInputBindingDef& binding : kGameInputBindings) {
        s << binding.iniKey << L"=" << binding.defaultVk << L"\r\n";
    }
