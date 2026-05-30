// Renderer geometry helpers and maze mesh construction.
// Included inside Renderer private section before runtime simulation helpers.

    void AddQuad(std::vector<Vertex>& v, std::vector<uint32_t>& i,
                 XMFLOAT3 a, XMFLOAT3 b, XMFLOAT3 c, XMFLOAT3 d,
                 XMFLOAT3 n, XMFLOAT3 t, float material, float uScale, float vScale) {
        uint32_t base = static_cast<uint32_t>(v.size());
        v.push_back({a, n, t, {0.0f, 0.0f}, material});
        v.push_back({b, n, t, {uScale, 0.0f}, material});
        v.push_back({c, n, t, {uScale, vScale}, material});
        v.push_back({d, n, t, {0.0f, vScale}, material});
        i.insert(i.end(), {base, base + 1, base + 2, base, base + 2, base + 3});
    }

    void AddQuadUV(std::vector<Vertex>& v, std::vector<uint32_t>& i,
                   XMFLOAT3 a, XMFLOAT3 b, XMFLOAT3 c, XMFLOAT3 d,
                   XMFLOAT3 n, XMFLOAT3 t, XMFLOAT2 auv, XMFLOAT2 buv, XMFLOAT2 cuv, XMFLOAT2 duv,
                   float material) {
        uint32_t base = static_cast<uint32_t>(v.size());
        v.push_back({a, n, t, auv, material});
        v.push_back({b, n, t, buv, material});
        v.push_back({c, n, t, cuv, material});
        v.push_back({d, n, t, duv, material});
        i.insert(i.end(), {base, base + 1, base + 2, base, base + 2, base + 3});
    }

    static XMFLOAT3 Add3(XMFLOAT3 a, XMFLOAT3 b) {
        return {a.x + b.x, a.y + b.y, a.z + b.z};
    }

    static XMFLOAT3 Scale3(XMFLOAT3 a, float s) {
        return {a.x * s, a.y * s, a.z * s};
    }

    static XMFLOAT3 Sub3(XMFLOAT3 a, XMFLOAT3 b) {
        return {a.x - b.x, a.y - b.y, a.z - b.z};
    }

    static float Dot3(XMFLOAT3 a, XMFLOAT3 b) {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    static XMFLOAT3 Cross3(XMFLOAT3 a, XMFLOAT3 b) {
        return {
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        };
    }

    static float Length3(XMFLOAT3 a) {
        return std::sqrt(std::max(0.0f, Dot3(a, a)));
    }

    static XMFLOAT3 Normalize3(XMFLOAT3 a, XMFLOAT3 fallback = {0.0f, 1.0f, 0.0f}) {
        float len = Length3(a);
        if (len <= 0.0001f) return fallback;
        return Scale3(a, 1.0f / len);
    }

    static XMFLOAT3 Lerp3(XMFLOAT3 a, XMFLOAT3 b, float t) {
        return {Lerp(a.x, b.x, t), Lerp(a.y, b.y, t), Lerp(a.z, b.z, t)};
    }

    static XMFLOAT3 RotateYVec(XMFLOAT3 a, float angle) {
        float c = std::cos(angle);
        float s = std::sin(angle);
        return {a.x * c + a.z * s, a.y, -a.x * s + a.z * c};
    }

    static XMFLOAT3 OrientedOffset(XMFLOAT3 right, XMFLOAT3 up, XMFLOAT3 forward, float x, float y, float z) {
        return Add3(Add3(Scale3(right, x), Scale3(up, y)), Scale3(forward, z));
    }

    void AddOrientedBox(std::vector<Vertex>& v, std::vector<uint32_t>& i,
                        XMFLOAT3 center, XMFLOAT3 half, float yaw, float material) {
        float c = std::cos(yaw);
        float s = std::sin(yaw);
        XMFLOAT3 right{c, 0.0f, -s};
        XMFLOAT3 up{0.0f, 1.0f, 0.0f};
        XMFLOAT3 forward{s, 0.0f, c};
        auto p = [&](float x, float y, float z) {
            return Add3(center, OrientedOffset(right, up, forward, x * half.x, y * half.y, z * half.z));
        };
        auto face = [&](XMFLOAT3 a, XMFLOAT3 b, XMFLOAT3 c0, XMFLOAT3 d, XMFLOAT3 n, XMFLOAT3 t) {
            AddQuadUV(v, i, a, b, c0, d, n, t, {0, 0}, {1, 0}, {1, 1}, {0, 1}, material);
        };
        face(p(-1, -1,  1), p( 1, -1,  1), p( 1,  1,  1), p(-1,  1,  1), forward, right);
        face(p( 1, -1, -1), p(-1, -1, -1), p(-1,  1, -1), p( 1,  1, -1), Scale3(forward, -1.0f), Scale3(right, -1.0f));
        face(p( 1, -1,  1), p( 1, -1, -1), p( 1,  1, -1), p( 1,  1,  1), right, Scale3(forward, -1.0f));
        face(p(-1, -1, -1), p(-1, -1,  1), p(-1,  1,  1), p(-1,  1, -1), Scale3(right, -1.0f), forward);
        face(p(-1,  1,  1), p( 1,  1,  1), p( 1,  1, -1), p(-1,  1, -1), up, right);
        face(p(-1, -1, -1), p( 1, -1, -1), p( 1, -1,  1), p(-1, -1,  1), Scale3(up, -1.0f), right);
    }

    void AddFloorCard(std::vector<Vertex>& v, std::vector<uint32_t>& i,
                      XMFLOAT3 center, float width, float depth, float yaw, float y, float material) {
        float c = std::cos(yaw);
        float s = std::sin(yaw);
        XMFLOAT3 right{c, 0.0f, -s};
        XMFLOAT3 forward{s, 0.0f, c};
        XMFLOAT3 a = Add3(center, Add3(Scale3(right, -width * 0.5f), Scale3(forward,  depth * 0.5f)));
        XMFLOAT3 b = Add3(center, Add3(Scale3(right,  width * 0.5f), Scale3(forward,  depth * 0.5f)));
        XMFLOAT3 c0 = Add3(center, Add3(Scale3(right,  width * 0.5f), Scale3(forward, -depth * 0.5f)));
        XMFLOAT3 d = Add3(center, Add3(Scale3(right, -width * 0.5f), Scale3(forward, -depth * 0.5f)));
        a.y = b.y = c0.y = d.y = y;
        AddQuadUV(v, i, a, b, c0, d, {0, 1, 0}, right, {0, 0}, {1, 0}, {1, 1}, {0, 1}, material);
    }

    void AddCeilingCard(std::vector<Vertex>& v, std::vector<uint32_t>& i,
                        XMFLOAT3 center, float width, float depth, float yaw, float y, float material) {
        float c = std::cos(yaw);
        float s = std::sin(yaw);
        XMFLOAT3 right{c, 0.0f, -s};
        XMFLOAT3 forward{s, 0.0f, c};
        XMFLOAT3 a = Add3(center, Add3(Scale3(right, -width * 0.5f), Scale3(forward, -depth * 0.5f)));
        XMFLOAT3 b = Add3(center, Add3(Scale3(right,  width * 0.5f), Scale3(forward, -depth * 0.5f)));
        XMFLOAT3 c0 = Add3(center, Add3(Scale3(right,  width * 0.5f), Scale3(forward,  depth * 0.5f)));
        XMFLOAT3 d = Add3(center, Add3(Scale3(right, -width * 0.5f), Scale3(forward,  depth * 0.5f)));
        a.y = b.y = c0.y = d.y = y;
        AddQuadUV(v, i, a, b, c0, d, {0, -1, 0}, right, {0, 0}, {1, 0}, {1, 1}, {0, 1}, material);
    }

    bool AppendStaticPropMesh(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices,
                              const StaticPropMesh& mesh, XMFLOAT3 origin, float yaw,
                              float scaleX, float scaleY, float scaleZ,
                              float pitch = 0.0f, float materialOverride = -1.0f,
                              std::vector<uint32_t>* shadowIndices = nullptr,
                              float materialVariant = 0.0f) const {
        if (mesh.vertices.empty()) return false;
        if (vertices.size() + mesh.vertices.size() > static_cast<size_t>(std::numeric_limits<uint32_t>::max())) return false;

        float c = std::cos(yaw);
        float s = std::sin(yaw);
        XMFLOAT3 right{c, 0.0f, -s};
        XMFLOAT3 worldUp{0.0f, 1.0f, 0.0f};
        XMFLOAT3 forward{s, 0.0f, c};
        float cp = std::cos(pitch);
        float sp = std::sin(pitch);
        XMFLOAT3 up = Normalize3(Add3(Scale3(worldUp, cp), Scale3(forward, sp)), worldUp);
        forward = Normalize3(Add3(Scale3(forward, cp), Scale3(worldUp, -sp)), forward);
        const float invX = 1.0f / std::max(0.001f, std::abs(scaleX));
        const float invY = 1.0f / std::max(0.001f, std::abs(scaleY));
        const float invZ = 1.0f / std::max(0.001f, std::abs(scaleZ));
        XMFLOAT3 meshSpan{
            std::max(0.001f, mesh.max.x - mesh.min.x),
            std::max(0.001f, mesh.max.y - mesh.min.y),
            std::max(0.001f, mesh.max.z - mesh.min.z)
        };
        auto generatedUv = [&](const Vertex& src) {
            float nx = (src.pos.x - mesh.min.x) / meshSpan.x;
            float ny = (src.pos.y - mesh.min.y) / meshSpan.y;
            float nz = (src.pos.z - mesh.min.z) / meshSpan.z;
            XMFLOAT3 nAbs{std::abs(src.normal.x), std::abs(src.normal.y), std::abs(src.normal.z)};
            XMFLOAT2 uv = nAbs.y >= nAbs.x && nAbs.y >= nAbs.z
                ? XMFLOAT2{nx, nz}
                : (nAbs.x >= nAbs.z ? XMFLOAT2{nz, 1.0f - ny} : XMFLOAT2{nx, 1.0f - ny});
            int materialId = std::clamp(static_cast<int>(std::floor(src.material)), 0, kMaterialCount - 1);
            if (materialId == 22) {
                uv = {0.20f + uv.x * 0.60f, 0.28f + uv.y * 0.36f};
            }
            return uv;
        };

        uint32_t base = static_cast<uint32_t>(vertices.size());
        vertices.reserve(vertices.size() + mesh.vertices.size());
        indices.reserve(indices.size() + mesh.vertices.size());
        for (const Vertex& src : mesh.vertices) {
            XMFLOAT3 pos = Add3(origin, Add3(Scale3(right, src.pos.x * scaleX),
                Add3(Scale3(up, src.pos.y * scaleY), Scale3(forward, src.pos.z * scaleZ))));
            XMFLOAT3 normal = Normalize3(Add3(Scale3(right, src.normal.x * invX),
                Add3(Scale3(up, src.normal.y * invY), Scale3(forward, src.normal.z * invZ))), up);
            XMFLOAT3 tangent = Normalize3(Add3(Scale3(right, src.tangent.x * scaleX),
                Add3(Scale3(up, src.tangent.y * scaleY), Scale3(forward, src.tangent.z * scaleZ))), right);
            XMFLOAT2 uv = mesh.generatedUvFallback ? generatedUv(src) : src.uv;
            float outMaterial = materialOverride >= 0.0f ? materialOverride : src.material;
            int materialId = std::clamp(static_cast<int>(std::floor(outMaterial)), 0, kMaterialCount - 1);
            if (materialOverride < 0.0f && materialId == 22) {
                outMaterial = 22.020f + std::fmod(std::abs(materialVariant), 0.92f);
            }
            vertices.push_back({pos, normal, tangent, uv, outMaterial});
        }
        for (uint32_t n = 0; n < static_cast<uint32_t>(mesh.vertices.size()); ++n) {
            uint32_t idx = base + n;
            indices.push_back(idx);
            if (shadowIndices) shadowIndices->push_back(idx);
        }
        return true;
    }

    bool AppendStaticPropMeshGrounded(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices,
                                      const StaticPropMesh& mesh, XMFLOAT3 floorCenter, float yaw,
                                      float scaleX, float scaleY, float scaleZ,
                                      float pitch = 0.0f, float materialOverride = -1.0f,
                                      std::vector<uint32_t>* shadowIndices = nullptr,
                                      float materialVariant = 0.0f) const {
        if (mesh.vertices.empty()) return false;

        float c = std::cos(yaw);
        float s = std::sin(yaw);
        XMFLOAT3 right{c, 0.0f, -s};
        XMFLOAT3 worldUp{0.0f, 1.0f, 0.0f};
        XMFLOAT3 forward{s, 0.0f, c};
        float cp = std::cos(pitch);
        float sp = std::sin(pitch);
        XMFLOAT3 up = Normalize3(Add3(Scale3(worldUp, cp), Scale3(forward, sp)), worldUp);
        forward = Normalize3(Add3(Scale3(forward, cp), Scale3(worldUp, -sp)), forward);

        float minY = std::numeric_limits<float>::max();
        for (const Vertex& src : mesh.vertices) {
            float y = right.y * src.pos.x * scaleX + up.y * src.pos.y * scaleY + forward.y * src.pos.z * scaleZ;
            minY = std::min(minY, y);
        }
        XMFLOAT3 origin{floorCenter.x, floorCenter.y - minY, floorCenter.z};
        return AppendStaticPropMesh(vertices, indices, mesh, origin, yaw, scaleX, scaleY, scaleZ, pitch, materialOverride, shadowIndices, materialVariant);
    }

    XMFLOAT2 FloorUv(float x, float z) const {
        return {x / settings_.floorTextureMeters, z / settings_.floorTextureMeters};
    }

    XMFLOAT2 CeilingUv(float x, float z) const {
        float scaleX = settings_.ceilingTextureMeters > 0.001f
            ? std::max(0.2f, settings_.ceilingTextureMeters)
            : std::max(0.2f, maze_.tileW * (2.0f / 3.0f));
        float scaleZ = settings_.ceilingTextureMeters > 0.001f
            ? std::max(0.2f, settings_.ceilingTextureMeters)
            : std::max(0.2f, maze_.tileD * (2.0f / 3.0f));
        float originX = -static_cast<float>(maze_.w) * maze_.tileW * 0.5f;
        float originZ = -static_cast<float>(maze_.h) * maze_.tileD * 0.5f;
        return {(x - originX) / scaleX, (z - originZ) / scaleZ};
    }

    XMFLOAT2 WallUvX(float x, float y) const {
        return {-x / settings_.wallTextureMeters, -y / settings_.wallTextureMeters};
    }

    XMFLOAT2 WallUvZ(float z, float y) const {
        return {-z / settings_.wallTextureMeters, -y / settings_.wallTextureMeters};
    }

    void AppendStaticIndexChunks(const std::vector<Vertex>& vertices,
                                 const std::vector<uint32_t>& sourceIndices,
                                 UINT rangeStart,
                                 UINT rangeCount,
                                 std::vector<uint32_t>& destIndices,
                                 std::vector<StaticIndexChunk>& chunks) const {
        constexpr int kChunkTiles = 4;
        int chunksX = std::max(1, (maze_.w + kChunkTiles - 1) / kChunkTiles);
        int chunksY = std::max(1, (maze_.h + kChunkTiles - 1) / kChunkTiles);
        struct ChunkBuild {
            std::vector<uint32_t> indices;
            XMFLOAT3 min{std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
            XMFLOAT3 max{-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max()};
            int minTileX = std::numeric_limits<int>::max();
            int minTileY = std::numeric_limits<int>::max();
            int maxTileX = std::numeric_limits<int>::min();
            int maxTileY = std::numeric_limits<int>::min();
        };
        std::vector<ChunkBuild> build(static_cast<size_t>(chunksX * chunksY));
        float ox = -static_cast<float>(maze_.w) * maze_.tileW * 0.5f;
        float oz = -static_cast<float>(maze_.h) * maze_.tileD * 0.5f;
        UINT rangeEnd = std::min<UINT>(rangeStart + rangeCount, static_cast<UINT>(sourceIndices.size()));

        auto extendBounds = [](ChunkBuild& b, const XMFLOAT3& p) {
            b.min.x = std::min(b.min.x, p.x);
            b.min.y = std::min(b.min.y, p.y);
            b.min.z = std::min(b.min.z, p.z);
            b.max.x = std::max(b.max.x, p.x);
            b.max.y = std::max(b.max.y, p.y);
            b.max.z = std::max(b.max.z, p.z);
        };

        for (UINT i = rangeStart; i + 2 < rangeEnd; i += 3) {
            uint32_t ia = sourceIndices[i];
            uint32_t ib = sourceIndices[i + 1];
            uint32_t ic = sourceIndices[i + 2];
            if (ia >= vertices.size() || ib >= vertices.size() || ic >= vertices.size()) continue;
            const XMFLOAT3& a = vertices[ia].pos;
            const XMFLOAT3& b = vertices[ib].pos;
            const XMFLOAT3& c = vertices[ic].pos;
            float cx = (a.x + b.x + c.x) / 3.0f;
            float cz = (a.z + b.z + c.z) / 3.0f;
            int tileX = std::clamp(static_cast<int>(std::floor((cx - ox) / std::max(0.001f, maze_.tileW))), 0, std::max(0, maze_.w - 1));
            int tileY = std::clamp(static_cast<int>(std::floor((cz - oz) / std::max(0.001f, maze_.tileD))), 0, std::max(0, maze_.h - 1));
            int chunkX = std::clamp(tileX / kChunkTiles, 0, chunksX - 1);
            int chunkY = std::clamp(tileY / kChunkTiles, 0, chunksY - 1);
            ChunkBuild& chunk = build[static_cast<size_t>(chunkY * chunksX + chunkX)];
            chunk.minTileX = std::min(chunk.minTileX, tileX);
            chunk.minTileY = std::min(chunk.minTileY, tileY);
            chunk.maxTileX = std::max(chunk.maxTileX, tileX);
            chunk.maxTileY = std::max(chunk.maxTileY, tileY);
            chunk.indices.push_back(ia);
            chunk.indices.push_back(ib);
            chunk.indices.push_back(ic);
            extendBounds(chunk, a);
            extendBounds(chunk, b);
            extendBounds(chunk, c);
        }

        chunks.clear();
        chunks.reserve(build.size());
        for (ChunkBuild& b : build) {
            if (b.indices.empty()) continue;
            StaticIndexChunk chunk{};
            chunk.startIndex = static_cast<UINT>(destIndices.size());
            chunk.indexCount = static_cast<UINT>(b.indices.size());
            chunk.center = {
                (b.min.x + b.max.x) * 0.5f,
                (b.min.y + b.max.y) * 0.5f,
                (b.min.z + b.max.z) * 0.5f
            };
            float dx = b.max.x - chunk.center.x;
            float dy = b.max.y - chunk.center.y;
            float dz = b.max.z - chunk.center.z;
            chunk.radius = std::sqrt(dx * dx + dy * dy + dz * dz);
            chunk.minTileX = std::clamp(b.minTileX - 1, 0, std::max(0, maze_.w - 1));
            chunk.minTileY = std::clamp(b.minTileY - 1, 0, std::max(0, maze_.h - 1));
            chunk.maxTileX = std::clamp(b.maxTileX + 1, 0, std::max(0, maze_.w - 1));
            chunk.maxTileY = std::clamp(b.maxTileY + 1, 0, std::max(0, maze_.h - 1));
            destIndices.insert(destIndices.end(), b.indices.begin(), b.indices.end());
            chunks.push_back(chunk);
        }
    }

    void CreateMazeMesh() {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        std::vector<uint32_t> waterIndices;
        std::vector<uint32_t> liquidIndices;
        std::vector<uint32_t> transparentIndices;
        std::vector<uint32_t> propShadowIndices;
        vertices.reserve(maze_.w * maze_.h * 64);
        indices.reserve(maze_.w * maze_.h * 96);
        waterIndices.reserve(maze_.w * maze_.h * 6);
        liquidIndices.reserve(maze_.w * maze_.h * 18);
        transparentIndices.reserve(maze_.w * maze_.h * 12);
        propShadowIndices.reserve(maze_.w * maze_.h * 24);
        propLookPoints_.clear();
        sparkEmitters_.clear();
        runtimeLamps_.clear();
        staticOpaqueChunks_.clear();
        staticFloorCeilingChunks_.clear();
        staticWaterChunks_.clear();
        staticTransparentChunks_.clear();
        mapOverlayCachedVerts_.clear();
        mapOverlayNextUpdateTime_ = 0.0f;
        lampDamagePixels_.assign(static_cast<size_t>(std::max(0, maze_.w * maze_.h)), 0);
        wetFootstepTiles_.assign(static_cast<size_t>(std::max(0, maze_.w * maze_.h)), 0);
        wetCeilingTiles_.assign(static_cast<size_t>(std::max(0, maze_.w * maze_.h)), 0);
        wetDripEmitters_.clear();
        wetFloorFootprints_.clear();
        lampDamageDirty_ = true;
        steamEmitters_.clear();
        bloodScarePoints_.clear();
        exitSignLightPos_ = {};
        exitSignLightStrength_ = 0.0f;

        const float tileW = maze_.tileW;
        const float tileD = maze_.tileD;
        const float tileAvg = maze_.TileAverage();
        const float tileMin = maze_.TileMinimum();
        const float wallH = settings_.wallHeightMeters;
        float ox = -static_cast<float>(maze_.w) * tileW * 0.5f;
        float oz = -static_cast<float>(maze_.h) * tileD * 0.5f;

        struct ExitPortal {
            Tile tile{};
            int dx = 0;
            int dy = 1;
            float yaw = kPi;
            XMFLOAT3 inward{0.0f, 0.0f, -1.0f};
            XMFLOAT3 right{-1.0f, 0.0f, 0.0f};
            XMFLOAT3 wallCenter{};
            float halfSpan = kTile * 0.5f;
            bool valid = false;
        };

        auto makeExitPortal = [&]() {
            ExitPortal portal{};
            portal.tile = maze_.exit;
            struct DoorSide { int dx; int dy; };
            std::vector<DoorSide> sides;
            Tile e = maze_.exit;
            if (runtimeMode_ == RendererRuntimeMode::MainMenu) {
                sides.push_back({-1, 0});
            } else {
                if (e.y == maze_.h - 2) sides.push_back({0, 1});
                if (e.x == maze_.w - 2) sides.push_back({1, 0});
                if (e.y == 1) sides.push_back({0, -1});
                if (e.x == 1) sides.push_back({-1, 0});
            }
            const DoorSide fallbackSides[] = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};
            sides.insert(sides.end(), std::begin(fallbackSides), std::end(fallbackSides));

            for (const DoorSide& side : sides) {
                if (runtimeMode_ != RendererRuntimeMode::MainMenu && maze_.IsOpen(e.x + side.dx, e.y + side.dy)) continue;
                XMFLOAT3 c = maze_.WorldCenter(e, 0.0f);
                portal.dx = side.dx;
                portal.dy = side.dy;
                portal.inward = {-static_cast<float>(side.dx), 0.0f, -static_cast<float>(side.dy)};
                portal.yaw = std::atan2(portal.inward.x, portal.inward.z);
                XMVECTOR rightVec = XMVector3Normalize(XMVector3Cross(XMVectorSet(0, 1, 0, 0), XMLoadFloat3(&portal.inward)));
                XMStoreFloat3(&portal.right, rightVec);
                portal.wallCenter = {c.x + side.dx * tileW * 0.5f, 0.0f, c.z + side.dy * tileD * 0.5f};
                portal.halfSpan = (side.dy != 0 ? tileW : tileD) * 0.5f;
                portal.valid = true;
                break;
            }
            return portal;
        };

        ExitPortal exitPortal = makeExitPortal();

        auto addFloorCeilingRun = [&](int y, int x0, int x1) {
            float z0 = oz + y * tileD;
            float z1 = z0 + tileD;
            for (int x = x0; x < x1; ++x) {
                float l = ox + x * tileW;
                float r = l + tileW;
                AddQuadUV(vertices, indices,
                    {l, 0, z1}, {r, 0, z1}, {r, 0, z0}, {l, 0, z0},
                    {0, 1, 0}, {1, 0, 0},
                    FloorUv(l, z1), FloorUv(r, z1), FloorUv(r, z0), FloorUv(l, z0), 1.0f);
                AddQuadUV(vertices, indices,
                    {l, wallH, z0}, {r, wallH, z0}, {r, wallH, z1}, {l, wallH, z1},
                    {0, -1, 0}, {1, 0, 0},
                    CeilingUv(l, z0), CeilingUv(r, z0), CeilingUv(r, z1), CeilingUv(l, z1), 2.0f);
            }
        };

        auto addNorthWallRun = [&](int y, int x0, int x1) {
            float z = oz + y * tileD;
            for (int x = x0; x < x1; ++x) {
                float l = ox + x * tileW;
                float r = l + tileW;
                AddQuadUV(vertices, indices,
                    {r, 0, z}, {l, 0, z}, {l, wallH, z}, {r, wallH, z},
                    {0, 0, 1}, {-1, 0, 0},
                    WallUvX(r, 0), WallUvX(l, 0), WallUvX(l, wallH), WallUvX(r, wallH), 0.0f);
            }
        };

        auto addSouthWallRun = [&](int y, int x0, int x1) {
            float z = oz + (y + 1) * tileD;
            for (int x = x0; x < x1; ++x) {
                float l = ox + x * tileW;
                float r = l + tileW;
                AddQuadUV(vertices, indices,
                    {l, 0, z}, {r, 0, z}, {r, wallH, z}, {l, wallH, z},
                    {0, 0, -1}, {1, 0, 0},
                    WallUvX(l, 0), WallUvX(r, 0), WallUvX(r, wallH), WallUvX(l, wallH), 0.0f);
            }
        };

        auto addWestWallRun = [&](int x, int y0, int y1) {
            float l = ox + x * tileW;
            for (int y = y0; y < y1; ++y) {
                float z0 = oz + y * tileD;
                float z1 = z0 + tileD;
                AddQuadUV(vertices, indices,
                    {l, 0, z0}, {l, 0, z1}, {l, wallH, z1}, {l, wallH, z0},
                    {1, 0, 0}, {0, 0, 1},
                    WallUvZ(z0, 0), WallUvZ(z1, 0), WallUvZ(z1, wallH), WallUvZ(z0, wallH), 0.0f);
            }
        };

        auto addEastWallRun = [&](int x, int y0, int y1) {
            float r = ox + (x + 1) * tileW;
            for (int y = y0; y < y1; ++y) {
                float z0 = oz + y * tileD;
                float z1 = z0 + tileD;
                AddQuadUV(vertices, indices,
                    {r, 0, z1}, {r, 0, z0}, {r, wallH, z0}, {r, wallH, z1},
                    {-1, 0, 0}, {0, 0, -1},
                    WallUvZ(z1, 0), WallUvZ(z0, 0), WallUvZ(z0, wallH), WallUvZ(z1, wallH), 0.0f);
            }
        };

        auto addExitDoorwayWall = [&]() {
            if (!exitPortal.valid) return;
            XMFLOAT3 up{0.0f, 1.0f, 0.0f};
            constexpr float kExitDoorHalfW = 0.60f;
            constexpr float kExitDoorHalfH = 1.05f;
            constexpr float kExitDoorCenterY = 1.05f;
            constexpr float kExitFramePostHalfW = 0.055f;
            constexpr float kExitFrameTopHalfH = 0.070f;
            constexpr float kExitDoorwayHalfW = kExitDoorHalfW + kExitFramePostHalfW * 2.0f;
            constexpr float kExitDoorwayTop = kExitDoorCenterY + kExitDoorHalfH + kExitFrameTopHalfH * 2.0f;
            float openingHalf = std::min(exitPortal.halfSpan * 0.86f, kExitDoorwayHalfW);
            float doorwayTop = std::min(wallH - 0.04f, kExitDoorwayTop);
            float vestibuleH = doorwayTop;
            float vestibuleLength = std::min(
                std::max(tileAvg * 7.5f, settings_.fogEndMeters + tileAvg * 3.0f),
                tileAvg * 14.0f);
            float vestibuleHalf = openingHalf;

            auto wallPoint = [&](float along, float y, float push = 0.0f) {
                return Add3(exitPortal.wallCenter,
                    Add3(Scale3(exitPortal.right, along), Add3({0.0f, y, 0.0f}, Scale3(exitPortal.inward, push))));
            };

            auto addWallPatch = [&](float a, float b, float y0, float y1) {
                if (b <= a || y1 <= y0) return;
                AddQuadUV(vertices, indices,
                    wallPoint(a, y0), wallPoint(b, y0), wallPoint(b, y1), wallPoint(a, y1),
                    exitPortal.inward, exitPortal.right,
                    {a / settings_.wallTextureMeters, y0 / settings_.wallTextureMeters},
                    {b / settings_.wallTextureMeters, y0 / settings_.wallTextureMeters},
                    {b / settings_.wallTextureMeters, y1 / settings_.wallTextureMeters},
                    {a / settings_.wallTextureMeters, y1 / settings_.wallTextureMeters},
                    0.0f);
            };

            addWallPatch(-exitPortal.halfSpan, -openingHalf, 0.0f, wallH);
            addWallPatch(openingHalf, exitPortal.halfSpan, 0.0f, wallH);
            addWallPatch(-openingHalf, openingHalf, doorwayTop, wallH);

            XMFLOAT3 outward = Scale3(exitPortal.inward, -1.0f);
            XMFLOAT3 nearCenter = Add3(exitPortal.wallCenter, Scale3(outward, 0.05f));
            XMFLOAT3 farCenter = Add3(exitPortal.wallCenter, Scale3(outward, vestibuleLength));
            auto p = [&](float along, float y, float depth) {
                return Add3(nearCenter, Add3(Scale3(exitPortal.right, along), Add3(Scale3(up, y), Scale3(outward, depth))));
            };

            if (runtimeMode_ == RendererRuntimeMode::MainMenu) {
                constexpr int kStepCount = 9;
                float stepDepth = vestibuleLength / static_cast<float>(kStepCount);
                float totalRise = std::min(wallH * 0.76f, 2.18f);
                float stepRise = totalRise / static_cast<float>(kStepCount);
                for (int i = 0; i < kStepCount; ++i) {
                    float d0 = static_cast<float>(i) * stepDepth;
                    float d1 = static_cast<float>(i + 1) * stepDepth;
                    float y0 = static_cast<float>(i) * stepRise;
                    float y1 = static_cast<float>(i + 1) * stepRise;
                    float stairMaterial = 1.0f;
                    AddQuadUV(vertices, indices,
                        p(-vestibuleHalf, y0, d0), p(vestibuleHalf, y0, d0), p(vestibuleHalf, y0, d1), p(-vestibuleHalf, y0, d1),
                        {0, 1, 0}, exitPortal.right,
                        FloorUv(p(-vestibuleHalf, y0, d0).x, p(-vestibuleHalf, y0, d0).z),
                        FloorUv(p(vestibuleHalf, y0, d0).x, p(vestibuleHalf, y0, d0).z),
                        FloorUv(p(vestibuleHalf, y0, d1).x, p(vestibuleHalf, y0, d1).z),
                        FloorUv(p(-vestibuleHalf, y0, d1).x, p(-vestibuleHalf, y0, d1).z),
                        stairMaterial);
                    AddQuadUV(vertices, indices,
                        p(vestibuleHalf, y0, d1), p(-vestibuleHalf, y0, d1), p(-vestibuleHalf, y1, d1), p(vestibuleHalf, y1, d1),
                        Scale3(outward, -1.0f), Scale3(exitPortal.right, -1.0f),
                        {0, 0}, {1, 0}, {1, stepRise / settings_.wallTextureMeters}, {0, stepRise / settings_.wallTextureMeters},
                        0.0f);
                }
                auto addStairSide = [&](float side) {
                    XMFLOAT3 normal = Scale3(exitPortal.right, -side);
                    AddQuadUV(vertices, indices,
                        p(side * vestibuleHalf, 0.0f, vestibuleLength), p(side * vestibuleHalf, 0.0f, 0.0f),
                        p(side * vestibuleHalf, vestibuleH, 0.0f), p(side * vestibuleHalf, vestibuleH + totalRise * 0.44f, vestibuleLength),
                        normal, Scale3(outward, -1.0f),
                        {0, 0}, {vestibuleLength / settings_.wallTextureMeters, 0},
                        {vestibuleLength / settings_.wallTextureMeters, vestibuleH / settings_.wallTextureMeters}, {0, (vestibuleH + totalRise) / settings_.wallTextureMeters},
                        0.0f);
                };
                addStairSide(-1.0f);
                addStairSide(1.0f);
                XMFLOAT3 c0 = p(-vestibuleHalf, vestibuleH + totalRise * 0.78f, vestibuleLength);
                XMFLOAT3 c1 = p(vestibuleHalf, vestibuleH + totalRise * 0.78f, vestibuleLength);
                XMFLOAT3 c2 = p(vestibuleHalf, vestibuleH + 0.48f, stepDepth * 0.72f);
                XMFLOAT3 c3 = p(-vestibuleHalf, vestibuleH + 0.48f, stepDepth * 0.72f);
                AddQuadUV(vertices, indices,
                    c0, c1, c2, c3,
                    {0, -1, 0}, exitPortal.right,
                    CeilingUv(c0.x, c0.z), CeilingUv(c1.x, c1.z), CeilingUv(c2.x, c2.z), CeilingUv(c3.x, c3.z),
                    2.0f);
                AddQuadUV(vertices, indices,
                    p(vestibuleHalf, totalRise, vestibuleLength), p(-vestibuleHalf, totalRise, vestibuleLength),
                    p(-vestibuleHalf, vestibuleH + totalRise * 0.78f, vestibuleLength), p(vestibuleHalf, vestibuleH + totalRise * 0.78f, vestibuleLength),
                    Scale3(outward, -1.0f), Scale3(exitPortal.right, -1.0f),
                    {0, 0}, {1, 0}, {1, 1}, {0, 1}, 0.0f);
                return;
            }

            AddQuadUV(vertices, indices,
                p(-vestibuleHalf, 0.0f, 0.0f), p(vestibuleHalf, 0.0f, 0.0f), p(vestibuleHalf, 0.0f, vestibuleLength), p(-vestibuleHalf, 0.0f, vestibuleLength),
                {0, 1, 0}, exitPortal.right,
                FloorUv(nearCenter.x, nearCenter.z), FloorUv(Add3(nearCenter, Scale3(exitPortal.right, vestibuleHalf * 2.0f)).x, nearCenter.z),
                FloorUv(Add3(farCenter, Scale3(exitPortal.right, vestibuleHalf)).x, Add3(farCenter, Scale3(exitPortal.right, vestibuleHalf)).z),
                FloorUv(Add3(farCenter, Scale3(exitPortal.right, -vestibuleHalf)).x, Add3(farCenter, Scale3(exitPortal.right, -vestibuleHalf)).z),
                1.0f);
            XMFLOAT3 vc0 = p(-vestibuleHalf, vestibuleH, vestibuleLength);
            XMFLOAT3 vc1 = p(vestibuleHalf, vestibuleH, vestibuleLength);
            XMFLOAT3 vc2 = p(vestibuleHalf, vestibuleH, 0.0f);
            XMFLOAT3 vc3 = p(-vestibuleHalf, vestibuleH, 0.0f);
            AddQuadUV(vertices, indices,
                vc0, vc1, vc2, vc3,
                {0, -1, 0}, exitPortal.right,
                CeilingUv(vc0.x, vc0.z), CeilingUv(vc1.x, vc1.z),
                CeilingUv(vc2.x, vc2.z), CeilingUv(vc3.x, vc3.z),
                2.0f);
            auto addVestibuleSide = [&](float side) {
                XMFLOAT3 normal = Scale3(exitPortal.right, -side);
                AddQuadUV(vertices, indices,
                    p(side * vestibuleHalf, 0.0f, vestibuleLength), p(side * vestibuleHalf, 0.0f, 0.0f),
                    p(side * vestibuleHalf, vestibuleH, 0.0f), p(side * vestibuleHalf, vestibuleH, vestibuleLength),
                    normal, Scale3(outward, -1.0f),
                    {0, 0}, {vestibuleLength / settings_.wallTextureMeters, 0},
                    {vestibuleLength / settings_.wallTextureMeters, vestibuleH / settings_.wallTextureMeters}, {0, vestibuleH / settings_.wallTextureMeters},
                    0.0f);
            };
            addVestibuleSide(-1.0f);
            addVestibuleSide(1.0f);
            AddQuadUV(vertices, indices,
                p(vestibuleHalf, 0.0f, vestibuleLength), p(-vestibuleHalf, 0.0f, vestibuleLength),
                p(-vestibuleHalf, vestibuleH, vestibuleLength), p(vestibuleHalf, vestibuleH, vestibuleLength),
                Scale3(outward, -1.0f), Scale3(exitPortal.right, -1.0f),
                {0, 0}, {1, 0}, {1, 1}, {0, 1}, 10.0f);
        };

        auto addNorthWallRunWithPortal = [&](int y, int x0, int x1) {
            if (exitPortal.valid && exitPortal.dy == -1 && exitPortal.tile.y == y && exitPortal.tile.x >= x0 && exitPortal.tile.x < x1) {
                if (x0 < exitPortal.tile.x) addNorthWallRun(y, x0, exitPortal.tile.x);
                addExitDoorwayWall();
                if (exitPortal.tile.x + 1 < x1) addNorthWallRun(y, exitPortal.tile.x + 1, x1);
            } else {
                addNorthWallRun(y, x0, x1);
            }
        };

        auto addSouthWallRunWithPortal = [&](int y, int x0, int x1) {
            if (exitPortal.valid && exitPortal.dy == 1 && exitPortal.tile.y == y && exitPortal.tile.x >= x0 && exitPortal.tile.x < x1) {
                if (x0 < exitPortal.tile.x) addSouthWallRun(y, x0, exitPortal.tile.x);
                addExitDoorwayWall();
                if (exitPortal.tile.x + 1 < x1) addSouthWallRun(y, exitPortal.tile.x + 1, x1);
            } else {
                addSouthWallRun(y, x0, x1);
            }
        };

        auto addWestWallRunWithPortal = [&](int x, int y0, int y1) {
            if (exitPortal.valid && exitPortal.dx == -1 && exitPortal.tile.x == x && exitPortal.tile.y >= y0 && exitPortal.tile.y < y1) {
                if (y0 < exitPortal.tile.y) addWestWallRun(x, y0, exitPortal.tile.y);
                addExitDoorwayWall();
                if (exitPortal.tile.y + 1 < y1) addWestWallRun(x, exitPortal.tile.y + 1, y1);
            } else {
                addWestWallRun(x, y0, y1);
            }
        };

        auto addEastWallRunWithPortal = [&](int x, int y0, int y1) {
            if (exitPortal.valid && exitPortal.dx == 1 && exitPortal.tile.x == x && exitPortal.tile.y >= y0 && exitPortal.tile.y < y1) {
                if (y0 < exitPortal.tile.y) addEastWallRun(x, y0, exitPortal.tile.y);
                addExitDoorwayWall();
                if (exitPortal.tile.y + 1 < y1) addEastWallRun(x, exitPortal.tile.y + 1, y1);
            } else {
                addEastWallRun(x, y0, y1);
            }
        };

        for (int y = 0; y < maze_.h; ++y) {
            int x = 0;
            while (x < maze_.w) {
                while (x < maze_.w && !(maze_.IsOpen(x, y) && !maze_.IsOpen(x, y - 1))) ++x;
                int start = x;
                while (x < maze_.w && maze_.IsOpen(x, y) && !maze_.IsOpen(x, y - 1)) ++x;
                if (start < x) addNorthWallRunWithPortal(y, start, x);
            }

            x = 0;
            while (x < maze_.w) {
                while (x < maze_.w && !(maze_.IsOpen(x, y) && !maze_.IsOpen(x, y + 1))) ++x;
                int start = x;
                while (x < maze_.w && maze_.IsOpen(x, y) && !maze_.IsOpen(x, y + 1)) ++x;
                if (start < x) addSouthWallRunWithPortal(y, start, x);
            }
        }

        for (int x = 0; x < maze_.w; ++x) {
            int y = 0;
            while (y < maze_.h) {
                while (y < maze_.h && !(maze_.IsOpen(x, y) && !maze_.IsOpen(x - 1, y))) ++y;
                int start = y;
                while (y < maze_.h && maze_.IsOpen(x, y) && !maze_.IsOpen(x - 1, y)) ++y;
                if (start < y) addWestWallRunWithPortal(x, start, y);
            }

            y = 0;
            while (y < maze_.h) {
                while (y < maze_.h && !(maze_.IsOpen(x, y) && !maze_.IsOpen(x + 1, y))) ++y;
                int start = y;
                while (y < maze_.h && maze_.IsOpen(x, y) && !maze_.IsOpen(x + 1, y)) ++y;
                if (start < y) addEastWallRunWithPortal(x, start, y);
            }
        }

        auto tileHash = [&](int x, int y, float salt) {
            return LampHash(static_cast<float>(x) + salt * 3.17f, static_cast<float>(y) - salt * 5.31f);
        };

        struct FloorFootprint {
            float x = 0.0f;
            float z = 0.0f;
            float hx = 0.0f;
            float hz = 0.0f;
            float c = 1.0f;
            float s = 0.0f;
        };
        std::vector<FloorFootprint> floorReservations;
        floorReservations.reserve(512);

        auto makeFootprint = [&](float px, float pz, float width, float depth, float yaw, float pad) {
            FloorFootprint fp{};
            fp.x = px;
            fp.z = pz;
            fp.hx = width * 0.5f + pad;
            fp.hz = depth * 0.5f + pad;
            fp.c = std::cos(yaw);
            fp.s = std::sin(yaw);
            return fp;
        };
        auto axisDot = [](float ax, float az, float bx, float bz) {
            return ax * bx + az * bz;
        };
        auto footprintOverlap = [&](const FloorFootprint& a, const FloorFootprint& b) {
            auto separatedOn = [&](float ax, float az) {
                float dx = b.x - a.x;
                float dz = b.z - a.z;
                float center = std::abs(axisDot(dx, dz, ax, az));
                float ar = a.hx * std::abs(axisDot(a.c, -a.s, ax, az)) +
                    a.hz * std::abs(axisDot(a.s, a.c, ax, az));
                float br = b.hx * std::abs(axisDot(b.c, -b.s, ax, az)) +
                    b.hz * std::abs(axisDot(b.s, b.c, ax, az));
                return center > ar + br;
            };
            if (separatedOn(a.c, -a.s)) return false;
            if (separatedOn(a.s, a.c)) return false;
            if (separatedOn(b.c, -b.s)) return false;
            if (separatedOn(b.s, b.c)) return false;
            return true;
        };
        auto footprintFitsMaze = [&](float px, float pz, float width, float depth, float yaw, float wallPad) {
            float c = std::cos(yaw);
            float s = std::sin(yaw);
            XMFLOAT3 right{c, 0.0f, -s};
            XMFLOAT3 forward{s, 0.0f, c};
            float hx = width * 0.5f + wallPad;
            float hz = depth * 0.5f + wallPad;
            int sxCount = std::clamp(static_cast<int>(std::ceil((width + wallPad * 2.0f) / (tileMin * 0.18f))) + 2, 3, 13);
            int szCount = std::clamp(static_cast<int>(std::ceil((depth + wallPad * 2.0f) / (tileMin * 0.18f))) + 2, 3, 13);
            for (int sy = 0; sy < szCount; ++sy) {
                float fy = szCount == 1 ? 0.0f : static_cast<float>(sy) / static_cast<float>(szCount - 1);
                float ly = Lerp(-hz, hz, fy);
                for (int sx = 0; sx < sxCount; ++sx) {
                    float fx = sxCount == 1 ? 0.0f : static_cast<float>(sx) / static_cast<float>(sxCount - 1);
                    float lx = Lerp(-hx, hx, fx);
                    XMFLOAT3 p = Add3({px, 0.0f, pz}, OrientedOffset(right, {0, 1, 0}, forward, lx, 0.0f, ly));
                    Tile tile = maze_.TileFromWorld(p.x, p.z);
                    if (!maze_.IsOpen(tile.x, tile.y)) return false;
                }
            }
            return true;
        };
        auto floorFootprintClear = [&](float px, float pz, float width, float depth, float yaw, float pad = 0.055f) {
            if (!footprintFitsMaze(px, pz, width, depth, yaw, pad)) return false;
            FloorFootprint candidate = makeFootprint(px, pz, width, depth, yaw, pad);
            for (const FloorFootprint& reserved : floorReservations) {
                if (footprintOverlap(candidate, reserved)) return false;
            }
            return true;
        };
        auto reserveFloorFootprint = [&](float px, float pz, float width, float depth, float yaw, float pad = 0.075f) {
            if (!floorFootprintClear(px, pz, width, depth, yaw, pad)) return false;
            floorReservations.push_back(makeFootprint(px, pz, width, depth, yaw, pad));
            return true;
        };
        auto longFloorFootprintClear = [&](float px, float pz, float width, float depth, float yaw, float pad = 0.055f) {
            if (!floorFootprintClear(px, pz, width, depth, yaw, pad)) return false;
            float c = std::cos(yaw);
            float s = std::sin(yaw);
            XMFLOAT3 right{c, 0.0f, -s};
            XMFLOAT3 forward{s, 0.0f, c};
            int steps = std::clamp(static_cast<int>(width / (tileMin * 0.30f)) + 2, 4, 22);
            for (int i = 0; i <= steps; ++i) {
                float along = (static_cast<float>(i) / static_cast<float>(steps) - 0.5f) * width;
                const float laterals[] = {-0.46f, 0.0f, 0.46f};
                for (float lateral : laterals) {
                    XMFLOAT3 p = Add3({px, 0.0f, pz}, OrientedOffset(right, {0, 1, 0}, forward, along, 0.0f, lateral * depth));
                    Tile tile = maze_.TileFromWorld(p.x, p.z);
                    if (!maze_.IsOpen(tile.x, tile.y)) return false;
                }
            }
            return true;
        };
        auto constrainedHallwayTile = [&](Tile t) {
            if (!maze_.IsOpen(t.x, t.y)) return false;
            return maze_.OpenNeighborCount(t) <= 2 && !IsRoomLike(t);
        };
        auto reserveRealisticFloorFootprint = [&](float& px, float& pz, float width, float depth,
                                                  float& yaw, float pad, float seed) {
            Tile t = maze_.TileFromWorld(px, pz);
            if (!constrainedHallwayTile(t)) {
                return reserveFloorFootprint(px, pz, width, depth, yaw, pad);
            }

            struct CandidatePlacement {
                float x = 0.0f;
                float z = 0.0f;
                float yaw = 0.0f;
                float score = -1.0e9f;
            };
            CandidatePlacement best{};
            XMFLOAT3 center = maze_.WorldCenter(t, 0.0f);
            const Tile dirs[] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
            int openCount = 0;
            for (Tile d : dirs) {
                if (!maze_.IsOpen(t.x + d.x, t.y + d.y)) continue;
                ++openCount;
                float dirYaw = std::atan2(static_cast<float>(d.x), static_cast<float>(d.y));
                float jitter = (LampHash(center.x + seed * 11.7f + d.x * 3.1f, center.z - seed * 9.3f + d.y * 4.7f) - 0.5f) * 0.22f;
                const float shifts[] = {0.20f, 0.08f, -0.02f};
                const float flips[] = {0.0f, kPi};
                for (float flip : flips) {
                    for (float shift : shifts) {
                        float cx = center.x + static_cast<float>(d.x) * tileW * shift;
                        float cz = center.z + static_cast<float>(d.y) * tileD * shift;
                        float candidateYaw = dirYaw + flip + jitter;
                        if (!floorFootprintClear(cx, cz, width, depth, candidateYaw, pad)) continue;
                        float sideClear = static_cast<float>(maze_.LocalOpenCount(maze_.TileFromWorld(cx, cz), 1));
                        float shiftScore = shift * 3.2f;
                        float longAxisScore = depth >= width ? 1.0f : 0.35f;
                        float score = sideClear * 0.22f + shiftScore + longAxisScore - std::abs(jitter) * 0.15f;
                        if (score > best.score) {
                            best = {cx, cz, candidateYaw, score};
                        }
                    }
                }
            }

            if (openCount <= 0 || best.score <= -1.0e8f) return false;
            px = best.x;
            pz = best.z;
            yaw = best.yaw;
            return reserveFloorFootprint(px, pz, width, depth, yaw, pad);
        };

        auto adjustLongHallwayPlacement = [&](float& px, float& pz, float& yaw, float width, float depth, float seed) {
            Tile t = maze_.TileFromWorld(px, pz);
            if (!constrainedHallwayTile(t)) return true;
            XMFLOAT3 center = maze_.WorldCenter(t, 0.0f);
            const Tile dirs[] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
            float bestScore = -1.0e9f;
            float bestX = px;
            float bestZ = pz;
            float bestYaw = yaw;
            for (Tile d : dirs) {
                if (!maze_.IsOpen(t.x + d.x, t.y + d.y)) continue;
                float dirYaw = std::atan2(static_cast<float>(d.x), static_cast<float>(d.y));
                float jitter = (LampHash(center.x + seed * 4.9f + d.x, center.z - seed * 6.3f + d.y) - 0.5f) * 0.18f;
                const float shifts[] = {0.18f, 0.06f};
                const float flips[] = {0.0f, kPi};
                for (float shift : shifts) {
                    for (float flip : flips) {
                        float cx = center.x + static_cast<float>(d.x) * tileW * shift;
                        float cz = center.z + static_cast<float>(d.y) * tileD * shift;
                        float candidateYaw = dirYaw + flip + jitter;
                        if (!longFloorFootprintClear(cx, cz, width, depth, candidateYaw, 0.075f)) continue;
                        float score = shift * 3.0f + static_cast<float>(maze_.LocalOpenCount(maze_.TileFromWorld(cx, cz), 1)) * 0.20f;
                        if (score > bestScore) {
                            bestScore = score;
                            bestX = cx;
                            bestZ = cz;
                            bestYaw = candidateYaw;
                        }
                    }
                }
            }
            if (bestScore <= -1.0e8f) return false;
            px = bestX;
            pz = bestZ;
            yaw = bestYaw;
            return true;
        };

        auto propSpan = [](const StaticPropMesh& mesh, int axis) {
            if (axis == 0) return std::max(0.001f, mesh.max.x - mesh.min.x);
            if (axis == 1) return std::max(0.001f, mesh.max.y - mesh.min.y);
            return std::max(0.001f, mesh.max.z - mesh.min.z);
        };
        auto cabinetSize = [](bool tall) {
            return tall
                ? XMFLOAT3{0.66f, 1.34f, 0.40f}
                : XMFLOAT3{0.78f, 0.92f, 0.42f};
        };
        auto pickChairMesh = [&](float seed, bool waitingChair) -> const StaticPropMesh* {
            int order[3] = {0, 1, 2};
            if (waitingChair) {
                order[0] = 1;
                order[1] = 0;
                order[2] = 2;
            } else {
                order[0] = 2;
                order[1] = 0;
                order[2] = 1;
            }
            std::array<const StaticPropMesh*, 3> candidates{};
            int count = 0;
            for (int idx : order) {
                if (!chairPropMeshes_[static_cast<size_t>(idx)].vertices.empty()) {
                    candidates[static_cast<size_t>(count++)] = &chairPropMeshes_[static_cast<size_t>(idx)];
                }
            }
            if (count <= 0) return nullptr;
            int pick = std::clamp(static_cast<int>(seed * static_cast<float>(count)), 0, count - 1);
            return candidates[static_cast<size_t>(pick)];
        };

        auto nearestLampXZ = [&](float px, float pz) {
            float strideX = tileW;
            float strideZ = tileD;
            float originX = ox + tileW * 0.5f;
            float originZ = oz + tileD * 0.5f;
            float cellX = std::floor((px - originX) / std::max(0.001f, strideX) + 0.5f);
            float cellZ = std::floor((pz - originZ) / std::max(0.001f, strideZ) + 0.5f);
            return XMFLOAT2{originX + cellX * strideX, originZ + cellZ * strideZ};
        };

        auto addBakedPropShadow = [&](float px, float pz, float width, float depth, float height, float yaw, float seed) {
            if (gEffectDebugViewer && gDebugSliceEffect != DebugSliceEffect::CeilingLamps) return;
            if (width <= 0.03f || depth <= 0.03f || height <= 0.04f) return;
            XMFLOAT2 lamp = nearestLampXZ(px, pz);
            float dx = px - lamp.x;
            float dz = pz - lamp.y;
            float distXZ = std::sqrt(dx * dx + dz * dz);
            float distFade = std::clamp(distXZ / std::max(0.001f, tileAvg * 3.8f), 0.0f, 1.0f);
            float dirLen = std::max(0.001f, distXZ);
            float dirX = dx / dirLen;
            float dirZ = dz / dirLen;
            float verticalGap = std::max(0.35f, wallH - std::min(height, wallH * 0.86f));
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
        };

        auto addChair = [&](XMFLOAT3 c, float yaw, bool waitingChair) {
            const StaticPropMesh* chairMesh = pickChairMesh(LampHash(c.x + yaw * 0.37f, c.z + (waitingChair ? 9.1f : 17.4f)), waitingChair);
            if (!chairMesh) return false;
            float px = c.x;
            float pz = c.z;
            float placeYaw = yaw;
            if (!reserveRealisticFloorFootprint(px, pz, waitingChair ? 1.02f : 1.05f, waitingChair ? 0.96f : 1.02f,
                    placeYaw, 0.075f, LampHash(c.x + 2.3f, c.z - 1.7f))) return false;
            float scale = waitingChair ? 1.04f : 1.10f;
            float fabricVariant = LampHash(px + placeYaw * 1.7f + (waitingChair ? 0.31f : 0.73f), pz - placeYaw * 0.9f);
            AppendStaticPropMeshGrounded(vertices, indices, *chairMesh, {px, 0.0f, pz}, placeYaw, scale, scale, scale,
                0.0f, -1.0f, &propShadowIndices, fabricVariant);
            addBakedPropShadow(px, pz, waitingChair ? 0.88f : 0.94f, waitingChair ? 0.82f : 0.90f,
                waitingChair ? 0.96f : 1.05f, placeYaw, LampHash(px + 5.1f, pz - 2.8f));
            propLookPoints_.push_back({px, 0.72f, pz});
            return true;
            float seatY = waitingChair ? 0.43f : 0.48f;
            float bodyMat = 8.0f;
            XMFLOAT3 right{std::cos(yaw), 0.0f, -std::sin(yaw)};
            XMFLOAT3 up{0.0f, 1.0f, 0.0f};
            XMFLOAT3 forward{std::sin(yaw), 0.0f, std::cos(yaw)};
            auto off = [&](float x, float y, float z) {
                return Add3(c, OrientedOffset(right, up, forward, x, y, z));
            };
            AddOrientedBox(vertices, indices, {c.x, seatY - 0.033f, c.z}, {0.42f, 0.018f, 0.37f}, yaw, 10.0f);
            AddOrientedBox(vertices, indices, {c.x, seatY + 0.010f, c.z}, {0.36f, 0.048f, 0.31f}, yaw, bodyMat);
            AddOrientedBox(vertices, indices, off(0.0f, seatY + 0.015f, 0.33f), {0.32f, 0.014f, 0.030f}, yaw, 10.0f);
            AddOrientedBox(vertices, indices, off(0.0f, seatY + 0.015f, -0.33f), {0.32f, 0.014f, 0.030f}, yaw, 10.0f);
            AddOrientedBox(vertices, indices, off(-0.37f, seatY + 0.015f, 0.0f), {0.020f, 0.014f, 0.29f}, yaw, 10.0f);
            AddOrientedBox(vertices, indices, off(0.37f, seatY + 0.015f, 0.0f), {0.020f, 0.014f, 0.29f}, yaw, 10.0f);
            AddOrientedBox(vertices, indices, off(0.0f, 0.83f, -0.315f),
                {0.39f, 0.30f, 0.036f}, yaw, bodyMat);
            AddOrientedBox(vertices, indices, off(0.0f, 0.84f, -0.356f), {0.42f, 0.015f, 0.018f}, yaw, 10.0f);
            AddOrientedBox(vertices, indices, off(0.0f, 1.125f, -0.347f), {0.37f, 0.018f, 0.020f}, yaw, 10.0f);
            AddOrientedBox(vertices, indices, off(-0.31f, 0.71f, -0.30f), {0.026f, 0.34f, 0.026f}, yaw, 10.0f);
            AddOrientedBox(vertices, indices, off(0.31f, 0.71f, -0.30f), {0.026f, 0.34f, 0.026f}, yaw, 10.0f);
            AddOrientedBox(vertices, indices, off(-0.12f, 0.88f, -0.347f), {0.015f, 0.24f, 0.014f}, yaw, 10.0f);
            AddOrientedBox(vertices, indices, off(0.12f, 0.88f, -0.347f), {0.015f, 0.24f, 0.014f}, yaw, 10.0f);
            const float legX[2] = {-0.25f, 0.25f};
            const float legZ[2] = {-0.22f, 0.22f};
            for (float lx : legX) {
                for (float lz : legZ) {
                    XMFLOAT3 leg = off(lx, 0.21f, lz);
                    AddOrientedBox(vertices, indices, leg, {0.025f, 0.21f, 0.025f}, yaw, 10.0f);
                }
            }
            AddOrientedBox(vertices, indices, off(0.0f, 0.25f, 0.22f), {0.25f, 0.014f, 0.014f}, yaw, 10.0f);
            AddOrientedBox(vertices, indices, off(0.0f, 0.25f, -0.22f), {0.25f, 0.014f, 0.014f}, yaw, 10.0f);
            AddOrientedBox(vertices, indices, off(-0.25f, 0.25f, 0.0f), {0.014f, 0.014f, 0.22f}, yaw, 10.0f);
            AddOrientedBox(vertices, indices, off(0.25f, 0.25f, 0.0f), {0.014f, 0.014f, 0.22f}, yaw, 10.0f);
            if (waitingChair) {
                AddOrientedBox(vertices, indices, off(-0.43f, 0.63f, 0.02f), {0.035f, 0.035f, 0.30f}, yaw, 10.0f);
                AddOrientedBox(vertices, indices, off(0.43f, 0.63f, 0.02f), {0.035f, 0.035f, 0.30f}, yaw, 10.0f);
                AddOrientedBox(vertices, indices, off(-0.43f, 0.48f, -0.18f), {0.020f, 0.18f, 0.020f}, yaw, 10.0f);
                AddOrientedBox(vertices, indices, off(0.43f, 0.48f, -0.18f), {0.020f, 0.18f, 0.020f}, yaw, 10.0f);
                AddOrientedBox(vertices, indices, off(0.0f, 0.55f, 0.30f), {0.41f, 0.016f, 0.018f}, yaw, 10.0f);
            }
            if (!waitingChair) {
                AddOrientedBox(vertices, indices, {c.x, 0.24f, c.z}, {0.055f, 0.24f, 0.055f}, yaw, 10.0f);
                AddOrientedBox(vertices, indices, {c.x, 0.50f, c.z}, {0.075f, 0.040f, 0.075f}, yaw, 10.0f);
                for (int k = 0; k < 4; ++k) {
                    float armYaw = yaw + k * kPi * 0.5f;
                    XMFLOAT3 foot = Add3({c.x, 0.08f, c.z}, {std::sin(armYaw) * 0.26f, 0.0f, std::cos(armYaw) * 0.26f});
                    AddOrientedBox(vertices, indices, foot, {0.16f, 0.025f, 0.025f}, armYaw, 10.0f);
                    XMFLOAT3 caster = Add3(foot, {std::sin(armYaw) * 0.13f, -0.035f, std::cos(armYaw) * 0.13f});
                    AddOrientedBox(vertices, indices, caster, {0.042f, 0.022f, 0.018f}, armYaw, 10.0f);
                    AddOrientedBox(vertices, indices, Add3(caster, {0.0f, 0.0f, 0.018f}), {0.030f, 0.018f, 0.010f}, armYaw, 5.0f);
                }
                AddOrientedBox(vertices, indices, off(-0.40f, 0.64f, -0.02f), {0.035f, 0.035f, 0.28f}, yaw, 10.0f);
                AddOrientedBox(vertices, indices, off(0.40f, 0.64f, -0.02f), {0.035f, 0.035f, 0.28f}, yaw, 10.0f);
                AddOrientedBox(vertices, indices, off(-0.40f, 0.74f, -0.16f), {0.026f, 0.026f, 0.13f}, yaw, 10.0f);
                AddOrientedBox(vertices, indices, off(0.40f, 0.74f, -0.16f), {0.026f, 0.026f, 0.13f}, yaw, 10.0f);
            }
            propLookPoints_.push_back({c.x, 0.72f, c.z});
            return true;
        };

        auto addStandingTable = [&](float px, float pz, float width, float depth, float yaw, float seed) {
            if (deskPropMesh_.vertices.empty()) return false;
            if (!reserveFloorFootprint(px, pz, width + 0.22f, depth + 0.22f, yaw, 0.085f)) return false;
            float topY = 0.70f + seed * 0.05f;
            float scaleX = depth / propSpan(deskPropMesh_, 0);
            float scaleY = topY + 0.035f;
            float scaleZ = width / propSpan(deskPropMesh_, 2);
            AppendStaticPropMesh(vertices, indices, deskPropMesh_, {px, 0.0f, pz}, yaw + kPi * 0.5f, scaleX, scaleY, scaleZ,
                0.0f, -1.0f, &propShadowIndices);
            addBakedPropShadow(px, pz, width, depth, topY + 0.08f, yaw, seed);
            propLookPoints_.push_back({px, topY, pz});
            return true;
            XMFLOAT3 c{px, 0.0f, pz};
            XMFLOAT3 right{std::cos(yaw), 0.0f, -std::sin(yaw)};
            XMFLOAT3 up{0.0f, 1.0f, 0.0f};
            XMFLOAT3 forward{std::sin(yaw), 0.0f, std::cos(yaw)};
            auto off = [&](float x, float y, float z) {
                return Add3(c, OrientedOffset(right, up, forward, x, y, z));
            };
            float topMat = seed < 0.42f ? 8.0f : 10.0f;
            AddOrientedBox(vertices, indices, off(0.0f, topY, 0.0f), {width * 0.5f, 0.042f, depth * 0.5f}, yaw, topMat);
            AddOrientedBox(vertices, indices, off(0.0f, topY - 0.055f, depth * 0.5f - 0.035f), {width * 0.48f, 0.030f, 0.026f}, yaw, 10.0f);
            AddOrientedBox(vertices, indices, off(0.0f, topY - 0.055f, -depth * 0.5f + 0.035f), {width * 0.48f, 0.030f, 0.026f}, yaw, 10.0f);
            AddOrientedBox(vertices, indices, off(width * 0.5f - 0.035f, topY - 0.055f, 0.0f), {0.026f, 0.030f, depth * 0.44f}, yaw, 10.0f);
            AddOrientedBox(vertices, indices, off(-width * 0.5f + 0.035f, topY - 0.055f, 0.0f), {0.026f, 0.030f, depth * 0.44f}, yaw, 10.0f);
            const float lx[2] = {-0.42f, 0.42f};
            const float lz[2] = {-0.42f, 0.42f};
            for (float sx : lx) {
                for (float sz : lz) {
                    AddOrientedBox(vertices, indices, off(sx * width, topY * 0.50f, sz * depth), {0.035f, topY * 0.46f, 0.035f}, yaw, 10.0f);
                }
            }
            AddOrientedBox(vertices, indices, off(0.0f, 0.36f, depth * 0.38f), {width * 0.37f, 0.018f, 0.018f}, yaw, 10.0f);
            AddOrientedBox(vertices, indices, off(0.0f, 0.36f, -depth * 0.38f), {width * 0.37f, 0.018f, 0.018f}, yaw, 10.0f);
            if (seed > 0.52f) {
                AddOrientedBox(vertices, indices, off((seed - 0.5f) * width * 0.45f, topY + 0.060f, (seed - 0.5f) * depth * -0.25f),
                    {0.20f, 0.018f, 0.145f}, yaw + 0.18f, 9.0f);
                AddOrientedBox(vertices, indices, off(width * 0.20f, topY + 0.075f, depth * -0.18f),
                    {0.045f, 0.055f, 0.045f}, yaw, 5.0f);
            }
            propLookPoints_.push_back({px, topY, pz});
            return true;
        };

        auto addSideTable = [&](float px, float pz, float width, float depth, float yaw, float seed) {
            if (deskPropMesh_.vertices.empty()) return false;
            if (!reserveFloorFootprint(px, pz, width + 0.24f, depth * 0.72f + 0.36f, yaw, 0.085f)) return false;
            float height = 0.68f + seed * 0.10f;
            float scaleX = depth / propSpan(deskPropMesh_, 0);
            float scaleY = height + 0.030f;
            float scaleZ = width / propSpan(deskPropMesh_, 2);
            AppendStaticPropMesh(vertices, indices, deskPropMesh_, {px, 0.0f, pz}, yaw + kPi * 0.5f, scaleX, scaleY, scaleZ,
                0.0f, -1.0f, &propShadowIndices);
            addBakedPropShadow(px, pz, width, depth, height + 0.06f, yaw, seed);
            propLookPoints_.push_back({px, height * 0.72f, pz});
            return true;
            XMFLOAT3 c{px, 0.0f, pz};
            XMFLOAT3 right{std::cos(yaw), 0.0f, -std::sin(yaw)};
            XMFLOAT3 up{0.0f, 1.0f, 0.0f};
            XMFLOAT3 forward{std::sin(yaw), 0.0f, std::cos(yaw)};
            auto off = [&](float x, float y, float z) {
                return Add3(c, OrientedOffset(right, up, forward, x, y, z));
            };
            float bodyMat = seed < 0.46f ? 8.0f : 10.0f;
            float trimMat = 10.0f;
            AddOrientedBox(vertices, indices, off(0.0f, height * 0.50f, 0.0f), {width * 0.50f, height * 0.50f, depth * 0.50f}, yaw, bodyMat);
            AddOrientedBox(vertices, indices, off(0.0f, height + 0.030f, 0.0f), {width * 0.54f, 0.035f, depth * 0.54f}, yaw, trimMat);
            AddOrientedBox(vertices, indices, off(0.0f, 0.045f, 0.0f), {width * 0.53f, 0.040f, depth * 0.52f}, yaw, trimMat);
            AddOrientedBox(vertices, indices, off(-width * 0.52f, height * 0.52f, 0.0f), {0.026f, height * 0.46f, depth * 0.50f}, yaw, trimMat);
            AddOrientedBox(vertices, indices, off(width * 0.52f, height * 0.52f, 0.0f), {0.026f, height * 0.46f, depth * 0.50f}, yaw, trimMat);
            AddOrientedBox(vertices, indices, off(0.0f, height * 0.52f, -depth * 0.535f), {width * 0.47f, height * 0.39f, 0.014f}, yaw, 5.0f);
            AddOrientedBox(vertices, indices, off(0.0f, height * 0.52f, -depth * 0.566f), {0.010f, height * 0.35f, 0.016f}, yaw, trimMat);
            for (int door = -1; door <= 1; door += 2) {
                float dx = door * width * 0.245f;
                AddOrientedBox(vertices, indices, off(dx, height * 0.52f, -depth * 0.585f), {width * 0.215f, height * 0.34f, 0.012f}, yaw, bodyMat);
                AddOrientedBox(vertices, indices, off(dx - door * width * 0.060f, height * 0.52f, -depth * 0.612f), {0.012f, height * 0.115f, 0.014f}, yaw, trimMat);
                if (seed > 0.62f && door == (seed > 0.80f ? 1 : -1)) {
                    float doorYaw = yaw + door * (0.42f + seed * 0.22f);
                    XMFLOAT3 openCenter = off(dx + door * width * 0.075f, height * 0.50f, -depth * 0.66f);
                    AddOrientedBox(vertices, indices, openCenter, {width * 0.19f, height * 0.31f, 0.012f}, doorYaw, bodyMat);
                }
            }
            for (int legX = -1; legX <= 1; legX += 2) {
                for (int legZ = -1; legZ <= 1; legZ += 2) {
                    AddOrientedBox(vertices, indices, off(legX * width * 0.39f, 0.085f, legZ * depth * 0.36f), {0.026f, 0.085f, 0.026f}, yaw, trimMat);
                }
            }
            if (seed > 0.35f) {
                AddOrientedBox(vertices, indices, off(-width * 0.18f, height + 0.080f, -depth * 0.16f), {width * 0.22f, 0.018f, 0.125f}, yaw + 0.25f, 9.0f);
                AddOrientedBox(vertices, indices, off(width * 0.22f, height + 0.090f, depth * 0.10f), {0.045f, 0.055f, 0.045f}, yaw, 5.0f);
            }
            propLookPoints_.push_back({px, height * 0.72f, pz});
            return true;
        };

        auto addTippedChair = [&](float px, float pz, float yaw, bool waitingChair, float seed) {
            const StaticPropMesh* chairMesh = pickChairMesh(seed, waitingChair);
            if (!chairMesh) return false;
            if (!reserveRealisticFloorFootprint(px, pz, waitingChair ? 1.10f : 1.18f, waitingChair ? 1.26f : 1.34f,
                    yaw, 0.075f, seed)) return false;
            float scale = waitingChair ? 1.02f : 1.08f;
            float pitch = seed < 0.5f ? kPi * 0.5f : -kPi * 0.5f;
            AppendStaticPropMeshGrounded(vertices, indices, *chairMesh, {px, 0.0f, pz}, yaw, scale, scale, scale, pitch,
                -1.0f, &propShadowIndices, seed);
            addBakedPropShadow(px, pz, waitingChair ? 1.10f : 1.18f, waitingChair ? 1.26f : 1.34f,
                0.46f, yaw, seed);
            propLookPoints_.push_back({px, 0.22f, pz});
            return true;
            XMFLOAT3 c{px, 0.0f, pz};
            XMFLOAT3 right{std::cos(yaw), 0.0f, -std::sin(yaw)};
            XMFLOAT3 up{0.0f, 1.0f, 0.0f};
            XMFLOAT3 forward{std::sin(yaw), 0.0f, std::cos(yaw)};
            auto off = [&](float x, float y, float z) {
                return Add3(c, OrientedOffset(right, up, forward, x, y, z));
            };
            AddOrientedBox(vertices, indices, off(0.0f, 0.12f, 0.02f), {0.36f, 0.030f, 0.31f}, yaw, 8.0f);
            AddOrientedBox(vertices, indices, off(0.0f, 0.16f, -0.44f), {0.38f, 0.030f, 0.25f}, yaw + 0.08f, 8.0f);
            AddOrientedBox(vertices, indices, off(-0.31f, 0.16f, -0.23f), {0.028f, 0.028f, 0.34f}, yaw, 10.0f);
            AddOrientedBox(vertices, indices, off(0.31f, 0.16f, -0.23f), {0.028f, 0.028f, 0.34f}, yaw, 10.0f);
            AddOrientedBox(vertices, indices, off(-0.22f, 0.11f, 0.28f), {0.025f, 0.11f, 0.025f}, yaw, 10.0f);
            AddOrientedBox(vertices, indices, off(0.24f, 0.10f, 0.25f), {0.025f, 0.10f, 0.025f}, yaw, 10.0f);
            AddOrientedBox(vertices, indices, off((seed - 0.5f) * 0.36f, 0.055f, 0.0f), {0.18f, 0.020f, 0.020f}, yaw + 0.45f, 10.0f);
            if (!waitingChair) {
                AddOrientedBox(vertices, indices, off(0.02f, 0.15f, 0.02f), {0.050f, 0.15f, 0.050f}, yaw, 10.0f);
                AddOrientedBox(vertices, indices, off(-0.38f, 0.065f, 0.03f), {0.12f, 0.020f, 0.018f}, yaw + 0.20f, 10.0f);
                AddOrientedBox(vertices, indices, off(0.38f, 0.065f, -0.05f), {0.12f, 0.020f, 0.018f}, yaw - 0.18f, 10.0f);
            }
            propLookPoints_.push_back({px, 0.24f, pz});
            return true;
        };

        auto addTrashBin = [&](float px, float pz, float yaw, bool tipped, float seed) {
            if (trashBinPropMesh_.vertices.empty()) return false;
            seed = std::clamp(seed, 0.0f, 1.0f);
            constexpr float targetHeight = 0.48f;
            float scale = targetHeight / propSpan(trashBinPropMesh_, 1);
            float diameter = std::max(propSpan(trashBinPropMesh_, 0), propSpan(trashBinPropMesh_, 2)) * scale;
            float footprintW = diameter + 0.08f;
            float footprintD = tipped ? targetHeight + 0.16f : footprintW;
            if (!reserveRealisticFloorFootprint(px, pz, footprintW, footprintD, yaw, tipped ? 0.065f : 0.055f, seed)) return false;
            float pitch = tipped ? (seed < 0.5f ? kPi * 0.5f : -kPi * 0.5f) : 0.0f;
            AppendStaticPropMeshGrounded(vertices, indices, trashBinPropMesh_, {px, 0.0f, pz}, yaw,
                scale, scale, scale, pitch, -1.0f, &propShadowIndices);
            addBakedPropShadow(px, pz, footprintW, footprintD, tipped ? diameter * 0.62f : targetHeight, yaw, seed);
            propLookPoints_.push_back({px, tipped ? diameter * 0.45f : targetHeight * 0.55f, pz});
            return true;
        };

        auto addDeskLampOnSurface = [&](float px, float pz, float surfaceY, float tableYaw,
                                         float tableWidth, float tableDepth, float seed) {
            if (deskLampPropMesh_.vertices.empty()) return false;
            float c = std::cos(tableYaw);
            float s = std::sin(tableYaw);
            XMFLOAT3 right{c, 0.0f, -s};
            XMFLOAT3 forward{s, 0.0f, c};
            float ox = (seed - 0.5f) * tableWidth * 0.34f;
            float oz = (LampHash(px + seed * 7.1f, pz - seed * 5.3f) - 0.5f) * tableDepth * 0.30f;
            XMFLOAT3 p = Add3({px, 0.0f, pz}, Add3(Scale3(right, ox), Scale3(forward, oz)));
            constexpr float targetHeight = 0.42f;
            float scale = targetHeight / propSpan(deskLampPropMesh_, 1);
            AppendStaticPropMeshGrounded(vertices, indices, deskLampPropMesh_, {p.x, surfaceY + 0.012f, p.z},
                tableYaw + (seed - 0.5f) * 0.85f, scale, scale, scale, 0.0f, -1.0f, &propShadowIndices);
            propLookPoints_.push_back({p.x, surfaceY + targetHeight * 0.62f, p.z});
            return true;
        };

        auto addCassetteAt = [&](float px, float pz, float yaw, float floorY, float seed) {
            if (cassettePropMesh_.vertices.empty()) return false;
            float width = 0.100f;
            float depth = 0.064f;
            if (!floorFootprintClear(px, pz, width, depth, yaw, 0.024f)) return false;
            float scale = width / propSpan(cassettePropMesh_, 0);
            AppendStaticPropMeshGrounded(vertices, indices, cassettePropMesh_, {px, floorY + 0.002f, pz},
                yaw + (seed - 0.5f) * 0.38f, scale, scale, scale, 0.0f, -1.0f, &propShadowIndices);
            return true;
        };

        auto addDebugPropInspectionModel = [&]() {
            if (!gEffectDebugViewer || gDebugSliceEffect != DebugSliceEffect::Props) return;
            int propIndex = WrapDebugPropIndex(gDebugPropIndex);
            const StaticPropMesh* mesh = nullptr;
            switch (propIndex) {
            case 0: mesh = &chairPropMeshes_[0]; break;
            case 1: mesh = &chairPropMeshes_[1]; break;
            case 2:
            case 3: mesh = &chairPropMeshes_[2]; break;
            case 4: mesh = &cabinetPropMesh_; break;
            case 5: mesh = &deskPropMesh_; break;
            case 6:
            case 7: mesh = &trashBinPropMesh_; break;
            case 8: mesh = &deskLampPropMesh_; break;
            case 9: mesh = &cassettePropMesh_; break;
            case 10: mesh = &airVentPropMesh_; break;
            case 11: mesh = &exitSignPropMesh_; break;
            case 12: mesh = &ceilingLampPropMeshes_[0]; break;
            case 13: mesh = &ceilingLampPropMeshes_[1]; break;
            case 14: mesh = &ceilingLampPropMeshes_[2]; break;
            case 15: mesh = &ceilingLampPropMeshes_[3]; break;
            default: break;
            }

            int tiles = std::clamp(gDebugSliceTiles, 1, 5);
            float centerX = ox + (1.0f + static_cast<float>(tiles) * 0.5f) * tileW;
            float centerZ = oz + (1.0f + static_cast<float>(tiles) * 0.5f) * tileD;
            float targetMax = 1.22f;
            float yaw = kPi;
            float pitch = 0.0f;
            switch (propIndex) {
            case 3:
                pitch = kPi * 0.5f;
                targetMax = 1.12f;
                break;
            case 6:
                targetMax = 0.58f;
                break;
            case 7:
                pitch = kPi * 0.5f;
                targetMax = 0.62f;
                break;
            case 4:
                targetMax = 1.44f;
                break;
            case 5:
                targetMax = 1.62f;
                yaw = kPi * 0.5f;
                break;
            case 8:
                targetMax = 0.56f;
                break;
            case 9:
                targetMax = 0.58f;
                break;
            case 10:
                targetMax = 0.86f;
                break;
            case 11:
                targetMax = 1.18f;
                break;
            case 12:
            case 13:
            case 14:
            case 15:
                targetMax = 1.36f;
                yaw = kPi * 0.5f;
                break;
            default:
                break;
            }

            if (mesh && !mesh->vertices.empty()) {
                float spanX = propSpan(*mesh, 0);
                float spanY = propSpan(*mesh, 1);
                float spanZ = propSpan(*mesh, 2);
                float spanMax = std::max(spanX, std::max(spanY, spanZ));
                float scale = std::clamp(targetMax / std::max(0.001f, spanMax), 0.035f, 12.0f);
                bool wallMounted = propIndex == 10 || propIndex == 11;
                bool suspendedLamp = propIndex >= 12 && propIndex <= 15;
                if (wallMounted || suspendedLamp) {
                    float c = std::cos(yaw);
                    float s = std::sin(yaw);
                    XMFLOAT3 right{c, 0.0f, -s};
                    XMFLOAT3 up{0.0f, 1.0f, 0.0f};
                    XMFLOAT3 forward{s, 0.0f, c};
                    XMFLOAT3 meshCenter{
                        (mesh->min.x + mesh->max.x) * 0.5f,
                        (mesh->min.y + mesh->max.y) * 0.5f,
                        (mesh->min.z + mesh->max.z) * 0.5f
                    };
                    XMFLOAT3 desiredCenter{
                        centerX,
                        wallMounted ? settings_.wallHeightMeters * 0.54f : 1.08f,
                        wallMounted ? centerZ - tileD * 0.34f : centerZ
                    };
                    XMFLOAT3 centeredOffset = Add3(Scale3(right, meshCenter.x * scale),
                        Add3(Scale3(up, meshCenter.y * scale), Scale3(forward, meshCenter.z * scale)));
                    XMFLOAT3 origin = Add3(desiredCenter, Scale3(centeredOffset, -1.0f));
                    AppendStaticPropMesh(vertices, indices, *mesh, origin, yaw, scale, scale, scale,
                        0.0f, -1.0f, &propShadowIndices);
                    propLookPoints_.push_back(desiredCenter);
                    return;
                }
                AppendStaticPropMeshGrounded(vertices, indices, *mesh, {centerX, 0.0f, centerZ},
                    yaw, scale, scale, scale, pitch, -1.0f, &propShadowIndices);
                float lookY = std::clamp((mesh->max.y - mesh->min.y) * scale * 0.55f, 0.16f, 1.15f);
                propLookPoints_.push_back({centerX, lookY, centerZ});
                return;
            }

            if (propIndex == 10) {
                float panelY = settings_.wallHeightMeters * 0.54f;
                float panelZ = centerZ - tileD * 0.34f;
                AddOrientedBox(vertices, indices, {centerX, panelY, panelZ}, {0.52f, 0.18f, 0.018f}, kPi, 10.0f);
                AddOrientedBox(vertices, indices, {centerX, panelY, panelZ - 0.026f}, {0.42f, 0.11f, 0.010f}, kPi, 5.0f);
                for (int slot = -3; slot <= 3; ++slot) {
                    AddOrientedBox(vertices, indices,
                        {centerX, panelY + static_cast<float>(slot) * 0.030f, panelZ - 0.044f},
                        {0.36f, 0.0048f, 0.006f}, kPi, 8.0f);
                }
                propLookPoints_.push_back({centerX, panelY, panelZ});
                return;
            }

            AddOrientedBox(vertices, indices, {centerX, 0.18f, centerZ}, {0.42f, 0.18f, 0.42f}, 0.0f, 5.0f);
            propLookPoints_.push_back({centerX, 0.18f, centerZ});
        };

        addDebugPropInspectionModel();

        auto addRoomClutterGroup = [&](Tile t, int groupIndex, uint32_t scatterSeed) {
            if (!IsRoomLike(t)) return false;
            XMFLOAT3 c = maze_.WorldCenter(t, 0.0f);
            float yaw = Rand01(groupIndex, 311, scatterSeed) * kPi * 2.0f;
            float kind = Rand01(groupIndex, 313, scatterSeed);
            float px = c.x + (Rand01(groupIndex, 317, scatterSeed) - 0.5f) * tileW * 0.44f;
            float pz = c.z + (Rand01(groupIndex, 331, scatterSeed) - 0.5f) * tileD * 0.44f;
            if (kind < 0.38f) {
                float w = 1.10f + Rand01(groupIndex, 337, scatterSeed) * 0.42f;
                float d = 0.68f + Rand01(groupIndex, 347, scatterSeed) * 0.28f;
                float chairRing = std::max(w, d) * 0.5f + 0.74f;
                if (!longFloorFootprintClear(px, pz, chairRing * 2.0f + 1.08f, chairRing * 2.0f + 1.02f, yaw, 0.070f)) return false;
                float tableSeed = Rand01(groupIndex, 349, scatterSeed);
                if (!addStandingTable(px, pz, w, d, yaw, tableSeed)) return false;
                if (Rand01(groupIndex, 351, scatterSeed) < 0.64f) {
                    addDeskLampOnSurface(px, pz, 0.745f + tableSeed * 0.05f, yaw, w, d, Rand01(groupIndex, 352, scatterSeed));
                }
                int chairs = 2 + static_cast<int>(Rand01(groupIndex, 353, scatterSeed) * 3.0f);
                int placedChairs = 0;
                for (int i = 0; i < chairs; ++i) {
                    float a = yaw + static_cast<float>(i) * kPi * 2.0f / static_cast<float>(chairs) + RandRange(-0.22f, 0.22f);
                    float radius = chairRing + Rand01(groupIndex * 7 + i, 359, scatterSeed) * 0.18f;
                    XMFLOAT3 chairPos{px + std::sin(a) * radius, 0.0f, pz + std::cos(a) * radius};
                    float chairYaw = a + kPi + RandRange(-0.32f, 0.32f);
                    bool placed = false;
                    if (Rand01(groupIndex * 7 + i, 367, scatterSeed) < 0.22f) {
                        placed = addTippedChair(chairPos.x, chairPos.z, chairYaw, false, Rand01(groupIndex * 7 + i, 371, scatterSeed));
                    } else {
                        placed = addChair(chairPos, chairYaw, Rand01(groupIndex * 7 + i, 373, scatterSeed) < 0.42f);
                    }
                    if (placed) ++placedChairs;
                }
                if (Rand01(groupIndex, 375, scatterSeed) < 0.72f) {
                    float binA = yaw + Rand01(groupIndex, 377, scatterSeed) * kPi * 2.0f;
                    float binR = chairRing * (0.42f + Rand01(groupIndex, 381, scatterSeed) * 0.28f);
                    addTrashBin(px + std::sin(binA) * binR, pz + std::cos(binA) * binR,
                        binA + kPi * 0.5f, Rand01(groupIndex, 385, scatterSeed) < 0.38f,
                        Rand01(groupIndex, 387, scatterSeed));
                }
                return true;
            }
            if (kind < 0.62f) {
                float sideW = 1.12f + Rand01(groupIndex, 379, scatterSeed) * 0.44f;
                float sideD = 0.66f + Rand01(groupIndex, 383, scatterSeed) * 0.30f;
                float chairRadius = std::max(sideW, sideD) * 0.5f + 0.76f;
                if (!longFloorFootprintClear(px, pz, chairRadius * 2.0f + 0.80f, chairRadius * 1.55f, yaw, 0.070f)) return false;
                float sideSeed = Rand01(groupIndex, 389, scatterSeed);
                if (!addSideTable(px, pz, sideW, sideD, yaw, sideSeed)) return false;
                if (Rand01(groupIndex, 391, scatterSeed) < 0.58f) {
                    addDeskLampOnSurface(px, pz, 0.720f + sideSeed * 0.10f, yaw, sideW, sideD, Rand01(groupIndex, 393, scatterSeed));
                }
                addTippedChair(px + std::sin(yaw + 0.8f) * chairRadius, pz + std::cos(yaw + 0.8f) * chairRadius,
                    yaw + kPi * 0.72f, Rand01(groupIndex, 397, scatterSeed) < 0.5f, Rand01(groupIndex, 401, scatterSeed));
                float binA = yaw - 0.95f + (Rand01(groupIndex, 403, scatterSeed) - 0.5f) * 0.58f;
                addTrashBin(px + std::sin(binA) * chairRadius * 0.72f, pz + std::cos(binA) * chairRadius * 0.72f,
                    binA + kPi * 0.5f, Rand01(groupIndex, 405, scatterSeed) < 0.52f,
                    Rand01(groupIndex, 407, scatterSeed));
                return true;
            }
            if (kind < 0.82f && !trashBinPropMesh_.vertices.empty()) {
                float clusterRadius = 0.70f + Rand01(groupIndex, 409, scatterSeed) * 0.36f;
                if (!longFloorFootprintClear(px, pz, clusterRadius * 2.0f + 1.10f, clusterRadius * 2.0f + 1.00f, yaw, 0.070f)) return false;
                float furnitureYaw = yaw + (Rand01(groupIndex, 411, scatterSeed) - 0.5f) * 0.85f;
                float furnitureA = yaw + kPi * (0.40f + Rand01(groupIndex, 413, scatterSeed) * 0.40f);
                if (Rand01(groupIndex, 415, scatterSeed) < 0.45f) {
                    float tableW = 0.82f + Rand01(groupIndex, 417, scatterSeed) * 0.26f;
                    float tableD = 0.52f + Rand01(groupIndex, 419, scatterSeed) * 0.20f;
                    float smallTableSeed = Rand01(groupIndex, 421, scatterSeed);
                    float tableX = px + std::sin(furnitureA) * 0.38f;
                    float tableZ = pz + std::cos(furnitureA) * 0.38f;
                    if (addSideTable(tableX, tableZ, tableW, tableD, furnitureYaw, smallTableSeed) &&
                        Rand01(groupIndex, 422, scatterSeed) < 0.46f) {
                        addDeskLampOnSurface(tableX, tableZ, 0.720f + smallTableSeed * 0.10f,
                            furnitureYaw, tableW, tableD, Rand01(groupIndex, 424, scatterSeed));
                    }
                } else if (Rand01(groupIndex, 423, scatterSeed) < 0.55f) {
                    addChair({px + std::sin(furnitureA) * 0.48f, 0.0f, pz + std::cos(furnitureA) * 0.48f},
                        furnitureYaw + kPi, Rand01(groupIndex, 425, scatterSeed) < 0.5f);
                }
                int bins = 2 + static_cast<int>(Rand01(groupIndex, 427, scatterSeed) * 3.0f);
                int placedBins = 0;
                for (int i = 0; i < bins; ++i) {
                    float a = yaw + static_cast<float>(i) * kPi * 2.0f / static_cast<float>(bins)
                        + (Rand01(groupIndex * 17 + i, 429, scatterSeed) - 0.5f) * 0.72f;
                    float radius = clusterRadius * (0.46f + Rand01(groupIndex * 17 + i, 431, scatterSeed) * 0.52f);
                    bool tipped = i == 0
                        ? Rand01(groupIndex * 17 + i, 433, scatterSeed) < 0.70f
                        : Rand01(groupIndex * 17 + i, 433, scatterSeed) < 0.46f;
                    if (addTrashBin(px + std::sin(a) * radius, pz + std::cos(a) * radius,
                            a + (Rand01(groupIndex * 17 + i, 435, scatterSeed) - 0.5f) * 0.80f,
                            tipped, Rand01(groupIndex * 17 + i, 437, scatterSeed))) {
                        ++placedBins;
                    }
                }
                if (placedBins == 0 && addTrashBin(px, pz, yaw, Rand01(groupIndex, 439, scatterSeed) < 0.55f,
                        Rand01(groupIndex, 441, scatterSeed))) {
                    ++placedBins;
                }
                return placedBins > 0;
            }
            int chairs = 3 + static_cast<int>(Rand01(groupIndex, 409, scatterSeed) * 4.0f);
            float ring = 0.88f + Rand01(groupIndex, 407, scatterSeed) * 0.28f;
            if (!longFloorFootprintClear(px, pz, ring * 2.0f + 1.02f, ring * 2.0f + 1.02f, yaw, 0.070f)) return false;
            int placedChairs = 0;
            for (int i = 0; i < chairs; ++i) {
                float a = yaw + static_cast<float>(i) * kPi * 2.0f / static_cast<float>(chairs);
                float radius = ring + Rand01(groupIndex * 11 + i, 419, scatterSeed) * 0.20f;
                float cx = px + std::sin(a) * radius;
                float cz = pz + std::cos(a) * radius;
                float chairYaw = a + RandRange(-0.50f, 0.50f);
                bool placed = false;
                if (Rand01(groupIndex * 11 + i, 421, scatterSeed) < 0.34f) {
                    placed = addTippedChair(cx, cz, chairYaw, true, Rand01(groupIndex * 11 + i, 431, scatterSeed));
                } else {
                    placed = addChair({cx, 0.0f, cz}, chairYaw, true);
                }
                if (placed) ++placedChairs;
            }
            if (Rand01(groupIndex, 443, scatterSeed) < 0.58f) {
                addTrashBin(px, pz, yaw + Rand01(groupIndex, 445, scatterSeed) * kPi,
                    Rand01(groupIndex, 447, scatterSeed) < 0.42f, Rand01(groupIndex, 449, scatterSeed));
            }
            return placedChairs > 0;
        };

        constexpr float kA4PaperShortMeters = 0.210f;
        constexpr float kA4PaperLongMeters = 0.297f;
        auto loosePaperMaterial = [&](float seed, float variantSeed) {
            if (seed < 0.50f) {
                int slot = std::clamp(static_cast<int>(variantSeed * static_cast<float>(kRandomLoosePageAtlasSlots)), 0, kRandomLoosePageAtlasSlots - 1);
                float encodedSlot = (static_cast<float>(slot) + 0.5f) / static_cast<float>(kRandomLoosePageAtlasSlots);
                return static_cast<float>(kRandomLoosePageMaterial) + encodedSlot;
            }
            return 9.54f + std::min(0.34f, variantSeed * 0.34f);
        };

        auto addPaperAt = [&](float px, float pz, float yaw, float lift, float material) {
            if (!floorFootprintClear(px, pz, kA4PaperShortMeters, kA4PaperLongMeters, yaw)) return false;
            float paperY = std::clamp(lift, 0.0025f, 0.0105f);
            AddFloorCard(vertices, indices, {px, 0.0f, pz}, kA4PaperShortMeters, kA4PaperLongMeters, yaw, paperY, material);
            return true;
        };

        auto addLoosePapers = [&](Tile t, int count, bool hallwaySpill) {
            XMFLOAT3 c = maze_.WorldCenter(t, 0.0f);
            float spreadX = hallwaySpill ? tileW * 1.55f : tileW * 0.82f;
            float spreadZ = hallwaySpill ? tileD * 1.55f : tileD * 0.82f;
            for (int p = 0; p < count; ++p) {
                float px = c.x + (tileHash(t.x, t.y, 5.0f + p * 1.71f) - 0.5f) * spreadX;
                float pz = c.z + (tileHash(t.x, t.y, 7.0f + p * 1.93f) - 0.5f) * spreadZ;
                float yaw = tileHash(t.x, t.y, 9.0f + p * 2.11f) * kPi * 2.0f;
                float lift = 0.0030f + p * 0.00012f + tileHash(t.x, t.y, 17.0f + p) * 0.0014f;
                float material = loosePaperMaterial(tileHash(t.x, t.y, 23.0f + p * 2.19f), tileHash(t.x, t.y, 25.0f + p * 2.31f));
                if (addPaperAt(px, pz, yaw, lift, material) &&
                    tileHash(t.x, t.y, 31.0f + p * 2.37f) < 0.010f) {
                    addCassetteAt(px, pz, yaw, lift + 0.003f, tileHash(t.x, t.y, 37.0f + p));
                }
            }
            propLookPoints_.push_back({c.x, 0.18f, c.z});
        };

        auto addWallVent = [&](Tile t, int side, float seed) {
            XMFLOAT3 c = maze_.WorldCenter(t, 0.0f);
            float wallSpan = (side == 0 || side == 1) ? tileW : tileD;
            if (wallSpan < 1.38f || wallH < 1.12f) return;
            float lateralLimit = std::max(0.0f, wallSpan * 0.5f - 0.68f);
            float lateral = (tileHash(t.x, t.y, 41.0f + seed) - 0.5f) * lateralLimit * 2.0f;
            bool lowVent = tileHash(t.x, t.y, 42.2f + seed) < 0.34f;
            float ventW = lowVent ? 0.68f : 0.92f;
            float ventH = lowVent ? 0.24f : 0.34f;
            constexpr float kLowVentFloorGap = 0.085f;
            constexpr float kHighVentCeilingGap = 0.095f;
            constexpr float kVentVerticalClearance = 0.055f;
            float minVentY = ventH * 0.5f + kVentVerticalClearance;
            float maxVentY = std::max(minVentY, wallH - ventH * 0.5f - kVentVerticalClearance);
            float yCenter = lowVent
                ? kLowVentFloorGap + ventH * 0.5f
                : wallH - kHighVentCeilingGap - ventH * 0.5f;
            yCenter = std::clamp(yCenter, minVentY, maxVentY);
            float yaw = 0.0f;
            XMFLOAT3 center{c.x, yCenter, c.z};
            if (side == 0) {
                yaw = 0.0f;
                center = {c.x + lateral, yCenter, c.z - tileD * 0.5f + 0.018f};
            } else if (side == 1) {
                yaw = kPi;
                center = {c.x + lateral, yCenter, c.z + tileD * 0.5f - 0.018f};
            } else if (side == 2) {
                yaw = kPi * 0.5f;
                center = {c.x - tileW * 0.5f + 0.018f, yCenter, c.z + lateral};
            } else {
                yaw = -kPi * 0.5f;
                center = {c.x + tileW * 0.5f - 0.018f, yCenter, c.z + lateral};
            }
            if (!airVentPropMesh_.vertices.empty()) {
                float spanX = std::max(0.001f, propSpan(airVentPropMesh_, 0));
                float spanY = std::max(0.001f, propSpan(airVentPropMesh_, 1));
                float scale = std::min(ventW / spanX, ventH / spanY);
                scale = std::clamp(scale, 0.05f, 8.0f);
                if (AppendStaticPropMesh(vertices, indices, airVentPropMesh_, center, yaw, scale, scale, scale)) {
                    XMFLOAT3 right{std::cos(yaw), 0.0f, -std::sin(yaw)};
                    XMFLOAT3 up{0.0f, 1.0f, 0.0f};
                    XMFLOAT3 forward{std::sin(yaw), 0.0f, std::cos(yaw)};
                    XMFLOAT3 emitterPos = Add3(center, OrientedOffset(right, up, forward, 0.0f, lowVent ? 0.02f : -0.02f, 0.090f));
                    steamEmitters_.push_back({emitterPos, forward, tileHash(t.x, t.y, 45.0f + seed) * 5.0f, false});
                    propLookPoints_.push_back(center);
                    return;
                }
            }
            XMFLOAT3 right{std::cos(yaw), 0.0f, -std::sin(yaw)};
            XMFLOAT3 up{0.0f, 1.0f, 0.0f};
            XMFLOAT3 forward{std::sin(yaw), 0.0f, std::cos(yaw)};
            auto voff = [&](float x, float y, float z) {
                return Add3(center, OrientedOffset(right, up, forward, x, y, z));
            };
            AddOrientedBox(vertices, indices, voff(0.0f, 0.0f, 0.010f), {ventW * 0.50f, ventH * 0.50f, 0.010f}, yaw, 8.0f);
            AddOrientedBox(vertices, indices, voff(0.0f, 0.0f, 0.022f), {ventW * 0.38f, ventH * 0.31f, 0.006f}, yaw, 5.0f);
            AddOrientedBox(vertices, indices, voff(-ventW * 0.47f, 0.0f, 0.027f), {0.014f, ventH * 0.43f, 0.010f}, yaw, 8.0f);
            AddOrientedBox(vertices, indices, voff(ventW * 0.47f, 0.0f, 0.027f), {0.014f, ventH * 0.43f, 0.010f}, yaw, 8.0f);
            AddOrientedBox(vertices, indices, voff(0.0f, -ventH * 0.43f, 0.027f), {ventW * 0.45f, 0.012f, 0.010f}, yaw, 8.0f);
            AddOrientedBox(vertices, indices, voff(0.0f, ventH * 0.43f, 0.027f), {ventW * 0.45f, 0.012f, 0.010f}, yaw, 8.0f);
            for (int s = -3; s <= 3; ++s) {
                float yOff = static_cast<float>(s) * ventH * 0.115f;
                float stagger = (s & 1) ? 0.006f : -0.004f;
                AddOrientedBox(vertices, indices, voff(stagger, yOff, 0.034f + static_cast<float>(s + 3) * 0.0008f),
                    {ventW * 0.36f, 0.006f, 0.007f}, yaw, 8.0f);
            }
            const XMFLOAT2 screws[] = {{-0.43f, -0.37f}, {0.43f, -0.37f}, {-0.43f, 0.37f}, {0.43f, 0.37f}};
            for (const XMFLOAT2& screw : screws) {
                AddOrientedBox(vertices, indices, voff(screw.x * ventW, screw.y * ventH, 0.039f), {0.014f, 0.014f, 0.005f}, yaw, 10.0f);
            }
            steamEmitters_.push_back({voff(0.0f, lowVent ? 0.02f : -0.02f, 0.082f), forward, tileHash(t.x, t.y, 45.0f + seed) * 5.0f, false});
            propLookPoints_.push_back(center);
        };

        auto addMetalCabinet = [&](float px, float pz, float yaw, float seed, bool tall) {
            XMFLOAT3 cab = cabinetSize(tall);
            float width = cab.x;
            float height = cab.y;
            float depth = cab.z;
            if (cabinetPropMesh_.vertices.empty()) return false;
            if (!reserveFloorFootprint(px, pz, width, depth, yaw, 0.090f)) return false;
            float scaleX = width / propSpan(cabinetPropMesh_, 0);
            float scaleY = height / propSpan(cabinetPropMesh_, 1);
            float scaleZ = depth / propSpan(cabinetPropMesh_, 2);
            AppendStaticPropMesh(vertices, indices, cabinetPropMesh_, {px, 0.0f, pz}, yaw, scaleX, scaleY, scaleZ,
                0.0f, 10.0f, &propShadowIndices);
            addBakedPropShadow(px, pz, width, depth, height, yaw, seed);
            propLookPoints_.push_back({px, height * 0.74f, pz});
            return true;
            XMFLOAT3 c{px, height * 0.5f, pz};
            XMFLOAT3 right{std::cos(yaw), 0.0f, -std::sin(yaw)};
            XMFLOAT3 up{0.0f, 1.0f, 0.0f};
            XMFLOAT3 forward{std::sin(yaw), 0.0f, std::cos(yaw)};
            auto off = [&](float x, float y, float z) {
                return Add3(c, OrientedOffset(right, up, forward, x, y, z));
            };

            float bodyMat = 8.0f;
            float trimMat = 10.0f;
            AddOrientedBox(vertices, indices, c, {width * 0.48f, height * 0.49f, depth * 0.48f}, yaw, bodyMat);
            AddOrientedBox(vertices, indices, off(0.0f, height * 0.505f, -depth * 0.025f), {width * 0.52f, 0.026f, depth * 0.52f}, yaw, trimMat);
            AddOrientedBox(vertices, indices, off(0.0f, -height * 0.505f, 0.0f), {width * 0.51f, 0.024f, depth * 0.50f}, yaw, trimMat);
            AddOrientedBox(vertices, indices, off(-width * 0.505f, 0.0f, -depth * 0.02f), {0.018f, height * 0.48f, depth * 0.49f}, yaw, trimMat);
            AddOrientedBox(vertices, indices, off(width * 0.505f, 0.0f, -depth * 0.02f), {0.018f, height * 0.48f, depth * 0.49f}, yaw, trimMat);
            AddOrientedBox(vertices, indices, off(0.0f, height * 0.03f, -depth * 0.515f), {width * 0.44f, height * 0.42f, 0.010f}, yaw, bodyMat);
            AddOrientedBox(vertices, indices, off(0.0f, height * 0.03f, -depth * 0.542f), {0.010f, height * 0.40f, 0.012f}, yaw, trimMat);

            int rows = tall ? 4 : 2;
            float usableH = height * 0.78f;
            float drawerH = usableH / static_cast<float>(rows) * 0.76f;
            float startY = -usableH * 0.5f + drawerH * 0.5f;
            for (int r = 0; r < rows; ++r) {
                float ry = startY + static_cast<float>(r) * usableH / static_cast<float>(rows);
                float variant = LampHash(seed * 17.0f + static_cast<float>(r) * 3.3f, px - pz);
                float drawerW = width * (tall ? 0.78f : 0.82f);
                if (variant < 0.18f) {
                    AddOrientedBox(vertices, indices, off(0.0f, ry, -depth * 0.555f), {drawerW * 0.46f, drawerH * 0.40f, 0.012f}, yaw, trimMat);
                    AddOrientedBox(vertices, indices, off(0.0f, ry, -depth * 0.575f), {drawerW * 0.32f, 0.010f, 0.010f}, yaw, trimMat);
                } else if (variant < 0.46f) {
                    float pull = 0.13f + variant * 0.30f;
                    AddOrientedBox(vertices, indices, off(0.0f, ry, -depth * 0.56f - pull * 0.48f), {drawerW * 0.44f, drawerH * 0.38f, pull * 0.5f}, yaw, bodyMat);
                    AddOrientedBox(vertices, indices, off(0.0f, ry, -depth * 0.555f), {drawerW * 0.48f, drawerH * 0.42f, 0.010f}, yaw, trimMat);
                    AddOrientedBox(vertices, indices, off(0.0f, ry + drawerH * 0.02f, -depth * 0.59f - pull), {drawerW * 0.30f, 0.010f, 0.010f}, yaw, trimMat);
                } else {
                    AddOrientedBox(vertices, indices, off(0.0f, ry, -depth * 0.560f), {drawerW * 0.46f, drawerH * 0.40f, 0.012f}, yaw, bodyMat);
                    AddOrientedBox(vertices, indices, off(0.0f, ry + drawerH * 0.31f, -depth * 0.580f), {drawerW * 0.40f, 0.006f, 0.006f}, yaw, trimMat);
                }
                if (variant >= 0.18f) {
                    AddOrientedBox(vertices, indices, off(0.0f, ry, -depth * 0.595f), {drawerW * 0.20f, 0.008f, 0.010f}, yaw, trimMat);
                }
            }
            int ventSlots = tall ? 4 : 3;
            for (int s = 0; s < ventSlots; ++s) {
                float sy = height * 0.30f - static_cast<float>(s) * 0.055f;
                AddOrientedBox(vertices, indices, off(width * 0.515f, sy, depth * 0.10f), {0.006f, 0.007f, depth * 0.22f}, yaw, trimMat);
            }
            for (int sx = -1; sx <= 1; sx += 2) {
                for (int sz = -1; sz <= 1; sz += 2) {
                    AddOrientedBox(vertices, indices, off(sx * width * 0.36f, -height * 0.51f - 0.030f, sz * depth * 0.31f), {0.030f, 0.030f, 0.030f}, yaw, trimMat);
                }
            }

            if (!tall && seed > 0.56f) {
                float doorYaw = yaw + (seed > 0.76f ? -0.48f : 0.48f);
                float side = seed > 0.76f ? 1.0f : -1.0f;
                AddOrientedBox(vertices, indices,
                    off(side * width * 0.30f, height * 0.02f, -depth * 0.65f),
                    {width * 0.20f, height * 0.32f, 0.012f}, doorYaw, bodyMat);
                AddOrientedBox(vertices, indices,
                    off(side * width * 0.12f, height * 0.02f, -depth * 0.70f),
                    {0.010f, height * 0.12f, 0.010f}, doorYaw, trimMat);
            }
            propLookPoints_.push_back({px, height * 0.74f, pz});
            return true;
        };

        auto addMetalCabinetAgainstWall = [&](Tile t, int side, float seed) {
            XMFLOAT3 c = maze_.WorldCenter(t, 0.0f);
            bool tall = seed < 0.62f;
            XMFLOAT3 cab = cabinetSize(tall);
            float width = cab.x;
            float depth = cab.z;
            float wallSpan = (side == 0 || side == 1) ? tileW : tileD;
            float lateralLimit = std::max(0.0f, wallSpan * 0.5f - width * 0.5f - 0.10f);
            float lateral = (LampHash(c.x + seed * 9.1f, c.z - seed * 3.7f) - 0.5f) * lateralLimit * 2.0f;
            float px = c.x;
            float pz = c.z;
            float yaw = 0.0f;
            if (side == 0) {
                yaw = kPi;
                px = c.x + lateral;
                pz = c.z - tileD * 0.5f + depth * 0.5f + 0.075f;
            } else if (side == 1) {
                yaw = 0.0f;
                px = c.x + lateral;
                pz = c.z + tileD * 0.5f - depth * 0.5f - 0.075f;
            } else if (side == 2) {
                yaw = -kPi * 0.5f;
                px = c.x - tileW * 0.5f + depth * 0.5f + 0.075f;
                pz = c.z + lateral;
            } else {
                yaw = kPi * 0.5f;
                px = c.x + tileW * 0.5f - depth * 0.5f - 0.075f;
                pz = c.z + lateral;
            }
            return addMetalCabinet(px, pz, yaw, seed, tall);
        };

        auto addExitDoor = [&]() {
            if (!exitPortal.valid) return;
            XMFLOAT3 c = maze_.WorldCenter(exitPortal.tile, 0.0f);
            float bx = exitPortal.wallCenter.x;
            float bz = exitPortal.wallCenter.z;
            XMFLOAT3 inward = exitPortal.inward;
            XMFLOAT3 up{0.0f, 1.0f, 0.0f};
            XMFLOAT3 right = exitPortal.right;
            constexpr float fixedDoorCenterY = 1.05f;
            constexpr float fixedDoorHalfW = 0.60f;
            constexpr float fixedDoorHalfH = 1.05f;
            constexpr float fixedFramePostHalfW = 0.055f;
            constexpr float fixedFrameTopHalfH = 0.070f;
            XMFLOAT3 doorCenter{bx + inward.x * 0.026f, fixedDoorCenterY, bz + inward.z * 0.026f};
            XMFLOAT3 forward = inward;
            exitDoorCenter_ = doorCenter;
            exitDoorNormal_ = inward;
            exitDoorRight_ = right;
            exitDoorHinge_ = Add3(doorCenter, OrientedOffset(right, up, forward, -fixedDoorHalfW, 0.0f, 0.0f));
            constexpr float framePostCenterX = fixedDoorHalfW + fixedFramePostHalfW;
            constexpr float framePostHalfH = (fixedDoorCenterY + fixedDoorHalfH + fixedFrameTopHalfH * 2.0f) * 0.5f;
            constexpr float framePostCenterY = framePostHalfH;
            constexpr float frameTopCenterY = fixedDoorCenterY + fixedDoorHalfH + fixedFrameTopHalfH;
            constexpr float frameOuterHalfW = fixedDoorHalfW + fixedFramePostHalfW * 2.0f;
            AddOrientedBox(vertices, indices, Add3(doorCenter, OrientedOffset(right, up, forward, -framePostCenterX, framePostCenterY - doorCenter.y, 0.0f)), {fixedFramePostHalfW, framePostHalfH, 0.038f}, exitPortal.yaw, 10.0f);
            AddOrientedBox(vertices, indices, Add3(doorCenter, OrientedOffset(right, up, forward, framePostCenterX, framePostCenterY - doorCenter.y, 0.0f)), {fixedFramePostHalfW, framePostHalfH, 0.038f}, exitPortal.yaw, 10.0f);
            AddOrientedBox(vertices, indices, Add3(doorCenter, OrientedOffset(right, up, forward, 0.0f, frameTopCenterY - doorCenter.y, 0.0f)), {frameOuterHalfW, fixedFrameTopHalfH, 0.038f}, exitPortal.yaw, 10.0f);
            constexpr float fixedSignTargetH = 0.28f;
            float signY = doorCenter.y + fixedDoorHalfH + fixedSignTargetH * 0.5f + 0.24f;
            signY = std::min(signY, wallH - fixedSignTargetH * 0.5f - 0.12f);
            XMFLOAT3 sign = {bx + forward.x * 0.028f, signY, bz + forward.z * 0.028f};
            exitSignLightPos_ = Add3(sign, OrientedOffset(right, up, forward, 0.0f, -0.02f, 0.16f));
            exitSignLightStrength_ = 1.0f;
            if (!exitSignPropMesh_.vertices.empty()) {
                float spanX = std::max(0.001f, propSpan(exitSignPropMesh_, 0));
                float spanY = std::max(0.001f, propSpan(exitSignPropMesh_, 1));
                float targetW = std::min(1.04f, exitPortal.halfSpan * 1.30f);
                float targetH = fixedSignTargetH;
                float scale = std::clamp(std::min(targetW / spanX, targetH / spanY), 0.05f, 8.0f);
                XMFLOAT3 localCenter{
                    (exitSignPropMesh_.min.x + exitSignPropMesh_.max.x) * 0.5f,
                    (exitSignPropMesh_.min.y + exitSignPropMesh_.max.y) * 0.5f,
                    (exitSignPropMesh_.min.z + exitSignPropMesh_.max.z) * 0.5f
                };
                auto appendExitSignModel = [&](XMFLOAT3 signCenter, XMFLOAT3 signRight, XMFLOAT3 signForward, float yaw) {
                    XMFLOAT3 origin = Add3(signCenter, Add3(Scale3(signRight, -localCenter.x * scale),
                        Add3(Scale3(up, -localCenter.y * scale), Scale3(signForward, -localCenter.z * scale))));
                    return AppendStaticPropMesh(vertices, indices, exitSignPropMesh_, origin, yaw, scale, scale, scale, 0.0f, 7.0f);
                };
                bool appended = appendExitSignModel(Add3(sign, Scale3(forward, 0.006f)), right, forward, exitPortal.yaw);
                appended = appendExitSignModel(Add3(sign, Scale3(forward, -0.006f)), Scale3(right, -1.0f), Scale3(forward, -1.0f), exitPortal.yaw + kPi) || appended;
                if (!appended) {
                    StartupProfileLine(L"Emergency exit sign mesh was loaded but could not be appended; no handmade fallback was drawn.");
                }
            } else {
                StartupProfileLine(L"Emergency exit sign mesh missing; no handmade fallback was drawn.");
            }
        };
        addExitDoor();

        float paperDensity = std::clamp(settings_.paperDensity, 0.0f, 4.0f);
        float hallwayPaperDensity = std::clamp(settings_.hallwayPaperRunDensity, 0.0f, 4.0f);
        float chairChance = std::min(1.0f, 0.030f * std::clamp(settings_.chairDensity, 0.0f, 4.0f));
        float loosePaperChance = std::min(1.0f, 0.082f * paperDensity);
        float paperHallwayChance = std::min(1.0f, 0.13f * hallwayPaperDensity);
        float ventChance = gEffectDebugViewer && gDebugSliceEffect == DebugSliceEffect::AirVents ? 1.0f : 0.026f;
        float waterDamageChance = 0.0f;
        float waterLikeDamageChance = std::min(1.0f, 0.003f * std::clamp(settings_.waterDamageDensity, 0.0f, 4.0f));

        auto waterMaterial = [](float seed, float bandStart, float bandWidth) {
            float h = std::fmod(std::abs(seed) * 37.719f + 0.137f, 1.0f);
            float safeWidth = std::max(0.0f, std::min(bandWidth, 0.043f - bandStart));
            return 11.006f + bandStart + h * safeWidth;
        };
        constexpr float kWaterFloorLift = 0.008f;
        float waterCeilingY = wallH - 0.020f;
        struct WaterTileSurface {
            bool active = false;
            bool suppressCard = false;
            int side = 0;
            int mode = 0;
            float seed = 0.0f;
            float score = -1.0f;
        };
        std::vector<WaterTileSurface> floorWaterTiles(static_cast<size_t>(maze_.w * maze_.h));
        std::vector<WaterTileSurface> ceilingWaterTiles(static_cast<size_t>(maze_.w * maze_.h));
        auto waterTileIndex = [&](Tile t) {
            return static_cast<size_t>(t.y * maze_.w + t.x);
        };
        auto mergeWaterMode = [](int a, int b) {
            if (a == 3 && b == 3) return 3;
            if (a == 3) a = 0;
            if (b == 3) b = 0;
            bool central = a == 0 || a == 1 || b == 0 || b == 1;
            bool edge = a == 1 || a == 2 || b == 1 || b == 2;
            if (central && edge) return 1;
            if (edge) return 2;
            return 0;
        };
        auto oppositeWaterSide = [](int side) {
            if (side == 0) return 1;
            if (side == 1) return 0;
            if (side == 2) return 3;
            return 2;
        };
        auto markWaterTile = [&](Tile t, bool ceiling, int side, int mode, float seed, float score, bool suppressCard = false) {
            if (!maze_.IsOpen(t.x, t.y) || (!gEffectDebugViewer && (t == maze_.start || t == maze_.exit))) return;
            WaterTileSurface& surface = (ceiling ? ceilingWaterTiles : floorWaterTiles)[waterTileIndex(t)];
            side = std::clamp(side, 0, 3);
            mode = std::clamp(mode, 0, 3);
            if (ceiling) {
                MarkWetCeilingTile(t);
            } else {
                MarkWetFootstepTile(t);
            }
            if (!surface.active) {
                surface.active = true;
                surface.suppressCard = suppressCard;
                surface.side = side;
                surface.mode = mode;
                surface.seed = seed;
                surface.score = score;
                return;
            }
            surface.suppressCard = surface.suppressCard && suppressCard;
            surface.mode = mergeWaterMode(surface.mode, mode);
            if (score >= surface.score) {
                surface.side = side;
                surface.seed = seed;
                surface.score = score;
            }
        };
        auto emitFloorWaterPoolCard = [&](Tile owner, float cx, float cz, int side, float seed,
                                           float width, float depth, float yaw, float uvModeBase,
                                           float score) {
            if (!maze_.IsOpen(owner.x, owner.y) || (!gEffectDebugViewer && (owner == maze_.start || owner == maze_.exit))) return false;
            float w = width;
            float d = depth;
            for (int attempt = 0; attempt < 4; ++attempt) {
                if (footprintFitsMaze(cx, cz, w, d, yaw, 0.020f)) {
                    float cYaw = std::cos(yaw);
                    float sYaw = std::sin(yaw);
                    XMFLOAT3 right{cYaw, 0.0f, -sYaw};
                    XMFLOAT3 forward{sYaw, 0.0f, cYaw};
                    XMFLOAT3 center{cx, kWaterFloorLift, cz};
                    XMFLOAT3 a = Add3(center, Add3(Scale3(right, -w * 0.5f), Scale3(forward,  d * 0.5f)));
                    XMFLOAT3 b = Add3(center, Add3(Scale3(right,  w * 0.5f), Scale3(forward,  d * 0.5f)));
                    XMFLOAT3 c0 = Add3(center, Add3(Scale3(right,  w * 0.5f), Scale3(forward, -d * 0.5f)));
                    XMFLOAT3 d0 = Add3(center, Add3(Scale3(right, -w * 0.5f), Scale3(forward, -d * 0.5f)));
                    AddQuadUV(vertices, waterIndices, a, b, c0, d0, {0, 1, 0}, right,
                        {0, uvModeBase}, {1, uvModeBase}, {1, uvModeBase + 1.0f}, {0, uvModeBase + 1.0f},
                        waterMaterial(seed, 0.0f, 0.014f));
                    markWaterTile(owner, false, side, 0, seed, score, true);
                    MarkWetFootstepArea(cx, cz, w, d, yaw);
                    return true;
                }
                w *= 0.86f;
                d *= 0.86f;
            }
            return false;
        };
        struct PendingWallWaterPool {
            Tile owner{};
            int side = 0;
            float cx = 0.0f;
            float cz = 0.0f;
            float width = 0.0f;
            float depth = 0.0f;
            float yaw = 0.0f;
            float seed = 0.0f;
            float score = 0.0f;
        };
        std::vector<PendingWallWaterPool> pendingWallWaterPools;
        pendingWallWaterPools.reserve(128);
        auto queueWallWaterPoolCard = [&](Tile owner, float cx, float cz, int side, float seed,
                                          float width, float depth, float yaw, float score) {
            if (!maze_.IsOpen(owner.x, owner.y) || (!gEffectDebugViewer && (owner == maze_.start || owner == maze_.exit))) return;
            pendingWallWaterPools.push_back({owner, side, cx, cz, width, depth, yaw, seed, score});
        };
        auto emitMergedWallWaterPools = [&]() {
            std::vector<uint8_t> used(pendingWallWaterPools.size(), 0);
            for (size_t i = 0; i < pendingWallWaterPools.size(); ++i) {
                if (used[i]) continue;
                const PendingWallWaterPool& first = pendingWallWaterPools[i];
                int axis = first.side < 2 ? 0 : 1;
                float minX = std::numeric_limits<float>::max();
                float maxX = -std::numeric_limits<float>::max();
                float minZ = std::numeric_limits<float>::max();
                float maxZ = -std::numeric_limits<float>::max();
                float seedSum = 0.0f;
                float bestScore = -1.0f;
                int bestSide = first.side;
                int count = 0;
                auto include = [&](size_t index) {
                    const PendingWallWaterPool& p = pendingWallWaterPools[index];
                    used[index] = 1;
                    ++count;
                    seedSum += p.seed;
                    if (p.score > bestScore) {
                        bestScore = p.score;
                        bestSide = p.side;
                    }
                    float cYaw = std::cos(p.yaw);
                    float sYaw = std::sin(p.yaw);
                    XMFLOAT3 right{cYaw, 0.0f, -sYaw};
                    XMFLOAT3 forward{sYaw, 0.0f, cYaw};
                    std::array<XMFLOAT3, 4> corners{
                        Add3({p.cx, 0.0f, p.cz}, Add3(Scale3(right, -p.width * 0.5f), Scale3(forward,  p.depth * 0.5f))),
                        Add3({p.cx, 0.0f, p.cz}, Add3(Scale3(right,  p.width * 0.5f), Scale3(forward,  p.depth * 0.5f))),
                        Add3({p.cx, 0.0f, p.cz}, Add3(Scale3(right,  p.width * 0.5f), Scale3(forward, -p.depth * 0.5f))),
                        Add3({p.cx, 0.0f, p.cz}, Add3(Scale3(right, -p.width * 0.5f), Scale3(forward, -p.depth * 0.5f)))
                    };
                    for (const XMFLOAT3& c : corners) {
                        minX = std::min(minX, c.x);
                        maxX = std::max(maxX, c.x);
                        minZ = std::min(minZ, c.z);
                        maxZ = std::max(maxZ, c.z);
                    }
                };
                include(i);
                for (size_t j = i + 1; j < pendingWallWaterPools.size(); ++j) {
                    if (used[j]) continue;
                    const PendingWallWaterPool& p = pendingWallWaterPools[j];
                    if ((p.side < 2 ? 0 : 1) != axis) continue;
                    int tileDx = std::abs(p.owner.x - first.owner.x);
                    int tileDy = std::abs(p.owner.y - first.owner.y);
                    if (!(p.owner == first.owner) && tileDx + tileDy > 1) continue;
                    include(j);
                }
                if (count <= 1) {
                    emitFloorWaterPoolCard(first.owner, first.cx, first.cz, first.side, first.seed,
                        first.width, first.depth, first.yaw, 5.0f, first.score);
                    continue;
                }
                float finalCx = (minX + maxX) * 0.5f;
                float finalCz = (minZ + maxZ) * 0.5f;
                float finalW = std::max(0.05f, maxX - minX);
                float finalD = std::max(0.05f, maxZ - minZ);
                if (!footprintFitsMaze(finalCx, finalCz, finalW, finalD, 0.0f, 0.020f)) {
                    float marginX = tileW * 0.012f;
                    float marginZ = tileD * 0.012f;
                    float l = ox + static_cast<float>(first.owner.x) * tileW + marginX;
                    float r = l + tileW - marginX * 2.0f;
                    float z0 = oz + static_cast<float>(first.owner.y) * tileD + marginZ;
                    float z1 = z0 + tileD - marginZ * 2.0f;
                    minX = std::clamp(minX, l, r);
                    maxX = std::clamp(maxX, l, r);
                    minZ = std::clamp(minZ, z0, z1);
                    maxZ = std::clamp(maxZ, z0, z1);
                    finalCx = (minX + maxX) * 0.5f;
                    finalCz = (minZ + maxZ) * 0.5f;
                    finalW = std::max(0.05f, maxX - minX);
                    finalD = std::max(0.05f, maxZ - minZ);
                }
                emitFloorWaterPoolCard(first.owner, finalCx, finalCz, bestSide,
                    seedSum / static_cast<float>(std::max(1, count)), finalW, finalD,
                    0.0f, 5.0f, std::max(1.18f, bestScore));
            }
        };
        auto addCircularFloorWaterPool = [&](Tile origin, int side, float seed, float strength) {
            if (!maze_.IsOpen(origin.x, origin.y) || (!gEffectDebugViewer && (origin == maze_.start || origin == maze_.exit))) return false;
            XMFLOAT3 c = maze_.WorldCenter(origin, 0.0f);
            float minTile = std::max(0.10f, std::min(tileW, tileD));
            float h0 = LampHash(seed * 17.0f + c.x, c.z + 3.1f);
            float h1 = LampHash(seed * 23.0f - c.z, c.x + 5.7f);
            float h2 = LampHash(seed * 31.0f + c.x * 0.5f, c.z * 0.5f);
            float radius = minTile * (0.42f + h0 * 0.20f + std::clamp(strength, 0.35f, 1.35f) * 0.18f);
            float cx = c.x + (h1 - 0.5f) * tileW * 0.20f;
            float cz = c.z + (h2 - 0.5f) * tileD * 0.20f;
            float yaw = (LampHash(seed * 43.0f + c.x, c.z) - 0.5f) * 0.18f;
            float width = radius * 2.0f;
            float depth = radius * (1.88f + LampHash(seed * 47.0f - c.x, c.z) * 0.16f);
            return emitFloorWaterPoolCard(origin, cx, cz, side, seed, width, depth, yaw, 0.0f, 1.34f);
        };
        auto markWaterBlob = [&](Tile origin, bool ceiling, int primarySide, int centerMode, float seed, float strength) {
            if (!ceiling && !gEffectDebugViewer) {
                if (addCircularFloorWaterPool(origin, primarySide, seed, strength)) return;
            }
            int radiusTiles = 0;
            if (gEffectDebugViewer) {
                radiusTiles = std::max(1, gDebugSliceTiles / 2);
            } else if (ceiling && centerMode != 3 && strength > 1.02f) {
                radiusTiles = 1;
            }
            float blobRadius = gEffectDebugViewer
                ? (0.70f + static_cast<float>(gDebugSliceTiles) * 0.36f + LampHash(seed * 19.0f, seed * 7.0f) * 0.22f)
                : (ceiling
                    ? (0.82f + std::clamp(strength, 0.35f, 1.35f) * 0.25f)
                    : (0.46f + std::clamp(strength, 0.35f, 1.35f) * 0.18f));
            for (int dy = -radiusTiles; dy <= radiusTiles; ++dy) {
                for (int dx = -radiusTiles; dx <= radiusTiles; ++dx) {
                    Tile t{origin.x + dx, origin.y + dy};
                    if (!maze_.IsOpen(t.x, t.y)) continue;
                    float dist = std::sqrt(static_cast<float>(dx * dx + dy * dy));
                    float edgeNoise = (tileHash(t.x, t.y, seed * 61.0f + (ceiling ? 17.0f : 5.0f)) - 0.5f) * 0.46f;
                    if (dist > 0.01f && dist + edgeNoise > blobRadius) continue;
                    int side = primarySide;
                    if (std::abs(dx) > std::abs(dy)) side = dx < 0 ? 2 : 3;
                    else if (std::abs(dy) > 0) side = dy < 0 ? 0 : 1;
                    int mode = centerMode;
                    if (dist > 0.75f) {
                        mode = ceiling ? 1 : 0;
                    }
                    float score = 1.30f - dist * 0.18f + tileHash(t.x, t.y, seed * 37.0f + 23.0f) * 0.08f;
                    markWaterTile(t, ceiling, side, mode,
                        seed + static_cast<float>(dx) * 0.071f + static_cast<float>(dy) * 0.113f,
                        score);
                }
            }
        };
        auto addWaterAttentionPoint = [&](const XMFLOAT3& pos, const XMFLOAT3& source, const XMFLOAT3& normal,
            float radius, float seed, bool requireFacing) {
            if (gEffectDebugViewer || bloodScarePoints_.size() > 384) return;
            BloodScarePoint scare{};
            scare.pos = pos;
            scare.source = source;
            scare.normal = normal;
            scare.radius = std::clamp(radius, maze_.TileAverage() * 0.78f, maze_.TileAverage() * 1.45f);
            scare.focusDelaySeconds = 0.20f + LampHash(seed * 19.0f + pos.x, pos.z - seed * 7.0f) * 0.46f;
            scare.dreadScale = 0.42f;
            scare.requireFacing = requireFacing;
            scare.revealBlood = false;
            bloodScarePoints_.push_back(scare);
        };
        auto emitWaterTileCard = [&](Tile t, bool ceiling, const WaterTileSurface& surface) {
            if (!surface.active) return;
            if (surface.suppressCard) return;
            float l = ox + static_cast<float>(t.x) * tileW;
            float r = l + tileW;
            float z0 = oz + static_cast<float>(t.y) * tileD;
            float z1 = z0 + tileD;
            float y = ceiling ? waterCeilingY : kWaterFloorLift;
            float u = static_cast<float>(surface.side);
            int neighborMask = 0;
            auto neighborWet = [&](int nx, int ny) {
                if (!maze_.IsOpen(nx, ny)) return false;
                const WaterTileSurface& neighbor = (ceiling ? ceilingWaterTiles : floorWaterTiles)[static_cast<size_t>(ny * maze_.w + nx)];
                return neighbor.active && !neighbor.suppressCard;
            };
            auto neighborOpen = [&](int nx, int ny) {
                return maze_.IsOpen(nx, ny);
            };
            if (neighborWet(t.x, t.y - 1)) neighborMask |= 1;
            if (neighborWet(t.x, t.y + 1)) neighborMask |= 2;
            if (neighborWet(t.x - 1, t.y)) neighborMask |= 4;
            if (neighborWet(t.x + 1, t.y)) neighborMask |= 8;
            if (neighborWet(t.x - 1, t.y - 1)) neighborMask |= 16;
            if (neighborWet(t.x + 1, t.y - 1)) neighborMask |= 32;
            if (neighborWet(t.x - 1, t.y + 1)) neighborMask |= 64;
            if (neighborWet(t.x + 1, t.y + 1)) neighborMask |= 128;
            float vMode = static_cast<float>(surface.mode + neighborMask * 8);
            float material = waterMaterial(surface.seed, 0.0f, 0.014f);
            float h0 = LampHash(surface.seed * 17.0f + static_cast<float>(t.x), static_cast<float>(t.y) + 3.1f);
            float h1 = LampHash(surface.seed * 23.0f - static_cast<float>(t.y), static_cast<float>(t.x) + 5.7f);
            float h2 = LampHash(surface.seed * 31.0f + static_cast<float>(t.x) * 0.5f, static_cast<float>(t.y) * 0.5f);
            float h3 = LampHash(surface.seed * 41.0f + static_cast<float>(t.x) * 1.7f, static_cast<float>(t.y) * 2.3f);
            float sizeScore = std::clamp(0.70f + (surface.score - 0.75f) * 0.22f, 0.54f, 1.05f);
            float halfW = tileW * (ceiling ? (0.30f + h0 * 0.20f) : (0.20f + h0 * 0.17f)) * sizeScore;
            float halfD = tileD * (ceiling ? (0.30f + h1 * 0.20f) : (0.18f + h1 * 0.16f)) * sizeScore;
            if (surface.mode == 3) {
                halfW *= 0.62f;
                halfD *= 0.62f;
            }
            float cx = (l + r) * 0.5f + (h2 - 0.5f) * tileW * (ceiling ? 0.18f : 0.24f);
            float cz = (z0 + z1) * 0.5f + (h3 - 0.5f) * tileD * (ceiling ? 0.18f : 0.22f);
            auto pullTowardSide = [&](int side, float amount) {
                if (side == 0) cz -= tileD * amount;
                else if (side == 1) cz += tileD * amount;
                else if (side == 2) cx -= tileW * amount;
                else cx += tileW * amount;
            };
            if (surface.mode > 0 && surface.mode != 3) {
                pullTowardSide(surface.side, ceiling ? 0.16f : 0.13f);
                if (surface.side == 0 || surface.side == 1) halfD = std::max(halfD, tileD * (ceiling ? 0.43f : 0.36f));
                else halfW = std::max(halfW, tileW * (ceiling ? 0.43f : 0.36f));
            }
            float wetOverlapZ = tileD * (ceiling ? 0.010f : 0.030f);
            float wetOverlapX = tileW * (ceiling ? 0.010f : 0.030f);
            float floorSeepZ = ceiling ? 0.0f : tileD * 0.085f;
            float floorSeepX = ceiling ? 0.0f : tileW * 0.085f;
            if (neighborMask & 1) z0 -= wetOverlapZ;
            else if (!ceiling && neighborOpen(t.x, t.y - 1)) z0 = std::max(z0 - floorSeepZ, cz - halfD);
            else z0 = std::max(z0 + tileD * 0.055f, cz - halfD);
            if (neighborMask & 2) z1 += wetOverlapZ;
            else if (!ceiling && neighborOpen(t.x, t.y + 1)) z1 = std::min(z1 + floorSeepZ, cz + halfD);
            else z1 = std::min(z1 - tileD * 0.055f, cz + halfD);
            if (neighborMask & 4) l -= wetOverlapX;
            else if (!ceiling && neighborOpen(t.x - 1, t.y)) l = std::max(l - floorSeepX, cx - halfW);
            else l = std::max(l + tileW * 0.055f, cx - halfW);
            if (neighborMask & 8) r += wetOverlapX;
            else if (!ceiling && neighborOpen(t.x + 1, t.y)) r = std::min(r + floorSeepX, cx + halfW);
            else r = std::min(r - tileW * 0.055f, cx + halfW);
            if (r - l < tileW * 0.12f || z1 - z0 < tileD * 0.12f) return;
            if (ceiling) {
                AddQuadUV(vertices, waterIndices,
                    {l, y, z0}, {r, y, z0}, {r, y, z1}, {l, y, z1},
                    {0, -1, 0}, {1, 0, 0},
                    {u, vMode}, {u + 1.0f, vMode}, {u + 1.0f, vMode + 1.0f}, {u, vMode + 1.0f}, material);
            } else {
                AddQuadUV(vertices, waterIndices,
                    {l, y, z1}, {r, y, z1}, {r, y, z0}, {l, y, z0},
                    {0, 1, 0}, {1, 0, 0},
                    {u, vMode}, {u + 1.0f, vMode}, {u + 1.0f, vMode + 1.0f}, {u, vMode + 1.0f}, material);
                MarkWetFootstepArea((l + r) * 0.5f, (z0 + z1) * 0.5f, r - l, z1 - z0, 0.0f);
            }
        };
        auto neighborForSideWater = [](Tile t, int side) {
            if (side == 0) return Tile{t.x, t.y - 1};
            if (side == 1) return Tile{t.x, t.y + 1};
            if (side == 2) return Tile{t.x - 1, t.y};
            return Tile{t.x + 1, t.y};
        };
        auto sideDirectionWater = [](int side) {
            if (side == 0) return XMFLOAT3{0.0f, 0.0f, -1.0f};
            if (side == 1) return XMFLOAT3{0.0f, 0.0f, 1.0f};
            if (side == 2) return XMFLOAT3{-1.0f, 0.0f, 0.0f};
            return XMFLOAT3{1.0f, 0.0f, 0.0f};
        };
        auto sideForwardYawWater = [](int side) {
            if (side == 0) return kPi;
            if (side == 1) return 0.0f;
            if (side == 2) return -kPi * 0.5f;
            return kPi * 0.5f;
        };
        auto wallHasWaterSurface = [&](Tile tile, int side) {
            if (!maze_.IsOpen(tile.x, tile.y)) return false;
            if (side == 0) return !maze_.IsOpen(tile.x, tile.y - 1);
            if (side == 1) return !maze_.IsOpen(tile.x, tile.y + 1);
            if (side == 2) return !maze_.IsOpen(tile.x - 1, tile.y);
            return !maze_.IsOpen(tile.x + 1, tile.y);
        };
        auto wallWaterSupportSpan = [&](Tile t, int side, float& minAlong, float& maxAlong) {
            if (!wallHasWaterSurface(t, side)) return false;
            if (side == 0 || side == 1) {
                int x0 = t.x;
                int x1 = t.x;
                while (wallHasWaterSurface({x0 - 1, t.y}, side)) --x0;
                while (wallHasWaterSurface({x1 + 1, t.y}, side)) ++x1;
                minAlong = ox + static_cast<float>(x0) * tileW;
                maxAlong = ox + static_cast<float>(x1 + 1) * tileW;
            } else {
                int y0 = t.y;
                int y1 = t.y;
                while (wallHasWaterSurface({t.x, y0 - 1}, side)) --y0;
                while (wallHasWaterSurface({t.x, y1 + 1}, side)) ++y1;
                minAlong = oz + static_cast<float>(y0) * tileD;
                maxAlong = oz + static_cast<float>(y1 + 1) * tileD;
            }
            return maxAlong - minAlong > 0.20f;
        };
        auto addWaterWallCard = [&](Tile t, int side, float lateral, float yCenter, float w, float h, bool sourceFromCeiling, float seed) {
            XMFLOAT3 c = maze_.WorldCenter(t, 0.0f);
            XMFLOAT3 normal{0.0f, 0.0f, 1.0f};
            XMFLOAT3 right{1.0f, 0.0f, 0.0f};
            XMFLOAT3 center{c.x, yCenter, c.z};
            constexpr float kWaterWallDecalInset = 0.0045f;
            float minAlong = 0.0f;
            float maxAlong = 0.0f;
            if (!wallWaterSupportSpan(t, side, minAlong, maxAlong)) return false;
            constexpr float wallDecalMargin = 0.10f;
            minAlong += wallDecalMargin;
            maxAlong -= wallDecalMargin;
            float available = maxAlong - minAlong;
            if (available < 0.24f) return false;
            w = std::min(w, available);
            float halfW = w * 0.5f;
            float desiredAlong = (side == 0 || side == 1) ? c.x + lateral : c.z + lateral;
            float clampedAlong = std::clamp(desiredAlong, minAlong + halfW, maxAlong - halfW);
            if (side == 0) {
                normal = {0.0f, 0.0f, 1.0f};
                right = {1.0f, 0.0f, 0.0f};
                center = {clampedAlong, yCenter, c.z - tileD * 0.5f + kWaterWallDecalInset};
            } else if (side == 1) {
                normal = {0.0f, 0.0f, -1.0f};
                right = {-1.0f, 0.0f, 0.0f};
                center = {clampedAlong, yCenter, c.z + tileD * 0.5f - kWaterWallDecalInset};
            } else if (side == 2) {
                normal = {1.0f, 0.0f, 0.0f};
                right = {0.0f, 0.0f, 1.0f};
                center = {c.x - tileW * 0.5f + kWaterWallDecalInset, yCenter, clampedAlong};
            } else {
                normal = {-1.0f, 0.0f, 0.0f};
                right = {0.0f, 0.0f, -1.0f};
                center = {c.x + tileW * 0.5f - kWaterWallDecalInset, yCenter, clampedAlong};
            }
            h = sourceFromCeiling
                ? wallH - 0.003f
                : std::clamp(h, 0.08f, wallH - 0.003f);
            center.y = sourceFromCeiling
                ? wallH - 0.0015f - h * 0.5f
                : 0.0015f + h * 0.5f;
            XMFLOAT3 up{0.0f, 1.0f, 0.0f};
            XMFLOAT3 a = Add3(center, Add3(Scale3(right, -w * 0.5f), Scale3(up, -h * 0.5f)));
            XMFLOAT3 b = Add3(center, Add3(Scale3(right,  w * 0.5f), Scale3(up, -h * 0.5f)));
            XMFLOAT3 c0 = Add3(center, Add3(Scale3(right,  w * 0.5f), Scale3(up,  h * 0.5f)));
            XMFLOAT3 d0 = Add3(center, Add3(Scale3(right, -w * 0.5f), Scale3(up,  h * 0.5f)));
            float wallUvBase = sourceFromCeiling ? 3.0f : 4.0f;
            AddQuadUV(vertices, waterIndices, a, b, c0, d0, normal, right,
                {0, wallUvBase + 0.999f}, {1, wallUvBase + 0.999f},
                {1, wallUvBase + 0.001f}, {0, wallUvBase + 0.001f},
                waterMaterial(seed, 0.037f, 0.011f));
            if (sourceFromCeiling) {
                float minTile = std::max(0.10f, std::min(tileW, tileD));
                float h0 = LampHash(seed * 67.0f + center.x, center.z);
                float h1 = LampHash(seed * 71.0f - center.z, center.x);
                float poolW = std::clamp(w * (0.78f + h0 * 0.42f), minTile * 0.30f, minTile * 0.82f);
                float poolD = minTile * (0.34f + h1 * 0.30f);
                float poolYaw = std::atan2(normal.x, normal.z);
                XMFLOAT3 poolCenter = Add3({center.x, 0.0f, center.z}, Scale3(normal, poolD * 0.48f + 0.020f));
                queueWallWaterPoolCard(t, poolCenter.x, poolCenter.z, side, seed + 0.83f,
                    poolW, poolD, poolYaw, 1.18f);
            }
            XMFLOAT3 attentionSource = sourceFromCeiling
                ? XMFLOAT3{center.x, wallH - 0.010f, center.z}
                : XMFLOAT3{center.x, 0.035f, center.z};
            addWaterAttentionPoint(center, attentionSource, normal,
                std::max(w, h) * 0.82f, seed + (sourceFromCeiling ? 0.57f : 0.29f), true);
            return true;
        };
        auto addWaterHorizontalSpill = [&](Tile t, int side, bool ceiling, float seed, float strength) {
            Tile n = neighborForSideWater(t, side);
            bool openNeighbor = maze_.IsOpen(n.x, n.y);
            XMFLOAT3 c = maze_.WorldCenter(t, 0.0f);
            XMFLOAT3 dir = sideDirectionWater(side);
            XMFLOAT3 lateralAxis = (side == 0 || side == 1)
                ? XMFLOAT3{1.0f, 0.0f, 0.0f}
                : XMFLOAT3{0.0f, 0.0f, 1.0f};
            float axis = (side == 0 || side == 1) ? tileD : tileW;
            float cross = (side == 0 || side == 1) ? tileW : tileD;
            float h0 = LampHash(seed * 17.0f + c.x, c.z + static_cast<float>(side) * 3.1f);
            float h1 = LampHash(seed * 23.0f - c.z, c.x + static_cast<float>(side) * 5.7f);
            float h2 = LampHash(seed * 31.0f + c.x * 0.5f, c.z * 0.5f);
            float length = axis * (0.50f + h0 * 0.58f) * std::clamp(strength, 0.68f, 1.32f);
            float width = cross * (0.24f + h1 * 0.42f);
            float along = openNeighbor ? axis * 0.50f + length * 0.04f : axis * 0.50f - length * 0.42f;
            float lateral = (h2 - 0.5f) * std::max(0.0f, cross - width) * 0.62f;
            XMFLOAT3 center = Add3(c, Add3(Scale3(dir, along), Scale3(lateralAxis, lateral)));
            float yaw = sideForwardYawWater(side) + (h1 - 0.5f) * 0.24f;
            if (footprintFitsMaze(center.x, center.z, width, length, yaw, openNeighbor ? 0.018f : 0.035f)) {
                if (ceiling) {
                    markWaterTile(t, true, side, 1, seed, 1.15f);
                    if (openNeighbor) {
                        markWaterTile(n, true, oppositeWaterSide(side), 2, seed, 0.86f);
                    }
                } else {
                    markWaterTile(t, false, side, 0, seed, 0.82f);
                    if (openNeighbor) {
                        markWaterTile(n, false, oppositeWaterSide(side), 0, seed, 0.62f);
                    }
                }
            }
            if (openNeighbor && LampHash(seed * 41.0f + static_cast<float>(side), c.x - c.z) < 0.48f) {
                XMFLOAT3 nc = maze_.WorldCenter(n, 0.0f);
                XMFLOAT3 satellite = Add3(nc, Add3(Scale3(dir, -axis * (0.18f + h2 * 0.12f)), Scale3(lateralAxis, -lateral * 0.36f)));
                float sw = width * (0.48f + h2 * 0.24f);
                float sl = length * (0.44f + h1 * 0.20f);
                if (footprintFitsMaze(satellite.x, satellite.z, sw, sl, yaw + (h0 - 0.5f) * 0.36f, 0.026f)) {
                    if (ceiling) {
                        markWaterTile(n, true, oppositeWaterSide(side), 2, seed + 0.031f, 0.74f);
                    } else {
                        markWaterTile(n, false, oppositeWaterSide(side), 0, seed + 0.031f, 0.54f);
                    }
                }
            } else if (!openNeighbor && wallHasWaterSurface(t, side)) {
                float wallW = width * (0.82f + h2 * 0.48f);
                float wallHgt = ceiling ? wallH - 0.003f : 0.12f + h0 * 0.18f;
                float yCenter = ceiling
                    ? wallH - wallHgt * 0.5f - 0.0015f
                    : wallHgt * 0.5f + 0.0015f;
                addWaterWallCard(t, side, lateral, yCenter, wallW, wallHgt, ceiling, seed + (ceiling ? 0.47f : 0.19f));
            }
        };
        auto emitFloorWaterBridge = [&](Tile t, int side) {
            Tile n = neighborForSideWater(t, side);
            if (!maze_.IsOpen(t.x, t.y) || !maze_.IsOpen(n.x, n.y)) return;
            const WaterTileSurface& aSurface = floorWaterTiles[waterTileIndex(t)];
            const WaterTileSurface& bSurface = floorWaterTiles[waterTileIndex(n)];
            if (!aSurface.active || !bSurface.active) return;
            if (aSurface.suppressCard || bSurface.suppressCard) return;

            float l = ox + static_cast<float>(t.x) * tileW;
            float r = l + tileW;
            float z0 = oz + static_cast<float>(t.y) * tileD;
            float z1 = z0 + tileD;
            constexpr float bridgeLift = 0.0018f;
            float y = kWaterFloorLift + bridgeLift;
            float material = waterMaterial((aSurface.seed + bSurface.seed) * 0.5f + static_cast<float>(side) * 0.071f, 0.0f, 0.014f);
            float seed = (aSurface.seed + bSurface.seed) * 0.5f + static_cast<float>(side) * 0.173f;
            float h0 = LampHash(seed * 17.0f + static_cast<float>(t.x), static_cast<float>(t.y));
            float h1 = LampHash(seed * 23.0f - static_cast<float>(t.y), static_cast<float>(t.x));
            float h2 = LampHash(seed * 31.0f + static_cast<float>(t.x) * 0.5f, static_cast<float>(t.y) * 0.5f);

            if (side == 1) {
                float seamZ = z1;
                float depth = std::min(tileD * (0.18f + h0 * 0.08f), 0.38f);
                float span = tileW * (0.92f + h1 * 0.08f);
                float cx = (l + r) * 0.5f + (h2 - 0.5f) * std::max(0.0f, tileW - span) * 0.72f;
                float x0 = std::max(l + tileW * 0.006f, cx - span * 0.5f);
                float x1 = std::min(r - tileW * 0.006f, cx + span * 0.5f);
                AddQuadUV(vertices, waterIndices,
                    {x0, y, seamZ + depth * 0.5f},
                    {x1, y, seamZ + depth * 0.5f},
                    {x1, y, seamZ - depth * 0.5f},
                    {x0, y, seamZ - depth * 0.5f},
                    {0, 1, 0}, {1, 0, 0},
                    {0, 4}, {1, 4}, {1, 5}, {0, 5}, material);
            } else if (side == 3) {
                float seamX = r;
                float width = std::min(tileW * (0.18f + h0 * 0.08f), 0.38f);
                float span = tileD * (0.92f + h1 * 0.08f);
                float cz = (z0 + z1) * 0.5f + (h2 - 0.5f) * std::max(0.0f, tileD - span) * 0.72f;
                float zz0 = std::max(z0 + tileD * 0.006f, cz - span * 0.5f);
                float zz1 = std::min(z1 - tileD * 0.006f, cz + span * 0.5f);
                AddQuadUV(vertices, waterIndices,
                    {seamX - width * 0.5f, y, zz1},
                    {seamX + width * 0.5f, y, zz1},
                    {seamX + width * 0.5f, y, zz0},
                    {seamX - width * 0.5f, y, zz0},
                    {0, 1, 0}, {1, 0, 0},
                    {0, 4}, {1, 4}, {1, 5}, {0, 5}, material);
            }
        };

        for (int y = 1; y < maze_.h - 1; ++y) {
            for (int x = 1; x < maze_.w - 1; ++x) {
                if (!maze_.IsOpen(x, y)) continue;
                Tile t{x, y};
                if (!gEffectDebugViewer && (t == maze_.start || t == maze_.exit)) continue;
                XMFLOAT3 c = maze_.WorldCenter(t, 0.0f);
                float h0 = tileHash(x, y, 0.1f);
                float h1 = tileHash(x, y, 1.7f);
                float h2 = tileHash(x, y, 2.9f);
                bool openN = maze_.IsOpen(x, y - 1);
                bool openS = maze_.IsOpen(x, y + 1);
                bool openW = maze_.IsOpen(x - 1, y);
                bool openE = maze_.IsOpen(x + 1, y);
                bool corridorNS = openN && openS && !openW && !openE;
                bool corridorEW = openW && openE && !openN && !openS;

                if (h0 < chairChance && IsRoomLike(t)) {
                    float chairX = c.x + (h1 - 0.5f) * std::max(0.0f, tileW - 1.12f) * 0.42f;
                    float chairZ = c.z + (h2 - 0.5f) * std::max(0.0f, tileD - 1.12f) * 0.42f;
                    float chairYaw = h1 * kPi * 2.0f;
                    addChair({chairX, 0.0f, chairZ}, chairYaw, h2 < 0.55f);
                }

                bool paperHallway = false;
                if (corridorEW) paperHallway = tileHash(x / 4, y, 26.0f) < paperHallwayChance;
                if (corridorNS) paperHallway = paperHallway || tileHash(x, y / 4, 27.0f) < paperHallwayChance;
                if (paperHallway) {
                    int count = std::max(1, static_cast<int>((11.0f + tileHash(x, y, 4.0f) * 18.0f) * hallwayPaperDensity));
                    addLoosePapers(t, count, true);
                } else if (h1 < loosePaperChance) {
                    int count = std::max(1, static_cast<int>((2.0f + tileHash(x, y, 4.0f) * 6.0f) * paperDensity));
                    addLoosePapers(t, count, false);
                }

                int ventSides[4]{};
                int ventSideCount = 0;
                if (!openN) ventSides[ventSideCount++] = 0;
                if (!openS) ventSides[ventSideCount++] = 1;
                if (!openW) ventSides[ventSideCount++] = 2;
                if (!openE) ventSides[ventSideCount++] = 3;
                if (ventSideCount > 0 && tileHash(x, y, 31.0f) < ventChance) {
                    int sideIndex = std::min(ventSideCount - 1, static_cast<int>(tileHash(x, y, 32.0f) * ventSideCount));
                    addWallVent(t, ventSides[sideIndex], tileHash(x, y, 33.0f));
                }

                if (h2 < waterDamageChance) {
                    int primarySide = std::min(3, static_cast<int>(tileHash(x, y, 30.0f) * 4.0f));
                    float floorSeed = tileHash(x, y, 21.0f);
                    float strength = 0.74f + tileHash(x, y, 34.0f) * 0.62f;
                    float ceilingSize = tileHash(x, y, 42.0f);
                    bool compactCeiling = ceilingSize < 0.35f;
                    bool spreadingCeiling = ceilingSize >= 0.58f;
                    float ceilingSeed = tileHash(x, y, 29.0f);
                    markWaterBlob(t, false, primarySide, 0, floorSeed, strength * 0.92f);
                    markWaterBlob(t, true, primarySide, compactCeiling ? 3 : 0, ceilingSeed, compactCeiling ? strength * 0.32f : strength);
                    if (tileHash(x, y, 35.0f) < 0.42f) {
                        addWaterHorizontalSpill(t, primarySide, false, tileHash(x, y, 35.0f), strength * 0.58f);
                    }
                    if (spreadingCeiling) {
                        addWaterHorizontalSpill(t, primarySide, true, tileHash(x, y, 36.0f), strength * (0.58f + tileHash(x, y, 37.0f) * 0.24f));
                    }
                    if (tileHash(x, y, 38.0f) < 0.36f) {
                        int secondarySide = (primarySide + 1 + std::min(2, static_cast<int>(tileHash(x, y, 39.0f) * 3.0f))) & 3;
                        bool secondaryCeiling = spreadingCeiling && tileHash(x, y, 40.0f) > 0.42f;
                        addWaterHorizontalSpill(t, secondarySide, secondaryCeiling, tileHash(x, y, 41.0f), strength * 0.72f);
                    }
                    int waterWallSides[4]{};
                    int waterWallSideCount = 0;
                    if (!openN) waterWallSides[waterWallSideCount++] = 0;
                    if (!openS) waterWallSides[waterWallSideCount++] = 1;
                    if (!openW) waterWallSides[waterWallSideCount++] = 2;
                    if (!openE) waterWallSides[waterWallSideCount++] = 3;
                    if (waterWallSideCount > 0) {
                        int wallSide = waterWallSides[std::min(waterWallSideCount - 1,
                            static_cast<int>(tileHash(x, y, 43.0f) * static_cast<float>(waterWallSideCount)))];
                        bool ceilingRun = spreadingCeiling || tileHash(x, y, 44.0f) < 0.58f;
                        addWaterHorizontalSpill(t, wallSide, ceilingRun, tileHash(x, y, 45.0f), strength * 0.94f);
                        if (!ceilingRun) {
                            addWaterHorizontalSpill(t, wallSide, false, tileHash(x, y, 47.0f), strength * 0.70f);
                        }
                    }
                }
            }
        }

        if (false && gEffectDebugViewer && DebugSliceEffectIsWater(gDebugSliceEffect)) {
            bool showFloorWater = gDebugSliceEffect == DebugSliceEffect::FloorWater ||
                gDebugSliceEffect == DebugSliceEffect::WallWater;
            bool showCeilingWater = gDebugSliceEffect == DebugSliceEffect::CeilingWater ||
                gDebugSliceEffect == DebugSliceEffect::WallWater;
            int debugCenterX = 1 + gDebugSliceTiles / 2;
            int debugCenterY = 1 + gDebugSliceTiles / 2;
            for (int y = 1; y < maze_.h - 1; ++y) {
                for (int x = 1; x < maze_.w - 1; ++x) {
                    if (!maze_.IsOpen(x, y)) continue;
                    Tile t{x, y};
                    bool openN = maze_.IsOpen(x, y - 1);
                    bool openS = maze_.IsOpen(x, y + 1);
                    bool openW = maze_.IsOpen(x - 1, y);
                    bool openE = maze_.IsOpen(x + 1, y);
                    int wallSide = !openN ? 0 : (!openW ? 2 : (!openE ? 3 : (!openS ? 1 : 0)));
                    float seed = tileHash(x, y, 140.0f);

                    if (gDebugSliceEffect == DebugSliceEffect::WallWater) {
                        if (!openN) {
                            addWaterHorizontalSpill(t, 0, true, seed + 0.01f, 1.26f);
                        }
                        if (!openW) {
                            addWaterHorizontalSpill(t, 2, true, seed + 0.03f, 1.18f);
                        }
                        if (!openE) {
                            addWaterHorizontalSpill(t, 3, true, seed + 0.05f, 1.14f);
                        }
                        continue;
                    }

                    if (x != debugCenterX || y != debugCenterY) continue;
                    if (showFloorWater) {
                        markWaterBlob(t, false, wallSide, 0, seed + 0.11f, 1.24f);
                    }
                    if (showCeilingWater) {
                        int mode = gDebugSliceTiles <= 1 ? 3 : 1;
                        markWaterBlob(t, true, wallSide, mode, seed + 0.21f, 1.28f);
                    }
                }
            }
        }

        auto addCeilingWaterBorderRunoff = [&](Tile t, int side, const WaterTileSurface& surface, float salt) {
            if (!surface.active || surface.mode == 3 || !wallHasWaterSurface(t, side)) return;
            XMFLOAT3 c = maze_.WorldCenter(t, 0.0f);
            float span = (side == 0 || side == 1) ? tileW : tileD;
            float seed = surface.seed + salt + static_cast<float>(side) * 0.071f;
            float h0 = LampHash(seed * 17.0f + c.x, c.z + static_cast<float>(side) * 3.1f);
            float h1 = LampHash(seed * 23.0f - c.z, c.x + static_cast<float>(side) * 5.7f);
            float h2 = LampHash(seed * 31.0f + c.x * 0.5f, c.z * 0.5f);
            float lateral = (h2 - 0.5f) * span * 0.48f;
            float wallW = span * (0.30f + h1 * 0.38f);
            float wallHgt = wallH * (0.52f + h0 * 0.46f);
            float yCenter = wallH - wallHgt * 0.5f - 0.0015f;
            addWaterWallCard(t, side, lateral, yCenter, wallW, wallHgt, true, seed + 0.47f);
        };

        for (int y = 1; y < maze_.h - 1; ++y) {
            for (int x = 1; x < maze_.w - 1; ++x) {
                Tile t{x, y};
                if (!maze_.IsOpen(x, y)) continue;
                const WaterTileSurface& ceilingSurface = ceilingWaterTiles[waterTileIndex(t)];
                if (!ceilingSurface.active || ceilingSurface.mode == 3) continue;

                if (ceilingSurface.mode >= 1) {
                    addCeilingWaterBorderRunoff(t, ceilingSurface.side, ceilingSurface, 0.0f);
                    continue;
                }

                for (int side = 0; side < 4; ++side) {
                    if (!wallHasWaterSurface(t, side)) continue;
                    float edgeBias = 0.18f + std::clamp(ceilingSurface.score - 1.0f, 0.0f, 0.45f);
                    if (!gEffectDebugViewer &&
                        LampHash(ceilingSurface.seed * 53.0f + static_cast<float>(side) * 11.0f,
                            static_cast<float>(x * 17 + y * 31)) > edgeBias) {
                        continue;
                    }
                    addCeilingWaterBorderRunoff(t, side, ceilingSurface, 0.19f + static_cast<float>(side) * 0.043f);
                }
            }
        }

        emitMergedWallWaterPools();

        for (int y = 1; y < maze_.h - 1; ++y) {
            for (int x = 1; x < maze_.w - 1; ++x) {
                Tile t{x, y};
                emitFloorWaterBridge(t, 1);
                emitFloorWaterBridge(t, 3);
            }
        }

        for (int y = 1; y < maze_.h - 1; ++y) {
            for (int x = 1; x < maze_.w - 1; ++x) {
                Tile t{x, y};
                if (!maze_.IsOpen(x, y)) continue;
                size_t idx = waterTileIndex(t);
                const WaterTileSurface& floorSurface = floorWaterTiles[idx];
                const WaterTileSurface& ceilingSurface = ceilingWaterTiles[idx];
                XMFLOAT3 center = maze_.WorldCenter(t, 0.0f);
                if (floorSurface.active) {
                    XMFLOAT3 source{center.x, 0.075f, center.z};
                    addWaterAttentionPoint(source, source, {0.0f, 1.0f, 0.0f},
                        maze_.TileAverage() * 1.12f, floorSurface.seed + 0.13f, false);
                }
                if (ceilingSurface.active) {
                    XMFLOAT3 source{center.x, wallH - 0.055f, center.z};
                    addWaterAttentionPoint(source, source, {0.0f, -1.0f, 0.0f},
                        maze_.TileAverage() * 1.18f, ceilingSurface.seed + 0.31f, false);
                }
                emitWaterTileCard(t, false, floorWaterTiles[idx]);
                emitWaterTileCard(t, true, ceilingWaterTiles[idx]);
            }
        }

        std::vector<Tile> openTiles;
        openTiles.reserve(static_cast<size_t>(maze_.w * maze_.h));
        for (int y = 1; y < maze_.h - 1; ++y) {
            for (int x = 1; x < maze_.w - 1; ++x) {
                Tile t{x, y};
                if (maze_.IsOpen(x, y) && (gEffectDebugViewer || (!(t == maze_.start) && !(t == maze_.exit)))) {
                    openTiles.push_back(t);
                }
            }
        }

        if (!openTiles.empty()) {
            uint32_t scatterSeed = runtimeSeed_ ^ 0x61c88647u;

            float cabinetDensity = std::clamp(settings_.metalCabinetDensity, 0.0f, 4.0f);
            int cabinetTarget = cabinetDensity <= 0.001f
                ? 0
                : std::clamp(static_cast<int>(std::round(static_cast<float>(openTiles.size()) * 0.014f * cabinetDensity)), 2, 76);
            int placedCabinets = 0;
            int cabinetAttempts = cabinetTarget * 10;
            for (int cidx = 0; cidx < cabinetAttempts && placedCabinets < cabinetTarget; ++cidx) {
                size_t tileIndex = std::min(openTiles.size() - 1,
                    static_cast<size_t>(Rand01(cidx, 971, scatterSeed) * static_cast<float>(openTiles.size())));
                Tile t = openTiles[tileIndex];
                if (t == maze_.start || t == maze_.exit) continue;
                int sides[4]{};
                int sideCount = 0;
                if (!maze_.IsOpen(t.x, t.y - 1)) sides[sideCount++] = 0;
                if (!maze_.IsOpen(t.x, t.y + 1)) sides[sideCount++] = 1;
                if (!maze_.IsOpen(t.x - 1, t.y)) sides[sideCount++] = 2;
                if (!maze_.IsOpen(t.x + 1, t.y)) sides[sideCount++] = 3;
                if (sideCount == 0) continue;
                int side = sides[std::min(sideCount - 1, static_cast<int>(Rand01(cidx, 977, scatterSeed) * static_cast<float>(sideCount)))];
                if (addMetalCabinetAgainstWall(t, side, Rand01(cidx, 983, scatterSeed))) {
                    ++placedCabinets;
                }
            }

            bool emitWaterLiquid = false;
            auto addBloodScare = [&](XMFLOAT3 pos, float span) {
                if (span < 0.56f) return;
                BloodScarePoint p{};
                p.pos = pos;
                p.source = pos.y < 0.35f
                    ? XMFLOAT3{pos.x, 0.18f, pos.z}
                    : XMFLOAT3{pos.x, std::clamp(pos.y, 0.18f, wallH - 0.055f), pos.z};
                p.radius = std::clamp(1.25f + span * 0.72f, 1.75f, 4.35f);
                p.focusDelaySeconds = 0.26f + LampHash(pos.x * 3.1f + span, pos.z * 5.7f) * 0.58f;
                p.waterLiquid = emitWaterLiquid;
                if (emitWaterLiquid) {
                    p.dreadScale = 0.30f;
                }
                bloodScarePoints_.push_back(p);
            };

            std::unordered_map<int, int> bloodCeilingLayers;
            std::unordered_set<int> bloodCenterSeepCovered;
            std::vector<FloorFootprint> bloodCeilingReservations;
            constexpr float kLiquidFloorReservationPad = 0.010f;
            auto liquidMaterial = [&](float rawSeed) {
                return (emitWaterLiquid ? 25.0f : 14.0f) + rawSeed;
            };
            auto bloodTileKey = [&](Tile tile) {
                return tile.y * std::max(1, maze_.w) + tile.x;
            };
            std::unordered_set<int> waterDamageTiles;
            auto waterDamageTileBlocked = [&](Tile tile) {
                return !gEffectDebugViewer &&
                    waterDamageTiles.find(bloodTileKey(tile)) != waterDamageTiles.end();
            };
            auto reserveWaterDamageTile = [&](Tile tile) {
                if (maze_.IsOpen(tile.x, tile.y)) waterDamageTiles.insert(bloodTileKey(tile));
            };
            auto reserveWaterDamageCoveredTile = [&](Tile tile) {
                if (!maze_.IsOpen(tile.x, tile.y)) return;
                waterDamageTiles.insert(bloodTileKey(tile));
                bloodCenterSeepCovered.insert(bloodTileKey(tile));
            };
            auto bloodCeilingFootprintClear = [&](float px, float pz, float width, float depth, float yaw, float pad = 0.012f) {
                if (!footprintFitsMaze(px, pz, width, depth, yaw, pad)) return false;
                FloorFootprint candidate = makeFootprint(px, pz, width, depth, yaw, pad);
                for (const FloorFootprint& reserved : bloodCeilingReservations) {
                    if (footprintOverlap(candidate, reserved)) return false;
                }
                return true;
            };
            auto reserveBloodCeilingFootprint = [&](float px, float pz, float width, float depth, float yaw, float pad = 0.012f) {
                if (!bloodCeilingFootprintClear(px, pz, width, depth, yaw, pad)) return false;
                bloodCeilingReservations.push_back(makeFootprint(px, pz, width, depth, yaw, pad));
                return true;
            };
            auto nextBloodCeilingY = [&](float px, float pz, float seed) {
                Tile tile = maze_.TileFromWorld(px, pz);
                int key = bloodTileKey(tile);
                int& layer = bloodCeilingLayers[key];
                ++layer;
                return wallH - kBloodCeilingDecalInset;
            };

            struct LiquidCanvasSurface {
                bool active = false;
                uint32_t sourceMask = 0;
                bool centerSource = false;
                bool downstream = false;
                float seed = 0.0f;
                float score = -1.0e9f;
            };
            std::vector<LiquidCanvasSurface> floorBloodCanvas(static_cast<size_t>(maze_.w * maze_.h));
            std::vector<LiquidCanvasSurface> ceilingBloodCanvas(static_cast<size_t>(maze_.w * maze_.h));
            std::vector<LiquidCanvasSurface> floorWaterCanvas(static_cast<size_t>(maze_.w * maze_.h));
            std::vector<LiquidCanvasSurface> ceilingWaterCanvas(static_cast<size_t>(maze_.w * maze_.h));
            struct WallLiquidCanvasSurface {
                bool active = false;
                float minAlong = 0.0f;
                float maxAlong = 0.0f;
                float seed = 0.0f;
                float score = -1.0e9f;
            };
            std::vector<WallLiquidCanvasSurface> wallWaterCanvas(static_cast<size_t>(maze_.w * maze_.h * 4));

            auto liquidCanvasVector = [&](bool water, bool ceiling) -> std::vector<LiquidCanvasSurface>& {
                if (water) return ceiling ? ceilingWaterCanvas : floorWaterCanvas;
                return ceiling ? ceilingBloodCanvas : floorBloodCanvas;
            };
            auto markLiquidCanvasTile = [&](Tile tile, bool water, bool ceiling, uint32_t sourceMask,
                                            bool centerSource, bool downstream, float seed, float score) {
                if (!maze_.IsOpen(tile.x, tile.y) || (!gEffectDebugViewer && (tile == maze_.start || tile == maze_.exit))) return false;
                size_t idx = static_cast<size_t>(tile.y * maze_.w + tile.x);
                std::vector<LiquidCanvasSurface>& canvas = liquidCanvasVector(water, ceiling);
                if (idx >= canvas.size()) return false;
                LiquidCanvasSurface& surface = canvas[idx];
                surface.active = true;
                surface.sourceMask |= sourceMask & 0x0fu;
                surface.centerSource = surface.centerSource || centerSource;
                surface.downstream = surface.downstream || downstream;
                if (score >= surface.score) {
                    surface.seed = seed;
                    surface.score = score;
                }
                if (ceiling) {
                    MarkWetCeilingTile(tile);
                } else {
                    MarkWetFootstepTile(tile);
                }
                return true;
            };
            auto markLiquidCanvasArea = [&](float px, float pz, float width, float depth, float yaw,
                                            bool water, bool ceiling, uint32_t sourceMask, bool centerSource,
                                            float seed, float score, bool downstream = false) {
                if (width <= 0.02f || depth <= 0.02f) return false;
                FloorFootprint area = makeFootprint(px, pz, width, depth, yaw, 0.0f);
                Tile sourceTile = maze_.TileFromWorld(px, pz);
                float cYaw = std::cos(yaw);
                float sYaw = std::sin(yaw);
                XMFLOAT3 right{cYaw, 0.0f, -sYaw};
                XMFLOAT3 forward{sYaw, 0.0f, cYaw};
                float minX = std::numeric_limits<float>::max();
                float maxX = -std::numeric_limits<float>::max();
                float minZ = std::numeric_limits<float>::max();
                float maxZ = -std::numeric_limits<float>::max();
                const float xs[] = {-width * 0.5f, width * 0.5f};
                const float zs[] = {-depth * 0.5f, depth * 0.5f};
                for (float lx : xs) {
                    for (float lz : zs) {
                        XMFLOAT3 p = Add3({px, 0.0f, pz}, OrientedOffset(right, {0.0f, 1.0f, 0.0f}, forward, lx, 0.0f, lz));
                        minX = std::min(minX, p.x);
                        maxX = std::max(maxX, p.x);
                        minZ = std::min(minZ, p.z);
                        maxZ = std::max(maxZ, p.z);
                    }
                }
                int x0 = std::clamp(static_cast<int>(std::floor((minX - ox) / tileW)) - 1, 0, std::max(0, maze_.w - 1));
                int x1 = std::clamp(static_cast<int>(std::floor((maxX - ox) / tileW)) + 1, 0, std::max(0, maze_.w - 1));
                int y0 = std::clamp(static_cast<int>(std::floor((minZ - oz) / tileD)) - 1, 0, std::max(0, maze_.h - 1));
                int y1 = std::clamp(static_cast<int>(std::floor((maxZ - oz) / tileD)) + 1, 0, std::max(0, maze_.h - 1));
                bool marked = false;
                for (int ty = y0; ty <= y1; ++ty) {
                    for (int tx = x0; tx <= x1; ++tx) {
                        Tile tile{tx, ty};
                        if (!maze_.IsOpen(tx, ty)) continue;
                        XMFLOAT3 tc = maze_.WorldCenter(tile, 0.0f);
                        FloorFootprint tileArea = makeFootprint(tc.x, tc.z, tileW * 1.002f, tileD * 1.002f, 0.0f, 0.0f);
                        if (!footprintOverlap(area, tileArea)) continue;
                        float tileMinX = ox + static_cast<float>(tx) * tileW;
                        float tileMaxX = tileMinX + tileW;
                        float tileMinZ = oz + static_cast<float>(ty) * tileD;
                        float tileMaxZ = tileMinZ + tileD;
                        uint32_t tileMask = sourceMask;
                        if (!water) {
                            float edgePadX = tileW * 0.020f;
                            float edgePadZ = tileD * 0.020f;
                            if (minZ <= tileMinZ + edgePadZ) tileMask |= 1u << 0;
                            if (maxZ >= tileMaxZ - edgePadZ) tileMask |= 1u << 1;
                            if (minX <= tileMinX + edgePadX) tileMask |= 1u << 2;
                            if (maxX >= tileMaxX - edgePadX) tileMask |= 1u << 3;
                        }
                        bool tileCenterSource = centerSource && tile == sourceTile;
                        marked = markLiquidCanvasTile(tile, water, ceiling, tileMask, tileCenterSource, downstream, seed, score) || marked;
                    }
                }
                return marked;
            };
            auto liquidCanvasMaterial = [](bool water, float seed) {
                float h = std::fmod(std::abs(seed) * 37.719f + 0.137f, 1.0f);
                return (water ? 25.0f : 14.0f) + 0.990f + h * 0.0085f;
            };
            auto emitLiquidCanvasTiles = [&]() {
                auto emitSet = [&](std::vector<LiquidCanvasSurface>& canvas, bool water, bool ceiling) {
                    for (int ty = 0; ty < maze_.h; ++ty) {
                        for (int tx = 0; tx < maze_.w; ++tx) {
                            size_t idx = static_cast<size_t>(ty * maze_.w + tx);
                            if (idx >= canvas.size()) continue;
                            const LiquidCanvasSurface& surface = canvas[idx];
                            if (!surface.active || !maze_.IsOpen(tx, ty)) continue;
                            float l = ox + static_cast<float>(tx) * tileW;
                            float r = l + tileW;
                            float z0 = oz + static_cast<float>(ty) * tileD;
                            float z1 = z0 + tileD;
                            float y = ceiling ? wallH - kBloodCeilingDecalInset : kBloodFloorDecalLift;
                            uint32_t code = (surface.sourceMask & 0x0fu) | (surface.centerSource ? 0x10u : 0u);
                            auto edgeContinues = [&](int nx, int ny) {
                                if (!maze_.IsOpen(nx, ny)) return true;
                                size_t nidx = static_cast<size_t>(ny * maze_.w + nx);
                                return nidx < canvas.size() && canvas[nidx].active;
                            };
                            uint32_t continueMask = 0;
                            if (edgeContinues(tx, ty - 1)) continueMask |= 1u << 0;
                            if (edgeContinues(tx, ty + 1)) continueMask |= 1u << 1;
                            if (edgeContinues(tx - 1, ty)) continueMask |= 1u << 2;
                            if (edgeContinues(tx + 1, ty)) continueMask |= 1u << 3;
                            float ux = static_cast<float>(code);
                            float vy = static_cast<float>(continueMask | ((water && surface.downstream) ? 0x10u : 0u));
                            float u0 = ux + 0.001f;
                            float u1 = ux + 0.999f;
                            float v0 = vy + 0.001f;
                            float v1 = vy + 0.999f;
                            if (water && !ceiling) y = kWaterFloorLift + 0.0015f;
                            if (ceiling) {
                                AddQuadUV(vertices, liquidIndices,
                                    {l, y, z0}, {r, y, z0}, {r, y, z1}, {l, y, z1},
                                    {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f},
                                    {u0, v0}, {u1, v0}, {u1, v1}, {u0, v1},
                                    liquidCanvasMaterial(water, surface.seed));
                            } else {
                                AddQuadUV(vertices, liquidIndices,
                                    {l, y, z1}, {r, y, z1}, {r, y, z0}, {l, y, z0},
                                    {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f},
                                    {u0, v1}, {u1, v1}, {u1, v0}, {u0, v0},
                                    liquidCanvasMaterial(water, surface.seed));
                            }
                        }
                    }
                };
                emitSet(floorBloodCanvas, false, false);
                emitSet(ceilingBloodCanvas, false, true);
                emitSet(floorWaterCanvas, true, false);
                emitSet(ceilingWaterCanvas, true, true);
            };

            auto addBloodFloor = [&](float px, float pz, float w, float d, float yaw, float seed, float layerLift) {
                if (!longFloorFootprintClear(px, pz, w, d, yaw, kLiquidFloorReservationPad)) {
                    w *= 0.62f;
                    d *= 0.62f;
                    if (!floorFootprintClear(px, pz, w, d, yaw, kLiquidFloorReservationPad)) return false;
                }
                if (!reserveFloorFootprint(px, pz, w, d, yaw, kLiquidFloorReservationPad)) return false;
                markLiquidCanvasArea(px, pz, w, d, yaw, emitWaterLiquid, false, 0u, true, seed, std::max(w, d));
                MarkWetFootstepArea(px, pz, w, d, yaw);
                addBloodScare({px, 0.10f, pz}, std::max(w, d));
                return true;
            };

            auto addWaterCeilingDripFloorResponse = [&](float px, float pz, float w, float d, float yaw, float seed) {
                if (!emitWaterLiquid) return;
                float floorW = w * (0.46f + Rand01(static_cast<int>(seed * 10000.0f), 817, scatterSeed) * 0.18f);
                float floorD = d * (0.46f + Rand01(static_cast<int>(seed * 10000.0f), 823, scatterSeed) * 0.18f);
                float offsetX = (Rand01(static_cast<int>(seed * 10000.0f), 829, scatterSeed) - 0.5f) * std::min(tileW, floorW) * 0.18f;
                float offsetZ = (Rand01(static_cast<int>(seed * 10000.0f), 839, scatterSeed) - 0.5f) * std::min(tileD, floorD) * 0.18f;
                float floorX = px + offsetX;
                float floorZ = pz + offsetZ;
                if (!footprintFitsMaze(floorX, floorZ, floorW, floorD, yaw, kLiquidFloorReservationPad)) return;
                markLiquidCanvasArea(floorX, floorZ, floorW, floorD, yaw,
                    true, false, 0u, true, seed + 0.19f, std::max(floorW, floorD));
                MarkWetFootstepArea(floorX, floorZ, floorW, floorD, yaw, 0.01f, 9.0f);
                MarkWetCeilingDripEmitter({floorX, 0.10f, floorZ}, seed);
                addBloodScare({floorX, 0.10f, floorZ}, std::max(floorW, floorD));
            };

            auto addBloodCeiling = [&](float px, float pz, float w, float d, float yaw, float seed) {
                if (!longFloorFootprintClear(px, pz, w, d, yaw)) {
                    w *= 0.58f;
                    d *= 0.58f;
                    if (!floorFootprintClear(px, pz, w, d, yaw)) return false;
                }
                if (!reserveBloodCeilingFootprint(px, pz, w, d, yaw)) return false;
                markLiquidCanvasArea(px, pz, w, d, yaw, emitWaterLiquid, true, 0u, true, seed, std::max(w, d));
                addWaterCeilingDripFloorResponse(px, pz, w, d, yaw, seed);
                addBloodScare({px, wallH - 0.08f, pz}, std::max(w, d));
                return true;
            };

            auto addLiquidCeilingOverlay = [&](float px, float pz, float w, float d, float yaw, float seed, float rawSeed) {
                if (!footprintFitsMaze(px, pz, w, d, yaw, 0.010f)) return false;
                bloodCeilingReservations.push_back(makeFootprint(px, pz, w, d, yaw, 0.002f));
                bool fromWall = rawSeed >= 0.96f;
                markLiquidCanvasArea(px, pz, w, d, yaw, emitWaterLiquid, true, 0u, !fromWall, seed, std::max(w, d));
                addWaterCeilingDripFloorResponse(px, pz, w, d, yaw, seed);
                addBloodScare({px, wallH - 0.08f, pz}, std::max(w, d));
                return true;
            };

            auto addLiquidFloorOverlay = [&](float px, float pz, float w, float d, float yaw, float seed, float layerLift, float rawSeed) {
                if (!footprintFitsMaze(px, pz, w, d, yaw, kLiquidFloorReservationPad)) return false;
                floorReservations.push_back(makeFootprint(px, pz, w, d, yaw, 0.002f));
                bool fromWall = rawSeed >= 0.96f;
                markLiquidCanvasArea(px, pz, w, d, yaw, emitWaterLiquid, false, 0u, !fromWall, seed, std::max(w, d));
                MarkWetFootstepArea(px, pz, w, d, yaw);
                addBloodScare({px, 0.10f, pz}, std::max(w, d));
                return true;
            };

            auto addCenterSeepFloor = [&](float px, float pz, float w, float d, float yaw, float seed, float layerLift) {
                if (!reserveFloorFootprint(px, pz, w, d, yaw, kLiquidFloorReservationPad)) return false;
                markLiquidCanvasArea(px, pz, w, d, yaw, emitWaterLiquid, false, 0u, true, seed, std::max(w, d));
                MarkWetFootstepArea(px, pz, w, d, yaw);
                addBloodScare({px, 0.10f, pz}, std::max(w, d));
                return true;
            };

            auto addCenterSeepCeiling = [&](float px, float pz, float w, float d, float yaw, float seed) {
                if (!reserveBloodCeilingFootprint(px, pz, w, d, yaw, 0.010f)) return false;
                markLiquidCanvasArea(px, pz, w, d, yaw, emitWaterLiquid, true, 0u, true, seed, std::max(w, d));
                addWaterCeilingDripFloorResponse(px, pz, w, d, yaw, seed);
                addBloodScare({px, wallH - 0.08f, pz}, std::max(w, d));
                return true;
            };

            auto addBloodCeilingFloorPair = [&](float px, float pz, float w, float d, float yaw, float seed, float floorScale = 0.92f) {
                float floorW = w * floorScale;
                float floorD = d * floorScale;
                if (!floorFootprintClear(px, pz, floorW, floorD, yaw, kLiquidFloorReservationPad)) return false;
                if (!bloodCeilingFootprintClear(px, pz, w, d, yaw, 0.010f)) return false;
                floorReservations.push_back(makeFootprint(px, pz, floorW, floorD, yaw, kLiquidFloorReservationPad));
                bloodCeilingReservations.push_back(makeFootprint(px, pz, w, d, yaw, 0.010f));
                markLiquidCanvasArea(px, pz, floorW, floorD, yaw, emitWaterLiquid, false, 0u, true, seed, std::max(floorW, floorD));
                MarkWetFootstepArea(px, pz, floorW, floorD, yaw);
                MarkWetCeilingDripEmitter({px, 0.10f, pz}, seed);
                markLiquidCanvasArea(px, pz, w, d, yaw, emitWaterLiquid, true, 0u, true, seed, std::max(w, d));
                addBloodScare({px, 0.10f, pz}, std::max(floorW, floorD));
                addBloodScare({px, wallH - 0.08f, pz}, std::max(w, d));
                return true;
            };

            auto wallHasSurface = [&](Tile tile, int side) {
                if (!maze_.IsOpen(tile.x, tile.y)) return false;
                if (side == 0) return !maze_.IsOpen(tile.x, tile.y - 1);
                if (side == 1) return !maze_.IsOpen(tile.x, tile.y + 1);
                if (side == 2) return !maze_.IsOpen(tile.x - 1, tile.y);
                return !maze_.IsOpen(tile.x + 1, tile.y);
            };
            auto wallWaterCanvasIndex = [&](Tile tile, int side) {
                return static_cast<size_t>((tile.y * maze_.w + tile.x) * 4 + side);
            };
            auto markWaterWallCanvas = [&](Tile tile, int side, float centerAlong, float width, float seed, float score) {
                if (!wallHasSurface(tile, side) || (!gEffectDebugViewer && (tile == maze_.start || tile == maze_.exit))) return false;
                if (side < 0 || side > 3 || width <= 0.025f) return false;
                float tileMin = 0.0f;
                float tileMax = 0.0f;
                if (side == 0 || side == 1) {
                    tileMin = ox + static_cast<float>(tile.x) * tileW;
                    tileMax = tileMin + tileW;
                } else {
                    tileMin = oz + static_cast<float>(tile.y) * tileD;
                    tileMax = tileMin + tileD;
                }
                float half = width * 0.5f;
                float minAlong = std::clamp(centerAlong - half, tileMin, tileMax);
                float maxAlong = std::clamp(centerAlong + half, tileMin, tileMax);
                if (maxAlong - minAlong < 0.035f) return false;
                size_t idx = wallWaterCanvasIndex(tile, side);
                if (idx >= wallWaterCanvas.size()) return false;
                WallLiquidCanvasSurface& surface = wallWaterCanvas[idx];
                if (!surface.active) {
                    surface.active = true;
                    surface.minAlong = minAlong;
                    surface.maxAlong = maxAlong;
                    surface.seed = seed;
                    surface.score = score;
                    return true;
                }
                surface.minAlong = std::min(surface.minAlong, minAlong);
                surface.maxAlong = std::max(surface.maxAlong, maxAlong);
                if (score >= surface.score) {
                    surface.seed = seed;
                    surface.score = score;
                }
                return true;
            };
            auto emitWaterWallCanvasRuns = [&]() {
                constexpr float kWaterWallCanvasInset = 0.0062f;
                auto waterWallCanvasMaterial = [](float seed) {
                    return 25.965f + std::fmod(std::abs(seed), 1.0f) * 0.0245f;
                };
                auto surfaceAt = [&](int x, int y, int side) -> WallLiquidCanvasSurface* {
                    if (x < 0 || y < 0 || x >= maze_.w || y >= maze_.h) return nullptr;
                    size_t idx = static_cast<size_t>((y * maze_.w + x) * 4 + side);
                    if (idx >= wallWaterCanvas.size() || !wallWaterCanvas[idx].active) return nullptr;
                    return &wallWaterCanvas[idx];
                };
                auto emitRun = [&](int side, int fixed, int start, int end) {
                    float minAlong = std::numeric_limits<float>::max();
                    float maxAlong = -std::numeric_limits<float>::max();
                    float seedSum = 0.0f;
                    int count = 0;
                    for (int i = start; i <= end; ++i) {
                        WallLiquidCanvasSurface* surface = (side == 0 || side == 1)
                            ? surfaceAt(i, fixed, side)
                            : surfaceAt(fixed, i, side);
                        if (!surface) continue;
                        minAlong = std::min(minAlong, surface->minAlong);
                        maxAlong = std::max(maxAlong, surface->maxAlong);
                        seedSum += surface->seed;
                        ++count;
                    }
                    if (count <= 0 || maxAlong - minAlong < 0.035f) return;

                    XMFLOAT3 normal{0.0f, 0.0f, 1.0f};
                    XMFLOAT3 right{1.0f, 0.0f, 0.0f};
                    XMFLOAT3 center{0.0f, wallH * 0.5f, 0.0f};
                    if (side == 0) {
                        normal = {0.0f, 0.0f, 1.0f};
                        right = {1.0f, 0.0f, 0.0f};
                        center = {(minAlong + maxAlong) * 0.5f, wallH * 0.5f,
                            oz + static_cast<float>(fixed) * tileD + kWaterWallCanvasInset};
                    } else if (side == 1) {
                        normal = {0.0f, 0.0f, -1.0f};
                        right = {-1.0f, 0.0f, 0.0f};
                        center = {(minAlong + maxAlong) * 0.5f, wallH * 0.5f,
                            oz + static_cast<float>(fixed + 1) * tileD - kWaterWallCanvasInset};
                    } else if (side == 2) {
                        normal = {1.0f, 0.0f, 0.0f};
                        right = {0.0f, 0.0f, 1.0f};
                        center = {ox + static_cast<float>(fixed) * tileW + kWaterWallCanvasInset,
                            wallH * 0.5f, (minAlong + maxAlong) * 0.5f};
                    } else {
                        normal = {-1.0f, 0.0f, 0.0f};
                        right = {0.0f, 0.0f, -1.0f};
                        center = {ox + static_cast<float>(fixed + 1) * tileW - kWaterWallCanvasInset,
                            wallH * 0.5f, (minAlong + maxAlong) * 0.5f};
                    }

                    XMFLOAT3 up{0.0f, 1.0f, 0.0f};
                    float width = maxAlong - minAlong;
                    float height = wallH - 0.003f;
                    center.y = 0.0015f + height * 0.5f;
                    XMFLOAT3 a = Add3(center, Add3(Scale3(right, -width * 0.5f), Scale3(up, -height * 0.5f)));
                    XMFLOAT3 b = Add3(center, Add3(Scale3(right,  width * 0.5f), Scale3(up, -height * 0.5f)));
                    XMFLOAT3 c0 = Add3(center, Add3(Scale3(right,  width * 0.5f), Scale3(up,  height * 0.5f)));
                    XMFLOAT3 d0 = Add3(center, Add3(Scale3(right, -width * 0.5f), Scale3(up,  height * 0.5f)));
                    AddQuadUV(vertices, liquidIndices, a, b, c0, d0, normal, right,
                        {0.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f},
                        waterWallCanvasMaterial(seedSum / static_cast<float>(std::max(1, count))));
                };

                for (int side = 0; side < 2; ++side) {
                    for (int y = 0; y < maze_.h; ++y) {
                        int x = 0;
                        while (x < maze_.w) {
                            while (x < maze_.w && !surfaceAt(x, y, side)) ++x;
                            int start = x;
                            while (x < maze_.w && surfaceAt(x, y, side)) ++x;
                            if (start < x) emitRun(side, y, start, x - 1);
                        }
                    }
                }
                for (int side = 2; side < 4; ++side) {
                    for (int x = 0; x < maze_.w; ++x) {
                        int y = 0;
                        while (y < maze_.h) {
                            while (y < maze_.h && !surfaceAt(x, y, side)) ++y;
                            int start = y;
                            while (y < maze_.h && surfaceAt(x, y, side)) ++y;
                            if (start < y) emitRun(side, x, start, y - 1);
                        }
                    }
                }
            };

            auto wallSupportSpan = [&](Tile t, int side, float& minAlong, float& maxAlong) {
                if (!wallHasSurface(t, side)) return false;
                if (side == 0 || side == 1) {
                    int x0 = t.x;
                    int x1 = t.x;
                    while (wallHasSurface({x0 - 1, t.y}, side)) --x0;
                    while (wallHasSurface({x1 + 1, t.y}, side)) ++x1;
                    minAlong = ox + static_cast<float>(x0) * tileW;
                    maxAlong = ox + static_cast<float>(x1 + 1) * tileW;
                } else {
                    int y0 = t.y;
                    int y1 = t.y;
                    while (wallHasSurface({t.x, y0 - 1}, side)) --y0;
                    while (wallHasSurface({t.x, y1 + 1}, side)) ++y1;
                    minAlong = oz + static_cast<float>(y0) * tileD;
                    maxAlong = oz + static_cast<float>(y1 + 1) * tileD;
                }
                return maxAlong - minAlong > 0.20f;
            };

            auto addBloodWall = [&](Tile t, int side, float lateral, float yCenter, float w, float h, float seed) {
                XMFLOAT3 c = maze_.WorldCenter(t, 0.0f);
                XMFLOAT3 normal{0.0f, 0.0f, 1.0f};
                XMFLOAT3 right{1.0f, 0.0f, 0.0f};
                XMFLOAT3 center{c.x, yCenter, c.z};
                float minAlong = 0.0f;
                float maxAlong = 0.0f;
                if (!wallSupportSpan(t, side, minAlong, maxAlong)) return false;
                constexpr float kBloodWallDecalInset = 0.0050f;
                constexpr float wallDecalMargin = 0.13f;
                minAlong += wallDecalMargin;
                maxAlong -= wallDecalMargin;
                float available = maxAlong - minAlong;
                if (available < 0.28f) return false;
                w = std::min(w, available);
                float halfW = w * 0.5f;
                float desiredAlong = (side == 0 || side == 1) ? c.x + lateral : c.z + lateral;
                float clampedAlong = std::clamp(desiredAlong, minAlong + halfW, maxAlong - halfW);
                if (side == 0) {
                    normal = {0.0f, 0.0f, 1.0f};
                    right = {1.0f, 0.0f, 0.0f};
                    center = {clampedAlong, yCenter, c.z - tileD * 0.5f + kBloodWallDecalInset};
                } else if (side == 1) {
                    normal = {0.0f, 0.0f, -1.0f};
                    right = {-1.0f, 0.0f, 0.0f};
                    center = {clampedAlong, yCenter, c.z + tileD * 0.5f - kBloodWallDecalInset};
                } else if (side == 2) {
                    normal = {1.0f, 0.0f, 0.0f};
                    right = {0.0f, 0.0f, 1.0f};
                    center = {c.x - tileW * 0.5f + kBloodWallDecalInset, yCenter, clampedAlong};
                } else {
                    normal = {-1.0f, 0.0f, 0.0f};
                    right = {0.0f, 0.0f, -1.0f};
                    center = {c.x + tileW * 0.5f - kBloodWallDecalInset, yCenter, clampedAlong};
                }
                center = Add3(center, Scale3(normal, 0.0008f + std::fmod(std::abs(seed) * 9.713f, 1.0f) * 0.0018f));
                XMFLOAT3 up{0.0f, 1.0f, 0.0f};
                constexpr float wallBloodFloorMargin = 0.002f;
                constexpr float wallBloodCeilingMargin = 0.004f;
                h = wallH - wallBloodFloorMargin - wallBloodCeilingMargin;
                center.y = wallBloodFloorMargin + h * 0.5f;
                XMFLOAT3 a = Add3(center, Add3(Scale3(right, -w * 0.5f), Scale3(up, -h * 0.5f)));
                XMFLOAT3 b = Add3(center, Add3(Scale3(right,  w * 0.5f), Scale3(up, -h * 0.5f)));
                XMFLOAT3 c0 = Add3(center, Add3(Scale3(right,  w * 0.5f), Scale3(up,  h * 0.5f)));
                XMFLOAT3 d0 = Add3(center, Add3(Scale3(right, -w * 0.5f), Scale3(up,  h * 0.5f)));
                AddQuadUV(vertices, liquidIndices, a, b, c0, d0, normal, right, {0, 1}, {1, 1}, {1, 0}, {0, 0}, liquidMaterial(0.11f + std::fmod(seed, 0.83f)));
                float bottomY = center.y - h * 0.5f;
                if (bottomY > wallBloodFloorMargin + 0.045f) {
                    int dripStrips = 1 + static_cast<int>(LampHash(seed * 41.0f + center.x, center.z - seed * 13.0f) * 3.0f);
                    for (int strip = 0; strip < dripStrips; ++strip) {
                        float r0 = LampHash(seed * 53.0f + static_cast<float>(strip) * 7.1f, center.x + center.z);
                        float r1 = LampHash(seed * 67.0f + static_cast<float>(strip) * 11.3f, center.z - center.x);
                        float stripW = std::min(w * (0.10f + r0 * 0.18f), 0.34f);
                        float offset = (r1 - 0.5f) * std::max(0.0f, w - stripW) * 0.62f;
                        float bridgeTop = std::min(bottomY + 0.12f + r0 * 0.10f, wallH - wallBloodCeilingMargin);
                        float bridgeBottom = wallBloodFloorMargin;
                        float bridgeH = bridgeTop - bridgeBottom;
                        if (bridgeH <= 0.035f) continue;
                        XMFLOAT3 bridgeCenter = Add3(center, Add3(Scale3(right, offset), {0.0f, (bridgeTop + bridgeBottom) * 0.5f - center.y, 0.0f}));
                        XMFLOAT3 ba = Add3(bridgeCenter, Add3(Scale3(right, -stripW * 0.5f), Scale3(up, -bridgeH * 0.5f)));
                        XMFLOAT3 bb = Add3(bridgeCenter, Add3(Scale3(right,  stripW * 0.5f), Scale3(up, -bridgeH * 0.5f)));
                        XMFLOAT3 bc = Add3(bridgeCenter, Add3(Scale3(right,  stripW * 0.5f), Scale3(up,  bridgeH * 0.5f)));
                        XMFLOAT3 bd = Add3(bridgeCenter, Add3(Scale3(right, -stripW * 0.5f), Scale3(up,  bridgeH * 0.5f)));
                        AddQuadUV(vertices, liquidIndices, ba, bb, bc, bd, normal, right, {0, 1}, {1, 1}, {1, 0}, {0, 0},
                            liquidMaterial(0.37f + std::fmod(seed + r0 * 0.61f + static_cast<float>(strip) * 0.17f, 0.51f)));
                    }
                }
                BloodScarePoint scare{};
                scare.pos = center;
                float sourceY = center.y + h * 0.40f;
                float topY = center.y + h * 0.5f;
                if (topY > wallH * 0.78f) {
                    sourceY = std::max(sourceY, topY - 0.035f);
                }
                scare.source = {center.x, std::clamp(sourceY, 0.18f, wallH - 0.055f), center.z};
                scare.normal = normal;
                scare.radius = std::clamp(1.25f + std::max(w, h) * 0.72f, 1.75f, 4.35f);
                scare.focusDelaySeconds = 0.30f + LampHash(seed * 43.0f + center.x, center.z) * 0.64f;
                scare.requireFacing = true;
                scare.waterLiquid = emitWaterLiquid;
                if (emitWaterLiquid) {
                    scare.dreadScale = 0.30f;
                }
                bloodScarePoints_.push_back(scare);
                return true;
            };

            auto neighborForSide = [](Tile t, int side) {
                if (side == 0) return Tile{t.x, t.y - 1};
                if (side == 1) return Tile{t.x, t.y + 1};
                if (side == 2) return Tile{t.x - 1, t.y};
                return Tile{t.x + 1, t.y};
            };

            auto oppositeSide = [](int side) {
                if (side == 0) return 1;
                if (side == 1) return 0;
                if (side == 2) return 3;
                return 2;
            };

            auto sideForwardYaw = [](int side) {
                if (side == 0) return kPi;
                if (side == 1) return 0.0f;
                if (side == 2) return -kPi * 0.5f;
                return kPi * 0.5f;
            };

            auto liquidCardYawForSide = [](int side) {
                if (side == 0) return 0.0f;
                if (side == 1) return kPi;
                if (side == 2) return kPi * 0.5f;
                return -kPi * 0.5f;
            };

            auto sideDirection = [](int side) {
                if (side == 0) return XMFLOAT3{0.0f, 0.0f, -1.0f};
                if (side == 1) return XMFLOAT3{0.0f, 0.0f, 1.0f};
                if (side == 2) return XMFLOAT3{-1.0f, 0.0f, 0.0f};
                return XMFLOAT3{1.0f, 0.0f, 0.0f};
            };

            auto addBloodCeilingWallRunoffs = [&](Tile t, float px, float pz, float w, float d, float yaw,
                float seed, int tag, int sourceSide) {
                XMFLOAT3 c = maze_.WorldCenter(t, 0.0f);
                float halfX = std::abs(std::cos(yaw)) * w * 0.5f + std::abs(std::sin(yaw)) * d * 0.5f;
                float halfZ = std::abs(std::sin(yaw)) * w * 0.5f + std::abs(std::cos(yaw)) * d * 0.5f;
                float xMin = c.x - tileW * 0.5f;
                float xMax = c.x + tileW * 0.5f;
                float zMin = c.z - tileD * 0.5f;
                float zMax = c.z + tileD * 0.5f;
                bool touches[4] = {
                    pz - halfZ <= zMin + tileD * 0.10f,
                    pz + halfZ >= zMax - tileD * 0.10f,
                    px - halfX <= xMin + tileW * 0.10f,
                    px + halfX >= xMax - tileW * 0.10f
                };

                for (int side = 0; side < 4; ++side) {
                    if (!touches[side] || side == sourceSide || !wallHasSurface(t, side)) continue;
                    float r0 = Rand01(tag * 23 + side, 1031, scatterSeed);
                    float lateral = (side == 0 || side == 1)
                        ? px - c.x + (Rand01(tag * 23 + side, 1049, scatterSeed) - 0.5f) * tileW * 0.12f
                        : pz - c.z + (Rand01(tag * 23 + side, 1051, scatterSeed) - 0.5f) * tileD * 0.12f;
                    float wallSpan = side == 0 || side == 1 ? tileW : tileD;
                    float contactW = std::clamp((side == 0 || side == 1 ? halfX : halfZ) * (0.95f + r0 * 0.42f),
                        0.42f, wallSpan * 0.96f);
                    float runH = wallH * (0.74f + Rand01(tag * 23 + side, 1057, scatterSeed) * 0.25f);
                    float yCenter = wallH - 0.060f - runH * 0.5f;
                    addBloodWall(t, side, lateral, yCenter, contactW, runH,
                        std::fmod(seed + 0.31f + static_cast<float>(side) * 0.111f, 0.83f));
                }
            };

            auto addFloorNeighborSpill = [&](Tile t, float sourceX, float sourceZ, float seed, int tag, float strength) {
                XMFLOAT3 c = maze_.WorldCenter(t, 0.0f);
                for (int side = 0; side < 4; ++side) {
                    Tile n = neighborForSide(t, side);
                    if (!maze_.IsOpen(n.x, n.y)) continue;
                    float sideBias = 0.0f;
                    if (side == 0) sideBias = (c.z - sourceZ) / std::max(0.001f, tileD * 0.5f);
                    else if (side == 1) sideBias = (sourceZ - c.z) / std::max(0.001f, tileD * 0.5f);
                    else if (side == 2) sideBias = (c.x - sourceX) / std::max(0.001f, tileW * 0.5f);
                    else sideBias = (sourceX - c.x) / std::max(0.001f, tileW * 0.5f);
                    float chance = std::clamp(0.40f + strength * 0.34f + std::max(0.0f, sideBias) * 0.16f, 0.0f, 0.90f);
                    float r0 = Rand01(tag * 19 + side, 1009, scatterSeed);
                    if (r0 > chance) continue;
                    XMFLOAT3 dir = sideDirection(side);
                    float axis = side == 0 || side == 1 ? tileD : tileW;
                    float cross = side == 0 || side == 1 ? tileW : tileD;
                    float originalOverlap = axis * (0.16f + Rand01(tag * 19 + side, 1011, scatterSeed) * 0.08f);
                    float neighborReach = axis * (0.20f + Rand01(tag * 19 + side, 1013, scatterSeed) * 0.26f) *
                        std::clamp(strength, 0.70f, 1.12f);
                    float length = originalOverlap + neighborReach;
                    float width = cross * (0.24f + Rand01(tag * 19 + side, 1019, scatterSeed) * 0.28f);
                    float halfAxis = axis * 0.5f;
                    float centerOffset = halfAxis + (neighborReach - originalOverlap) * 0.5f;
                    XMFLOAT3 lateralAxis = side == 0 || side == 1
                        ? XMFLOAT3{1.0f, 0.0f, 0.0f}
                        : XMFLOAT3{0.0f, 0.0f, 1.0f};
                    float sourceLateral = (side == 0 || side == 1) ? sourceX - c.x : sourceZ - c.z;
                    float lateralLimit = std::max(0.0f, cross - width) * 0.48f;
                    float lateral = std::clamp(sourceLateral +
                        (Rand01(tag * 19 + side, 1017, scatterSeed) - 0.5f) * lateralLimit * 0.42f,
                        -lateralLimit, lateralLimit);
                    XMFLOAT3 spillCenter = Add3(c, Add3(Scale3(dir, centerOffset), Scale3(lateralAxis, lateral)));
                    float yaw = sideForwardYaw(side) + (Rand01(tag * 19 + side, 1021, scatterSeed) - 0.5f) * 0.18f;
                    addBloodFloor(spillCenter.x, spillCenter.z, width, length, yaw,
                        std::fmod(seed + 0.17f + static_cast<float>(side) * 0.071f, 0.93f),
                        static_cast<float>(side + 1) * kBloodFloorDecalLayerStep * 5.0f);
                }
            };

            auto addWaterFloorBorderContinuation = [&](Tile t, int side, const XMFLOAT3& wallCenter, const XMFLOAT3& right,
                const XMFLOAT3& inward, float width, float sourceDepth, float material, float seed, int tag) {
                (void)sourceDepth;
                (void)material;
                Tile n = neighborForSide(t, oppositeSide(side));
                if (!maze_.IsOpen(n.x, n.y)) return false;
                float axisLength = (side == 0 || side == 1) ? tileD : tileW;
                float crossLength = (side == 0 || side == 1) ? tileW : tileD;
                float continuationDepth = axisLength * (0.54f + Rand01(tag, 1511, scatterSeed) * 0.38f);
                float continuationWidth = std::min(crossLength * 0.98f, width * (1.06f + Rand01(tag, 1517, scatterSeed) * 0.20f));
                float startDistance = std::max(0.04f, axisLength - 0.020f);
                XMFLOAT3 nearCenter = Add3({wallCenter.x, kBloodFloorDecalLift, wallCenter.z},
                    Scale3(inward, startDistance));
                nearCenter = Add3(nearCenter, Scale3(right, (Rand01(tag, 1523, scatterSeed) - 0.5f) * width * 0.10f));
                XMFLOAT3 farCenter = Add3(nearCenter, Scale3(inward, continuationDepth));
                if (!footprintFitsMaze((nearCenter.x + farCenter.x) * 0.5f, (nearCenter.z + farCenter.z) * 0.5f,
                        continuationWidth, continuationDepth, sideForwardYaw(side), 0.010f)) {
                    return false;
                }
                float cx = (nearCenter.x + farCenter.x) * 0.5f;
                float cz = (nearCenter.z + farCenter.z) * 0.5f;
                float yaw = sideForwardYaw(side);
                markLiquidCanvasArea(cx,
                    cz,
                    continuationWidth,
                    continuationDepth,
                    yaw,
                    emitWaterLiquid,
                    false,
                    1u << static_cast<uint32_t>(side),
                    false,
                    seed + 0.41f,
                    std::max(continuationWidth, continuationDepth),
                    true);
                MarkWetFootstepArea(cx,
                    cz,
                    continuationWidth,
                    continuationDepth,
                    yaw,
                    0.02f,
                    7.5f);
                return true;
            };

            struct PendingLiquidFloorSeam {
                Tile owner{};
                int side = 0;
                float cx = 0.0f;
                float cz = 0.0f;
                float width = 0.0f;
                float depth = 0.0f;
                float yaw = 0.0f;
                float material = 25.0f;
                bool water = false;
            };
            std::vector<PendingLiquidFloorSeam> pendingLiquidFloorSeams;
            pendingLiquidFloorSeams.reserve(128);
            auto queueLiquidFloorSeam = [&](Tile owner, int side, float cx, float cz, float width, float depth, float yaw, float material) {
                pendingLiquidFloorSeams.push_back({owner, side, cx, cz, width, depth, yaw, material, emitWaterLiquid});
            };
            auto drawLiquidFloorSeam = [&](const PendingLiquidFloorSeam& p) {
                markLiquidCanvasArea(p.cx, p.cz, p.width, p.depth, p.yaw,
                    p.water, false, 1u << static_cast<uint32_t>(p.side), false, p.material, std::max(p.width, p.depth));
                MarkWetFootstepArea(p.cx, p.cz, p.width, p.depth, p.yaw, 0.02f, p.water ? 7.5f : 0.0f);
            };
            auto emitMergedLiquidFloorSeams = [&]() {
                if (pendingLiquidFloorSeams.empty()) return;
                for (const PendingLiquidFloorSeam& seam : pendingLiquidFloorSeams) {
                    drawLiquidFloorSeam(seam);
                }
                pendingLiquidFloorSeams.clear();
            };

            auto addCeilingPropagation = [&](Tile t, float px, float pz, float w, float d, float yaw, float seed, int tag, int sourceSide) {
                XMFLOAT3 c = maze_.WorldCenter(t, 0.0f);
                float halfX = std::abs(std::cos(yaw)) * w * 0.5f + std::abs(std::sin(yaw)) * d * 0.5f;
                float halfZ = std::abs(std::sin(yaw)) * w * 0.5f + std::abs(std::cos(yaw)) * d * 0.5f;
                float xMin = c.x - tileW * 0.5f;
                float xMax = c.x + tileW * 0.5f;
                float zMin = c.z - tileD * 0.5f;
                float zMax = c.z + tileD * 0.5f;
                bool touches[4] = {
                    pz - halfZ <= zMin + tileD * 0.10f,
                    pz + halfZ >= zMax - tileD * 0.10f,
                    px - halfX <= xMin + tileW * 0.10f,
                    px + halfX >= xMax - tileW * 0.10f
                };
                addBloodCeilingWallRunoffs(t, px, pz, w, d, yaw, seed, tag, sourceSide);

                for (int side = 0; side < 4; ++side) {
                    if (!touches[side]) continue;
                    float r0 = Rand01(tag * 23 + side, 1031, scatterSeed);
                    Tile n = neighborForSide(t, side);
                    if (maze_.IsOpen(n.x, n.y)) {
                        bool fromWallLeak = sourceSide >= 0;
                        if (fromWallLeak && side != oppositeSide(sourceSide)) continue;
                        if (!fromWallLeak && r0 > 0.62f) continue;
                        if (fromWallLeak && !gBloodDebugEveryWall && !settings_.bloodStudyView && r0 > 0.86f) continue;
                        XMFLOAT3 dir = sideDirection(side);
                        float axis = side == 0 || side == 1 ? tileD : tileW;
                        float span = side == 0 || side == 1 ? tileW : tileD;
                        float length = fromWallLeak
                            ? axis * (0.30f + Rand01(tag * 23 + side, 1037, scatterSeed) * 0.24f)
                            : axis * (0.66f + Rand01(tag * 23 + side, 1037, scatterSeed) * 0.36f);
                        float width = std::clamp((side == 0 || side == 1 ? halfX : halfZ) * (1.08f + r0 * 0.28f),
                            span * 0.34f, span * 0.96f);
                        float centerX = c.x + dir.x * (axis * 0.5f + length * 0.24f);
                        float centerZ = c.z + dir.z * (axis * 0.5f + length * 0.24f);
                        float spreadYaw = sideForwardYaw(side) + (Rand01(tag * 23 + side, 1041, scatterSeed) - 0.5f) * 0.26f;
                        float spreadSeed = std::fmod(seed + 0.23f + static_cast<float>(side) * 0.093f, 0.87f);
                        if (addBloodCeilingFloorPair(centerX, centerZ, width, length, spreadYaw, spreadSeed, 0.84f)) {
                            addBloodCeilingWallRunoffs(n, centerX, centerZ, width, length, spreadYaw,
                                spreadSeed, tag * 31 + side + 17, oppositeSide(side));
                        }
                    }
                }
            };

            auto addBloodRun = [&](Tile t, int runIndex) {
                XMFLOAT3 c = maze_.WorldCenter(t, 0.0f);
                bool openE = maze_.IsOpen(t.x + 1, t.y);
                bool openW = maze_.IsOpen(t.x - 1, t.y);
                bool openN = maze_.IsOpen(t.x, t.y - 1);
                bool openS = maze_.IsOpen(t.x, t.y + 1);
                bool ew = openE || openW;
                bool ns = openN || openS;
                float yaw = Rand01(runIndex, 761, scatterSeed) * kPi * 2.0f;
                if (ew || ns) {
                    bool useEW = ew && (!ns || Rand01(runIndex, 763, scatterSeed) < 0.55f);
                    yaw = useEW ? 0.0f : kPi * 0.5f;
                    yaw += (Rand01(runIndex, 769, scatterSeed) - 0.5f) * 0.22f;
                }

                XMFLOAT3 right{std::cos(yaw), 0.0f, -std::sin(yaw)};
                XMFLOAT3 up{0.0f, 1.0f, 0.0f};
                XMFLOAT3 forward{std::sin(yaw), 0.0f, std::cos(yaw)};
                float runLength = tileAvg * (1.85f + Rand01(runIndex, 773, scatterSeed) * 4.65f);
                float runWidth = 0.44f + Rand01(runIndex, 787, scatterSeed) * 0.82f;
                float alongOffset = (Rand01(runIndex, 797, scatterSeed) - 0.5f) * tileAvg * 0.55f;
                float crossOffset = (Rand01(runIndex, 809, scatterSeed) - 0.5f) * tileAvg * 0.42f;
                XMFLOAT3 center = Add3(c, OrientedOffset(right, up, forward, alongOffset, 0.0f, crossOffset));
                float seed = Rand01(runIndex, 811, scatterSeed);

                bool placed = addBloodFloor(center.x, center.z, runLength, runWidth, yaw, seed, 0.0f);
                if (!placed) {
                    runLength *= 0.68f;
                    runWidth *= 0.82f;
                    placed = addBloodFloor(center.x, center.z, runLength, runWidth, yaw, seed, 0.0f);
                }
                if (!placed) return false;
                addFloorNeighborSpill(t, center.x, center.z, seed, runIndex, std::clamp(runLength / std::max(0.01f, tileAvg) * 0.36f, 0.65f, 1.22f));

                if (Rand01(runIndex, 821, scatterSeed) < 0.64f) {
                    XMFLOAT3 ceilingCenter = Add3(center, OrientedOffset(right, up, forward,
                        (Rand01(runIndex, 823, scatterSeed) - 0.5f) * runLength * 0.16f,
                        0.0f,
                        (Rand01(runIndex, 827, scatterSeed) - 0.5f) * runWidth * 0.55f));
                    float ceilingW = runLength * (0.68f + Rand01(runIndex, 829, scatterSeed) * 0.42f);
                    float ceilingD = runWidth * (0.76f + Rand01(runIndex, 839, scatterSeed) * 0.58f);
                    float ceilingYaw = yaw + (Rand01(runIndex, 853, scatterSeed) - 0.5f) * 0.38f;
                    float ceilingSeed = Rand01(runIndex, 857, scatterSeed);
                    if (addBloodCeilingFloorPair(ceilingCenter.x, ceilingCenter.z, ceilingW, ceilingD, ceilingYaw, ceilingSeed, 0.86f)) {
                        addCeilingPropagation(t, ceilingCenter.x, ceilingCenter.z, ceilingW, ceilingD, ceilingYaw, ceilingSeed, runIndex, -1);
                    }
                }

                int satelliteCount = 18 + static_cast<int>(Rand01(runIndex, 859, scatterSeed) * 36.0f);
                for (int i = 0; i < satelliteCount; ++i) {
                    float along = (Rand01(runIndex * 37 + i, 863, scatterSeed) - 0.5f) * runLength * 1.14f;
                    float cross = (Rand01(runIndex * 37 + i, 877, scatterSeed) - 0.5f) * runWidth * 2.45f;
                    XMFLOAT3 p = Add3(center, OrientedOffset(right, up, forward, along, 0.0f, cross));
                    float sizeBias = std::pow(Rand01(runIndex * 37 + i, 881, scatterSeed), 2.15f);
                    float spotW = 0.045f + sizeBias * 0.25f;
                    float spotD = 0.035f + std::pow(Rand01(runIndex * 37 + i, 883, scatterSeed), 2.05f) * 0.22f;
                    float spotYaw = yaw + (Rand01(runIndex * 37 + i, 887, scatterSeed) - 0.5f) * 1.25f;
                    addBloodFloor(p.x, p.z, spotW, spotD, spotYaw, Rand01(runIndex * 37 + i, 907, scatterSeed),
                        static_cast<float>(i) * kBloodFloorDecalLayerStep);
                }

                int sideCandidates[4]{};
                int sideCount = 0;
                bool mostlyEW = std::abs(right.x) >= std::abs(right.z);
                if (mostlyEW) {
                    if (!maze_.IsOpen(t.x, t.y - 1)) sideCandidates[sideCount++] = 0;
                    if (!maze_.IsOpen(t.x, t.y + 1)) sideCandidates[sideCount++] = 1;
                } else {
                    if (!maze_.IsOpen(t.x - 1, t.y)) sideCandidates[sideCount++] = 2;
                    if (!maze_.IsOpen(t.x + 1, t.y)) sideCandidates[sideCount++] = 3;
                }
                if (sideCount == 0) {
                    if (!maze_.IsOpen(t.x, t.y - 1)) sideCandidates[sideCount++] = 0;
                    if (!maze_.IsOpen(t.x, t.y + 1)) sideCandidates[sideCount++] = 1;
                    if (!maze_.IsOpen(t.x - 1, t.y)) sideCandidates[sideCount++] = 2;
                    if (!maze_.IsOpen(t.x + 1, t.y)) sideCandidates[sideCount++] = 3;
                }
                for (int sidx = 0; sidx < sideCount; ++sidx) {
                    if (Rand01(runIndex * 13 + sidx, 911, scatterSeed) < (sidx == 0 ? 0.92f : 0.52f)) {
                        int side = sideCandidates[sidx];
                        float lateral = (side == 0 || side == 1)
                            ? center.x - c.x + (Rand01(runIndex * 13 + sidx, 919, scatterSeed) - 0.5f) * tileW * 0.35f
                            : center.z - c.z + (Rand01(runIndex * 13 + sidx, 929, scatterSeed) - 0.5f) * tileD * 0.35f;
                        float wallLen = runLength * (0.48f + Rand01(runIndex * 13 + sidx, 937, scatterSeed) * 0.48f);
                        float wallH = 0.68f + Rand01(runIndex * 13 + sidx, 941, scatterSeed) * 1.74f;
                        float wallY = 0.62f + Rand01(runIndex * 13 + sidx, 947, scatterSeed) * 1.46f;
                        addBloodWall(t, side, lateral, wallY, wallLen, wallH, Rand01(runIndex * 13 + sidx, 953, scatterSeed));
                    }
                }
                return true;
            };

            auto addBloodBurst = [&](Tile t, int burstIndex) {
                XMFLOAT3 c = maze_.WorldCenter(t, 0.0f);
                float r0 = Rand01(burstIndex, 601, scatterSeed);
                float r1 = Rand01(burstIndex, 607, scatterSeed);
                float r2 = Rand01(burstIndex, 613, scatterSeed);
                float yaw = r2 * kPi * 2.0f;
                float px = c.x + (r0 - 0.5f) * tileW * 0.48f;
                float pz = c.z + (r1 - 0.5f) * tileD * 0.48f;
                float base = 0.62f + Rand01(burstIndex, 617, scatterSeed) * 0.98f;
                addBloodFloor(px, pz, base * (0.96f + Rand01(burstIndex, 619, scatterSeed) * 0.74f), base * (0.58f + Rand01(burstIndex, 623, scatterSeed) * 0.70f), yaw, r0, 0.0f);
                addFloorNeighborSpill(t, px, pz, r0, burstIndex, std::clamp(base / std::max(0.01f, tileAvg), 0.48f, 1.08f));
                if (Rand01(burstIndex, 631, scatterSeed) < 0.78f) {
                    float ceilingX = px + (Rand01(burstIndex, 641, scatterSeed) - 0.5f) * 0.45f;
                    float ceilingZ = pz + (Rand01(burstIndex, 643, scatterSeed) - 0.5f) * 0.45f;
                    float ceilingW = base * (0.78f + Rand01(burstIndex, 647, scatterSeed) * 0.86f);
                    float ceilingD = base * (0.64f + Rand01(burstIndex, 653, scatterSeed) * 0.78f);
                    float ceilingYaw = yaw + Rand01(burstIndex, 659, scatterSeed) * 0.9f;
                    if (addBloodCeilingFloorPair(ceilingX, ceilingZ, ceilingW, ceilingD, ceilingYaw, r1, 0.86f)) {
                        addCeilingPropagation(t, ceilingX, ceilingZ, ceilingW, ceilingD, ceilingYaw, r1, burstIndex, -1);
                    }
                }

                int sides[4]{};
                int sideCount = 0;
                if (!maze_.IsOpen(t.x, t.y - 1)) sides[sideCount++] = 0;
                if (!maze_.IsOpen(t.x, t.y + 1)) sides[sideCount++] = 1;
                if (!maze_.IsOpen(t.x - 1, t.y)) sides[sideCount++] = 2;
                if (!maze_.IsOpen(t.x + 1, t.y)) sides[sideCount++] = 3;
                for (int sidx = 0; sidx < sideCount; ++sidx) {
                    if (Rand01(burstIndex * 7 + sidx, 661, scatterSeed) > 0.68f && sidx > 0) continue;
                    float lateral = (Rand01(burstIndex * 7 + sidx, 673, scatterSeed) - 0.5f) *
                        ((sides[sidx] == 0 || sides[sidx] == 1) ? tileW : tileD) * 0.54f;
                    float yc = 0.88f + Rand01(burstIndex * 7 + sidx, 677, scatterSeed) * 1.26f;
                    float sw = 0.56f + Rand01(burstIndex * 7 + sidx, 683, scatterSeed) * 1.20f;
                    float sh = 0.52f + Rand01(burstIndex * 7 + sidx, 691, scatterSeed) * 1.30f;
                    addBloodWall(t, sides[sidx], lateral, yc, sw, sh, Rand01(burstIndex * 7 + sidx, 701, scatterSeed));
                }

                int droplets = 24 + static_cast<int>(Rand01(burstIndex, 709, scatterSeed) * 42.0f);
                for (int d = 0; d < droplets; ++d) {
                    float dyaw = Rand01(burstIndex * 31 + d, 719, scatterSeed) * kPi * 2.0f;
                    float radius = std::pow(Rand01(burstIndex * 31 + d, 727, scatterSeed), 0.56f) * tileAvg * 1.54f;
                    float dx = std::cos(dyaw) * radius;
                    float dz = std::sin(dyaw) * radius;
                    float tiny = std::pow(Rand01(burstIndex * 31 + d, 733, scatterSeed), 2.45f);
                    float dw = 0.028f + tiny * 0.20f;
                    float dd = 0.024f + std::pow(Rand01(burstIndex * 31 + d, 739, scatterSeed), 2.35f) * 0.18f;
                    addBloodFloor(px + dx, pz + dz, dw, dd, dyaw, Rand01(burstIndex * 31 + d, 743, scatterSeed),
                        static_cast<float>(d) * kBloodFloorDecalLayerStep);
                }
            };

            auto addBloodCenterDripTile = [&](Tile t, int dripIndex) {
                if (!maze_.IsOpen(t.x, t.y) || t == maze_.start || t == maze_.exit) return false;
                if (waterDamageTileBlocked(t)) return false;
                if (bloodCenterSeepCovered.find(bloodTileKey(t)) != bloodCenterSeepCovered.end()) return false;
                if (!maze_.IsOpen(t.x, t.y - 1) || !maze_.IsOpen(t.x, t.y + 1) ||
                    !maze_.IsOpen(t.x - 1, t.y) || !maze_.IsOpen(t.x + 1, t.y)) {
                    return false;
                }
                XMFLOAT3 c = maze_.WorldCenter(t, 0.0f);
                bool diagNW = maze_.IsOpen(t.x - 1, t.y - 1);
                bool diagNE = maze_.IsOpen(t.x + 1, t.y - 1);
                bool diagSW = maze_.IsOpen(t.x - 1, t.y + 1);
                bool diagSE = maze_.IsOpen(t.x + 1, t.y + 1);
                bool roomCanvas = diagNW && diagNE && diagSW && diagSE;
                bool eastWestCanvas = Rand01(dripIndex, 1303, scatterSeed) < 0.5f;
                float w = tileW * (roomCanvas ? (1.62f + Rand01(dripIndex, 1307, scatterSeed) * 0.24f) :
                    (eastWestCanvas ? (1.92f + Rand01(dripIndex, 1311, scatterSeed) * 0.24f) : 0.92f));
                float d = tileD * (roomCanvas ? (1.62f + Rand01(dripIndex, 1313, scatterSeed) * 0.24f) :
                    (eastWestCanvas ? 0.92f : (1.92f + Rand01(dripIndex, 1317, scatterSeed) * 0.24f)));
                float yaw = 0.0f;
                float px = c.x + (Rand01(dripIndex, 1321, scatterSeed) - 0.5f) * tileW * 0.10f;
                float pz = c.z + (Rand01(dripIndex, 1327, scatterSeed) - 0.5f) * tileD * 0.10f;
                float seed = Rand01(dripIndex, 1331, scatterSeed);
                bool placed = addBloodCeilingFloorPair(px, pz, w, d, yaw, seed, 1.0f);
                if (!placed) {
                    w = tileW * 0.96f;
                    d = tileD * 0.96f;
                    px = c.x;
                    pz = c.z;
                    placed = addBloodCeilingFloorPair(px, pz, w, d, yaw, seed, 1.0f);
                }
                if (placed) {
                    auto markCovered = [&](Tile covered) {
                        if (maze_.IsOpen(covered.x, covered.y)) bloodCenterSeepCovered.insert(bloodTileKey(covered));
                    };
                    markCovered(t);
                    if (w > tileW * 1.15f) {
                        markCovered({t.x - 1, t.y});
                        markCovered({t.x + 1, t.y});
                    }
                    if (d > tileD * 1.15f) {
                        markCovered({t.x, t.y - 1});
                        markCovered({t.x, t.y + 1});
                    }
                    if (w > tileW * 1.15f && d > tileD * 1.15f) {
                        markCovered({t.x - 1, t.y - 1});
                        markCovered({t.x + 1, t.y - 1});
                        markCovered({t.x - 1, t.y + 1});
                        markCovered({t.x + 1, t.y + 1});
                    }
                }
                return placed;
            };

            auto addBloodLeak = [&](Tile t, int side, int leakIndex, bool wallOnly = false) {
                if (!wallHasSurface(t, side)) return false;
                if (waterDamageTileBlocked(t)) return false;
                XMFLOAT3 c = maze_.WorldCenter(t, 0.0f);
                float seed = Rand01(leakIndex, 701, scatterSeed);
                float wallSpan = (side == 0 || side == 1) ? tileW : tileD;
                float leakW = wallSpan * (0.82f + Rand01(leakIndex, 707, scatterSeed) * 0.13f);
                float lateralLimit = std::max(0.0f, wallSpan * 0.5f - leakW * 0.5f - 0.16f);
                float lateral = (Rand01(leakIndex, 709, scatterSeed) - 0.5f) * lateralLimit * 2.0f;
                float topY = wallH - 0.0015f;
                float bottomY = 0.0015f;
                float h = std::max(0.2f, topY - bottomY);
                float centerY = (topY + bottomY) * 0.5f;
                XMFLOAT3 normal{0.0f, 0.0f, 1.0f};
                XMFLOAT3 right{1.0f, 0.0f, 0.0f};
                XMFLOAT3 inward{0.0f, 0.0f, 1.0f};
                XMFLOAT3 wallCenter{c.x, centerY, c.z};
                constexpr float kBloodLeakWallDecalInset = 0.0050f;
                constexpr float kBloodLeakSeamInset = 0.0010f;
                if (side == 0) {
                    normal = {0.0f, 0.0f, 1.0f};
                    right = {1.0f, 0.0f, 0.0f};
                    inward = {0.0f, 0.0f, 1.0f};
                    wallCenter = {c.x + lateral, centerY, c.z - tileD * 0.5f + kBloodLeakWallDecalInset};
                } else if (side == 1) {
                    normal = {0.0f, 0.0f, -1.0f};
                    right = {-1.0f, 0.0f, 0.0f};
                    inward = {0.0f, 0.0f, -1.0f};
                    wallCenter = {c.x + lateral, centerY, c.z + tileD * 0.5f - kBloodLeakWallDecalInset};
                } else if (side == 2) {
                    normal = {1.0f, 0.0f, 0.0f};
                    right = {0.0f, 0.0f, 1.0f};
                    inward = {1.0f, 0.0f, 0.0f};
                    wallCenter = {c.x - tileW * 0.5f + kBloodLeakWallDecalInset, centerY, c.z + lateral};
                } else {
                    normal = {-1.0f, 0.0f, 0.0f};
                    right = {0.0f, 0.0f, -1.0f};
                    inward = {-1.0f, 0.0f, 0.0f};
                    wallCenter = {c.x + tileW * 0.5f - kBloodLeakWallDecalInset, centerY, c.z + lateral};
                }
                wallCenter = Add3(wallCenter, Scale3(normal, 0.0008f + std::fmod(std::abs(seed) * 9.713f, 1.0f) * 0.0018f));

                XMFLOAT3 up{0.0f, 1.0f, 0.0f};
                XMFLOAT3 a = Add3(wallCenter, Add3(Scale3(right, -leakW * 0.5f), Scale3(up, -h * 0.5f)));
                XMFLOAT3 b = Add3(wallCenter, Add3(Scale3(right,  leakW * 0.5f), Scale3(up, -h * 0.5f)));
                XMFLOAT3 c0 = Add3(wallCenter, Add3(Scale3(right,  leakW * 0.5f), Scale3(up,  h * 0.5f)));
                XMFLOAT3 d0 = Add3(wallCenter, Add3(Scale3(right, -leakW * 0.5f), Scale3(up,  h * 0.5f)));
                float sourceMat = liquidMaterial(0.965f + seed * 0.025f);
                float wallMat = sourceMat;
                if (emitWaterLiquid) {
                    float centerAlong = (side == 0 || side == 1) ? wallCenter.x : wallCenter.z;
                    markWaterWallCanvas(t, side, centerAlong, leakW, seed, leakW * h);
                } else {
                    AddQuadUV(vertices, liquidIndices, a, b, c0, d0, normal, right, {0, 1}, {1, 1}, {1, 0}, {0, 0}, wallMat);
                }

                float bloodQuality = std::clamp(settings_.bloodShaderQuality, 0.25f, 1.0f);
                bool addSeamCards = emitWaterLiquid || !wallOnly || gBloodDebugEveryWall || bloodQuality >= 0.52f ||
                    Rand01(leakIndex, 741, scatterSeed) < bloodQuality * 0.75f;
                auto addCeilingSeamCard = [&](float depth, float material) {
                    (void)material;
                    float seamY = nextBloodCeilingY(wallCenter.x, wallCenter.z, seed + 0.37f);
                    XMFLOAT3 seamWallEdge = Add3({wallCenter.x, seamY, wallCenter.z},
                        Scale3(inward, -kBloodLeakWallDecalInset));
                    XMFLOAT3 seamCenter = Add3(seamWallEdge, Scale3(inward, kBloodLeakSeamInset));
                    XMFLOAT3 ceilingCenter = Add3(seamCenter, Scale3(inward, depth * 0.5f));
                    markLiquidCanvasArea(ceilingCenter.x, ceilingCenter.z, leakW, depth, liquidCardYawForSide(side),
                        emitWaterLiquid, true, 1u << static_cast<uint32_t>(side), false, seed + 0.37f, depth);
                };

                auto addFloorSeamCard = [&](float width, float depth, float material) {
                    XMFLOAT3 seamWallEdge = Add3({wallCenter.x, kBloodFloorDecalLift, wallCenter.z},
                        Scale3(inward, -kBloodLeakWallDecalInset));
                    XMFLOAT3 seamCenter = Add3(seamWallEdge, Scale3(inward, kBloodLeakSeamInset));
                    XMFLOAT3 floorCenter = Add3(seamCenter, Scale3(inward, depth * 0.5f));
                    queueLiquidFloorSeam(t, side, floorCenter.x, floorCenter.z, width, depth, liquidCardYawForSide(side), material);
                };

                float axisLength = (side == 0 || side == 1) ? tileD : tileW;
                Tile forwardTile = neighborForSide(t, oppositeSide(side));
                bool canSpreadForward = maze_.IsOpen(forwardTile.x, forwardTile.y);
                float sourceD = axisLength * (0.88f + Rand01(leakIndex, 719, scatterSeed) * 0.10f);
                if (canSpreadForward) {
                    sourceD += axisLength * (0.16f + Rand01(leakIndex, 721, scatterSeed) * 0.22f);
                }
                if (addSeamCards) {
                    addCeilingSeamCard(sourceD, sourceMat);
                    if (!emitWaterLiquid) {
                        float seamYaw = sideForwardYaw(side);
                        XMFLOAT3 seamWallEdge = Add3({wallCenter.x, wallH - kBloodCeilingDecalInset, wallCenter.z},
                            Scale3(inward, -kBloodLeakWallDecalInset));
                        XMFLOAT3 seamCenter = Add3(seamWallEdge, Scale3(inward, sourceD * 0.5f + kBloodLeakSeamInset));
                        addCeilingPropagation(t, seamCenter.x, seamCenter.z, leakW, sourceD, seamYaw, seed, leakIndex, side);
                    }
                }

                float poolW = std::max(leakW, wallSpan * 0.985f);
                float poolD = axisLength * (0.86f + Rand01(leakIndex, 727, scatterSeed) * 0.12f);
                if (canSpreadForward) {
                    poolD += axisLength * (0.18f + Rand01(leakIndex, 729, scatterSeed) * 0.24f);
                }
                const float bloodWidthScales[] = {1.08f, 1.0f, 0.94f, 0.86f, 0.72f, 0.60f};
                const float bloodDepthScales[] = {3.05f, 2.54f, 2.04f, 1.60f, 1.28f, 1.0f};
                bool foundBloodFloorFit = false;
                for (float dw : bloodWidthScales) {
                    for (float dd : bloodDepthScales) {
                        float candidateW = poolW * dw;
                        float candidateD = poolD * dd;
                        XMFLOAT3 candidateCenter = Add3({wallCenter.x, 0.0f, wallCenter.z}, Scale3(inward, candidateD * 0.5f + 0.006f));
                        if (!footprintFitsMaze(candidateCenter.x, candidateCenter.z, candidateW, candidateD, liquidCardYawForSide(side), kLiquidFloorReservationPad)) continue;
                        poolW = candidateW;
                        poolD = candidateD;
                        foundBloodFloorFit = true;
                        break;
                    }
                    if (foundBloodFloorFit) break;
                }
                XMFLOAT3 floorCenter = Add3({wallCenter.x, 0.0f, wallCenter.z}, Scale3(inward, poolD * 0.5f + 0.006f));
                if (addSeamCards) {
                    addFloorSeamCard(poolW, poolD, sourceMat);
                    MarkWetFootstepArea(floorCenter.x, floorCenter.z, poolW, poolD, liquidCardYawForSide(side), 0.02f, emitWaterLiquid ? 7.5f : 0.0f);
                    MarkWetCeilingDripEmitter({floorCenter.x, 0.10f, floorCenter.z}, seed);
                    if (canSpreadForward) {
                        addWaterFloorBorderContinuation(t, side, wallCenter, right, inward, poolW, poolD,
                            sourceMat, seed, leakIndex + 41000);
                    }
                }
                if (wallOnly || gBloodDebugEveryWall) return true;

                BloodScarePoint scare{};
                scare.pos = {floorCenter.x, 0.10f, floorCenter.z};
                scare.source = {wallCenter.x, wallH - 0.035f, wallCenter.z};
                scare.normal = normal;
                scare.radius = std::clamp(1.95f + std::max(poolW, poolD) * 0.96f, 2.65f, 5.65f);
                scare.focusDelaySeconds = 0.34f + Rand01(leakIndex, 733, scatterSeed) * 0.66f;
                scare.requireFacing = true;
                scare.waterLiquid = emitWaterLiquid;
                if (emitWaterLiquid) {
                    scare.dreadScale = 0.30f;
                }
                bloodScarePoints_.push_back(scare);
                return true;
            };

            auto emitWaterDamageUsingBloodSystem = [&]() {
                bool waterDebug = gEffectDebugViewer && DebugSliceEffectIsWater(gDebugSliceEffect);
                if (!waterDebug && waterLikeDamageChance <= 0.0001f) return;
                emitWaterLiquid = true;
                if (waterDebug) {
                    int debugIndex = 0;
                    for (Tile t : openTiles) {
                        int sideCount = 0;
                        for (int side = 0; side < 4; ++side) {
                            if (wallHasSurface(t, side)) {
                                ++sideCount;
                                addBloodLeak(t, side, 30000 + debugIndex++);
                            }
                        }
                        if (sideCount == 0) {
                            addBloodCenterDripTile(t, 33000 + debugIndex++);
                        }
                    }
                    emitWaterLiquid = false;
                    return;
                }

                int waterIndex = 0;
                for (Tile t : openTiles) {
                    if (t == maze_.start || t == maze_.exit) continue;
                    int sides[4]{};
                    int sideCount = 0;
                    if (wallHasSurface(t, 0)) sides[sideCount++] = 0;
                    if (wallHasSurface(t, 1)) sides[sideCount++] = 1;
                    if (wallHasSurface(t, 2)) sides[sideCount++] = 2;
                    if (wallHasSurface(t, 3)) sides[sideCount++] = 3;
                    float chance = waterLikeDamageChance;
                    if (sideCount == 0) chance *= 0.58f;
                    if (Rand01(waterIndex, 2501, scatterSeed) > chance) {
                        ++waterIndex;
                        continue;
                    }
                    if (sideCount == 0) {
                        addBloodCenterDripTile(t, 25000 + waterIndex);
                    } else {
                        int side = sides[std::min(sideCount - 1,
                            static_cast<int>(Rand01(waterIndex, 2507, scatterSeed) * static_cast<float>(sideCount)))];
                        addBloodLeak(t, side, 25000 + waterIndex);
                        if (sideCount >= 2) {
                            int secondSide = -1;
                            int opposite = oppositeSide(side);
                            for (int i = 0; i < sideCount; ++i) {
                                if (sides[i] == opposite) {
                                    secondSide = opposite;
                                    break;
                                }
                            }
                            if (secondSide < 0) {
                                int pick = std::min(sideCount - 1,
                                    static_cast<int>(Rand01(waterIndex, 2511, scatterSeed) * static_cast<float>(sideCount)));
                                for (int step = 0; step < sideCount; ++step) {
                                    int candidate = sides[(pick + step) % sideCount];
                                    if (candidate != side) {
                                        secondSide = candidate;
                                        break;
                                    }
                                }
                            }
                            if (secondSide >= 0) {
                                addBloodLeak(t, secondSide, 26000 + waterIndex);
                            }
                        }
                    }
                    ++waterIndex;
                }
                emitWaterLiquid = false;
            };

            // The old water pass reused blood leak geometry and could mix water walls with
            // blood floor/ceiling responses. Keep water on the dedicated water-like path.

            auto waterLikeMaterial = [](float seed, float rawSeed) {
                return 25.0f + rawSeed + std::fmod(std::abs(seed) * 0.0017f, 0.0009f);
            };

            auto addWaterLikeFloor = [&](float px, float pz, float w, float d, float yaw, float seed, float layerLift, float rawSeed) {
                float widthScale = 1.0f;
                float depthScale = 1.0f;
                const float widthScales[] = {2.18f, 1.86f, 1.56f, 1.28f, 1.0f, 0.86f};
                const float depthScales[] = {3.35f, 2.75f, 2.20f, 1.72f, 1.36f, 1.0f};
                bool found = false;
                for (float dw : widthScales) {
                    for (float dd : depthScales) {
                        if (footprintFitsMaze(px, pz, w * dw, d * dd, yaw, kLiquidFloorReservationPad)) {
                            widthScale = dw;
                            depthScale = dd;
                            found = true;
                            break;
                        }
                    }
                    if (found) break;
                }
                if (!found) {
                    const float narrowWidthScales[] = {0.78f, 0.66f, 0.54f};
                    for (float dw : narrowWidthScales) {
                        for (float dd : depthScales) {
                            if (footprintFitsMaze(px, pz, w * dw, d * dd, yaw, kLiquidFloorReservationPad)) {
                                widthScale = dw;
                                depthScale = dd;
                                found = true;
                                break;
                            }
                        }
                        if (found) break;
                    }
                }
                if (!found) return false;
                return addLiquidFloorOverlay(px, pz, w * widthScale, d * depthScale, yaw, seed, 0.010f + layerLift, rawSeed);
            };

            auto addWaterLikeCeiling = [&](float px, float pz, float w, float d, float yaw, float seed, float rawSeed) {
                float widthScale = 1.0f;
                float depthScale = 1.0f;
                const float widthScales[] = {2.28f, 1.92f, 1.58f, 1.30f, 1.0f, 0.86f};
                const float depthScales[] = {3.45f, 2.85f, 2.26f, 1.76f, 1.36f, 1.0f};
                bool found = false;
                for (float dw : widthScales) {
                    for (float dd : depthScales) {
                        if (footprintFitsMaze(px, pz, w * dw, d * dd, yaw, 0.010f)) {
                            widthScale = dw;
                            depthScale = dd;
                            found = true;
                            break;
                        }
                    }
                    if (found) break;
                }
                if (!found) {
                    const float narrowWidthScales[] = {0.78f, 0.66f, 0.54f};
                    for (float dw : narrowWidthScales) {
                        for (float dd : depthScales) {
                            if (footprintFitsMaze(px, pz, w * dw, d * dd, yaw, 0.010f)) {
                                widthScale = dw;
                                depthScale = dd;
                                found = true;
                                break;
                            }
                        }
                        if (found) break;
                    }
                }
                if (!found) return false;
                return addLiquidCeilingOverlay(px, pz, w * widthScale, d * depthScale, yaw, seed, rawSeed);
            };

            auto addWaterLikeCeilingFromWall = [&](XMFLOAT3 edge, XMFLOAT3 inward, float w, float d, float yaw,
                                                   float seed, float rawSeed) {
                const float widthScales[] = {1.40f, 1.22f, 1.0f, 0.86f, 0.72f, 0.60f};
                const float depthScales[] = {3.65f, 3.05f, 2.46f, 1.94f, 1.54f, 1.24f, 1.0f};
                for (float dw : widthScales) {
                    for (float dd : depthScales) {
                        float ww = w * dw;
                        float ddMeters = d * dd;
                        XMFLOAT3 center = Add3(edge, Scale3(inward, ddMeters * 0.5f + 0.010f));
                        if (!footprintFitsMaze(center.x, center.z, ww, ddMeters, yaw, 0.010f)) continue;
                        return addLiquidCeilingOverlay(center.x, center.z, ww, ddMeters, yaw, seed, rawSeed);
                    }
                }
                return false;
            };

            auto addWaterLikeFloorFromWall = [&](XMFLOAT3 edge, XMFLOAT3 inward, float w, float d, float yaw,
                                                 float seed, float layerLift, float rawSeed) {
                const float widthScales[] = {1.34f, 1.18f, 1.0f, 0.86f, 0.72f, 0.60f};
                const float depthScales[] = {3.85f, 3.20f, 2.62f, 2.08f, 1.62f, 1.28f, 1.0f};
                for (float dw : widthScales) {
                    for (float dd : depthScales) {
                        float ww = w * dw;
                        float ddMeters = d * dd;
                        XMFLOAT3 center = Add3(edge, Scale3(inward, ddMeters * 0.5f + 0.010f));
                        if (!footprintFitsMaze(center.x, center.z, ww, ddMeters, yaw, kLiquidFloorReservationPad)) continue;
                        return addLiquidFloorOverlay(center.x, center.z, ww, ddMeters, yaw,
                            seed, 0.010f + layerLift, rawSeed);
                    }
                }
                return false;
            };

            auto addWaterLikeCenterTile = [&](Tile t, int dripIndex) {
                if (!maze_.IsOpen(t.x, t.y) || (!gEffectDebugViewer && (t == maze_.start || t == maze_.exit))) return false;
                if (waterDamageTileBlocked(t)) return false;
                if (bloodCenterSeepCovered.find(bloodTileKey(t)) != bloodCenterSeepCovered.end()) return false;
                if (!maze_.IsOpen(t.x, t.y - 1) || !maze_.IsOpen(t.x, t.y + 1) ||
                    !maze_.IsOpen(t.x - 1, t.y) || !maze_.IsOpen(t.x + 1, t.y)) {
                    return false;
                }
                XMFLOAT3 c = maze_.WorldCenter(t, 0.0f);
                bool diagNW = maze_.IsOpen(t.x - 1, t.y - 1);
                bool diagNE = maze_.IsOpen(t.x + 1, t.y - 1);
                bool diagSW = maze_.IsOpen(t.x - 1, t.y + 1);
                bool diagSE = maze_.IsOpen(t.x + 1, t.y + 1);
                bool roomCanvas = diagNW && diagNE && diagSW && diagSE;
                bool eastWestCanvas = Rand01(dripIndex, 2303, scatterSeed) < 0.5f;
                float w = tileW * (roomCanvas ? (1.62f + Rand01(dripIndex, 2307, scatterSeed) * 0.24f) :
                    (eastWestCanvas ? (1.92f + Rand01(dripIndex, 2311, scatterSeed) * 0.24f) : 0.92f));
                float d = tileD * (roomCanvas ? (1.62f + Rand01(dripIndex, 2313, scatterSeed) * 0.24f) :
                    (eastWestCanvas ? 0.92f : (1.92f + Rand01(dripIndex, 2317, scatterSeed) * 0.24f)));
                float px = c.x + (Rand01(dripIndex, 2321, scatterSeed) - 0.5f) * tileW * 0.10f;
                float pz = c.z + (Rand01(dripIndex, 2327, scatterSeed) - 0.5f) * tileD * 0.10f;
                float seed = Rand01(dripIndex, 2331, scatterSeed);
                bool ceilingPlaced = addWaterLikeCeiling(px, pz, w, d, 0.0f, seed, 0.43f + std::fmod(seed, 0.50f));
                bool floorPlaced = addWaterLikeFloor(px, pz, w, d, 0.0f, seed, 0.0f, 0.43f + std::fmod(seed, 0.50f));
                bool placed = ceilingPlaced && floorPlaced;
                if (placed) {
                    MarkWetCeilingDripEmitter({px, 0.10f, pz}, seed);
                    auto markCovered = [&](Tile covered) {
                        reserveWaterDamageCoveredTile(covered);
                    };
                    markCovered(t);
                    if (w > tileW * 1.15f) {
                        markCovered({t.x - 1, t.y});
                        markCovered({t.x + 1, t.y});
                    }
                    if (d > tileD * 1.15f) {
                        markCovered({t.x, t.y - 1});
                        markCovered({t.x, t.y + 1});
                    }
                    if (w > tileW * 1.15f && d > tileD * 1.15f) {
                        markCovered({t.x - 1, t.y - 1});
                        markCovered({t.x + 1, t.y - 1});
                        markCovered({t.x - 1, t.y + 1});
                        markCovered({t.x + 1, t.y + 1});
                    }
                }
                return placed;
            };

            auto addWaterLikeLeak = [&](Tile t, int side, int leakIndex, bool wallOnly = false) {
                if (!wallHasSurface(t, side)) return false;
                if (waterDamageTileBlocked(t) ||
                    bloodCenterSeepCovered.find(bloodTileKey(t)) != bloodCenterSeepCovered.end()) {
                    return false;
                }
                XMFLOAT3 c = maze_.WorldCenter(t, 0.0f);
                float seed = Rand01(leakIndex, 2401, scatterSeed);
                float wallSpan = (side == 0 || side == 1) ? tileW : tileD;
                float leakW = wallSpan * (0.82f + Rand01(leakIndex, 2407, scatterSeed) * 0.13f);
                float lateralLimit = std::max(0.0f, wallSpan * 0.5f - leakW * 0.5f - 0.16f);
                float lateral = (Rand01(leakIndex, 2409, scatterSeed) - 0.5f) * lateralLimit * 2.0f;
                float topY = wallH - 0.0015f;
                float bottomY = 0.0015f;
                float h = std::max(0.2f, topY - bottomY);
                float centerY = (topY + bottomY) * 0.5f;
                XMFLOAT3 normal{0.0f, 0.0f, 1.0f};
                XMFLOAT3 right{1.0f, 0.0f, 0.0f};
                XMFLOAT3 inward{0.0f, 0.0f, 1.0f};
                XMFLOAT3 wallCenter{c.x, centerY, c.z};
                constexpr float kWaterLikeWallDecalInset = 0.0045f;
                constexpr float kWaterLikeSeamInset = 0.0010f;
                if (side == 0) {
                    normal = {0.0f, 0.0f, 1.0f};
                    right = {1.0f, 0.0f, 0.0f};
                    inward = {0.0f, 0.0f, 1.0f};
                    wallCenter = {c.x + lateral, centerY, c.z - tileD * 0.5f + kWaterLikeWallDecalInset};
                } else if (side == 1) {
                    normal = {0.0f, 0.0f, -1.0f};
                    right = {-1.0f, 0.0f, 0.0f};
                    inward = {0.0f, 0.0f, -1.0f};
                    wallCenter = {c.x + lateral, centerY, c.z + tileD * 0.5f - kWaterLikeWallDecalInset};
                } else if (side == 2) {
                    normal = {1.0f, 0.0f, 0.0f};
                    right = {0.0f, 0.0f, 1.0f};
                    inward = {1.0f, 0.0f, 0.0f};
                    wallCenter = {c.x - tileW * 0.5f + kWaterLikeWallDecalInset, centerY, c.z + lateral};
                } else {
                    normal = {-1.0f, 0.0f, 0.0f};
                    right = {0.0f, 0.0f, -1.0f};
                    inward = {-1.0f, 0.0f, 0.0f};
                    wallCenter = {c.x + tileW * 0.5f - kWaterLikeWallDecalInset, centerY, c.z + lateral};
                }
                wallCenter = Add3(wallCenter, Scale3(normal, 0.0008f + std::fmod(std::abs(seed) * 9.713f, 1.0f) * 0.0018f));

                XMFLOAT3 up{0.0f, 1.0f, 0.0f};
                XMFLOAT3 a = Add3(wallCenter, Add3(Scale3(right, -leakW * 0.5f), Scale3(up, -h * 0.5f)));
                XMFLOAT3 b = Add3(wallCenter, Add3(Scale3(right,  leakW * 0.5f), Scale3(up, -h * 0.5f)));
                XMFLOAT3 c0 = Add3(wallCenter, Add3(Scale3(right,  leakW * 0.5f), Scale3(up,  h * 0.5f)));
                XMFLOAT3 d0 = Add3(wallCenter, Add3(Scale3(right, -leakW * 0.5f), Scale3(up,  h * 0.5f)));
                float sourceMat = waterLikeMaterial(seed, 0.965f + seed * 0.025f);
                AddQuadUV(vertices, liquidIndices, a, b, c0, d0, normal, right, {0, 1}, {1, 1}, {1, 0}, {0, 0}, sourceMat);

                float axisLength = (side == 0 || side == 1) ? tileD : tileW;
                Tile forwardTile = neighborForSide(t, oppositeSide(side));
                bool canSpreadForward = maze_.IsOpen(forwardTile.x, forwardTile.y);
                float sourceD = axisLength * (0.88f + Rand01(leakIndex, 2419, scatterSeed) * 0.10f);
                if (canSpreadForward) sourceD += axisLength * (0.16f + Rand01(leakIndex, 2421, scatterSeed) * 0.22f);
                float seamYaw = liquidCardYawForSide(side);
                XMFLOAT3 ceilingEdge = Add3({wallCenter.x, wallH - kBloodCeilingDecalInset, wallCenter.z},
                    Scale3(inward, -kWaterLikeWallDecalInset));
                bool ceilingPlaced = addWaterLikeCeilingFromWall(ceilingEdge, inward, leakW, sourceD, seamYaw, seed, 0.965f + seed * 0.025f);
                float poolD = axisLength * (0.86f + Rand01(leakIndex, 2427, scatterSeed) * 0.12f);
                if (canSpreadForward) poolD += axisLength * (0.18f + Rand01(leakIndex, 2429, scatterSeed) * 0.24f);
                XMFLOAT3 floorEdge{wallCenter.x, 0.0f, wallCenter.z};
                bool floorPlaced = addWaterLikeFloorFromWall(floorEdge, inward, leakW, poolD, seamYaw, seed, 0.0f, 0.965f + seed * 0.025f);
                XMFLOAT3 floorCenter = Add3(floorEdge, Scale3(inward, poolD * 0.5f + 0.006f));
                if (floorPlaced && canSpreadForward) {
                    addWaterFloorBorderContinuation(t, side, wallCenter, right, inward, leakW, poolD,
                        sourceMat, seed, leakIndex + 43000);
                }
                if (ceilingPlaced || floorPlaced) {
                    reserveWaterDamageTile(t);
                    reserveWaterDamageCoveredTile(t);
                    if (canSpreadForward) reserveWaterDamageCoveredTile(forwardTile);
                }
                if (ceilingPlaced && floorPlaced) {
                    MarkWetCeilingDripEmitter({floorCenter.x, 0.10f, floorCenter.z}, seed);
                }

                if (!wallOnly && !gEffectDebugViewer) {
                    BloodScarePoint scare{};
                    scare.pos = {floorCenter.x, 0.10f, floorCenter.z};
                    scare.source = {wallCenter.x, wallH - 0.035f, wallCenter.z};
                    scare.normal = normal;
                    scare.radius = std::clamp(1.65f + std::max(leakW, poolD) * 0.70f, 2.10f, 4.80f);
                    scare.focusDelaySeconds = 0.22f + Rand01(leakIndex, 2433, scatterSeed) * 0.44f;
                    scare.requireFacing = true;
                    scare.dreadScale = 0.42f;
                    scare.revealBlood = false;
                    bloodScarePoints_.push_back(scare);
                }
                return true;
            };

            auto emitWaterLikeDamage = [&]() {
                bool waterDebug = gEffectDebugViewer && DebugSliceEffectIsWater(gDebugSliceEffect);
                if (!waterDebug && waterLikeDamageChance <= 0.0001f) return;
                if (waterDebug) {
                    int debugIndex = 0;
                    for (Tile t : openTiles) {
                        int sideCount = 0;
                        for (int side = 0; side < 4; ++side) {
                            if (wallHasSurface(t, side)) {
                                ++sideCount;
                                addWaterLikeLeak(t, side, 30000 + debugIndex++, true);
                            }
                        }
                        if (sideCount == 0) {
                            addWaterLikeCenterTile(t, 33000 + debugIndex++);
                        }
                    }
                    return;
                }
                int waterIndex = 0;
                for (Tile t : openTiles) {
                    if (t == maze_.start || t == maze_.exit) continue;
                    int sides[4]{};
                    int sideCount = 0;
                    if (wallHasSurface(t, 0)) sides[sideCount++] = 0;
                    if (wallHasSurface(t, 1)) sides[sideCount++] = 1;
                    if (wallHasSurface(t, 2)) sides[sideCount++] = 2;
                    if (wallHasSurface(t, 3)) sides[sideCount++] = 3;
                    float chance = waterLikeDamageChance;
                    if (sideCount == 0) chance *= 0.58f;
                    if (Rand01(waterIndex, 2501, scatterSeed) > chance) {
                        ++waterIndex;
                        continue;
                    }
                    if (sideCount == 0) {
                        addWaterLikeCenterTile(t, 25000 + waterIndex);
                    } else {
                        int side = sides[std::min(sideCount - 1,
                            static_cast<int>(Rand01(waterIndex, 2507, scatterSeed) * static_cast<float>(sideCount)))];
                        addWaterLikeLeak(t, side, 25000 + waterIndex);
                    }
                    ++waterIndex;
                }
            };

            if (settings_.waterDamageEnabled) emitWaterLikeDamage();

            if (settings_.bloodStudyView) {
                bloodStudyTile_ = FindBloodStudyTile();
                XMFLOAT3 studyCenter = maze_.WorldCenter(bloodStudyTile_, 0.0f);
                int studyLeaks = 0;
                for (int side = 0; side < 4; ++side) {
                    if (wallHasSurface(bloodStudyTile_, side) &&
                        addBloodLeak(bloodStudyTile_, side, 30000 + side)) {
                        ++studyLeaks;
                    }
                }
                if (studyLeaks <= 0) {
                    if (!addBloodCenterDripTile(bloodStudyTile_, 30017)) {
                        addBloodBurst(bloodStudyTile_, 30017);
                    }
                }
            }

            bool bloodWorldGeometry = !gBloodDebugEveryWall &&
                settings_.bloodWorldCoverage > 0.001f &&
                (settings_.bloodWorldFlicker || settings_.bloodWorldAlwaysOn);
            if (bloodWorldGeometry) {
                int worldLeakIndex = 0;
                float bloodQuality = std::clamp(settings_.bloodShaderQuality, 0.25f, 1.0f);
                float coverageScale = std::clamp(bloodQuality * bloodQuality * 1.35f, 0.16f, 1.0f);
                float densityGate = std::clamp(settings_.bloodSplatterDensity, 0.0f, 1.0f);
                float coverage = std::clamp(settings_.bloodWorldCoverage * densityGate * coverageScale * 0.03f, 0.0f, 1.0f);
                for (Tile t : openTiles) {
                    int wallSides = 0;
                    for (int side = 0; side < 4; ++side) {
                        if (!wallHasSurface(t, side)) continue;
                        ++wallSides;
                        if (coverage < 0.999f && Rand01(worldLeakIndex, 1201, scatterSeed) > coverage) {
                            ++worldLeakIndex;
                            continue;
                        }
                        addBloodLeak(t, side, 20000 + worldLeakIndex, true);
                        ++worldLeakIndex;
                    }
                    if (wallSides == 0) {
                        if (coverage >= 0.999f || Rand01(worldLeakIndex, 1207, scatterSeed) <= coverage * 0.62f) {
                            addBloodCenterDripTile(t, 20000 + worldLeakIndex);
                        }
                        ++worldLeakIndex;
                    }
                }
            }

            if (gBloodDebugEveryWall && gDebugSliceEffect == DebugSliceEffect::Blood) {
                int debugLeakIndex = 0;
                for (Tile t : openTiles) {
                    for (int side = 0; side < 4; ++side) {
                        if (wallHasSurface(t, side)) {
                            addBloodLeak(t, side, 10000 + debugLeakIndex++);
                        }
                    }
                    if (maze_.IsOpen(t.x, t.y - 1) && maze_.IsOpen(t.x, t.y + 1) &&
                        maze_.IsOpen(t.x - 1, t.y) && maze_.IsOpen(t.x + 1, t.y)) {
                        addBloodCenterDripTile(t, 13000 + debugLeakIndex++);
                    }
                }
            } else {
                float bloodLeakBudget = std::clamp(static_cast<float>(settings_.bloodBurstCount) * settings_.bloodSplatterDensity * 0.35f, 0.0f, 180.0f);
                int bloodLeaks = static_cast<int>(std::floor(bloodLeakBudget));
                float bloodLeakFraction = bloodLeakBudget - static_cast<float>(bloodLeaks);
                if (bloodLeaks < 180 && Rand01(9131, 9137, scatterSeed) < bloodLeakFraction) {
                    ++bloodLeaks;
                }
                int bloodLeakAttempts = bloodLeaks > 0 ? std::max(8, bloodLeaks * 14) : 0;
                int placedBloodLeaks = 0;
                for (int b = 0; b < bloodLeakAttempts && placedBloodLeaks < bloodLeaks; ++b) {
                    Tile t = openTiles[std::min(openTiles.size() - 1,
                        static_cast<size_t>(Rand01(b, 571, scatterSeed) * static_cast<float>(openTiles.size())))];
                    if (t == maze_.start || t == maze_.exit) continue;
                    int sides[4]{};
                    int sideCount = 0;
                    if (!maze_.IsOpen(t.x, t.y - 1)) sides[sideCount++] = 0;
                    if (!maze_.IsOpen(t.x, t.y + 1)) sides[sideCount++] = 1;
                    if (!maze_.IsOpen(t.x - 1, t.y)) sides[sideCount++] = 2;
                    if (!maze_.IsOpen(t.x + 1, t.y)) sides[sideCount++] = 3;
                    if (sideCount == 0) {
                        if (addBloodCenterDripTile(t, b)) {
                            ++placedBloodLeaks;
                        }
                        continue;
                    }
                    bool placedAnySide = false;
                    int firstIndex = std::min(sideCount - 1, static_cast<int>(Rand01(b, 579, scatterSeed) * static_cast<float>(sideCount)));
                    for (int step = 0; step < sideCount; ++step) {
                        int side = sides[(firstIndex + step) % sideCount];
                        placedAnySide = addBloodLeak(t, side, b * 4 + step) || placedAnySide;
                    }
                    if (placedAnySide) {
                        ++placedBloodLeaks;
                    }
                }
            }

            emitMergedLiquidFloorSeams();
            emitWaterWallCanvasRuns();
            emitLiquidCanvasTiles();

            float roomClutterDensity = std::clamp(settings_.chairDensity * 0.85f, 0.0f, 4.0f);
            int roomGroups = roomClutterDensity <= 0.001f
                ? 0
                : std::clamp(static_cast<int>(static_cast<float>(openTiles.size()) * 0.010f * roomClutterDensity), 4, 42);
            int roomAttempts = roomGroups * 7;
            int placedRoomGroups = 0;
            for (int g = 0; g < roomAttempts && placedRoomGroups < roomGroups; ++g) {
                size_t tileIndex = std::min(openTiles.size() - 1,
                    static_cast<size_t>(Rand01(g, 277, scatterSeed) * static_cast<float>(openTiles.size())));
                Tile t = openTiles[tileIndex];
                if (!IsRoomLike(t)) continue;
                if (addRoomClutterGroup(t, g, scatterSeed)) {
                    ++placedRoomGroups;
                }
            }

            int basePages = std::clamp(static_cast<int>(openTiles.size() / 3), 260, 900);
            int targetPages = std::clamp(static_cast<int>(basePages * paperDensity), 0, 3600);
            int attempts = targetPages * 4;
            int placed = 0;
            for (int i = 0; i < attempts && placed < targetPages; ++i) {
                size_t tileIndex = std::min(openTiles.size() - 1,
                    static_cast<size_t>(Rand01(i, 17, scatterSeed) * static_cast<float>(openTiles.size())));
                Tile t = openTiles[tileIndex];
                XMFLOAT3 c = maze_.WorldCenter(t, 0.0f);
                float px = c.x + (Rand01(i, 37, scatterSeed) - 0.5f) * std::max(0.10f, tileW - kA4PaperLongMeters - 0.10f);
                float pz = c.z + (Rand01(i, 41, scatterSeed) - 0.5f) * std::max(0.10f, tileD - kA4PaperLongMeters - 0.10f);
                float yaw = Rand01(i, 43, scatterSeed) * kPi * 2.0f;
                float lift = 0.0030f + Rand01(i, 47, scatterSeed) * 0.0015f;
                float material = loosePaperMaterial(Rand01(i, 52, scatterSeed), Rand01(i, 54, scatterSeed));
                if (addPaperAt(px, pz, yaw, lift, material)) {
                    if (Rand01(i, 49, scatterSeed) < 0.010f) {
                        addCassetteAt(px, pz, yaw, lift + 0.003f, Rand01(i, 51, scatterSeed));
                    }
                    ++placed;
                }
            }

            int baseRuns = std::clamp(static_cast<int>(openTiles.size() / 220), 5, 14);
            int runs = std::clamp(static_cast<int>(std::round(baseRuns * hallwayPaperDensity)), 0, 56);
            for (int run = 0; run < runs; ++run) {
                size_t tileIndex = std::min(openTiles.size() - 1,
                    static_cast<size_t>(Rand01(run, 53, scatterSeed) * static_cast<float>(openTiles.size())));
                Tile t = openTiles[tileIndex];
                bool ew = maze_.IsOpen(t.x - 1, t.y) && maze_.IsOpen(t.x + 1, t.y);
                bool ns = maze_.IsOpen(t.x, t.y - 1) && maze_.IsOpen(t.x, t.y + 1);
                if (!ew && !ns) continue;
                bool useEW = ew && (!ns || Rand01(run, 59, scatterSeed) < 0.5f);
                XMFLOAT3 c = maze_.WorldCenter(t, 0.0f);
                float runLen = (useEW ? tileW : tileD) * (2.4f + Rand01(run, 61, scatterSeed) * 4.8f);
                int count = std::max(1, static_cast<int>((22.0f + Rand01(run, 67, scatterSeed) * 34.0f) * hallwayPaperDensity));
                for (int p = 0; p < count; ++p) {
                    float along = (Rand01(run * 97 + p, 71, scatterSeed) - 0.5f) * runLen;
                    float cross = (Rand01(run * 97 + p, 73, scatterSeed) - 0.5f) * ((useEW ? tileD : tileW) * 0.82f);
                    float px = c.x + (useEW ? along : cross);
                    float pz = c.z + (useEW ? cross : along);
                    float yaw = Rand01(run * 97 + p, 97, scatterSeed) * kPi * 2.0f;
                    float lift = 0.0032f + p * 0.00010f + Rand01(run * 97 + p, 101, scatterSeed) * 0.0014f;
                    float material = loosePaperMaterial(Rand01(run * 97 + p, 109, scatterSeed), Rand01(run * 97 + p, 113, scatterSeed));
                    if (addPaperAt(px, pz, yaw, lift, material) &&
                        Rand01(run * 97 + p, 103, scatterSeed) < 0.010f) {
                        addCassetteAt(px, pz, yaw, lift + 0.003f, Rand01(run * 97 + p, 107, scatterSeed));
                    }
                }
                propLookPoints_.push_back({c.x, 0.16f, c.z});
            }
        }

        for (int tileY = 0; tileY < maze_.h; ++tileY) {
            for (int tileX = 0; tileX < maze_.w; ++tileX) {
                if (!maze_.IsOpen(tileX, tileY)) continue;

                Tile lampTile{tileX, tileY};
                XMFLOAT3 lampCenter = maze_.WorldCenter(lampTile, 0.0f);
                int cellX = tileX;
                int cellZ = tileY;
                float seed = LampSeed(cellX, cellZ);
                bool brokenZone = LampBrokenZone(cellX, cellZ);
                bool lampOn = !brokenZone && seed >= 1.0f - settings_.lampOnRatio;
                bool brokenPanel = brokenZone &&
                    LampHash(static_cast<float>(cellX) - 19.7f, static_cast<float>(cellZ) + 88.4f) < settings_.sparkEmitterRatio;
                bool wetLampTile = IsWetCeilingTile(lampTile) || IsWetFootstepTile(lampTile);
                bool jumpscareLamp = wetLampTile && lampOn && IsPlayableSimulationMode(runtimeMode_) &&
                    LampHash(static_cast<float>(cellX) + 151.3f, static_cast<float>(cellZ) - 207.9f) < settings_.sparkEmitterRatio;
                if (runtimeMode_ == RendererRuntimeMode::MainMenu) {
                    lampOn = lampTile == maze_.start;
                    brokenPanel = false;
                    jumpscareLamp = false;
                }
                float panelW = tileW * (1.01f / 3.0f);
                float panelD = tileD * (1.01f / 3.0f);
                float material = lampOn ? 3.0f + seed * 0.49f : 5.0f;
                AddCeilingCard(vertices, indices, {lampCenter.x, 0.0f, lampCenter.z},
                    panelW, panelD, 0.0f, wallH - 0.004f, material);

                if (lampOn) {
                    runtimeLamps_.push_back({
                        lampTile,
                        {lampCenter.x, wallH - 0.08f, lampCenter.z},
                        0.0f,
                        RandRange(0.08f, 0.72f),
                        false,
                        [&]() {
                            float humRoll = LampHash(static_cast<float>(cellX) + 14.7f, static_cast<float>(cellZ) - 42.3f);
                            if (humRoll < 0.05f) return 2;
                            if (humRoll < 0.15f) return 1;
                            return 0;
                        }()
                    });
                    if (jumpscareLamp && settings_.sparkParticles) {
                        sparkEmitters_.push_back({{lampCenter.x, wallH - 0.085f, lampCenter.z}});
                    }
                } else if (wetLampTile && brokenPanel && settings_.sparkParticles) {
                    sparkEmitters_.push_back({{lampCenter.x, wallH - 0.085f, lampCenter.z}});
                }
                if (brokenZone && !lampDamagePixels_.empty()) {
                    lampDamagePixels_[static_cast<size_t>(tileY * maze_.w + tileX)] = 255;
                    lampDamageDirty_ = true;
                }
            }
        }
        if (runtimeMode_ == RendererRuntimeMode::MainMenu && !lampDamagePixels_.empty()) {
            for (int tileY = 0; tileY < maze_.h; ++tileY) {
                for (int tileX = 0; tileX < maze_.w; ++tileX) {
                    if (!maze_.IsOpen(tileX, tileY) || Tile{tileX, tileY} == maze_.start) continue;
                    lampDamagePixels_[static_cast<size_t>(tileY * maze_.w + tileX)] = 255;
                }
            }
            Tile sparkTile{std::clamp(maze_.start.x + 1, 1, maze_.w - 2), std::max(1, maze_.start.y - 5)};
            if (maze_.IsOpen(sparkTile.x, sparkTile.y) && settings_.sparkParticles) {
                XMFLOAT3 sparkCenter = maze_.WorldCenter(sparkTile, 0.0f);
                sparkEmitters_.push_back({{sparkCenter.x, wallH - 0.085f, sparkCenter.z}});
            }
        }

        CreateLampDamageTexture();

        floorCeilingStartIndex_ = static_cast<UINT>(indices.size());
        for (int y = 0; y < maze_.h; ++y) {
            int x = 0;
            while (x < maze_.w) {
                while (x < maze_.w && !maze_.IsOpen(x, y)) ++x;
                int start = x;
                while (x < maze_.w && maze_.IsOpen(x, y)) ++x;
                if (start < x) addFloorCeilingRun(y, start, x);
            }
        }
        floorCeilingIndexCount_ = static_cast<UINT>(indices.size()) - floorCeilingStartIndex_;

        {
            std::vector<uint32_t> chunkedStaticIndices;
            chunkedStaticIndices.reserve(indices.size());
            UINT oldOpaqueCount = floorCeilingStartIndex_;
            UINT oldFloorCeilingStart = floorCeilingStartIndex_;
            UINT oldFloorCeilingCount = floorCeilingIndexCount_;
            AppendStaticIndexChunks(vertices, indices, 0, oldOpaqueCount, chunkedStaticIndices, staticOpaqueChunks_);
            floorCeilingStartIndex_ = static_cast<UINT>(chunkedStaticIndices.size());
            AppendStaticIndexChunks(vertices, indices, oldFloorCeilingStart, oldFloorCeilingCount, chunkedStaticIndices, staticFloorCeilingChunks_);
            floorCeilingIndexCount_ = static_cast<UINT>(chunkedStaticIndices.size()) - floorCeilingStartIndex_;
            indices.swap(chunkedStaticIndices);
        }

        staticWaterStartIndex_ = static_cast<UINT>(indices.size());
        AppendStaticIndexChunks(vertices, liquidIndices, 0, static_cast<UINT>(liquidIndices.size()), indices, staticWaterChunks_);
        staticWaterIndexCount_ = static_cast<UINT>(indices.size()) - staticWaterStartIndex_;
        staticTransparentStartIndex_ = static_cast<UINT>(indices.size());
        AppendStaticIndexChunks(vertices, transparentIndices, 0, static_cast<UINT>(transparentIndices.size()), indices, staticTransparentChunks_);
        staticTransparentIndexCount_ = static_cast<UINT>(indices.size()) - staticTransparentStartIndex_;
        staticPropShadowStartIndex_ = static_cast<UINT>(indices.size());
        indices.insert(indices.end(), propShadowIndices.begin(), propShadowIndices.end());
        staticPropShadowIndexCount_ = static_cast<UINT>(indices.size()) - staticPropShadowStartIndex_;

        {
            std::wstringstream counts;
            counts << L"Static scene geometry: vertices=" << vertices.size()
                << L", indices=" << indices.size()
                << L", opaqueIndices=" << std::min(floorCeilingStartIndex_, static_cast<UINT>(indices.size()))
                << L", floorCeilingIndices=" << floorCeilingIndexCount_
                << L", waterIndices=" << staticWaterIndexCount_
                << L", transparentIndices=" << staticTransparentIndexCount_
                << L", propShadowIndices=" << staticPropShadowIndexCount_
                << L", runtimeLamps=" << runtimeLamps_.size()
                << L", sparkEmitters=" << sparkEmitters_.size()
                << L", steamEmitters=" << steamEmitters_.size();
            StartupProfileLine(counts.str());
        }

        D3D11_BUFFER_DESC vb{};
        vb.ByteWidth = static_cast<UINT>(vertices.size() * sizeof(Vertex));
        vb.Usage = D3D11_USAGE_IMMUTABLE;
        vb.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        D3D11_SUBRESOURCE_DATA vd{vertices.data(), 0, 0};
        device_->CreateBuffer(&vb, &vd, &vertexBuffer_);

        D3D11_BUFFER_DESC ib{};
        ib.ByteWidth = static_cast<UINT>(indices.size() * sizeof(uint32_t));
        ib.Usage = D3D11_USAGE_IMMUTABLE;
        ib.BindFlags = D3D11_BIND_INDEX_BUFFER;
        D3D11_SUBRESOURCE_DATA id{indices.data(), 0, 0};
        device_->CreateBuffer(&ib, &id, &indexBuffer_);
        indexCount_ = static_cast<UINT>(indices.size());

        D3D11_BUFFER_DESC mb{};
        mb.ByteWidth = sizeof(Vertex) * 6;
        mb.Usage = D3D11_USAGE_DYNAMIC;
        mb.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        mb.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        device_->CreateBuffer(&mb, nullptr, &monsterBuffer_);

        D3D11_BUFFER_DESC db{};
        db.ByteWidth = sizeof(Vertex) * kDynamicVertexCapacity;
        db.Usage = D3D11_USAGE_DYNAMIC;
        db.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        db.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        device_->CreateBuffer(&db, nullptr, &dynamicBuffer_);

        D3D11_BUFFER_DESC ob{};
        ob.ByteWidth = sizeof(OverlayVertex) * kOverlayVertexCapacity;
        ob.Usage = D3D11_USAGE_DYNAMIC;
        ob.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        ob.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        device_->CreateBuffer(&ob, nullptr, &overlayBuffer_);
    }
