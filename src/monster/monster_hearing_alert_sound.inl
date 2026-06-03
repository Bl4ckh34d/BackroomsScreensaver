// Monster hearing and sound-alert helper functions. 
// Included inside Renderer's private section from monster_ai.inl.

    void AlertMonsterToSound(const XMFLOAT3& pos) {
        if (MonsterIgnoresPlayer()) return;
        Tile sound = gameWorld_.maze.TileFromWorld(pos.x, pos.z);
        if (!ValidMonsterTile(sound)) return;
        if (!MonsterGoalFarEnough(sound)) return;
        gameWorld_.monster.soundTile = sound;
        gameWorld_.monster.hasSound = true;
        gameWorld_.monster.hasLastKnown = false;
        gameWorld_.monster.chasingVisible = false;
        gameWorld_.monster.searchTimer = 0.0f;
        gameWorld_.monster.roamTimer = 0.0f;
        SetMonsterGoal(sound, true);
    }
