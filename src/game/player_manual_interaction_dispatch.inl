    void DispatchManualInteraction(bool interactPressed) {
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        if (interactPressed && !world.exitTransitionActive && TryCollectiblePagePickup()) {
            interactPressed = false;
        }
        if (interactPressed && !world.exitTransitionActive && TrySavePointInteract()) {
            interactPressed = false;
        }
        if (interactPressed && !world.exitTransitionActive && CanTriggerExitTransition()) {
            BeginExitTransition();
        }
    }
