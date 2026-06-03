    void AlertMonsterToPlayerTrigger(const XMFLOAT3& fallbackPos) {
        if (MonsterIgnoresPlayer()) return;
        Tile player = CameraTile();
        if (ValidMonsterTile(player)) {
            XMFLOAT3 ping = gameWorld_.maze.WorldCenter(player, 0.0f);
            AlertMonsterToSound(ping);
        } else {
            AlertMonsterToSound(fallbackPos);
        }
    }
