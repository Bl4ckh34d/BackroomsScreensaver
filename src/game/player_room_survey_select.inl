        std::sort(candidates.begin(), candidates.end(), [](const SurveyCandidate& a, const SurveyCandidate& b) {
            return a.score > b.score;
        });
        int maxTargets = pauseFirst ? 5 : 3;
        int selected = std::min<int>(static_cast<int>(candidates.size()), maxTargets);
        if (selected > 0) {
            std::sort(candidates.begin(), candidates.begin() + selected, [this](const SurveyCandidate& a, const SurveyCandidate& b) {
                return AngleWrap(a.yaw - gameWorld_.player.bodyYaw) < AngleWrap(b.yaw - gameWorld_.player.bodyYaw);
            });
            if (RandRange(0.0f, 1.0f) < 0.5f) {
                std::reverse(candidates.begin(), candidates.begin() + selected);
            }
            for (int i = 0; i < selected; ++i) {
                cameraRuntime_.roomSurveyYaws[static_cast<size_t>(i)] = candidates[static_cast<size_t>(i)].yaw;
                cameraRuntime_.roomSurveyPitches[static_cast<size_t>(i)] = candidates[static_cast<size_t>(i)].pitch;
            }
            cameraRuntime_.roomSurveyYawCount = selected;
            cameraRuntime_.roomSurveyPitchCount = selected;
        }
