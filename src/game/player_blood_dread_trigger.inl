            if (!point.triggered) {
                point.triggered = true;
                if (point.waterLiquid) {
                    point.activationTime = timeRuntime_.time;
                    point.focusTaken = true;
                } else {
                    point.activationTime = timeRuntime_.time;
                }
                if (point.revealBlood) {
                    IncludeBloodReveal(point);
                }
                scareRuntime_.bloodScareActiveUntil = timeRuntime_.time + 150.0f;
                continue;
            }
