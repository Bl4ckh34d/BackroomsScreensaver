    float RoomSurveyWeight() const {
        if (cameraRuntime_.roomSurveyTimer <= 0.0f || cameraRuntime_.roomSurveyDuration <= 0.001f) return 0.0f;
        float t = 1.0f - cameraRuntime_.roomSurveyTimer / cameraRuntime_.roomSurveyDuration;
        return SmoothStep(0.0f, 0.18f, t) * (1.0f - SmoothStep(0.90f, 1.0f, t));
    }

    int RoomSurveyIndex(float& localT) const {
        localT = 0.0f;
        if (cameraRuntime_.roomSurveyYawCount <= 0 || cameraRuntime_.roomSurveyTimer <= 0.0f || cameraRuntime_.roomSurveyDuration <= 0.001f) return -1;
        float t = Clamp01(1.0f - cameraRuntime_.roomSurveyTimer / cameraRuntime_.roomSurveyDuration);
        float segment = t * static_cast<float>(cameraRuntime_.roomSurveyYawCount);
        int index = std::min(cameraRuntime_.roomSurveyYawCount - 1, static_cast<int>(segment));
        localT = segment - static_cast<float>(index);
        return index;
    }

    float RoomSurveyYaw() const {
        if (cameraRuntime_.roomSurveyTimer <= 0.0f || cameraRuntime_.roomSurveyDuration <= 0.001f) return gameWorld_.player.yaw;
        float localT = 0.0f;
        int index = RoomSurveyIndex(localT);
        if (index < 0) {
            float t = 1.0f - cameraRuntime_.roomSurveyTimer / cameraRuntime_.roomSurveyDuration;
            float sweep = -std::cos(Clamp01(t) * kPi * 2.0f);
            return cameraRuntime_.roomSurveyCenter + cameraRuntime_.roomSurveyDirection * sweep * cameraRuntime_.roomSurveySpan;
        }

        float target = cameraRuntime_.roomSurveyYaws[static_cast<size_t>(index)];
        float from = index > 0 ? cameraRuntime_.roomSurveyYaws[static_cast<size_t>(index - 1)] : cameraRuntime_.roomSurveyCenter;
        float enter = SmoothStep(0.0f, 0.24f, localT);
        float yaw = from + AngleWrap(target - from) * enter;
        if (index + 1 < cameraRuntime_.roomSurveyYawCount) {
            float leave = SmoothStep(0.76f, 1.0f, localT);
            yaw += AngleWrap(cameraRuntime_.roomSurveyYaws[static_cast<size_t>(index + 1)] - yaw) * leave * 0.54f;
        }
        float inspect = (1.0f - SmoothStep(0.62f, 1.0f, localT)) * SmoothStep(0.18f, 0.52f, localT);
        yaw += std::sin(timeRuntime_.time * 4.1f + static_cast<float>(index) * 1.9f) * 0.026f * inspect;
        return yaw;
    }

    float RoomSurveyPitch() const {
        if (cameraRuntime_.roomSurveyTimer <= 0.0f || cameraRuntime_.roomSurveyDuration <= 0.001f || cameraRuntime_.roomSurveyPitchCount <= 0) return -0.045f;
        float localT = 0.0f;
        int index = RoomSurveyIndex(localT);
        if (index < 0) return -0.045f;
        index = std::min(index, cameraRuntime_.roomSurveyPitchCount - 1);
        float target = cameraRuntime_.roomSurveyPitches[static_cast<size_t>(index)];
        float from = index > 0 ? cameraRuntime_.roomSurveyPitches[static_cast<size_t>(index - 1)] : -0.045f;
        float enter = SmoothStep(0.0f, 0.28f, localT);
        return from + (target - from) * enter;
    }
