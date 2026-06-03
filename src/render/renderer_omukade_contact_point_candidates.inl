            std::array<XMFLOAT3, 6> candidates = {{
                {c.x - halfW, surfaceY, std::clamp(anchorBase.z + jitterA, c.z - halfD, c.z + halfD)},
                {c.x + halfW, surfaceY, std::clamp(anchorBase.z - jitterA, c.z - halfD, c.z + halfD)},
                {std::clamp(anchorBase.x + jitterB, c.x - halfW, c.x + halfW), surfaceY, c.z - halfD},
                {std::clamp(anchorBase.x - jitterB, c.x - halfW, c.x + halfW), surfaceY, c.z + halfD},
                {std::clamp(anchorBase.x + jitterB * 0.5f, c.x - halfW, c.x + halfW), 0.022f, std::clamp(anchorBase.z + jitterA * 0.5f, c.z - halfD, c.z + halfD)},
                {std::clamp(anchorBase.x - jitterB * 0.5f, c.x - halfW, c.x + halfW), settingsRuntime_.live.wallHeightMeters - 0.035f, std::clamp(anchorBase.z - jitterA * 0.5f, c.z - halfD, c.z + halfD)}
            }};
            std::array<XMFLOAT3, 6> normals = {{
                {1.0f, 0.0f, 0.0f},
                {-1.0f, 0.0f, 0.0f},
                {0.0f, 0.0f, 1.0f},
                {0.0f, 0.0f, -1.0f},
                {0.0f, 1.0f, 0.0f},
                {0.0f, -1.0f, 0.0f}
            }};
            std::array<bool, 6> usable = {{
                !maze.IsOpen(t.x - 1, t.y),
                !maze.IsOpen(t.x + 1, t.y),
                !maze.IsOpen(t.x, t.y - 1),
                !maze.IsOpen(t.x, t.y + 1),
                true,
                allowCeilingLimbContacts
            }};
