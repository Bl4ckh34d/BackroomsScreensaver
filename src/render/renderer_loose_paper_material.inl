    static float LoosePaperMaterial(float seed, float variantSeed) {
        if (seed < 0.50f) {
            int slot = std::clamp(static_cast<int>(variantSeed * static_cast<float>(kRandomLoosePageAtlasSlots)), 0, kRandomLoosePageAtlasSlots - 1);
            float encodedSlot = (static_cast<float>(slot) + 0.5f) / static_cast<float>(kRandomLoosePageAtlasSlots);
            return static_cast<float>(kRandomLoosePageMaterial) + encodedSlot;
        }
        return static_cast<float>(kRandomLoosePageMaterial);
    }
