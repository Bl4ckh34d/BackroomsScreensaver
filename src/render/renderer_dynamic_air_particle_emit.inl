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
