        float chaseLookBackWeight = threat ? ChaseLookBackWeight() : 0.0f;
        bool branchLookActive = !panicActive && cameraRuntime_.branchLookTimer > 0.0f && cameraRuntime_.branchLookDuration > 0.001f;
        float branchLookWeight = branchLookActive ? BranchLookWeight() : 0.0f;
        bool roomSurveyActive = !panicActive && cameraRuntime_.roomSurveyTimer > 0.0f && cameraRuntime_.roomSurveyDuration > 0.001f;
        float roomSurveyWeight = roomSurveyActive ? RoomSurveyWeight() : 0.0f;
        float stumbleAmount = 0.0f;
        if (viewRuntime_.stumbleTimer > 0.0f && viewRuntime_.stumbleDuration > 0.001f) {
            float t = 1.0f - viewRuntime_.stumbleTimer / viewRuntime_.stumbleDuration;
            stumbleAmount = std::sin(Clamp01(t) * kPi);
        }
