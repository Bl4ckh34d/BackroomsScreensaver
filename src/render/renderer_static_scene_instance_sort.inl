
        if (pendingStaticInstances.empty()) return;

        auto instanceChunkX = [](const PendingStaticInstance& instance) {
            return instance.minTileX / 4;
        };
        auto instanceChunkY = [](const PendingStaticInstance& instance) {
            return instance.minTileY / 4;
        };
        std::sort(pendingStaticInstances.begin(), pendingStaticInstances.end(),
            [&](const PendingStaticInstance& a, const PendingStaticInstance& b) {
                if (a.meshId != b.meshId) return a.meshId < b.meshId;
                if (a.castsShadow != b.castsShadow) return a.castsShadow < b.castsShadow;
                if (instanceChunkY(a) != instanceChunkY(b)) return instanceChunkY(a) < instanceChunkY(b);
                if (instanceChunkX(a) != instanceChunkX(b)) return instanceChunkX(a) < instanceChunkX(b);
                if (a.minTileY != b.minTileY) return a.minTileY < b.minTileY;
                return a.minTileX < b.minTileX;
            });
