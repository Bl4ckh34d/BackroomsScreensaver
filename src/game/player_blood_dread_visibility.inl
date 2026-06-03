            float dist = std::sqrt(dx * dx + dy * dy + dz * dz);
            float aim = dist > 0.001f ? (dx * flashlightDir.x + dy * flashlightDir.y + dz * flashlightDir.z) / dist : 1.0f;
            if (point.triggered && aim < -0.18f) continue;
