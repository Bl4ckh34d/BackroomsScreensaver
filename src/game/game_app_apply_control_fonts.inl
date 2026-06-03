    HWND controls[] = {
        app.gameTitle, app.gameSinglePlayer, app.gameSettings, app.gameDebug, app.gameExit, app.gameBack,
        app.customTitle, app.customLayerLabel, app.customLayer, app.customScaresLabel,
        app.customScareBrokenLamp, app.customScareAirVent, app.customScareWater, app.customScareBlood,
        app.customScareFlesh, app.customBossesLabel, app.customBossOmukade, app.customSizeLabel,
        app.customSizeXLabel, app.customSizeX, app.customSizeYLabel, app.customSizeY,
        app.customPages, app.customStart, app.customBack,
        app.debugPrevEffect, app.debugNextEffect, app.debugSize, app.debugReset, app.debugPrevProp, app.debugNextProp,
        app.debugSettings
    };
    for (HWND control : controls) ApplyDefaultGuiFont(control);
