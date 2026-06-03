    HWND defaultChecked[] = {
        app.customScareBrokenLamp,
        app.customScareAirVent,
        app.customScareWater,
        app.customScareBlood,
        app.customScareFlesh,
        app.customBossOmukade,
        app.customPages
    };
    for (HWND control : defaultChecked) SendMessageW(control, BM_SETCHECK, BST_CHECKED, 0);
