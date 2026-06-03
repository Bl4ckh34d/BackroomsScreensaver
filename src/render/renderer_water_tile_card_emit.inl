        if (ceiling) {
            AddQuadUV(build.vertices, build.waterIndices,
                {l, y, z0}, {r, y, z0}, {r, y, z1}, {l, y, z1},
                {0, -1, 0}, {1, 0, 0},
                {u, vMode}, {u + 1.0f, vMode}, {u + 1.0f, vMode + 1.0f}, {u, vMode + 1.0f}, material);
        } else {
            AddQuadUV(build.vertices, build.waterIndices,
                {l, y, z1}, {r, y, z1}, {r, y, z0}, {l, y, z0},
                {0, 1, 0}, {1, 0, 0},
                {u, vMode}, {u + 1.0f, vMode}, {u + 1.0f, vMode + 1.0f}, {u, vMode + 1.0f}, material);
            MarkWetFootstepArea((l + r) * 0.5f, (z0 + z1) * 0.5f, r - l, z1 - z0, 0.0f);
        }
