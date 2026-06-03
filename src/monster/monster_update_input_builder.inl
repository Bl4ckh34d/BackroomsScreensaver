    MonsterUpdateInput BuildMonsterUpdateInput(float dt) {
        MonsterUpdateInput input{};
        input.dt = dt;
        input.time = timeRuntime_.time;
        input.settings = &settingsRuntime_.live;
        input.maze = &gameWorld_.maze;
        input.player = &gameWorld_.player;
        input.playerSoundPulses = &gameWorld_.playerSoundPulses;
        input.ignorePlayer = MonsterIgnoresPlayer();
        input.preview = monsterPreview_.active;
        return input;
    }
