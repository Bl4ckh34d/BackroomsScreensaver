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
