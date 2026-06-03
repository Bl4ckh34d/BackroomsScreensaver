// Player camera attention room branch.

    float BranchLookWeight() const {
        if (cameraRuntime_.branchLookTimer <= 0.0f || cameraRuntime_.branchLookDuration <= 0.001f) return 0.0f;
        float t = 1.0f - cameraRuntime_.branchLookTimer / cameraRuntime_.branchLookDuration;
        return SmoothStep(0.0f, 0.16f, t) * (1.0f - SmoothStep(0.86f, 1.0f, t));
    }

    float BranchLookTargetYaw() const {
        float weight = BranchLookWeight();
        float scan = (std::sin(timeRuntime_.time * 4.7f + cameraRuntime_.branchLookYaw * 1.3f) * 0.020f +
            std::sin(timeRuntime_.time * 8.9f + cameraRuntime_.branchLookYaw) * 0.009f) * weight;
        return cameraRuntime_.branchLookYaw + scan;
    }

