            auto centerForSurface = [&](XMFLOAT3 normal) {
                normal = Normalize3(normal, up);
                XMFLOAT3 c = p;
                float surfaceGap = radius * 0.98f + 0.026f;
                if (normal.y > 0.72f) {
                    c.y = radius * 1.08f + 0.050f;
                } else if (normal.y < -0.72f) {
                    c.y = settingsRuntime_.live.wallHeightMeters - radius * 0.92f - 0.026f;
                } else {
                    c.y = std::clamp(c.y, radius * 0.92f + 0.026f,
                        settingsRuntime_.live.wallHeightMeters - radius * 0.92f - 0.026f);
                    if (std::abs(normal.x) > std::abs(normal.z)) {
                        c.x = tileCenter.x + (normal.x < 0.0f ? halfW : -halfW) + normal.x * surfaceGap;
                        c.z = std::clamp(c.z, tileCenter.z - halfD, tileCenter.z + halfD);
                    } else {
                        c.z = tileCenter.z + (normal.z < 0.0f ? halfD : -halfD) + normal.z * surfaceGap;
                        c.x = std::clamp(c.x, tileCenter.x - halfW, tileCenter.x + halfW);
                    }
                }
                return c;
            };
