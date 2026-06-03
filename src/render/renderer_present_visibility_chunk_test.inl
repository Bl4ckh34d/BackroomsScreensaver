        auto chunkVisible = [](const auto& chunk, XMFLOAT3 origin, XMFLOAT3 direction, float maxDistance, float coneCos) {
            float dx = chunk.center.x - origin.x;
            float dy = chunk.center.y - origin.y;
            float dz = chunk.center.z - origin.z;
            float padded = std::max(0.1f, maxDistance + chunk.radius);
            float distSq = dx * dx + dy * dy + dz * dz;
            if (distSq > padded * padded) return false;
            float depth = dx * direction.x + dy * direction.y + dz * direction.z;
            if (depth < -chunk.radius) return false;
            if (coneCos > -0.99f && distSq > 0.0001f) {
                float dist = std::sqrt(distSq);
                float radiusSlack = std::min(0.55f, chunk.radius / std::max(0.1f, dist));
                if (depth / dist < coneCos - radiusSlack) return false;
            }
            return true;
        };
