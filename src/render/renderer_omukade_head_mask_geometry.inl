        if (renderAssets_.skullMesh.empty() && !renderAssets_.monsterMeshLoaded) {
            LoadOmukadeMaskMesh();
        }
        bool externalSkull = !renderAssets_.skullMesh.empty();
        bool nativeMaskMesh = externalSkull && renderAssets_.monsterSkullNativeMaskAxes;
        XMFLOAT3 externalMaskCenter = skull;
        if (externalSkull) {
            float frontPulse = 1.0f + std::sin(timeRuntime_.time * 2.18f + monsterPosition.x * 0.11f - monsterPosition.z * 0.07f) * 0.030f;
            XMFLOAT3 maskFlesh = Add3(skull, OrientedOffset(hRight, hUp, hForward,
                0.0f, -0.045f * modelY, -0.070f * modelXZ));
            XMFLOAT3 throatFlesh = Add3(headRoot, OrientedOffset(hRight, hUp, hForward,
                0.0f, 0.070f * modelY, 0.135f * modelXZ));
            XMFLOAT3 jawFlesh = Add3(skull, OrientedOffset(hRight, hUp, hForward,
                0.0f, -0.250f * modelY, -0.035f * modelXZ));
            AppendDynamicEllipsoid(solidVerts, throatFlesh, hRight, hUp, hForward,
                {0.445f * modelXZ * frontPulse, 0.345f * modelY * frontPulse, 0.385f * modelXZ * frontPulse},
                debugEffectMonster ? 16 : 24, debugEffectMonster ? 8 : 12, gutMat + 0.032f);
            AppendDynamicEllipsoid(solidVerts, maskFlesh, hRight, hUp, hForward,
                {0.390f * modelXZ * frontPulse, 0.440f * modelY * frontPulse, 0.235f * modelXZ * frontPulse},
                debugEffectMonster ? 16 : 24, debugEffectMonster ? 8 : 12, gutMat + 0.046f);
            AppendDynamicEllipsoid(solidVerts, jawFlesh, hRight, hUp, hForward,
                {0.295f * modelXZ * frontPulse, 0.175f * modelY * frontPulse, 0.185f * modelXZ * frontPulse},
                debugEffectMonster ? 12 : 18, debugEffectMonster ? 6 : 9, gutMat + 0.058f);
            XMFLOAT3 maskCenter = Add3(skull, Scale3(hForward, 0.030f * modelXZ));
            maskCenter = Add3(maskCenter, Scale3(hUp, -0.035f * modelY));
            if (headAwareness > 0.001f) {
                XMFLOAT3 awareMask = Add3(skull, Scale3(hForward, 0.075f * modelXZ));
                maskCenter = Lerp3(maskCenter, awareMask, headAwareness * 0.54f);
            }
            externalMaskCenter = maskCenter;
            AppendExternalSkullMesh(solidVerts, maskCenter, hRight, hUp, hForward,
                1.10f * modelXZ, 1.10f * modelY);
        } else {
            XMFLOAT3 headMeat = Add3(headRoot, Scale3(hForward, bodyRadii[0] * 0.34f + 0.030f * modelXZ));
            AppendDynamicEllipsoid(solidVerts, headMeat, hRight, hUp, hForward,
                {0.420f * modelXZ, 0.400f * modelY, 0.135f * modelXZ}, 20, 10, gutMat + 0.04f);
        }
