        audioRuntime_.game.lampHumShouldPlay.assign(effectRuntime_.runtimeLamps.size(), 0);
        std::vector<uint8_t>& shouldPlay = audioRuntime_.game.lampHumShouldPlay;
        for (const LampHumCandidate& candidate : candidates) {
            shouldPlay[candidate.index] = 1;
        }
