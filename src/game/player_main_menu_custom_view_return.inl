            } else {
                menuRuntime_.customReturnTimer += std::max(0.0f, dt);
                float t = menuRuntime_.customReturnTimer;
                float turn = SmoothStep(0.02f, 0.95f, t);
                float walk = SmoothStep(0.20f, 1.46f, t);
                float bob = (std::sin(t * 9.8f) * 0.020f + std::sin(t * 19.6f) * 0.007f) *
                    SmoothStep(0.20f, 0.48f, t) * (1.0f - SmoothStep(1.22f, 1.62f, t));
                gameWorld_.player.position.x = Lerp(menuRuntime_.customReturnCamera.x, menuRuntime_.customStartCamera.x, walk);
                gameWorld_.player.position.y = Lerp(menuRuntime_.customReturnCamera.y, menuRuntime_.customStartCamera.y, walk) + bob;
                gameWorld_.player.position.z = Lerp(menuRuntime_.customReturnCamera.z, menuRuntime_.customStartCamera.z, walk);
                gameWorld_.player.yaw = menuRuntime_.customReturnYaw + AngleWrap(menuRuntime_.customStartYaw - menuRuntime_.customReturnYaw) * turn;
                gameWorld_.player.pitch = Lerp(menuRuntime_.customReturnPitch, menuRuntime_.customStartPitch, SmoothStep(0.16f, 1.0f, turn));
                if (t >= 1.68f) {
                    menuRuntime_.customViewActive = false;
                    gameWorld_.player.position = menuRuntime_.customStartCamera;
                    gameWorld_.player.yaw = menuRuntime_.customStartYaw;
                    gameWorld_.player.pitch = menuRuntime_.customStartPitch;
                }
            }
