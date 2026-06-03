        float remaining = std::min(distance, len);
        int steps = std::max(1, static_cast<int>(std::ceil(remaining / (gameWorld_.maze.TileMinimum() * 0.085f))));
        XMFLOAT3 pos = start;
        bool moved = false;
        for (int i = 0; i < steps; ++i) {
            float step = remaining / static_cast<float>(steps);
            XMFLOAT3 next{pos.x + dir.x * step, 0.0f, pos.z + dir.z * step};
            if (!MonsterFootprintOpen(next)) {
                XMFLOAT3 slideX{pos.x + dir.x * step, 0.0f, pos.z};
                XMFLOAT3 slideZ{pos.x, 0.0f, pos.z + dir.z * step};
                XMFLOAT3 directNext{pos.x + directDir.x * step, 0.0f, pos.z + directDir.z * step};
                bool preferX = std::abs(delta.x) > std::abs(delta.z);
                bool movedSlide = false;
                if (preferX && MonsterFootprintOpen(slideX)) {
                    pos = slideX;
                    movedSlide = true;
                } else if (!preferX && MonsterFootprintOpen(slideZ)) {
                    pos = slideZ;
                    movedSlide = true;
                } else if (MonsterFootprintOpen(directNext)) {
                    pos = directNext;
                    movedSlide = true;
                } else if (MonsterFootprintOpen(preferX ? slideZ : slideX)) {
                    pos = preferX ? slideZ : slideX;
                    movedSlide = true;
                }
                if (!movedSlide) {
                    if (!MonsterFootprintOpen(pos)) {
                        XMFLOAT3 center = gameWorld_.maze.WorldCenter(MonsterTile(), 0.0f);
                        if (MonsterFootprintOpen(center)) pos = Lerp3(pos, center, 0.35f);
                    }
                    break;
                }
                moved = true;
                continue;
            }
            pos = next;
            moved = true;
        }
