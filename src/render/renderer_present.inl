    void Render() {
        lastPresentCompleted_ = false;
        if (!presentEnabled_) {
            lastPresentCompleted_ = true;
            return;
        }
        if (!rtv_ || !dsv_) {
            lastPresentCompleted_ = true;
            return;
        }
        StartupProfile renderProfile(L"Render");
        BeginGpuProfileFrame();
        bool postAvailable = sceneColorRtv_ && sceneColorSrv_ && postVertexShader_ && postPixelShader_ && postSampler_;
        ID3D11RenderTargetView* sceneTarget = postAvailable ? sceneColorRtv_.Get() : rtv_.Get();

        float clear[4] = {0.004f, 0.004f, 0.004f, 1.0f};
        if (monsterPreview_) {
            if (monsterPreviewView_ == MonsterPreviewView::Orbit) {
                clear[0] = clear[1] = clear[2] = 0.93f;
            } else {
                clear[0] = clear[1] = clear[2] = 0.0f;
            }
        }
        context_->ClearRenderTargetView(rtv_.Get(), clear);
        if (postAvailable) {
            context_->ClearRenderTargetView(sceneColorRtv_.Get(), clear);
        }
        context_->ClearDepthStencilView(dsv_.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
        renderProfile.Mark(L"ClearTargets");
        MarkGpuProfile(GpuProfileMarker::ClearTargets);

        D3D11_VIEWPORT vp{};
        vp.Width = static_cast<float>(width_);
        vp.Height = static_cast<float>(height_);
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        context_->RSSetViewports(1, &vp);

        float deathProgress = deathActive_ ? Clamp01(deathTimer_ / 4.25f) : 0.0f;
        float deathFadeProgress = deathActive_ ? Clamp01((deathTimer_ - 0.10f) / 3.55f) : 0.0f;
        float deathFocus = deathActive_ ? SmoothStep(0.0f, 0.24f, deathTimer_) : 0.0f;
        XMFLOAT3 f = Forward();
        float stepPhase = stepPhase_;
        float runCameraMotion = 1.0f + runIntensity_ * 0.42f + runEffort_ * 0.55f;
        float stepWave = std::sin(stepPhase * 2.0f);
        float sideSway = stepWave * settings_.sideSwayAmount * 0.26f * runCameraMotion * (1.0f - deathFocus);
        XMVECTOR right = XMVectorSet(std::cos(bodyYaw_), 0.0f, -std::sin(bodyYaw_), 0.0f);
        XMVECTOR eye = XMLoadFloat3(&camera_) + right * sideSway;
        XMVECTOR worldUp = XMVectorSet(0, 1, 0, 0);
        float bobPitch = stepWave * 0.0045f * runCameraMotion * (1.0f - deathFocus);
        XMVECTOR viewDir = XMVector3Normalize(XMVectorSet(f.x, lookPitch_ + bobPitch, f.z, 0.0f));
        XMVECTOR viewRight = XMVector3Normalize(XMVector3Cross(worldUp, viewDir));
        if (monsterPreview_ && monsterPreviewView_ == MonsterPreviewView::Top) {
            XMVECTOR topTarget = XMVectorSet(monster_.x, 1.20f, monster_.z, 0.0f);
            viewDir = XMVector3Normalize(topTarget - eye);
            worldUp = XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f);
            viewRight = XMVector3Normalize(XMVector3Cross(worldUp, viewDir));
        }
        if (deathActive_) {
            float jitterStrength = (1.0f - SmoothStep(0.74f, 1.0f, deathProgress)) * (0.010f + deathProgress * 0.025f);
            float jitterX = (std::sin(time_ * 31.0f) + std::sin(time_ * 57.0f) * 0.45f) * jitterStrength;
            float jitterY = (std::sin(time_ * 43.0f) + std::sin(time_ * 71.0f) * 0.35f) * jitterStrength;
            viewDir = XMVector3Normalize(viewDir + viewRight * jitterX + worldUp * jitterY);
            viewRight = XMVector3Normalize(XMVector3Cross(worldUp, viewDir));
        }
        float tunnelRoll = tunnelLeanSide_ * SmoothStep(0.0f, 1.0f, tunnelLeanAmount_) * 0.115f;
        float roll = deathActive_
            ? (std::sin(time_ * 18.0f) * 0.075f + std::sin(time_ * 37.0f) * 0.045f) * SmoothStep(0.06f, 0.42f, deathProgress)
            : stumbleYawOffset_ * 0.12f * SmoothStep(0.0f, 0.35f, stumbleTimer_) + tunnelRoll;
        XMVECTOR up = XMVector3Normalize(worldUp * std::cos(roll) + viewRight * std::sin(roll));
        XMVECTOR at = eye + viewDir;
        XMMATRIX view = XMMatrixLookAtLH(eye, at, up);
        float aspect = static_cast<float>(std::max<LONG>(1, width_)) / static_cast<float>(std::max<LONG>(1, height_));
        float fovDegrees = monsterPreview_ ? 48.0f : 70.0f - dangerLevel_ * 3.5f;
        if (runtimeMode_ == RendererRuntimeMode::MainMenu) {
            fovDegrees = 84.0f;
        }
        if (gEffectDebugViewer || gBloodDebugEveryWall || settings_.bloodStudyView) {
            fovDegrees = 86.0f;
        }
        if (deathActive_) {
            fovDegrees = Lerp(70.0f, 58.0f, SmoothStep(0.05f, 0.66f, deathProgress));
        }
        float exitStepStart = ExitAlignSeconds() + std::max(0.05f, settings_.exitDoorOpenSeconds * 0.68f);
        float exitStepEnd = exitStepStart + settings_.exitStepSeconds;
        float exitFade = exitTransitionActive_ ? SmoothStep(exitStepEnd - settings_.exitFadeSeconds * 0.64f, exitStepEnd + settings_.exitFadeSeconds * 0.36f, exitTransitionTimer_) : 0.0f;
        float fadeIn = (fadeInTimer_ > 0.0f && settings_.fadeInSeconds > 0.001f)
            ? 1.0f - SmoothStep(0.0f, settings_.fadeInSeconds, settings_.fadeInSeconds - fadeInTimer_)
            : 0.0f;
        float transitionFade = std::max(std::max(exitFade, fadeIn), menuStartTransitionFade_);
        float viewFarMeters = monsterPreview_ ? 1000.0f : std::max(72.0f, settings_.fogEndMeters + maze_.TileAverage() * 12.0f);
        XMMATRIX proj = XMMatrixPerspectiveFovLH(fovDegrees * kPi / 180.0f, aspect, 0.045f, viewFarMeters);

        XMFLOAT3 flashlightForward = FlashlightForward();
        XMVECTOR lightDir = XMVector3Normalize(XMVectorSet(flashlightForward.x, flashlightForward.y, flashlightForward.z, 0.0f));
        XMVECTOR lightPos = eye + viewRight * 0.16f - up * 0.18f + viewDir * 0.08f;
        float shadowDistance = settings_.flashlightShadowDistanceMeters;
        float coneHalfRadians = std::clamp(settings_.flashlightConeDegrees, 20.0f, 140.0f) * 0.5f * kPi / 180.0f;
        float coneOuter = std::cos(coneHalfRadians);
        float coneInner = std::cos(std::max(3.0f * kPi / 180.0f, coneHalfRadians * 0.36f));
        float shadowFov = std::clamp(settings_.flashlightConeDegrees + 8.0f, 36.0f, 150.0f) * kPi / 180.0f;
        XMMATRIX lightView = XMMatrixLookAtLH(lightPos, lightPos + lightDir, up);
        XMMATRIX lightProj = XMMatrixPerspectiveFovLH(shadowFov, 1.0f, 0.06f, shadowDistance);
        XMMATRIX lightViewProj = lightView * lightProj;
        XMFLOAT3 lightPosFloat{};
        XMFLOAT3 lightDirFloat{};
        XMStoreFloat3(&lightPosFloat, lightPos);
        XMStoreFloat3(&lightDirFloat, lightDir);

        SceneConstants cb{};
        XMStoreFloat4x4(&cb.viewProj, view * proj);
        XMStoreFloat4x4(&cb.lightViewProj, lightViewProj);
        XMStoreFloat4x4(&cb.fixtureLightViewProj, XMMatrixIdentity());
        XMStoreFloat4x4(&cb.monsterEyeViewProj0, XMMatrixIdentity());
        XMStoreFloat4x4(&cb.monsterEyeViewProj1, XMMatrixIdentity());
        XMFLOAT3 eyePos{};
        XMStoreFloat3(&eyePos, eye);
        XMFLOAT3 viewDirFloat{};
        XMStoreFloat3(&viewDirFloat, viewDir);

        bool fixtureShadowActive = false;
        XMFLOAT3 fixtureShadowPos{0.0f, 0.0f, 0.0f};
        XMFLOAT3 fixtureShadowDir{0.0f, -1.0f, 0.0f};
        float fixtureShadowRange = std::max(settings_.wallHeightMeters + 0.75f, maze_.TileAverage() * 1.70f);
        XMMATRIX fixtureLightViewProj = XMMatrixIdentity();
        if (!monsterPreview_ && IsPlayableSimulationMode(runtimeMode_) && fixtureShadowDsv_ && fixtureShadowSrv_ &&
            settings_.lampIntensity > 0.001f && settings_.lampOnRatio > 0.001f && !runtimeLamps_.empty()) {
            Tile cameraTile = CameraTile();
            float maxCandidateDistance = std::clamp(maze_.TileAverage() * 3.2f, 5.5f, 10.5f);
            float maxCandidateDistanceSq = maxCandidateDistance * maxCandidateDistance;
            float bestScore = std::numeric_limits<float>::infinity();
            const RuntimeLampState* bestLamp = nullptr;
            for (const RuntimeLampState& lamp : runtimeLamps_) {
                if (lamp.broken || !maze_.InBounds(lamp.tile.x, lamp.tile.y) || !maze_.LineClear(cameraTile, lamp.tile)) continue;
                float dx = lamp.pos.x - eyePos.x;
                float dy = lamp.pos.y - eyePos.y;
                float dz = lamp.pos.z - eyePos.z;
                float distSq = dx * dx + dy * dy + dz * dz;
                if (distSq > maxCandidateDistanceSq) continue;
                float dist = std::sqrt(std::max(0.0001f, distSq));
                float viewFacing = (dx * viewDirFloat.x + dy * viewDirFloat.y + dz * viewDirFloat.z) / dist;
                if (dist > maze_.TileAverage() * 1.25f && viewFacing < -0.18f) continue;
                float screenBias = 1.0f - std::clamp((viewFacing + 0.18f) / 1.18f, 0.0f, 1.0f);
                float score = distSq * (0.62f + screenBias * 1.25f);
                if (score < bestScore) {
                    bestScore = score;
                    bestLamp = &lamp;
                }
            }
            if (bestLamp) {
                fixtureShadowActive = true;
                fixtureShadowPos = bestLamp->pos;
                fixtureShadowPos.y -= 0.015f;
                XMVECTOR fixturePos = XMLoadFloat3(&fixtureShadowPos);
                XMVECTOR fixtureDir = XMLoadFloat3(&fixtureShadowDir);
                XMVECTOR fixtureUp = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
                XMMATRIX fixtureView = XMMatrixLookAtLH(fixturePos, fixturePos + fixtureDir, fixtureUp);
                XMMATRIX fixtureProj = XMMatrixPerspectiveFovLH(116.0f * kPi / 180.0f, 1.0f, 0.035f, fixtureShadowRange);
                fixtureLightViewProj = fixtureView * fixtureProj;
                XMStoreFloat4x4(&cb.fixtureLightViewProj, fixtureLightViewProj);
            }
        }
        float flashlightIntensity = settings_.flashlightIntensity *
            (runtimeMode_ == RendererRuntimeMode::MainMenu ? 1.0f : DreadFlashlightMultiplier());
        if (runtimeMode_ == RendererRuntimeMode::PlayableGame && !flashlightEnabled_) flashlightIntensity = 0.0f;
        if (monsterPreview_) flashlightIntensity = 1.45f;
        cb.cameraPosTime = {eyePos.x, eyePos.y, eyePos.z, time_};
        cb.cameraDirAspect = {lightDirFloat.x, lightDirFloat.y, lightDirFloat.z, aspect};
        cb.lighting0 = {
            flashlightIntensity,
            settings_.flashlightAttenuation,
            settings_.ambientLight,
            std::max(2.0f, settings_.lampSpacing)
        };
        if (runtimeMode_ == RendererRuntimeMode::MainMenu) {
            float menuDoorAmbient = SmoothStep(0.04f, 0.78f, exitDoorAngle_ / 1.38f);
            cb.lighting0.z = Lerp(std::max(settings_.ambientLight, 0.045f), 0.160f, menuDoorAmbient);
        }
        if (monsterPreview_) {
            cb.lighting0.y = 0.030f;
            cb.lighting0.z = 0.22f;
        }
        if (gEffectDebugViewer) {
            cb.lighting0.x = std::clamp(cb.lighting0.x, 0.55f, 0.92f);
            cb.lighting0.y = std::max(cb.lighting0.y, 0.085f);
            cb.lighting0.z = kEffectDebugAmbientLight;
        } else if (gBloodDebugEveryWall || settings_.bloodStudyView) {
            cb.lighting0.x = std::max(cb.lighting0.x, 1.85f);
            cb.lighting0.y = std::min(cb.lighting0.y, 0.040f);
            cb.lighting0.z = std::max(cb.lighting0.z, 0.42f);
        }
        cb.lighting1 = {
            settings_.lampIntensity,
            settings_.lampOnRatio,
            settings_.lampFlickerRatio,
            settings_.brokenZoneRatio
        };
        if (gEffectDebugViewer) {
            cb.lighting1.x = (gDebugSliceEffect == DebugSliceEffect::CeilingLamps) ? kEffectDebugLampIntensity : 0.0f;
            cb.lighting1.y = (gDebugSliceEffect == DebugSliceEffect::CeilingLamps) ? 1.0f : 0.0f;
            cb.lighting1.z = 0.0f;
            cb.lighting1.w = (gDebugSliceEffect == DebugSliceEffect::BrokenLamps) ? 1.0f : 0.0f;
        } else if (gBloodDebugEveryWall || settings_.bloodStudyView) {
            cb.lighting1.x = std::max(cb.lighting1.x, 2.35f);
            cb.lighting1.y = 1.0f;
            cb.lighting1.z = 0.0f;
            cb.lighting1.w = 0.0f;
        }
        cb.fog0 = {
            settings_.fogStartMeters,
            settings_.fogEndMeters,
            settings_.fogDarkness,
            0.0f
        };
        if (monsterPreview_) {
            cb.fog0 = {1000.0f, 1001.0f, 0.0f, 0.0f};
        }
        if (gEffectDebugViewer || gBloodDebugEveryWall || settings_.bloodStudyView) {
            cb.fog0 = {1000.0f, 1001.0f, 0.0f, 0.0f};
        }
        cb.ao0 = {
            settings_.cornerAOIntensity,
            settings_.cornerAORadius,
            settings_.floorCeilingAOIntensity,
            maze_.TileAverage()
        };
        cb.post0 = {
            settings_.exposure,
            settings_.gamma,
            deathActive_ ? 1.0f : dangerLevel_,
            deathFadeProgress
        };
        cb.post1 = {
            cameraMotionBlur_.x,
            cameraMotionBlur_.y,
            std::clamp(settings_.bloomAmount, 0.0f, 2.0f),
            std::clamp(settings_.lensDirtAmount, 0.0f, 2.0f)
        };
        if (runtimeMode_ == RendererRuntimeMode::MainMenu) {
            float menuDoorGlow = SmoothStep(0.08f, 0.82f, exitDoorAngle_ / 1.38f);
            cb.post0.x = Lerp(cb.post0.x, std::max(cb.post0.x, 1.03f), menuDoorGlow);
            cb.post1.z = Lerp(cb.post1.z, std::max(cb.post1.z, 0.34f), menuDoorGlow);
            cb.post1.w = Lerp(cb.post1.w, std::max(cb.post1.w, 0.22f), menuDoorGlow);
        }
        float visionFlashT = visionFlashDuration_ > 0.001f ? Clamp01(visionFlashTimer_ / visionFlashDuration_) : 0.0f;
        cb.post2 = {
            visionFlashT * visionFlashT,
            0.0f,
            0.0f,
            0.0f
        };
        if (monsterPreview_) {
            cb.post0 = {1.0f, 2.2f, 0.0f, 0.0f};
            cb.post1 = {0.0f, 0.0f, 0.0f, 0.0f};
            cb.post2 = {0.0f, 0.0f, 0.0f, 0.0f};
        }
        cb.shadow0 = {
            lightPosFloat.x,
            lightPosFloat.y,
            lightPosFloat.z,
            settings_.flashlightShadows ? settings_.flashlightShadowStrength : 0.0f
        };
        cb.shadow1 = {
            lightDirFloat.x,
            lightDirFloat.y,
            lightDirFloat.z,
            settings_.flashlightShadowBias
        };
        cb.shadow2 = {
            1.0f / static_cast<float>(std::max<UINT>(1, shadowMapSize_)),
            shadowDistance,
            coneOuter,
            coneInner
        };
        cb.fixtureShadow0 = {
            fixtureShadowPos.x,
            fixtureShadowPos.y,
            fixtureShadowPos.z,
            fixtureShadowActive ? std::clamp(settings_.flashlightShadowStrength * 0.72f, 0.0f, 0.62f) : 0.0f
        };
        cb.fixtureShadow1 = {
            std::max(settings_.flashlightShadowBias * 1.35f, 0.00062f),
            fixtureShadowRange,
            1.0f / static_cast<float>(std::max<UINT>(1, fixtureShadowMapSize_)),
            0.0f
        };
        currentFixtureShadowActive_ = fixtureShadowActive;
        currentFixtureShadowPos_ = fixtureShadowPos;
        currentFixtureShadowRange_ = fixtureShadowRange;
        cb.maze0 = {
            -static_cast<float>(maze_.w) * maze_.tileW * 0.5f,
            -static_cast<float>(maze_.h) * maze_.tileD * 0.5f,
            maze_.tileW,
            maze_.tileD
        };
        cb.maze1 = {
            static_cast<float>(maze_.w),
            static_cast<float>(maze_.h),
            settings_.wallHeightMeters,
            maze_.TileAverage()
        };
        float dirtProgression = 0.48f;
        if (IsPlayableSimulationMode(runtimeMode_) && playableRun_.active) {
            if (playableRun_.customGame) {
                dirtProgression = Clamp01(static_cast<float>(playableRun_.customSpec.mapDirtPercent) / 100.0f);
            } else {
                float levelProgress = Clamp01((static_cast<float>(playableRun_.levelInLayer) - 1.0f) /
                    static_cast<float>(std::max(1, PlayableRunState::kLevelsPerLayer - 1)));
                float layerProgress = Clamp01(static_cast<float>(std::max(0, playableRun_.layer - 1)) * 0.18f);
                dirtProgression = Clamp01(levelProgress + layerProgress);
            }
        } else if (runtimeMode_ == RendererRuntimeMode::MainMenu) {
            dirtProgression = menuDarkLayerOneRun_ ? 0.72f : 0.42f;
        } else if (gEffectDebugViewer && DebugSliceEffectIsWater(gDebugSliceEffect)) {
            dirtProgression = 0.90f;
        } else {
            float damagePressure = (settings_.waterDamageEnabled ? 0.24f : 0.0f) +
                std::clamp(settings_.waterDamageDensity, 0.0f, 4.0f) * 0.16f +
                std::clamp(settings_.bloodSplatterDensity, 0.0f, 4.0f) * 0.12f +
                std::clamp(settings_.jumpscareFrequency, 0.0f, 1.0f) * 0.12f +
                std::clamp(settings_.brokenZoneRatio, 0.0f, 1.0f) * 0.18f;
            dirtProgression = Clamp01(0.34f + damagePressure);
        }
        cb.texture0 = {
            settings_.wallTextureMeters,
            settings_.floorTextureMeters,
            settings_.ceilingTextureMeters,
            dirtProgression
        };
        cb.transition0 = {
            transitionFade,
            exitTransitionActive_ ? exitDoorAngle_ : 0.0f,
            0.0f,
            0.0f
        };
        if (gEffectDebugViewer && DebugSliceEffectIsWater(gDebugSliceEffect)) {
            cb.transition0.w = 1.0f + DebugSliceLoopPhase();
        } else if (!gEffectDebugViewer) {
            cb.transition0.w = 0.0f;
        }
        float fleshAmount = 0.0f;
        if (fleshFlickerTimer_ > 0.0f && fleshFlickerDuration_ > 0.001f) {
            float elapsed = fleshFlickerDuration_ - fleshFlickerTimer_;
            float phase = Clamp01(elapsed / fleshFlickerDuration_);
            float envelope = SmoothStep(0.0f, 0.045f, elapsed) * (1.0f - SmoothStep(fleshFlickerDuration_ - 0.10f, fleshFlickerDuration_, elapsed));
            float strobe = (std::sin(phase * kPi * 4.0f) > 0.0f) ? 1.0f : 0.0f;
            fleshAmount = envelope * strobe * settings_.fleshFlickerIntensity;
        }
        if (settings_.fleshAlwaysOn) fleshAmount = std::max(fleshAmount, settings_.fleshFlickerIntensity);
        cb.horror0 = {
            Clamp01(fleshAmount),
            std::clamp(settings_.bloodWetness, 0.0f, 3.0f),
            std::clamp(settings_.fleshWetness, 0.0f, 4.0f),
            std::clamp(settings_.fleshParallaxScale, 0.0f, 0.50f)
        };
        float bloodStreamCount = static_cast<float>(std::clamp(settings_.bloodStreamCount, 4, 32));
        float bloodStreamThickness = std::clamp(settings_.bloodStreamThickness, 0.10f, 2.0f);
        float bloodShaderQuality = std::clamp(settings_.bloodShaderQuality, 0.25f, 1.0f);
        float bloodWorldAmount = 0.0f;
        if (bloodWorldFlickerTimer_ > 0.0f && bloodWorldFlickerDuration_ > 0.001f) {
            float elapsed = bloodWorldFlickerDuration_ - bloodWorldFlickerTimer_;
            float envelope = SmoothStep(0.0f, 0.055f, elapsed) *
                (1.0f - SmoothStep(bloodWorldFlickerDuration_ - 0.18f, bloodWorldFlickerDuration_, elapsed));
            float strobe = ((std::sin(elapsed * 41.0f) + std::sin(elapsed * 93.0f) * 0.48f + std::sin(elapsed * 151.0f) * 0.22f) > -0.06f) ? 1.0f : 0.0f;
            bloodWorldAmount = envelope * strobe * settings_.bloodWorldFlickerIntensity;
        }
        if (settings_.bloodWorldAlwaysOn && settings_.bloodWorldCoverage > 0.001f) {
            bloodWorldAmount = std::max(bloodWorldAmount, settings_.bloodWorldFlickerIntensity);
            if (bloodWorldActivationTime_ < -900.0f) bloodWorldActivationTime_ = time_ - 46.0f;
        }
        if (runtimeMode_ == RendererRuntimeMode::MainMenu && settings_.bloodWorldCoverage > 0.001f) {
            bloodWorldAmount = std::max(bloodWorldAmount, menuBloodAmount_);
        }
        cb.blood0 = {0.0f, 0.0f, 0.0f, 0.0f};
        cb.blood1 = {0.0f, 0.0f, 0.0f, 0.0f};
        cb.blood2 = {0.0f, 0.0f, 0.0f, 0.0f};
        cb.blood3 = {0.0f, 0.0f, 0.0f, 0.0f};
        cb.blood4 = {0.0f, 0.0f, 0.0f, 0.0f};
        cb.blood5 = {0.0f, 0.0f, 0.0f, 0.0f};
        cb.blood6 = {0.0f, 0.0f, 0.0f, 0.0f};
        cb.blood7 = {0.0f, 0.0f, 0.0f, 0.0f};
        cb.blood8 = {0.0f, 0.0f, 0.0f, 0.0f};
        if (gBloodDebugEveryWall) {
            float debugClock = DebugSliceClockSeconds();
            float cycleAge = std::fmod(std::max(0.0f, debugClock), std::max(0.1f, settings_.effectBloodLoopSeconds));
            float debugAge = std::min(cycleAge, settings_.effectBloodFullSpreadAge);
            cb.blood0 = {0.0f, 0.0f, time_ - debugAge, 1.0f};
            cb.blood1 = {bloodStreamCount, kEffectBloodRevealRadius, bloodStreamThickness, bloodShaderQuality};
        } else if (settings_.bloodStudyView) {
            cb.blood0 = {0.0f, 0.0f, -40.0f, 1.0f};
            cb.blood1 = {bloodStreamCount, kEffectBloodRevealRadius, bloodStreamThickness, bloodShaderQuality};
        } else if (bloodWorldAmount > 0.001f && settings_.bloodWorldCoverage > 0.001f) {
            XMFLOAT3 bloodCenter = {camera_.x, 0.0f, camera_.z};
            float revealRadius = kEffectBloodRevealRadius;
            if (runtimeMode_ == RendererRuntimeMode::MainMenu) {
                XMFLOAT3 c = maze_.WorldCenter(maze_.start, 0.0f);
                bloodCenter = {c.x + maze_.tileW * 0.14f, 0.0f, c.z + maze_.tileD * 0.50f};
                revealRadius = std::max(2.35f, std::max(maze_.tileW, maze_.tileD) * 1.35f);
            }
            cb.blood0 = {bloodCenter.x, bloodCenter.z, bloodWorldActivationTime_, Clamp01(bloodWorldAmount)};
            cb.blood1 = {bloodStreamCount, revealRadius, bloodStreamThickness, bloodShaderQuality};
        } else if (!bloodRevealRegions_.empty()) {
            std::array<const BloodRevealRegion*, 8> nearestRegions{};
            std::array<float, 8> nearestDistSq{};
            nearestDistSq.fill(std::numeric_limits<float>::infinity());
            size_t nearestCount = 0;
            for (const BloodRevealRegion& region : bloodRevealRegions_) {
                if (region.radius <= 0.001f || region.activationTime <= -999000.0f) continue;
                float dx = region.center.x - camera_.x;
                float dz = region.center.z - camera_.z;
                float distSq = dx * dx + dz * dz;
                if (nearestCount < nearestRegions.size()) {
                    nearestRegions[nearestCount] = &region;
                    nearestDistSq[nearestCount] = distSq;
                    ++nearestCount;
                } else {
                    size_t slot = nearestRegions.size() - 1;
                    for (size_t i = 0; i < nearestRegions.size(); ++i) {
                        if (nearestDistSq[i] > nearestDistSq[slot]) slot = i;
                    }
                    if (distSq >= nearestDistSq[slot]) continue;
                    nearestRegions[slot] = &region;
                    nearestDistSq[slot] = distSq;
                }
            }
            for (size_t i = 1; i < nearestCount; ++i) {
                const BloodRevealRegion* region = nearestRegions[i];
                float distSq = nearestDistSq[i];
                size_t j = i;
                while (j > 0 && distSq < nearestDistSq[j - 1]) {
                    nearestRegions[j] = nearestRegions[j - 1];
                    nearestDistSq[j] = nearestDistSq[j - 1];
                    --j;
                }
                nearestRegions[j] = region;
                nearestDistSq[j] = distSq;
            }
            if (nearestCount > 0 && nearestRegions[0]) {
                const BloodRevealRegion& primary = *nearestRegions[0];
                cb.blood0 = {primary.center.x, primary.center.z, primary.activationTime, 1.0f};
                cb.blood1 = {bloodStreamCount, std::max(1.0f, primary.radius), bloodStreamThickness, bloodShaderQuality};
                auto assignRegion = [&](size_t slot, const BloodRevealRegion& region) {
                    XMFLOAT4 value{
                        region.center.x,
                        region.center.z,
                        region.activationTime,
                        std::max(1.0f, region.radius)
                    };
                    switch (slot) {
                    case 2: cb.blood2 = value; break;
                    case 3: cb.blood3 = value; break;
                    case 4: cb.blood4 = value; break;
                    case 5: cb.blood5 = value; break;
                    case 6: cb.blood6 = value; break;
                    case 7: cb.blood7 = value; break;
                    case 8: cb.blood8 = value; break;
                    default: break;
                    }
                };
                size_t slot = 2;
                for (size_t i = 1; i < nearestCount && slot <= 8; ++i, ++slot) {
                    if (nearestRegions[i]) assignRegion(slot, *nearestRegions[i]);
                }
            }
        }
        cb.air0 = {
            airFocusDistance_,
            std::clamp(settings_.airParticleBlur, 0.0f, 3.0f),
            std::clamp(settings_.airParticleSize, 0.20f, 4.0f),
            settings_.airParticles ? std::clamp(settings_.airParticleDensity, 0.0f, 4.0f) : 0.0f
        };
        XMFLOAT3 exitLightPos = exitSignLightPos_;
        float exitLightStrength = exitSignLightStrength_;
        XMFLOAT3 exitLightDir = Normalize3(exitDoorNormal_, {0.0f, 0.0f, 1.0f});
        float exitDoorOpen = 0.0f;
        XMFLOAT3 doorwayLightPos{0.0f, 0.0f, 0.0f};
        float doorwayLightStrength = 0.0f;
        XMFLOAT3 doorwayPortalPos = Add3(exitDoorCenter_, Scale3(exitLightDir, 0.04f));
        float doorwayPortalHalfWidth = exitLightStrength > 0.001f ? 0.56f : 0.0f;
        if (runtimeMode_ == RendererRuntimeMode::MainMenu && exitDoorAngle_ > 0.001f) {
            float rawDoorOpen = exitDoorAngle_ / 1.38f;
            exitDoorOpen = SmoothStep(0.08f, 0.92f, rawDoorOpen);
            float doorwayLightOpen = SmoothStep(0.12f, 0.90f, rawDoorOpen);
            doorwayLightOpen *= doorwayLightOpen;
            XMFLOAT3 stairSource = Scale3(exitDoorNormal_, -2.65f);
            doorwayLightPos = Add3(exitDoorCenter_, stairSource);
            doorwayLightPos.y = exitDoorCenter_.y + 4.35f;
            doorwayLightStrength = doorwayLightOpen * 5.8f;
            doorwayPortalPos = Add3(exitDoorCenter_, Scale3(exitDoorNormal_, 0.04f));
            doorwayPortalHalfWidth = 0.56f;
        }
        cb.exitLight0 = {
            exitLightPos.x,
            exitLightPos.y,
            exitLightPos.z,
            exitLightStrength
        };
        cb.exitLight1 = {
            exitLightDir.x,
            exitLightDir.y,
            exitLightDir.z,
            exitDoorOpen
        };
        cb.exitLight2 = {
            doorwayLightPos.x,
            doorwayLightPos.y,
            doorwayLightPos.z,
            doorwayLightStrength
        };
        cb.exitLight3 = {
            doorwayPortalPos.x,
            doorwayPortalPos.y,
            doorwayPortalPos.z,
            doorwayPortalHalfWidth
        };
        float monsterFogRadius = std::max(maze_.TileAverage() * 2.60f, 5.80f);
        float monsterFogStrength = (monsterPreview_ || gEffectDebugViewer || gBloodDebugEveryWall || settings_.bloodStudyView)
            ? 0.0f
            : 0.72f;
        cb.monsterFog0 = {
            monster_.x,
            monster_.z,
            monsterFogRadius,
            monsterFogStrength
        };
        cb.monsterEye0 = {0.0f, 0.0f, 0.0f, 0.0f};
        cb.monsterEye1 = {0.0f, 0.0f, 0.0f, 0.0f};
        cb.monsterEye2 = {0.0f, 0.0f, 1.0f, 0.0f};
        cb.monsterEye3 = {1.0f / static_cast<float>(std::max<UINT>(1, monsterEyeShadowMapSize_)), 10.0f, 0.0f, 0.0f};
        bool fleshLightingSuppressed =
            (settings_.fleshAlwaysOn && settings_.fleshFlickerIntensity > 0.001f) ||
            (fleshFlickerTimer_ > 0.0f && fleshFlickerDuration_ > 0.001f);
        if (fleshLightingSuppressed) {
            // Shader uses transition0.z to keep the entire flesh event flashlight-only, including strobe gaps.
            cb.transition0.z = 1.0f;
            cb.lighting0.z = 0.0f;
            cb.lighting1.x = 0.0f;
            cb.lighting1.y = 0.0f;
            cb.lighting1.z = 0.0f;
        }
        bool useFleshTessellation = cb.horror0.x > 0.01f &&
            featureLevel_ >= D3D_FEATURE_LEVEL_11_0 &&
            hullShader_ && domainShader_ &&
            cb.horror0.w > 0.001f;
        std::array<XMFLOAT4, 2> sparkLights = fleshLightingSuppressed
            ? std::array<XMFLOAT4, 2>{XMFLOAT4{0.0f, 0.0f, 0.0f, 0.0f}, XMFLOAT4{0.0f, 0.0f, 0.0f, 0.0f}}
            : ActiveSparkLights();
        cb.sparkLight0 = sparkLights[0];
        cb.sparkLight1 = sparkLights[1];

        UINT stride = sizeof(Vertex);
        UINT offset = 0;
        float blendFactor[4] = {};

        UpdateDynamicGeometry();
        renderProfile.Mark(L"UpdateDynamicGeometry");
        MarkGpuProfile(GpuProfileMarker::DynamicGeometry);
        if (StartupProfileEnabled()) {
            std::wstringstream counts;
            counts << L"Dynamic scene geometry: opaqueVertices=" << dynamicOpaqueVertexCount_
                << L", transparentVertices=" << dynamicTransparentVertexCount_
                << L", airParticles=" << airParticles_.size()
                << L", sparks=" << sparks_.size()
                << L", steam=" << steam_.size()
                << L", ventDrops=" << ventDrops_.size();
            StartupProfileLine(counts.str());
        }

        std::array<XMMATRIX, 2> monsterEyeViewProj{
            XMMatrixIdentity(),
            XMMatrixIdentity()
        };
        float monsterEyeStrength = 0.0f;
        float monsterEyeRange = 0.0f;
        float monsterEyeAlert = 0.0f;
        float monsterEyeFov = 54.0f * kPi / 180.0f;
        XMFLOAT3 monsterEyeDir = Normalize3(monsterEyeForward_, {std::sin(monsterYaw_), 0.0f, std::cos(monsterYaw_)});
        if (runtimeMode_ != RendererRuntimeMode::MainMenu && !monsterPreview_ && !gEffectDebugViewer && monsterEyeWorldCount_ >= 2) {
            XMFLOAT3 midpoint = Scale3(Add3(monsterEyeWorld_[0], monsterEyeWorld_[1]), 0.5f);
            float dx = midpoint.x - camera_.x;
            float dz = midpoint.z - camera_.z;
            float distanceToPlayer = std::sqrt(dx * dx + dz * dz);
            float activeDistance = std::clamp(std::max(MonsterSightDistance(), settings_.fogEndMeters) + 4.0f, 9.0f, 22.0f);
            bool closeEnough = distanceToPlayer <= activeDistance;
            bool visualFocus = closeEnough && (monsterCanSeePlayerNow_ || MonsterVisualEncounterPlayer());
            monsterEyeAlert = std::max(monsterHeadChaseBlend_,
                visualFocus ? 1.0f : (monsterHasLastKnown_ ? 0.68f : (monsterHeardPlayerNow_ ? 0.48f : 0.0f)));
            if (visualFocus) {
                XMFLOAT3 cameraFocus{camera_.x, camera_.y + 0.02f, camera_.z};
                monsterEyeDir = Normalize3(Sub3(cameraFocus, midpoint), monsterEyeDir);
            } else if (monsterHasLastKnown_ && maze_.IsOpen(monsterLastKnownTile_.x, monsterLastKnownTile_.y)) {
                XMFLOAT3 known = maze_.WorldCenter(monsterLastKnownTile_, midpoint.y);
                monsterEyeDir = Normalize3(Sub3(known, midpoint), monsterEyeDir);
            }
            if (closeEnough) {
                monsterEyeRange = Lerp(8.0f, 14.0f, Clamp01(monsterEyeAlert));
                monsterEyeStrength = Lerp(0.16f, 0.88f, Clamp01(monsterEyeAlert));
            }
            XMVECTOR dirVec = XMVector3Normalize(XMLoadFloat3(&monsterEyeDir));
            XMFLOAT3 eyeUpFloat = Normalize3(monsterEyeUp_, {0.0f, 1.0f, 0.0f});
            XMVECTOR upVec = XMVector3Normalize(XMLoadFloat3(&eyeUpFloat));
            float upDot = std::abs(XMVectorGetX(XMVector3Dot(dirVec, upVec)));
            if (upDot > 0.93f) {
                upVec = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
                if (std::abs(XMVectorGetX(XMVector3Dot(dirVec, upVec))) > 0.93f) {
                    upVec = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
                }
            }
            monsterEyeFov = Lerp(54.0f, 38.0f, Clamp01(monsterEyeAlert)) * kPi / 180.0f;
            for (int i = 0; i < 2; ++i) {
                XMVECTOR eyeLightPos = XMLoadFloat3(&monsterEyeWorld_[static_cast<size_t>(i)]);
                monsterEyeViewProj[static_cast<size_t>(i)] =
                    XMMatrixLookAtLH(eyeLightPos, eyeLightPos + dirVec, upVec) *
                    XMMatrixPerspectiveFovLH(monsterEyeFov, 1.0f, 0.035f, std::max(0.5f, monsterEyeRange));
            }
            XMStoreFloat4x4(&cb.monsterEyeViewProj0, monsterEyeViewProj[0]);
            XMStoreFloat4x4(&cb.monsterEyeViewProj1, monsterEyeViewProj[1]);
            cb.monsterEye0 = {monsterEyeWorld_[0].x, monsterEyeWorld_[0].y, monsterEyeWorld_[0].z, monsterEyeStrength};
            cb.monsterEye1 = {monsterEyeWorld_[1].x, monsterEyeWorld_[1].y, monsterEyeWorld_[1].z, monsterEyeStrength};
            cb.monsterEye2 = {monsterEyeDir.x, monsterEyeDir.y, monsterEyeDir.z, Clamp01(monsterEyeAlert)};
            cb.monsterEye3 = {
                1.0f / static_cast<float>(std::max<UINT>(1, monsterEyeShadowMapSize_)),
                std::max(0.5f, monsterEyeRange),
                0.0f,
                0.0f
            };
        }

        auto chunkVisible = [](const auto& chunk, XMFLOAT3 origin, XMFLOAT3 direction, float maxDistance, float coneCos) {
            float dx = chunk.center.x - origin.x;
            float dy = chunk.center.y - origin.y;
            float dz = chunk.center.z - origin.z;
            float padded = std::max(0.1f, maxDistance + chunk.radius);
            float distSq = dx * dx + dy * dy + dz * dz;
            if (distSq > padded * padded) return false;
            float depth = dx * direction.x + dy * direction.y + dz * direction.z;
            if (depth < -chunk.radius) return false;
            if (coneCos > -0.99f && distSq > 0.0001f) {
                float dist = std::sqrt(distSq);
                float radiusSlack = std::min(0.55f, chunk.radius / std::max(0.1f, dist));
                if (depth / dist < coneCos - radiusSlack) return false;
            }
            return true;
        };

        std::vector<uint8_t> visibleMazeTiles;
        auto buildMazeVisibility = [&](XMFLOAT3 origin,
                                       XMFLOAT3 direction,
                                       float maxDistance,
                                       float coneCos,
                                       int wallHaloTiles) {
            if (maze_.w <= 0 || maze_.h <= 0) return false;
            Tile originTile = maze_.TileFromWorld(origin.x, origin.z);
            if (!maze_.IsOpen(originTile.x, originTile.y)) return false;
            visibleMazeTiles.assign(static_cast<size_t>(maze_.w * maze_.h), 0);
            auto mark = [&](int x, int y) {
                if (!maze_.InBounds(x, y)) return;
                visibleMazeTiles[static_cast<size_t>(y * maze_.w + x)] = 1;
            };
            auto markWithHalo = [&](int x, int y) {
                mark(x, y);
                for (int yy = -wallHaloTiles; yy <= wallHaloTiles; ++yy) {
                    for (int xx = -wallHaloTiles; xx <= wallHaloTiles; ++xx) {
                        if (std::abs(xx) + std::abs(yy) > wallHaloTiles) continue;
                        mark(x + xx, y + yy);
                    }
                }
            };

            int tileRadius = std::clamp(static_cast<int>(std::ceil(maxDistance / std::max(0.1f, maze_.TileMinimum()))) + wallHaloTiles,
                1, std::max(maze_.w, maze_.h));
            int minX = std::max(0, originTile.x - tileRadius);
            int maxX = std::min(maze_.w - 1, originTile.x + tileRadius);
            int minY = std::max(0, originTile.y - tileRadius);
            int maxY = std::min(maze_.h - 1, originTile.y + tileRadius);
            float maxDistanceSq = maxDistance * maxDistance;
            float tilePad = std::max(maze_.TileAverage() * 0.85f, 0.1f);
            float tilePadSq = tilePad * tilePad;
            for (int y = minY; y <= maxY; ++y) {
                for (int x = minX; x <= maxX; ++x) {
                    if (!maze_.IsOpen(x, y)) continue;
                    XMFLOAT3 center = maze_.WorldCenter({x, y}, origin.y);
                    float dx = center.x - origin.x;
                    float dz = center.z - origin.z;
                    float distSq = dx * dx + dz * dz;
                    if (distSq > maxDistanceSq + tilePadSq) continue;
                    if (coneCos > -0.99f && distSq > 0.0001f) {
                        float invDist = 1.0f / std::sqrt(distSq);
                        float facing = (dx * direction.x + dz * direction.z) * invDist;
                        float slack = std::min(0.42f, tilePad * invDist);
                        if (facing < coneCos - slack) continue;
                    }
                    if (!maze_.LineClear(originTile, {x, y})) continue;
                    markWithHalo(x, y);
                }
            }
            markWithHalo(originTile.x, originTile.y);
            return true;
        };

        auto chunkMazeVisible = [&](const auto& chunk, int forceTileRadius) {
            if (visibleMazeTiles.empty()) return true;
            int minX = std::max(0, chunk.minTileX - forceTileRadius);
            int maxX = std::min(maze_.w - 1, chunk.maxTileX + forceTileRadius);
            int minY = std::max(0, chunk.minTileY - forceTileRadius);
            int maxY = std::min(maze_.h - 1, chunk.maxTileY + forceTileRadius);
            for (int y = minY; y <= maxY; ++y) {
                const size_t row = static_cast<size_t>(y * maze_.w);
                for (int x = minX; x <= maxX; ++x) {
                    if (visibleMazeTiles[row + static_cast<size_t>(x)] != 0) return true;
                }
            }
            return false;
        };

        auto drawVisibleChunks = [&](const std::vector<StaticIndexChunk>& chunks,
                                     XMFLOAT3 origin,
                                     XMFLOAT3 direction,
                                     float maxDistance,
                                     float coneCos,
                                     float forceVisibleDistance = 0.0f,
                                     bool useMazeVisibility = false,
                                     int forceTileRadius = 1) {
            UINT drawn = 0;
            for (const StaticIndexChunk& chunk : chunks) {
                bool forceByDistance = false;
                if (forceVisibleDistance > 0.0f) {
                    float dx = chunk.center.x - origin.x;
                    float dy = chunk.center.y - origin.y;
                    float dz = chunk.center.z - origin.z;
                    float force = forceVisibleDistance + chunk.radius;
                    if (dx * dx + dy * dy + dz * dz <= force * force) {
                        forceByDistance = true;
                    }
                }
                if (!forceByDistance && !chunkVisible(chunk, origin, direction, maxDistance, coneCos)) continue;
                if (useMazeVisibility && !chunkMazeVisible(chunk, forceTileRadius)) continue;
                context_->DrawIndexed(chunk.indexCount, chunk.startIndex, 0);
                drawn += chunk.indexCount;
            }
            return drawn;
        };

        auto drawVisibleInstancedChunks = [&](const std::vector<StaticInstanceChunk>& chunks,
                                              XMFLOAT3 origin,
                                              XMFLOAT3 direction,
                                              float maxDistance,
                                              float coneCos,
                                              float forceVisibleDistance = 0.0f,
                                              bool useMazeVisibility = false,
                                              int forceTileRadius = 1) {
            UINT drawn = 0;
            for (const StaticInstanceChunk& chunk : chunks) {
                bool forceByDistance = false;
                if (forceVisibleDistance > 0.0f) {
                    float dx = chunk.center.x - origin.x;
                    float dy = chunk.center.y - origin.y;
                    float dz = chunk.center.z - origin.z;
                    float force = forceVisibleDistance + chunk.radius;
                    if (dx * dx + dy * dy + dz * dz <= force * force) {
                        forceByDistance = true;
                    }
                }
                if (!forceByDistance && !chunkVisible(chunk, origin, direction, maxDistance, coneCos)) continue;
                if (useMazeVisibility && !chunkMazeVisible(chunk, forceTileRadius)) continue;
                context_->DrawIndexedInstanced(chunk.indexCount, chunk.instanceCount, chunk.startIndex, chunk.baseVertex, chunk.startInstance);
                drawn += chunk.indexCount * chunk.instanceCount;
            }
            return drawn;
        };

        float mainHalfFov = fovDegrees * 0.5f * kPi / 180.0f;
        float mainHorizontalHalfFov = std::atan(std::tan(mainHalfFov) * std::max(0.1f, aspect));
        float mainConeCos = std::cos(std::min(kPi * 0.98f, std::max(mainHalfFov, mainHorizontalHalfFov) + 0.82f));
        float mainCullDistance = monsterPreview_
            ? 1000.0f
            : std::max(viewFarMeters, settings_.fogEndMeters + maze_.TileAverage() * 12.0f);
        float mainForceVisibleDistance = std::max(maze_.TileAverage() * 5.0f, 8.0f);
        float transparentCullDistance = monsterPreview_
            ? mainCullDistance
            : std::min(mainCullDistance, std::max(maze_.TileAverage() * 8.0f, settings_.fogEndMeters + maze_.TileAverage() * 4.0f));
        if (!monsterPreview_) {
            buildMazeVisibility(eyePos, viewDirFloat, mainCullDistance, mainConeCos, 4);
        }

        auto renderDepthShadow = [&](ID3D11DepthStencilView* shadowDsv, UINT shadowSize, const XMMATRIX& shadowViewProj,
                                     XMFLOAT3 shadowOrigin, XMFLOAT3 shadowDirection, float shadowRange, float shadowConeCos) {
            if (!shadowDsv) return;
            ID3D11ShaderResourceView* nullSrvs[] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
            context_->PSSetShaderResources(0, 14, nullSrvs);
            context_->DSSetShaderResources(0, 14, nullSrvs);
            context_->ClearDepthStencilView(shadowDsv, D3D11_CLEAR_DEPTH, 1.0f, 0);
            context_->OMSetRenderTargets(0, nullptr, shadowDsv);

            D3D11_VIEWPORT shadowVp{};
            shadowVp.Width = static_cast<float>(shadowSize);
            shadowVp.Height = static_cast<float>(shadowSize);
            shadowVp.MinDepth = 0.0f;
            shadowVp.MaxDepth = 1.0f;
            context_->RSSetViewports(1, &shadowVp);

            SceneConstants shadowCb = cb;
            XMStoreFloat4x4(&shadowCb.viewProj, shadowViewProj);
            UploadSceneConstants(shadowCb);

            context_->IASetInputLayout(inputLayout_.Get());
            context_->IASetPrimitiveTopology(useFleshTessellation
                ? D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST
                : D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            context_->IASetVertexBuffers(0, 1, vertexBuffer_.GetAddressOf(), &stride, &offset);
            context_->IASetIndexBuffer(indexBuffer_.Get(), DXGI_FORMAT_R32_UINT, 0);
            context_->VSSetShader(vertexShader_.Get(), nullptr, 0);
            context_->HSSetShader(useFleshTessellation ? hullShader_.Get() : nullptr, nullptr, 0);
            context_->DSSetShader(useFleshTessellation ? domainShader_.Get() : nullptr, nullptr, 0);
            context_->PSSetShader(shadowPixelShader_.Get(), nullptr, 0);
            context_->VSSetConstantBuffers(0, 1, constantBuffer_.GetAddressOf());
            context_->HSSetConstantBuffers(0, 1, constantBuffer_.GetAddressOf());
            context_->DSSetConstantBuffers(0, 1, constantBuffer_.GetAddressOf());
            if (useFleshTessellation) {
                ID3D11ShaderResourceView* dsSrvs[] = {nullptr, normalSrv_.Get(), nullptr, nullptr, nullptr, nullptr, nullptr};
                context_->DSSetShaderResources(0, 7, dsSrvs);
                context_->DSSetSamplers(0, 1, sampler_.GetAddressOf());
            }
            context_->RSSetState(shadowRasterState_.Get());
            context_->OMSetDepthStencilState(depthState_.Get(), 0);
            context_->OMSetBlendState(nullptr, blendFactor, 0xffffffff);
            if (!monsterPreview_) {
                if (!staticOpaqueChunks_.empty() || !staticFloorCeilingChunks_.empty()) {
                    StartupProfileLine(L"Render before ShadowStatic chunked DrawIndexed");
                    UINT drawn = drawVisibleChunks(staticOpaqueChunks_, shadowOrigin, shadowDirection, shadowRange, shadowConeCos,
                        maze_.TileAverage() * 1.4f, false, 1);
                    drawn += drawVisibleChunks(staticFloorCeilingChunks_, shadowOrigin, shadowDirection, shadowRange, shadowConeCos,
                        maze_.TileAverage() * 1.4f, false, 1);
                    if (drawn == 0) {
                        UINT shadowStaticIndexCount = staticWaterStartIndex_ > 0 ? staticWaterStartIndex_ :
                            (staticTransparentStartIndex_ > 0 ? staticTransparentStartIndex_ : indexCount_);
                        context_->DrawIndexed(shadowStaticIndexCount, 0, 0);
                    }
                    renderProfile.Mark(L"ShadowStatic");
                } else {
                    UINT shadowStaticIndexCount = staticWaterStartIndex_ > 0 ? staticWaterStartIndex_ :
                        (staticTransparentStartIndex_ > 0 ? staticTransparentStartIndex_ : indexCount_);
                    if (shadowStaticIndexCount > 0) {
                        StartupProfileLine(L"Render before ShadowStatic DrawIndexed");
                        context_->DrawIndexed(shadowStaticIndexCount, 0, 0);
                        renderProfile.Mark(L"ShadowStatic");
                    }
                }
                if (staticPropShadowIndexCount_ > 0) {
                    context_->PSSetShader(nullptr, nullptr, 0);
                    StartupProfileLine(staticPropShadowChunks_.empty()
                        ? L"Render before StaticPropShadow DrawIndexed"
                        : L"Render before StaticPropShadow chunked DrawIndexed");
                    UINT drawn = 0;
                    if (!staticPropShadowChunks_.empty()) {
                        drawn = drawVisibleChunks(staticPropShadowChunks_, shadowOrigin, shadowDirection, shadowRange, shadowConeCos,
                            maze_.TileAverage() * 1.4f, false, 1);
                    }
                    if (staticPropShadowChunks_.empty()) {
                        context_->DrawIndexed(staticPropShadowIndexCount_, staticPropShadowStartIndex_, 0);
                    }
                    renderProfile.Mark(L"StaticPropShadow");
                    context_->PSSetShader(shadowPixelShader_.Get(), nullptr, 0);
                }
                if (!instancedPropShadowChunks_.empty() && instancedVertexBuffer_ && instancedIndexBuffer_ && instancedInstanceBuffer_) {
                    ID3D11Buffer* instBuffers[] = {instancedVertexBuffer_.Get(), instancedInstanceBuffer_.Get()};
                    UINT instStrides[] = {sizeof(Vertex), sizeof(StaticInstanceData)};
                    UINT instOffsets[] = {0, 0};
                    context_->IASetInputLayout(instancedInputLayout_.Get());
                    context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
                    context_->IASetVertexBuffers(0, 2, instBuffers, instStrides, instOffsets);
                    context_->IASetIndexBuffer(instancedIndexBuffer_.Get(), DXGI_FORMAT_R32_UINT, 0);
                    context_->VSSetShader(instancedVertexShader_.Get(), nullptr, 0);
                    context_->HSSetShader(nullptr, nullptr, 0);
                    context_->DSSetShader(nullptr, nullptr, 0);
                    context_->PSSetShader(nullptr, nullptr, 0);
                    context_->VSSetConstantBuffers(0, 1, constantBuffer_.GetAddressOf());
                    StartupProfileLine(L"Render before InstancedPropShadow DrawIndexedInstanced");
                    drawVisibleInstancedChunks(instancedPropShadowChunks_, shadowOrigin, shadowDirection, shadowRange, shadowConeCos,
                        maze_.TileAverage() * 1.4f, false, 1);
                    renderProfile.Mark(L"InstancedPropShadow");
                    context_->PSSetShader(shadowPixelShader_.Get(), nullptr, 0);
                }
            }

            if (dynamicOpaqueVertexCount_ > 0 && runtimeMode_ != RendererRuntimeMode::MainMenu) {
                context_->HSSetShader(nullptr, nullptr, 0);
                context_->DSSetShader(nullptr, nullptr, 0);
                context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
                context_->IASetVertexBuffers(0, 1, dynamicBuffer_.GetAddressOf(), &stride, &offset);
                StartupProfileLine(L"Render before ShadowDynamic Draw");
                context_->Draw(dynamicOpaqueVertexCount_, 0);
                renderProfile.Mark(L"ShadowDynamic");
            }
        };

        if (shadowDsv_ && settings_.flashlightShadows && settings_.flashlightShadowStrength > 0.001f &&
            flashlightIntensity > 0.001f && transitionFade < 0.995f) {
            renderProfile.Mark(L"BeginShadowPass");
            renderDepthShadow(shadowDsv_.Get(), shadowMapSize_, lightViewProj,
                lightPosFloat, lightDirFloat, shadowDistance, std::cos(shadowFov * 0.5f));
        }
        MarkGpuProfile(GpuProfileMarker::FlashlightShadow);
        if (fixtureShadowActive && settings_.flashlightShadowStrength > 0.001f && transitionFade < 0.995f) {
            renderProfile.Mark(L"BeginFixtureShadow");
            renderDepthShadow(fixtureShadowDsv_.Get(), fixtureShadowMapSize_, fixtureLightViewProj,
                fixtureShadowPos, fixtureShadowDir, fixtureShadowRange, std::cos(116.0f * 0.5f * kPi / 180.0f));
        }
        MarkGpuProfile(GpuProfileMarker::FixtureShadow);
        if (monsterEyeStrength > 0.001f) {
            for (int eyeIndex = 0; eyeIndex < 2; ++eyeIndex) {
                if (monsterEyeShadowDsv_[static_cast<size_t>(eyeIndex)]) {
                    renderProfile.Mark(eyeIndex == 0 ? L"BeginMonsterEyeShadow0" : L"BeginMonsterEyeShadow1");
                    renderDepthShadow(
                        monsterEyeShadowDsv_[static_cast<size_t>(eyeIndex)].Get(),
                        monsterEyeShadowMapSize_,
                        monsterEyeViewProj[static_cast<size_t>(eyeIndex)],
                        monsterEyeWorld_[static_cast<size_t>(eyeIndex)],
                        monsterEyeDir,
                        std::max(0.5f, monsterEyeRange),
                        std::cos(monsterEyeFov * 0.5f));
                }
            }
        }
        MarkGpuProfile(GpuProfileMarker::MonsterEyeShadow);

        UploadSceneConstants(cb);
        renderProfile.Mark(L"UploadSceneConstants");
        UploadLampDamageTexture();
        renderProfile.Mark(L"UploadLampDamageTexture");
        UpdateCustomMenuTexture();
        MarkGpuProfile(GpuProfileMarker::Uploads);

        ID3D11ShaderResourceView* srvs[] = {
            albedoSrv_.Get(),
            normalSrv_.Get(),
            shadowSrv_.Get(),
            mazeSrv_.Get(),
            materialPropsSrv_.Get(),
            flashlightPatternSrv_.Get(),
            lampDamageSrv_.Get(),
            monsterEyeShadowSrv_[0].Get(),
            monsterEyeShadowSrv_[1].Get(),
            loosePagesSrv_.Get(),
            fixtureShadowSrv_.Get(),
            ceilingAlbedoSrv_.Get(),
            ceilingNormalSrv_.Get(),
            ceilingPropsSrv_.Get(),
            customMenuSrv_.Get(),
            doorAlbedoSrv_.Get(),
            doorNormalSrv_.Get(),
            doorPropsSrv_.Get(),
            doorFrameAlbedoSrv_.Get(),
            doorFrameNormalSrv_.Get(),
            doorFramePropsSrv_.Get()
        };
        ID3D11SamplerState* samplers[] = {sampler_.Get(), shadowSampler_.Get()};
        context_->OMSetRenderTargets(1, &sceneTarget, dsv_.Get());
        context_->RSSetViewports(1, &vp);
        context_->IASetInputLayout(inputLayout_.Get());
        context_->IASetPrimitiveTopology(useFleshTessellation
            ? D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST
            : D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context_->VSSetShader(vertexShader_.Get(), nullptr, 0);
        context_->HSSetShader(useFleshTessellation ? hullShader_.Get() : nullptr, nullptr, 0);
        context_->DSSetShader(useFleshTessellation ? domainShader_.Get() : nullptr, nullptr, 0);
        context_->PSSetShader(pixelShader_.Get(), nullptr, 0);
        context_->VSSetConstantBuffers(0, 1, constantBuffer_.GetAddressOf());
        context_->HSSetConstantBuffers(0, 1, constantBuffer_.GetAddressOf());
        context_->DSSetConstantBuffers(0, 1, constantBuffer_.GetAddressOf());
        context_->PSSetConstantBuffers(0, 1, constantBuffer_.GetAddressOf());
        context_->PSSetShaderResources(0, static_cast<UINT>(std::size(srvs)), srvs);
        if (useFleshTessellation) {
            context_->DSSetShaderResources(0, static_cast<UINT>(std::size(srvs)), srvs);
            context_->DSSetSamplers(0, 1, sampler_.GetAddressOf());
        }
        context_->PSSetSamplers(0, 2, samplers);
        context_->RSSetState(rasterState_.Get());
        context_->OMSetDepthStencilState(depthState_.Get(), 0);

        auto bindStaticScenePipeline = [&]() {
            context_->IASetInputLayout(inputLayout_.Get());
            context_->IASetPrimitiveTopology(useFleshTessellation
                ? D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST
                : D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            context_->IASetVertexBuffers(0, 1, vertexBuffer_.GetAddressOf(), &stride, &offset);
            context_->IASetIndexBuffer(indexBuffer_.Get(), DXGI_FORMAT_R32_UINT, 0);
            context_->VSSetShader(vertexShader_.Get(), nullptr, 0);
            context_->HSSetShader(useFleshTessellation ? hullShader_.Get() : nullptr, nullptr, 0);
            context_->DSSetShader(useFleshTessellation ? domainShader_.Get() : nullptr, nullptr, 0);
            context_->PSSetShader(pixelShader_.Get(), nullptr, 0);
            context_->VSSetConstantBuffers(0, 1, constantBuffer_.GetAddressOf());
            context_->HSSetConstantBuffers(0, 1, constantBuffer_.GetAddressOf());
            context_->DSSetConstantBuffers(0, 1, constantBuffer_.GetAddressOf());
            context_->PSSetConstantBuffers(0, 1, constantBuffer_.GetAddressOf());
            context_->PSSetShaderResources(0, 14, srvs);
            if (useFleshTessellation) {
                context_->DSSetShaderResources(0, 14, srvs);
                context_->DSSetSamplers(0, 1, sampler_.GetAddressOf());
            }
            context_->PSSetSamplers(0, 2, samplers);
        };

        auto bindInstancedScenePipeline = [&]() {
            ID3D11Buffer* buffers[] = {instancedVertexBuffer_.Get(), instancedInstanceBuffer_.Get()};
            UINT strides[] = {sizeof(Vertex), sizeof(StaticInstanceData)};
            UINT offsets[] = {0, 0};
            context_->IASetInputLayout(instancedInputLayout_.Get());
            context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            context_->IASetVertexBuffers(0, 2, buffers, strides, offsets);
            context_->IASetIndexBuffer(instancedIndexBuffer_.Get(), DXGI_FORMAT_R32_UINT, 0);
            context_->VSSetShader(instancedVertexShader_.Get(), nullptr, 0);
            context_->HSSetShader(nullptr, nullptr, 0);
            context_->DSSetShader(nullptr, nullptr, 0);
            context_->PSSetShader(pixelShader_.Get(), nullptr, 0);
            context_->VSSetConstantBuffers(0, 1, constantBuffer_.GetAddressOf());
            context_->PSSetConstantBuffers(0, 1, constantBuffer_.GetAddressOf());
            context_->PSSetShaderResources(0, 14, srvs);
            context_->PSSetSamplers(0, 2, samplers);
        };

        auto bindDynamicScenePipeline = [&]() {
            context_->IASetInputLayout(inputLayout_.Get());
            context_->HSSetShader(nullptr, nullptr, 0);
            context_->DSSetShader(nullptr, nullptr, 0);
            context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            context_->IASetVertexBuffers(0, 1, dynamicBuffer_.GetAddressOf(), &stride, &offset);
            context_->VSSetShader(vertexShader_.Get(), nullptr, 0);
            context_->PSSetShader(pixelShader_.Get(), nullptr, 0);
            context_->VSSetConstantBuffers(0, 1, constantBuffer_.GetAddressOf());
            context_->PSSetConstantBuffers(0, 1, constantBuffer_.GetAddressOf());
            context_->PSSetShaderResources(0, 14, srvs);
            context_->PSSetSamplers(0, 2, samplers);
        };

        if (!monsterPreview_) {
            bindStaticScenePipeline();
            context_->OMSetBlendState(nullptr, blendFactor, 0xffffffff);
            UINT opaqueIndexCount = std::min(floorCeilingStartIndex_, indexCount_);
            if (opaqueIndexCount > 0) {
                StartupProfileLine(staticOpaqueChunks_.empty()
                    ? L"Render before MainOpaque DrawIndexed"
                    : L"Render before MainOpaque chunked DrawIndexed");
                UINT drawn = 0;
                if (!staticOpaqueChunks_.empty()) {
                    drawn = drawVisibleChunks(staticOpaqueChunks_, eyePos, viewDirFloat, mainCullDistance, mainConeCos,
                        mainForceVisibleDistance, false, 1);
                    if (StartupProfileEnabled()) {
                        std::wstringstream counts;
                        counts << L"MainOpaque visible indices=" << drawn << L"/" << opaqueIndexCount;
                        StartupProfileLine(counts.str());
                    }
                }
                if (staticOpaqueChunks_.empty() || drawn == 0) {
                    context_->DrawIndexed(opaqueIndexCount, 0, 0);
                }
                if (!instancedOpaqueChunks_.empty() && instancedVertexBuffer_ && instancedIndexBuffer_ && instancedInstanceBuffer_) {
                    bindInstancedScenePipeline();
                    StartupProfileLine(L"Render before MainOpaque instanced DrawIndexedInstanced");
                    UINT instancedDrawn = drawVisibleInstancedChunks(instancedOpaqueChunks_, eyePos, viewDirFloat, mainCullDistance, mainConeCos,
                        mainForceVisibleDistance, true, 3);
                    if (StartupProfileEnabled()) {
                        std::wstringstream counts;
                        counts << L"MainOpaque visible instanced indices=" << instancedDrawn
                            << L"/" << (static_cast<uint64_t>(instancedIndexCount_) * static_cast<uint64_t>(instancedInstanceCount_));
                        StartupProfileLine(counts.str());
                    }
                    bindStaticScenePipeline();
                }
                renderProfile.Mark(L"MainOpaque");
            }
            MarkGpuProfile(GpuProfileMarker::MainOpaque);
            if (floorCeilingIndexCount_ > 0) {
                StartupProfileLine(staticFloorCeilingChunks_.empty()
                    ? L"Render before FloorCeiling DrawIndexed"
                    : L"Render before FloorCeiling chunked DrawIndexed");
                UINT drawn = 0;
                if (!staticFloorCeilingChunks_.empty()) {
                    drawn = drawVisibleChunks(staticFloorCeilingChunks_, eyePos, viewDirFloat, mainCullDistance, mainConeCos,
                        mainForceVisibleDistance, false, 1);
                    if (StartupProfileEnabled()) {
                        std::wstringstream counts;
                        counts << L"FloorCeiling visible indices=" << drawn << L"/" << floorCeilingIndexCount_;
                        StartupProfileLine(counts.str());
                    }
                }
                if (staticFloorCeilingChunks_.empty() || drawn == 0) {
                    context_->DrawIndexed(floorCeilingIndexCount_, floorCeilingStartIndex_, 0);
                }
                renderProfile.Mark(L"FloorCeiling");
            }
        } else {
            MarkGpuProfile(GpuProfileMarker::MainOpaque);
        }
        MarkGpuProfile(GpuProfileMarker::FloorCeiling);

        if (dynamicOpaqueVertexCount_ > 0 || dynamicTransparentVertexCount_ > 0) {
            bindDynamicScenePipeline();
        }
        if (dynamicOpaqueVertexCount_ > 0) {
            context_->OMSetDepthStencilState(depthState_.Get(), 0);
            context_->OMSetBlendState(nullptr, blendFactor, 0xffffffff);
            StartupProfileLine(L"Render before DynamicOpaque Draw");
            context_->Draw(dynamicOpaqueVertexCount_, 0);
            renderProfile.Mark(L"DynamicOpaque");
        }
        MarkGpuProfile(GpuProfileMarker::DynamicOpaque);
        if (!monsterPreview_) {
            if (staticWaterIndexCount_ > 0 || staticTransparentIndexCount_ > 0) {
                bindStaticScenePipeline();
            }
            if (staticWaterIndexCount_ > 0) {
                context_->OMSetDepthStencilState(liquidDepthStencilState_.Get(), 0);
                context_->OMSetBlendState(alphaBlend_.Get(), blendFactor, 0xffffffff);
                context_->PSSetShader(liquidPixelShader_ ? liquidPixelShader_.Get() : pixelShader_.Get(), nullptr, 0);
                StartupProfileLine(L"Render before StaticWater DrawIndexed");
                if (!staticWaterChunks_.empty()) {
                    drawVisibleChunks(staticWaterChunks_, eyePos, viewDirFloat, transparentCullDistance, mainConeCos,
                        mainForceVisibleDistance, true, 4);
                } else {
                    context_->DrawIndexed(staticWaterIndexCount_, staticWaterStartIndex_, 0);
                }
                renderProfile.Mark(L"StaticWater");
                context_->OMSetDepthStencilState(depthState_.Get(), 0);
                context_->OMSetBlendState(nullptr, blendFactor, 0xffffffff);
            }
            MarkGpuProfile(GpuProfileMarker::StaticWater);
            if (staticTransparentIndexCount_ > 0) {
                context_->OMSetDepthStencilState(depthReadOnlyState_.Get(), 0);
                context_->OMSetBlendState(alphaBlend_.Get(), blendFactor, 0xffffffff);
                context_->PSSetShader(pixelShader_.Get(), nullptr, 0);
                StartupProfileLine(L"Render before StaticTransparent DrawIndexed");
                if (!staticTransparentChunks_.empty()) {
                    drawVisibleChunks(staticTransparentChunks_, eyePos, viewDirFloat, transparentCullDistance, mainConeCos,
                        mainForceVisibleDistance, true, 4);
                } else {
                    context_->DrawIndexed(staticTransparentIndexCount_, staticTransparentStartIndex_, 0);
                }
                renderProfile.Mark(L"StaticTransparent");
                context_->OMSetDepthStencilState(depthState_.Get(), 0);
                context_->OMSetBlendState(nullptr, blendFactor, 0xffffffff);
            }
        } else {
            MarkGpuProfile(GpuProfileMarker::StaticWater);
        }
        MarkGpuProfile(GpuProfileMarker::StaticTransparent);
        if (dynamicTransparentVertexCount_ > 0) {
            bindDynamicScenePipeline();
            context_->OMSetDepthStencilState(depthReadOnlyState_.Get(), 0);
            context_->OMSetBlendState(alphaBlend_.Get(), blendFactor, 0xffffffff);
            StartupProfileLine(L"Render before DynamicTransparent Draw");
            context_->Draw(dynamicTransparentVertexCount_, dynamicOpaqueVertexCount_);
            renderProfile.Mark(L"DynamicTransparent");
        }
        MarkGpuProfile(GpuProfileMarker::DynamicTransparent);

        ID3D11ShaderResourceView* nullSrvs[] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
        context_->PSSetShaderResources(0, 14, nullSrvs);
        context_->DSSetShaderResources(0, 14, nullSrvs);
        context_->HSSetShader(nullptr, nullptr, 0);
        context_->DSSetShader(nullptr, nullptr, 0);
        if (postAvailable) {
            StartupProfileLine(L"Render before DrawPostProcess");
            DrawPostProcess();
            renderProfile.Mark(L"PostProcess");
        }
        MarkGpuProfile(GpuProfileMarker::PostProcess);

        if (!monsterPreview_) {
            StartupProfileLine(L"Render before DrawMapOverlay");
            DrawMapOverlay();
            renderProfile.Mark(L"MapOverlay");
            StartupProfileLine(L"Render before DrawDreadMeterOverlay");
            DrawDreadMeterOverlay();
            renderProfile.Mark(L"DreadOverlay");
            StartupProfileLine(L"Render before DrawGameHudOverlay");
            DrawGameHudOverlay();
            renderProfile.Mark(L"GameHudOverlay");
        }
        MarkGpuProfile(GpuProfileMarker::Overlays);

        context_->PSSetShaderResources(0, 14, nullSrvs);
        context_->DSSetShaderResources(0, 14, nullSrvs);
        context_->HSSetShader(nullptr, nullptr, 0);
        context_->DSSetShader(nullptr, nullptr, 0);
        EndGpuProfileFrame();
        StartupProfileLine(L"Render before Present");
        HRESULT presentHr = swapChain_->Present(presentSyncInterval_, presentFlags_);
        lastPresentCompleted_ = presentHr != DXGI_ERROR_WAS_STILL_DRAWING;
        renderProfile.Mark(L"Present");
    }
