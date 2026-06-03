            if (count <= 1) {
                EmitFloorWaterPoolCard(build, first.owner, first.cx, first.cz, first.side, first.seed,
                    first.width, first.depth, first.yaw, 5.0f, first.score);
                continue;
            }
            float finalCx = (minX + maxX) * 0.5f;
            float finalCz = (minZ + maxZ) * 0.5f;
            float finalW = std::max(0.05f, maxX - minX);
            float finalD = std::max(0.05f, maxZ - minZ);
            if (!FootprintFitsMaze(finalCx, finalCz, finalW, finalD, 0.0f, 0.020f, build.tileMin)) {
                float marginX = build.surface.tileW * 0.012f;
                float marginZ = build.surface.tileD * 0.012f;
                float l = build.surface.ox + static_cast<float>(first.owner.x) * build.surface.tileW + marginX;
                float r = l + build.surface.tileW - marginX * 2.0f;
                float z0 = build.surface.oz + static_cast<float>(first.owner.y) * build.surface.tileD + marginZ;
                float z1 = z0 + build.surface.tileD - marginZ * 2.0f;
                minX = std::clamp(minX, l, r);
                maxX = std::clamp(maxX, l, r);
                minZ = std::clamp(minZ, z0, z1);
                maxZ = std::clamp(maxZ, z0, z1);
                finalCx = (minX + maxX) * 0.5f;
                finalCz = (minZ + maxZ) * 0.5f;
                finalW = std::max(0.05f, maxX - minX);
                finalD = std::max(0.05f, maxZ - minZ);
            }
            EmitFloorWaterPoolCard(build, first.owner, finalCx, finalCz, bestSide,
                seedSum / static_cast<float>(std::max(1, count)), finalW, finalD,
                0.0f, 5.0f, std::max(1.18f, bestScore));
        }
