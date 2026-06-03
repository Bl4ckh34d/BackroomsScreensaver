    bool TryCollectiblePagePickup() {
        if (!sessionRuntime_.IsPlayableSimulation()) return false;
        GameWorldCollectibleAimResult target = gameWorld_.FindCollectiblePageInView(2.05f);
        if (!target.found) return false;
        GameWorldCollectiblePickupResult pickup = gameWorld_.CollectPage(
            target.pageSlot,
            sessionRuntime_.IsPlayableGame());
        if (!pickup.collected) return false;
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        gameWorld_.QueueAudioEvent(GameAudioEvent::OneShot(
            GameSound::PaperFlutter,
            AudioBus::Effects,
            world.playerPosition,
            0.72f,
            false).WithCategory(GameAudioEventCategory::Collectible));
        std::wostringstream notice;
        notice << L"Page collected  " << pickup.displayCollected << L"/" << kCollectiblePageMaterialCount;
        ShowGameNotification(notice.str(), 4.2f);
        return true;
    }

    bool TrySavePointInteract() {
        if (!sessionRuntime_.IsPlayableGame() || !gameWorld_.CanInteractWithSavePoint()) return false;
        if (SaveCurrentRunToFile()) {
            ShowGameNotification(L"Run saved", 3.8f);
        } else {
            ShowGameNotification(L"Save failed", 3.8f);
        }
        return true;
    }
