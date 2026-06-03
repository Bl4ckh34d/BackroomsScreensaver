    void ProcessPlayerAudibleSoundsForMonster(std::vector<PlayerAudibleSoundPulse>& soundPulses, bool seesPlayer, bool ignorePlayer) {
        if (ignorePlayer) return;
        for (PlayerAudibleSoundPulse& pulse : soundPulses) {
            if (pulse.processedByMonster || pulse.radius <= 0.01f) continue;
            pulse.processedByMonster = true;
            float dx = gameWorld_.monster.position.x - pulse.pos.x;
            float dz = gameWorld_.monster.position.z - pulse.pos.z;
            if (dx * dx + dz * dz > pulse.radius * pulse.radius) continue;
            if (!MonsterGoalFarEnough(gameWorld_.maze.TileFromWorld(pulse.pos.x, pulse.pos.z))) continue;
            if (!MonsterPassesSoundObstructionChance(pulse)) continue;
            pulse.heardByMonster = true;
            gameWorld_.monster.heardPlayerNow = true;
            if (!seesPlayer) {
                AlertMonsterToSound(pulse.pos);
            }
        }
    }
