// Liquid placement scare canvas start.

    void AddLiquidScarePoint(XMFLOAT3 pos, float span, float wallH, bool waterLiquid) {
        if (span < 0.56f) return;
        BloodScarePoint scare{};
        scare.pos = pos;
        scare.source = pos.y < 0.35f
            ? XMFLOAT3{pos.x, 0.18f, pos.z}
            : XMFLOAT3{pos.x, std::clamp(pos.y, 0.18f, wallH - 0.055f), pos.z};
        scare.radius = std::clamp(1.25f + span * 0.72f, 1.75f, 4.35f);
        scare.focusDelaySeconds = 0.26f + LampHash(pos.x * 3.1f + span, pos.z * 5.7f) * 0.58f;
        scare.waterLiquid = waterLiquid;
        if (waterLiquid) {
            scare.dreadScale = 0.30f;
        }
        scareRuntime_.bloodScarePoints.push_back(scare);
    }

    std::vector<LiquidCanvasSurface>& LiquidCanvasVector(LiquidCanvasBuildContext& build, bool water, bool ceiling) const {
        if (water) return ceiling ? build.ceilingWaterCanvas : build.floorWaterCanvas;
        return ceiling ? build.ceilingBloodCanvas : build.floorBloodCanvas;
    }
