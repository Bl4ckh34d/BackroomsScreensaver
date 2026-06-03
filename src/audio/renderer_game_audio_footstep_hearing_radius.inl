    float FootstepHearingRadius(float walkT, float runT, bool crouching, bool wetFootstep) const {
        float radius = 0.0f;
        if (crouching) {
            radius = TileHearingRadius(0.35f);
        } else {
            float runBlend = std::max(runT, gameWorld_.player.runEffort * 0.72f);
            if (runBlend > 0.01f) {
                radius = TileHearingRadius(Lerp(3.2f, 4.0f, runBlend));
            } else {
                radius = TileHearingRadius(Lerp(2.1f, 2.9f, walkT));
            }
        }
        return radius * (wetFootstep ? 1.25f : 1.0f);
    }
