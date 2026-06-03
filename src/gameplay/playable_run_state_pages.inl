    void GenerateLayerPageDistribution(std::mt19937& rng) {
        levelPageTargets.fill(1);
        std::array<int, kLevelsPerLayer> order{{0, 1, 2, 3, 4}};
        std::shuffle(order.begin(), order.end(), rng);
        for (int i = 0; i < 3; ++i) {
            levelPageTargets[static_cast<size_t>(order[static_cast<size_t>(i)])] = 2;
        }
        layerPageCollected.fill(0);
        layerPagesCollected = 0;
    }

    void EnsureLayerPageDistribution() {
        int total = 0;
        for (int count : levelPageTargets) total += count;
        if (total == kCollectiblePageMaterialCount) return;
        levelPageTargets = {{2, 2, 2, 1, 1}};
    }

    void DisableLayerPages() {
        layerPagesCollected = 0;
        layerPageCollected.fill(0);
        levelPageTargets.fill(0);
    }

    int LayerSecretTotal() const {
        return (customGame && !customSpec.eightPages) ? 0 : kCollectiblePageMaterialCount;
    }

    bool IsLayerPageCollected(int pageIndex) const {
        return pageIndex >= 0 && pageIndex < kCollectiblePageMaterialCount &&
            layerPageCollected[static_cast<size_t>(pageIndex)] != 0;
    }

    bool MarkLayerPageCollected(int pageIndex) {
        if (LayerSecretTotal() <= 0 || pageIndex < 0 || pageIndex >= kCollectiblePageMaterialCount) return false;

        uint8_t& collected = layerPageCollected[static_cast<size_t>(pageIndex)];
        if (collected != 0) return false;

        collected = 1;
        layerPagesCollected = std::clamp(layerPagesCollected + 1, 0, LayerSecretTotal());
        return true;
    }

    void RestoreLayerPageCollected(int pageIndex, bool collected) {
        if (pageIndex < 0 || pageIndex >= kCollectiblePageMaterialCount) return;
        if (LayerSecretTotal() <= 0) {
            layerPageCollected[static_cast<size_t>(pageIndex)] = 0;
            layerPagesCollected = 0;
            return;
        }
        layerPageCollected[static_cast<size_t>(pageIndex)] = collected ? 1 : 0;
    }

    void SetLayerPagesCollectedCount(int collectedCount) {
        layerPagesCollected = std::clamp(collectedCount, 0, LayerSecretTotal());
    }

    void ReconcileLayerPagesCollected() {
        int total = LayerSecretTotal();
        if (total <= 0) {
            layerPageCollected.fill(0);
            layerPagesCollected = 0;
            return;
        }

        int counted = 0;
        for (uint8_t collectedPage : layerPageCollected) {
            if (collectedPage) ++counted;
        }
        layerPagesCollected = std::clamp(std::max(layerPagesCollected, counted), 0, total);
    }

    int LayerPageStartForLevel(int targetLevelInLayer) const {
        int clamped = std::clamp(targetLevelInLayer, 1, kLevelsPerLayer);
        int start = 0;
        for (int i = 1; i < clamped; ++i) {
            start += levelPageTargets[static_cast<size_t>(i - 1)];
        }
        return std::clamp(start, 0, kCollectiblePageMaterialCount);
    }

    int LayerPageCountForLevel(int targetLevelInLayer) const {
        int index = std::clamp(targetLevelInLayer, 1, kLevelsPerLayer) - 1;
        int start = LayerPageStartForLevel(targetLevelInLayer);
        return std::clamp(levelPageTargets[static_cast<size_t>(index)], 0, kCollectiblePageMaterialCount - start);
    }

    int LayerPagesCollectedForLevel(int targetLevelInLayer) const {
        int start = LayerPageStartForLevel(targetLevelInLayer);
        int count = LayerPageCountForLevel(targetLevelInLayer);
        int found = 0;
        for (int i = 0; i < count; ++i) {
            int pageIndex = start + i;
            if (IsLayerPageCollected(pageIndex)) {
                ++found;
            }
        }
        return found;
    }
