        bool seesPlayer = MonsterCanSeePlayer();
        if (input.ignorePlayer) {
            seesPlayer = false;
            gameWorld_.monster.hasSound = false;
            gameWorld_.monster.hasLastKnown = false;
            gameWorld_.monster.chasingVisible = false;
            gameWorld_.monster.ClearChaseCommitment();
        }
        if (input.playerSoundPulses) {
            ProcessPlayerAudibleSoundsForMonster(*input.playerSoundPulses, seesPlayer, input.ignorePlayer);
        }
        bool heardPlayer = gameWorld_.monster.heardPlayerNow;
        bool moved = false;
        UpdateMonsterTargetSelection(dt, mt, ct, seesPlayer);
