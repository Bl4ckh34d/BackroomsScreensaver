                float limbMaterial = limbMat + std::fmod(static_cast<float>(limbId) * 0.007f, 0.10f);
                organicSegment(root, knee, limb * 2.55f, limb * 1.16f, limbMaterial, 8);
                organicSegment(knee, target, limb * 1.28f, limb * 0.58f, limbMaterial, 7);
                AppendDynamicEllipsoid(solidVerts, root, sideAxis, localUp, tangent,
                    {limb * 2.95f, limb * 1.95f, limb * 2.85f}, 9, 5, limbMaterial + 0.025f);
                AppendDynamicEllipsoid(solidVerts, knee, sideAxis, localUp, tangent,
                    {limb * 1.58f, limb * 1.24f, limb * 1.58f}, 8, 5, limbMaterial + 0.030f);
