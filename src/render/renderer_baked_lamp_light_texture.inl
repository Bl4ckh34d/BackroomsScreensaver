    static constexpr int kBakedLampLightTexelsPerTile = 32;

    float CurrentBakedLampDirtProgression() const {
        if (IsPlayableSimulationMode(sessionRuntime_.mode) && gameWorld_.PlayableRunActive()) {
            return gameWorld_.MapDirtProgression();
        }
        if (sessionRuntime_.mode == RendererRuntimeMode::MainMenu) {
            return menuRuntime_.darkLayerOneRun ? 0.72f : 0.42f;
        }
        if (gEffectDebugViewer && DebugSliceEffectIsWater(gDebugSliceEffect)) {
            return 0.90f;
        }
        float damagePressure = (settingsRuntime_.live.waterDamageEnabled ? 0.24f : 0.0f) +
            std::clamp(settingsRuntime_.live.waterDamageDensity, 0.0f, 4.0f) * 0.16f +
            std::clamp(settingsRuntime_.live.bloodSplatterDensity, 0.0f, 4.0f) * 0.12f +
            std::clamp(settingsRuntime_.live.jumpscareFrequency, 0.0f, 1.0f) * 0.12f +
            std::clamp(settingsRuntime_.live.brokenZoneRatio, 0.0f, 1.0f) * 0.18f;
        return Clamp01(0.34f + damagePressure);
    }

    float BakedLampApproxDirt(Tile tile, float dirtProgression, const Maze& maze) const {
        float cellX = static_cast<float>(tile.x);
        float cellY = static_cast<float>(tile.y);
        float broadGrime = LampHash(cellX * 0.23f + static_cast<float>(maze.w) * 0.017f,
                                    cellY * 0.23f + static_cast<float>(maze.h) * 0.017f);
        float clusterGrime = LampHash(cellX * 0.071f - static_cast<float>(maze.h) * 0.011f,
                                      cellY * 0.071f - static_cast<float>(maze.w) * 0.011f);
        float flyChance = SmoothStep(0.60f, 0.95f, LampHash(cellX + 311.0f, cellY + 311.0f) + dirtProgression * 0.22f);
        return Clamp01((0.10f + dirtProgression * 0.74f) * (0.30f + broadGrime * 0.74f) +
            SmoothStep(0.58f, 0.88f, clusterGrime) * (0.12f + dirtProgression * 0.22f) +
            flyChance * (0.04f + dirtProgression * 0.12f));
    }

    bool BakedLampReceiverTile(const Maze& maze, int x, int y) const {
        if (maze.IsOpen(x, y)) return true;
        MazeWallFeature feature = maze.WallFeature(x, y);
        return feature == MazeWallFeature::Window || feature == MazeWallFeature::Tunnel;
    }

    float BakedLampReceiverDirectScale(const Maze& maze, int x, int y) const {
        MazeWallFeature feature = maze.WallFeature(x, y);
        return feature == MazeWallFeature::Tunnel ? 0.24f : 1.0f;
    }

    bool BakedLampReceiverVisible(const Maze& maze, Tile receiver, Tile lamp) const {
        if (maze.IsVisionOpen(receiver.x, receiver.y)) {
            return maze.LineClear(receiver, lamp);
        }
        const Tile neighbors[4] = {
            {receiver.x + 1, receiver.y},
            {receiver.x - 1, receiver.y},
            {receiver.x, receiver.y + 1},
            {receiver.x, receiver.y - 1}
        };
        for (Tile n : neighbors) {
            if (maze.IsOpen(n.x, n.y) && maze.LineClear(n, lamp)) return true;
        }
        return false;
    }

    float BakedLampDamageAt(Tile tile, const Maze& maze) const {
        if (!maze.InBounds(tile.x, tile.y) || effectRuntime_.lampDamagePixels.empty()) return 0.0f;
        size_t index = static_cast<size_t>(tile.y * maze.w + tile.x);
        if (index >= effectRuntime_.lampDamagePixels.size()) return 0.0f;
        return static_cast<float>(effectRuntime_.lampDamagePixels[index]) / 255.0f;
    }

    void SmoothBakedLampLightPixels(const Maze& maze, int bakeW, int bakeH) {
        const size_t pixelCount = static_cast<size_t>(bakeW) * static_cast<size_t>(bakeH);
        if (pixelCount == 0 || effectRuntime_.bakedLampLightPixels.size() != pixelCount) return;

        const int radius = std::clamp(kBakedLampLightTexelsPerTile / 3, 2, 10);
        if (radius <= 0) return;

        std::vector<uint8_t> receiverMask(pixelCount, 0);
        for (int py = 0; py < bakeH; ++py) {
            const float cellY = (static_cast<float>(py) + 0.5f) / static_cast<float>(kBakedLampLightTexelsPerTile);
            const int y = std::clamp(static_cast<int>(std::floor(cellY)), 0, maze.h - 1);
            for (int px = 0; px < bakeW; ++px) {
                const float cellX = (static_cast<float>(px) + 0.5f) / static_cast<float>(kBakedLampLightTexelsPerTile);
                const int x = std::clamp(static_cast<int>(std::floor(cellX)), 0, maze.w - 1);
                receiverMask[static_cast<size_t>(py * bakeW + px)] = BakedLampReceiverTile(maze, x, y) ? 1 : 0;
            }
        }

        std::vector<XMFLOAT4> temp(pixelCount, XMFLOAT4{0.0f, 0.0f, 0.0f, 0.0f});
        for (int py = 0; py < bakeH; ++py) {
            for (int px = 0; px < bakeW; ++px) {
                const size_t dst = static_cast<size_t>(py * bakeW + px);
                if (!receiverMask[dst]) continue;
                XMFLOAT4 sum{0.0f, 0.0f, 0.0f, 0.0f};
                float weightSum = 0.0f;
                for (int ox = -radius; ox <= radius; ++ox) {
                    const int sx = px + ox;
                    if (sx < 0 || sx >= bakeW) continue;
                    const size_t src = static_cast<size_t>(py * bakeW + sx);
                    if (!receiverMask[src]) continue;
                    const float w = 1.0f - static_cast<float>(std::abs(ox)) / static_cast<float>(radius + 1);
                    const XMFLOAT4& p = effectRuntime_.bakedLampLightPixels[src];
                    sum.x += p.x * w;
                    sum.y += p.y * w;
                    sum.z += p.z * w;
                    sum.w += p.w * w;
                    weightSum += w;
                }
                if (weightSum > 0.001f) {
                    const float inv = 1.0f / weightSum;
                    temp[dst] = {sum.x * inv, sum.y * inv, sum.z * inv, sum.w * inv};
                }
            }
        }

        constexpr float kSmoothBlend = 0.72f;
        for (int py = 0; py < bakeH; ++py) {
            for (int px = 0; px < bakeW; ++px) {
                const size_t dst = static_cast<size_t>(py * bakeW + px);
                if (!receiverMask[dst]) continue;
                XMFLOAT4 sum{0.0f, 0.0f, 0.0f, 0.0f};
                float weightSum = 0.0f;
                for (int oy = -radius; oy <= radius; ++oy) {
                    const int sy = py + oy;
                    if (sy < 0 || sy >= bakeH) continue;
                    const size_t src = static_cast<size_t>(sy * bakeW + px);
                    if (!receiverMask[src]) continue;
                    const float w = 1.0f - static_cast<float>(std::abs(oy)) / static_cast<float>(radius + 1);
                    const XMFLOAT4& p = temp[src];
                    sum.x += p.x * w;
                    sum.y += p.y * w;
                    sum.z += p.z * w;
                    sum.w += p.w * w;
                    weightSum += w;
                }
                if (weightSum > 0.001f) {
                    const float inv = 1.0f / weightSum;
                    XMFLOAT4& base = effectRuntime_.bakedLampLightPixels[dst];
                    const XMFLOAT4 smoothed{sum.x * inv, sum.y * inv, sum.z * inv, sum.w * inv};
                    base.x = Lerp(base.x, smoothed.x, kSmoothBlend);
                    base.y = Lerp(base.y, smoothed.y, kSmoothBlend);
                    base.z = Lerp(base.z, smoothed.z, kSmoothBlend);
                    base.w = Lerp(base.w, smoothed.w, kSmoothBlend);
                }
            }
        }
    }

    void RebuildBakedLampLightPixels(const Maze& maze) {
        const int bakeW = std::max(1, maze.w * kBakedLampLightTexelsPerTile);
        const int bakeH = std::max(1, maze.h * kBakedLampLightTexelsPerTile);
        const size_t pixelCount = static_cast<size_t>(bakeW) * static_cast<size_t>(bakeH);
        effectRuntime_.bakedLampLightPixels.assign(pixelCount, XMFLOAT4{0.0f, 0.0f, 0.0f, 0.0f});
        if (maze.w <= 0 || maze.h <= 0 || effectRuntime_.runtimeLamps.empty()) {
            effectRuntime_.bakedLampLightDirty = true;
            return;
        }

        const float dirtProgression = CurrentBakedLampDirtProgression();
        const float tileAvg = std::max(0.10f, maze.TileAverage());
        const float reach = std::clamp(std::max(tileAvg * 2.65f, settingsRuntime_.live.lampSpacing * 1.05f),
            tileAvg * 2.35f, tileAvg * 3.65f);
        const float reachSq = reach * reach;
        const float sourceSearchReach = reach + tileAvg * 0.85f;
        const float sourceSearchReachSq = sourceSearchReach * sourceSearchReach;
        const int sourceTileRadius = std::clamp(
            static_cast<int>(std::ceil(sourceSearchReach / std::max(0.10f, maze.TileMinimum()))),
            3, 5);
        const float lampHeight = std::max(0.1f, settingsRuntime_.live.wallHeightMeters - 0.09f);
        const float receiverY = std::max(0.1f, settingsRuntime_.live.wallHeightMeters * 0.42f);
        const float verticalDelta = lampHeight - receiverY;
        const float originX = -static_cast<float>(maze.w) * maze.tileW * 0.5f;
        const float originZ = -static_cast<float>(maze.h) * maze.tileD * 0.5f;
        const size_t cellCount = static_cast<size_t>(maze.w) * static_cast<size_t>(maze.h);

        std::vector<int> lampLookup(cellCount, -1);
        for (size_t i = 0; i < effectRuntime_.runtimeLamps.size(); ++i) {
            const RuntimeLampState& lamp = effectRuntime_.runtimeLamps[i];
            if (lamp.broken || !maze.InBounds(lamp.tile.x, lamp.tile.y)) continue;
            lampLookup[static_cast<size_t>(lamp.tile.y * maze.w + lamp.tile.x)] = static_cast<int>(i);
        }

        struct BakedLampSource {
            const RuntimeLampState* lamp = nullptr;
            float seed = 0.0f;
            float variation = 1.0f;
            float dirtTint = 0.0f;
            float dirtTransmission = 1.0f;
            float damageDim = 1.0f;
            bool flickerEligible = false;
        };

        std::vector<std::vector<BakedLampSource>> visibleLampSources(cellCount);
        for (int y = 0; y < maze.h; ++y) {
            for (int x = 0; x < maze.w; ++x) {
                if (!BakedLampReceiverTile(maze, x, y)) continue;
                Tile receiver{x, y};
                std::vector<BakedLampSource>& sources = visibleLampSources[static_cast<size_t>(y * maze.w + x)];
                sources.reserve(12);
                XMFLOAT3 receiverCenter = maze.WorldCenter(receiver, receiverY);
                for (int yy = std::max(0, y - sourceTileRadius); yy <= std::min(maze.h - 1, y + sourceTileRadius); ++yy) {
                    for (int xx = std::max(0, x - sourceTileRadius); xx <= std::min(maze.w - 1, x + sourceTileRadius); ++xx) {
                        int lampIndex = lampLookup[static_cast<size_t>(yy * maze.w + xx)];
                        if (lampIndex < 0) continue;
                        const RuntimeLampState& lamp = effectRuntime_.runtimeLamps[static_cast<size_t>(lampIndex)];
                        float tileDx = lamp.pos.x - receiverCenter.x;
                        float tileDz = lamp.pos.z - receiverCenter.z;
                        if (tileDx * tileDx + tileDz * tileDz > sourceSearchReachSq) continue;
                        float damage = std::max(lamp.damage, BakedLampDamageAt(lamp.tile, maze));
                        if (damage >= 0.995f) continue;
                        if (!BakedLampReceiverVisible(maze, receiver, lamp.tile)) continue;

                        float seed = LampHash(static_cast<float>(lamp.tile.x), static_cast<float>(lamp.tile.y));
                        float variation = Lerp(0.86f, 1.14f,
                            LampHash(static_cast<float>(lamp.tile.x) + 53.0f, static_cast<float>(lamp.tile.y) + 53.0f));
                        float dirt = BakedLampApproxDirt(lamp.tile, dirtProgression, maze);
                        float dirtTint = SmoothStep(0.05f, 0.95f, dirt);
                        float dirtDim = Lerp(1.08f, 0.56f, SmoothStep(0.04f, 0.96f, dirt));
                        float damageDim = Lerp(1.0f, 0.24f, damage * damage);
                        bool flickerFixture = LampHash(static_cast<float>(lamp.tile.x) + 17.0f,
                                                       static_cast<float>(lamp.tile.y) + 17.0f) >=
                            1.0f - settingsRuntime_.live.lampFlickerRatio;
                        sources.push_back({
                            &lamp,
                            seed,
                            variation,
                            dirtTint,
                            Lerp(1.0f, 0.72f, dirtTint) * dirtDim,
                            damageDim,
                            flickerFixture || damage > 0.06f
                        });
                    }
                }
            }
        }

        for (int py = 0; py < bakeH; ++py) {
            const float cellY = (static_cast<float>(py) + 0.5f) / static_cast<float>(kBakedLampLightTexelsPerTile);
            const int y = std::clamp(static_cast<int>(std::floor(cellY)), 0, maze.h - 1);
            for (int px = 0; px < bakeW; ++px) {
                const float cellX = (static_cast<float>(px) + 0.5f) / static_cast<float>(kBakedLampLightTexelsPerTile);
                const int x = std::clamp(static_cast<int>(std::floor(cellX)), 0, maze.w - 1);
                if (!BakedLampReceiverTile(maze, x, y)) continue;

                const std::vector<BakedLampSource>& sources = visibleLampSources[static_cast<size_t>(y * maze.w + x)];
                if (sources.empty()) continue;
                const float receiverDirectScale = BakedLampReceiverDirectScale(maze, x, y);

                XMFLOAT3 samplePos{
                    originX + cellX * maze.tileW,
                    receiverY,
                    originZ + cellY * maze.tileD
                };
                float r = 0.0f;
                float g = 0.0f;
                float b = 0.0f;
                float totalContribution = 0.0f;
                float flickerContribution = 0.0f;

                for (const BakedLampSource& source : sources) {
                    const RuntimeLampState& lamp = *source.lamp;
                    float dx = lamp.pos.x - samplePos.x;
                    float dz = lamp.pos.z - samplePos.z;
                    float distXZSqr = dx * dx + dz * dz;
                    if (distXZSqr > reachSq) continue;

                    float distXZ = std::sqrt(std::max(0.0f, distXZSqr));
                    float roomFootprint = 1.0f - SmoothStep(reach * 0.52f, reach, distXZ);
                    if (roomFootprint <= 0.002f) continue;

                    float d2 = distXZSqr + verticalDelta * verticalDelta;
                    float baseFalloff = roomFootprint / (1.0f + d2 * 0.029f);
                    if (baseFalloff <= 0.00045f) continue;

                    float contribution = source.variation * source.damageDim * baseFalloff *
                        source.dirtTransmission * receiverDirectScale * 0.76f;
                    if (contribution <= 0.00045f) continue;

                    float warmR = Lerp(1.0f, 1.0f, source.dirtTint);
                    float warmG = Lerp(0.985f, 0.72f, source.dirtTint);
                    float warmB = Lerp(0.90f, 0.34f, source.dirtTint);
                    r += warmR * contribution;
                    g += warmG * contribution;
                    b += warmB * contribution;
                    totalContribution += contribution;

                    if (source.flickerEligible) {
                        flickerContribution += contribution;
                    }
                }

                float flickerAmount = totalContribution > 0.0001f
                    ? Clamp01(flickerContribution / totalContribution)
                    : 0.0f;
                effectRuntime_.bakedLampLightPixels[static_cast<size_t>(py * bakeW + px)] =
                    XMFLOAT4{r, g, b, flickerAmount};
            }
        }

        SmoothBakedLampLightPixels(maze, bakeW, bakeH);
        effectRuntime_.bakedLampLightDirty = true;
    }

    bool CreateBakedLampLightTexture() {
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        if (!d3dRuntime_.device || !world.maze || world.maze->w <= 0 || world.maze->h <= 0) return false;
        const Maze& maze = *world.maze;
        runtimeTextures_.bakedLampLightTexture.Reset();
        runtimeTextures_.bakedLampLightSrv.Reset();

        RebuildBakedLampLightPixels(maze);
        const int bakeW = std::max(1, maze.w * kBakedLampLightTexelsPerTile);
        const int bakeH = std::max(1, maze.h * kBakedLampLightTexelsPerTile);

        D3D11_TEXTURE2D_DESC td{};
        td.Width = static_cast<UINT>(bakeW);
        td.Height = static_cast<UINT>(bakeH);
        td.MipLevels = 1;
        td.ArraySize = 1;
        td.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        td.SampleDesc.Count = 1;
        td.Usage = D3D11_USAGE_DEFAULT;
        td.BindFlags = D3D11_BIND_SHADER_RESOURCE;

        D3D11_SUBRESOURCE_DATA init{};
        init.pSysMem = effectRuntime_.bakedLampLightPixels.data();
        init.SysMemPitch = static_cast<UINT>(bakeW * sizeof(XMFLOAT4));

        HRESULT hr = d3dRuntime_.device->CreateTexture2D(&td, &init, &runtimeTextures_.bakedLampLightTexture);
        if (FAILED(hr)) return false;

        D3D11_SHADER_RESOURCE_VIEW_DESC sd{};
        sd.Format = td.Format;
        sd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        sd.Texture2D.MostDetailedMip = 0;
        sd.Texture2D.MipLevels = 1;
        hr = d3dRuntime_.device->CreateShaderResourceView(runtimeTextures_.bakedLampLightTexture.Get(), &sd, &runtimeTextures_.bakedLampLightSrv);
        if (FAILED(hr)) return false;
        effectRuntime_.bakedLampLightDirty = false;
        return true;
    }

    void UploadBakedLampLightTexture() {
        if (!runtimeTextures_.bakedLampLightTexture) return;
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        if (!world.maze || world.maze->w <= 0 || world.maze->h <= 0) return;
        const Maze& maze = *world.maze;
        const int bakeW = std::max(1, maze.w * kBakedLampLightTexelsPerTile);
        const int bakeH = std::max(1, maze.h * kBakedLampLightTexelsPerTile);
        const size_t pixelCount = static_cast<size_t>(bakeW) * static_cast<size_t>(bakeH);
        if (effectRuntime_.bakedLampLightPixels.size() != pixelCount || effectRuntime_.lampDamageDirty) {
            RebuildBakedLampLightPixels(maze);
        }
        if (!effectRuntime_.bakedLampLightDirty || effectRuntime_.bakedLampLightPixels.empty()) return;
        d3dRuntime_.context->UpdateSubresource(
            runtimeTextures_.bakedLampLightTexture.Get(),
            0,
            nullptr,
            effectRuntime_.bakedLampLightPixels.data(),
            static_cast<UINT>(bakeW * sizeof(XMFLOAT4)),
            0);
        effectRuntime_.bakedLampLightDirty = false;
    }
