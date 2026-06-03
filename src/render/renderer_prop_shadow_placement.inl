// Baked prop shadow placement helpers.
// Included inside Renderer private section before maze mesh construction.

    XMFLOAT2 NearestMazeLampXZ(float px, float pz, const MazeSurfaceBuildContext& ctx) const {
        float strideX = ctx.tileW;
        float strideZ = ctx.tileD;
        float originX = ctx.ox + ctx.tileW * 0.5f;
        float originZ = ctx.oz + ctx.tileD * 0.5f;
        float cellX = std::floor((px - originX) / std::max(0.001f, strideX) + 0.5f);
        float cellZ = std::floor((pz - originZ) / std::max(0.001f, strideZ) + 0.5f);
        return XMFLOAT2{originX + cellX * strideX, originZ + cellZ * strideZ};
    }

    void AddBakedPropShadow(std::vector<Vertex>& vertices,
                            std::vector<uint32_t>& transparentIndices,
                            const MazeSurfaceBuildContext& ctx,
                            float tileAvg,
                            float px,
                            float pz,
                            float width,
                            float depth,
                            float height,
                            float yaw,
                            float seed) {
        if (gEffectDebugViewer && gDebugSliceEffect != DebugSliceEffect::CeilingLamps) return;
        if (width <= 0.03f || depth <= 0.03f || height <= 0.04f) return;
        XMFLOAT2 lamp = NearestMazeLampXZ(px, pz, ctx);
        float dx = px - lamp.x;
        float dz = pz - lamp.y;
        float distXZ = std::sqrt(dx * dx + dz * dz);
        float distFade = std::clamp(distXZ / std::max(0.001f, tileAvg * 3.8f), 0.0f, 1.0f);
        float dirLen = std::max(0.001f, distXZ);
        float dirX = dx / dirLen;
        float dirZ = dz / dirLen;
        float verticalGap = std::max(0.35f, ctx.wallH - std::min(height, ctx.wallH * 0.86f));
        float offset = std::min(tileAvg * 0.38f, distXZ * height / verticalGap * 0.24f);
        float shadowX = px + dirX * offset;
        float shadowZ = pz + dirZ * offset;
        float broad = std::max(width, depth);
        float cross = broad * (0.76f + distFade * 0.36f) + height * (0.05f + distFade * 0.12f);
        float along = std::max(depth, broad * 0.72f) * (0.94f + distFade * 0.72f) + height * (0.14f + distFade * 0.52f);
        float shadowYaw = distXZ > 0.08f ? std::atan2(dirX, dirZ) : yaw;
        float softness = std::clamp(0.10f + distFade * 0.82f + seed * 0.035f, 0.06f, 0.95f);
        float lift = 0.012f + seed * 0.003f;
        AddFloorCard(vertices, transparentIndices, {shadowX, 0.0f, shadowZ},
            std::max(0.10f, cross), std::max(0.12f, along), shadowYaw, lift, 24.0f + softness);
    }
