    void ResetCollectiblePagesForGeneration();
    bool PlaceCollectiblePage(int pageSlot, const CollectiblePage& page);

    void GenerateCollectiblePagesForCurrentLevel(bool enabled, float wallHeightMeters, std::mt19937& rng);
    GameWorldCollectibleAimResult FindCollectiblePageInView(float maxDistance) const;
    bool IsLayerPageCollected(int pageIndex) const;
    GameWorldCollectiblePickupResult CollectPage(size_t pageSlot, bool updatePlayableRun);
    void ClearSavePoint();
    void RestoreSavePoint(bool active, const XMFLOAT3& pos, float yaw);
    void RestoreCollectiblePageCollected(size_t pageSlot, bool collected, bool updatePlayableRun);
