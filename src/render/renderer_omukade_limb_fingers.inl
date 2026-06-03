                if (monsterDetail >= 1 || closeGrabWeight > 0.10f) {
                    for (int finger = -1; finger <= 1; ++finger) {
                        float f = static_cast<float>(finger);
                        float spread = f * limb * 0.68f * grabHandScale;
                        float hookCurl = (1.0f - std::abs(f)) * limb * 0.18f;
                        XMFLOAT3 base = Add3(padCenter, Add3(Scale3(palmRight, spread), Scale3(palmUp, limb * 0.34f * handScale)));
                        XMFLOAT3 mid = Add3(base, Add3(Scale3(palmUp, limb * 0.60f * handScale), Scale3(contactNormal, limb * 0.16f * (0.65f + swing))));
                        XMFLOAT3 tip = Add3(mid, Add3(Scale3(palmUp, limb * 0.20f * handScale), Scale3(contactNormal, -limb * (0.36f + swing * 0.18f) - hookCurl)));
                        organicSegment(base, mid, limb * 0.22f * handScale, limb * 0.13f * handScale, darkMat, 5);
                        organicSegment(mid, tip, limb * 0.14f * handScale, limb * 0.055f * handScale, darkMat, 5);
                    }
                }
