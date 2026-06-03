// Dynamic particle, page, save-point, and small-effect geometry helpers. 
// Included inside Renderer's private section from renderer_dynamic_geometry.inl.

    void AppendSparkBillboards(std::vector<Vertex>& verts) {
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        float tileAverage = world.maze ? world.maze->TileAverage() : kTile;
        XMVECTOR cam = XMLoadFloat3(&world.playerPosition);
        XMVECTOR up = XMVectorSet(0, 1, 0, 0);
        float maxDist = std::max(6.0f, std::min(settingsRuntime_.live.fogEndMeters + tileAverage * 2.0f, tileAverage * 6.0f));
        for (size_t i = 0; i < effectRuntime_.sparks.size(); ++i) {
            const SparkParticle& spark = effectRuntime_.sparks[i];
            float lifeLeft = Clamp01(1.0f - spark.age / std::max(0.001f, spark.life));
            if (lifeLeft <= 0.0f) continue;
            float radius = spark.size * (1.30f + lifeLeft * 2.35f);
            if (!DynamicBillboardVisible(spark.pos, radius, maxDist, 0.34f)) continue;
            XMFLOAT3 to = Sub3(spark.pos, world.playerPosition);
            float distSq = Dot3(to, to);
            float farT = Clamp01((std::sqrt(distSq) - maxDist * 0.48f) / std::max(0.1f, maxDist * 0.42f));
            if (farT > 0.35f && ((i + static_cast<size_t>(static_cast<int>(timeRuntime_.time * 18.0f))) & 1u) != 0u) continue;
            XMVECTOR pos = XMLoadFloat3(&spark.pos);
            XMVECTOR toCam = XMVector3Normalize(cam - pos);
            XMVECTOR right = XMVector3Normalize(XMVector3Cross(up, toCam));
            float size = spark.size * (0.55f + lifeLeft * lifeLeft * 1.35f);
            if (size * lifeLeft < 0.004f) continue;
            XMVECTOR halfW = right * size;
            XMVECTOR halfH = up * size;
            XMFLOAT3 n, t, a, b, c, d;
            XMStoreFloat3(&n, toCam);
            XMStoreFloat3(&t, right);
            XMStoreFloat3(&a, pos - halfW - halfH);
            XMStoreFloat3(&b, pos + halfW - halfH);
            XMStoreFloat3(&c, pos + halfW + halfH);
            XMStoreFloat3(&d, pos - halfW + halfH);
            AppendDynamicQuad(verts, a, b, c, d, n, t, 13.05f + lifeLeft * 0.38f);
        }
    }

    void AppendAirParticleBillboards(std::vector<Vertex>& verts) {
        if (!settingsRuntime_.live.airParticles || effectRuntime_.airParticles.empty() || monsterPreview_.active) return;
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        XMFLOAT3 lightOrigin = FlashlightOrigin();
        XMFLOAT3 lightDir = Normalize3(FlashlightForward(), {0.0f, 0.0f, 1.0f});
        XMFLOAT3 cameraForward = Normalize3(DirectionFromYawPitch(world.playerYaw, world.playerPitch), {0.0f, 0.0f, 1.0f});
        float maxDist = std::clamp(settingsRuntime_.live.flashlightShadowDistanceMeters * 0.70f, 5.5f, 12.0f);
        float coneHalf = std::clamp(settingsRuntime_.live.flashlightConeDegrees, 20.0f, 140.0f) * 0.5f * kPi / 180.0f;
        float coneOuter = std::cos(coneHalf);
        float coneInner = std::cos(std::max(3.0f * kPi / 180.0f, coneHalf * 0.50f));
        float levelDensity = AirParticleLevelDensityScale();
        int maxParticles = std::min<int>(static_cast<int>(effectRuntime_.airParticles.size()),
            std::clamp(static_cast<int>(1900.0f * std::clamp(settingsRuntime_.live.airParticleDensity, 0.0f, 4.0f) * levelDensity), 0, 5200));
        int emitted = 0;
        int farParticlePhase = static_cast<int>(timeRuntime_.time * 24.0f) & 1;
        XMFLOAT3 worldUp{0.0f, 1.0f, 0.0f};
        float maxDistSq = maxDist * maxDist;
        for (const AirParticle& p : effectRuntime_.airParticles) {
            if (emitted >= maxParticles) break;
            XMFLOAT3 pos = p.pos;
            XMFLOAT3 fromCamera = Sub3(pos, world.playerPosition);
            float cameraDepth = Dot3(fromCamera, cameraForward);
            if (cameraDepth <= 0.035f) continue;
            if (p.nearLayer < 0.5f && cameraDepth > maxDist * 0.55f) {
                int particlePhase = static_cast<int>(p.seed * 4096.0f) & 1;
                if (particlePhase != farParticlePhase) continue;
            }

            XMFLOAT3 fromLight = Sub3(pos, lightOrigin);
            float lightDistSq = Dot3(fromLight, fromLight);
            float lightDist = std::sqrt(std::max(0.0001f, lightDistSq));
            bool flashlightLit = false;
            if (lightDistSq >= 0.09f && lightDistSq <= maxDistSq && lightDist >= 0.30f && lightDist <= maxDist) {
                XMFLOAT3 ray = Scale3(fromLight, 1.0f / std::max(0.001f, lightDist));
                flashlightLit = SmoothStep(coneOuter, coneInner, Dot3(ray, lightDir)) > 0.018f;
            }
            float fixtureDist = std::numeric_limits<float>::infinity();
            bool fixtureLit = false;
            if (fixtureShadowRuntime_.active) {
                XMFLOAT3 fromFixture = Sub3(pos, fixtureShadowRuntime_.position);
                fixtureDist = std::sqrt(std::max(0.0001f, Dot3(fromFixture, fromFixture)));
                fixtureLit = fixtureDist <= fixtureShadowRuntime_.range * 0.92f && pos.y < fixtureShadowRuntime_.position.y + 0.05f;
            }
            if (!flashlightLit && !fixtureLit) continue;
            float lightingDist = flashlightLit ? lightDist : fixtureDist;
            if (fixtureLit && fixtureDist < lightingDist) lightingDist = fixtureDist;
            float focusBlur = Clamp01(Clamp01(std::abs(lightingDist - viewRuntime_.airFocusDistance) / (0.62f + lightingDist * 0.18f)) *
                std::clamp(settingsRuntime_.live.airParticleBlur, 0.0f, 3.0f));
            float distanceT = Clamp01(lightingDist / maxDist);
            float distanceScale = Lerp(0.52f, 0.085f, distanceT);
            if (p.nearLayer > 0.5f) {
                distanceScale = std::max(distanceScale, p.nearLayer > 1.5f ? 0.70f : 0.60f);
            }
            float size = p.size * distanceScale * (1.0f + focusBlur * 0.24f);
            float projectedPixels = (size / std::max(0.06f, cameraDepth)) * static_cast<float>(std::max<LONG>(1, hostRuntime_.height)) * 0.72f;
            if (projectedPixels < (p.nearLayer < 0.5f ? 0.34f : 0.20f)) continue;
            if (distanceT > 0.72f && ((emitted + static_cast<int>(timeRuntime_.time * 11.0f)) & 1) != 0) continue;
            float lifeFade = SmoothStep(0.0f, 2.8f, p.age) * (1.0f - SmoothStep(p.life - 5.2f, p.life, p.age));
            if (lifeFade <= 0.01f) continue;
            size *= Lerp(0.18f, 1.0f, lifeFade);
            XMFLOAT3 toCam = Normalize3(Sub3(world.playerPosition, pos), Scale3(lightDir, -1.0f));
            XMFLOAT3 right = Normalize3(Cross3(worldUp, toCam), {1.0f, 0.0f, 0.0f});
            XMFLOAT3 up = Normalize3(Cross3(toCam, right), worldUp);
            float angle = p.angle;
            XMFLOAT3 side = Normalize3(Add3(Scale3(right, std::cos(angle)), Scale3(up, std::sin(angle))), right);
            XMFLOAT3 vertical = Normalize3(Add3(Scale3(up, std::cos(angle)), Scale3(right, -std::sin(angle))), up);
            float aspect = std::clamp(p.aspect, 0.32f, 3.40f);
            float aspectRoot = std::sqrt(aspect);
            float halfW = size * std::clamp(aspectRoot, 0.58f, 1.84f);
            float halfH = size * std::clamp(1.0f / aspectRoot, 0.54f, 1.76f);
            XMFLOAT3 hw = Scale3(side, halfW);
            XMFLOAT3 hh = Scale3(vertical, halfH);
            float material = 15.0f + std::min(0.985f, lifeFade * 0.94f + p.seed * 0.035f);
            AppendDynamicQuad(verts,
                Add3(pos, Add3(Scale3(hw, -1.0f), Scale3(hh, -1.0f))),
                Add3(pos, Add3(hw, Scale3(hh, -1.0f))),
                Add3(pos, Add3(hw, hh)),
                Add3(pos, Add3(Scale3(hw, -1.0f), hh)),
                toCam, side, material);
            ++emitted;
        }
    }

    void AppendSteamBillboards(std::vector<Vertex>& verts) {
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        float tileAverage = world.maze ? world.maze->TileAverage() : kTile;
        XMVECTOR cam = XMLoadFloat3(&world.playerPosition);
        XMVECTOR up = XMVectorSet(0, 1, 0, 0);
        float maxDist = std::max(7.0f, std::min(settingsRuntime_.live.fogEndMeters + tileAverage * 2.0f, tileAverage * 5.5f));
        for (size_t i = 0; i < effectRuntime_.steam.size(); ++i) {
            const SteamParticle& sp = effectRuntime_.steam[i];
            float lifeLeft = Clamp01(1.0f - sp.age / std::max(0.001f, sp.life));
            if (lifeLeft <= 0.0f) continue;
            float radius = sp.size * (1.60f + (1.0f - lifeLeft) * 2.2f);
            if (!DynamicBillboardVisible(sp.pos, radius, maxDist, 0.40f)) continue;
            XMFLOAT3 to = Sub3(sp.pos, world.playerPosition);
            float dist = std::sqrt(Dot3(to, to));
            float farT = Clamp01((dist - maxDist * 0.45f) / std::max(0.1f, maxDist * 0.45f));
            if (farT > 0.30f && ((i + static_cast<size_t>(static_cast<int>(timeRuntime_.time * 9.0f))) & 1u) != 0u) continue;
            XMVECTOR pos = XMLoadFloat3(&sp.pos);
            XMVECTOR toCam = XMVector3Normalize(cam - pos);
            XMVECTOR right = XMVector3Normalize(XMVector3Cross(up, toCam));
            float size = sp.size * (0.65f + (1.0f - lifeLeft) * 1.55f);
            if (size * lifeLeft < 0.006f) continue;
            XMVECTOR halfW = right * size;
            XMVECTOR halfH = up * (size * 0.78f);
            XMFLOAT3 n, t, a, b, c, d;
            XMStoreFloat3(&n, toCam);
            XMStoreFloat3(&t, right);
            XMStoreFloat3(&a, pos - halfW - halfH);
            XMStoreFloat3(&b, pos + halfW - halfH);
            XMStoreFloat3(&c, pos + halfW + halfH);
            XMStoreFloat3(&d, pos - halfW + halfH);
            AppendDynamicQuad(verts, a, b, c, d, n, t, 12.58f + lifeLeft * 0.35f);
        }
    }

    void AppendVentDrops(std::vector<Vertex>& verts) {
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        float tileAverage = world.maze ? world.maze->TileAverage() : kTile;
        float maxDist = std::max(7.0f, tileAverage * 5.5f);
        for (const VentDrop& d : effectRuntime_.ventDrops) {
            if (!DynamicVisualCandidate(d.pos, std::max(d.halfW, d.halfH) * 2.0f, maxDist)) continue;
            XMFLOAT3 right = RotateYVec({1.0f, 0.0f, 0.0f}, d.yaw);
            XMFLOAT3 forward = RotateYVec({0.0f, 0.0f, 1.0f}, d.yaw);
            XMVECTOR upVec = XMVector3Normalize(XMVectorSet(0, std::cos(d.roll), 0, 0) + XMLoadFloat3(&forward) * std::sin(d.roll));
            XMVECTOR rightVec = XMLoadFloat3(&right);
            XMVECTOR normalVec = XMVector3Normalize(XMVector3Cross(upVec, rightVec));
            XMVECTOR center = XMLoadFloat3(&d.pos);
            XMVECTOR halfW = rightVec * d.halfW;
            XMVECTOR halfH = upVec * d.halfH;
            XMFLOAT3 n, t, a, b, c, e;
            XMStoreFloat3(&n, normalVec);
            XMStoreFloat3(&t, rightVec);
            XMStoreFloat3(&a, center - halfW - halfH);
            XMStoreFloat3(&b, center + halfW - halfH);
            XMStoreFloat3(&c, center + halfW + halfH);
            XMStoreFloat3(&e, center - halfW + halfH);
            AppendDynamicQuad(verts, a, b, c, e, n, t, 10.0f);
            AppendDynamicQuad(verts, b, a, e, c, Scale3(n, -1.0f), Scale3(t, -1.0f), 10.0f);
        }
    }
