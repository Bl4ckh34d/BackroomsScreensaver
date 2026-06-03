                if (brokenZone && !effectRuntime_.lampDamagePixels.empty()) {
                    effectRuntime_.lampDamagePixels[static_cast<size_t>(tileY * maze.w + tileX)] = 255;
                    effectRuntime_.lampDamageDirty = true;
                }
                if (forceLampOff && !effectRuntime_.lampDamagePixels.empty()) {
                    effectRuntime_.lampDamagePixels[static_cast<size_t>(tileY * maze.w + tileX)] = 255;
                    effectRuntime_.lampDamageDirty = true;
                }
            }
        }
