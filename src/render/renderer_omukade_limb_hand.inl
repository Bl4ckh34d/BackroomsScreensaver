                XMFLOAT3 palmRight{};
                XMFLOAT3 palmUp{};
                surfaceAxes(contactNormal, palmRight, palmUp);
                XMFLOAT3 palmCenter = Add3(target, Scale3(contactNormal, 0.022f + swing * 0.018f));
                float handScale = 0.86f + SmoothStep(0.72f, 1.0f, frontness) * 0.36f;
                float grabHandScale = handScale * (1.0f + closeGrabWeight * 0.10f);
                AppendDynamicEllipsoid(solidVerts, palmCenter, palmRight, palmUp, contactNormal,
                    {limb * 2.25f * grabHandScale, limb * 1.06f * grabHandScale, limb * 0.42f * grabHandScale}, 10, 5, limbMaterial + 0.045f);
                XMFLOAT3 padCenter = Add3(palmCenter, Scale3(contactNormal, 0.018f + swing * 0.010f));
                AppendDynamicEllipsoid(solidVerts, padCenter, palmRight, palmUp, contactNormal,
                    {limb * 1.52f * grabHandScale, limb * 0.62f * grabHandScale, limb * 0.050f * grabHandScale}, 8, 4, darkMat);
                if (closeGrabWeight > 0.72f && playerReachable) {
                    XMFLOAT3 chestProbe{playerPosition.x, playerPosition.y - 0.08f, playerPosition.z};
                    float grabRadius = std::max(maze.TileMinimum() * 0.15f, limb * (5.9f + frontness * 2.2f));
                    if (Length3(Sub3(palmCenter, chestProbe)) < grabRadius) {
                        BeginDeath();
                    }
                }
