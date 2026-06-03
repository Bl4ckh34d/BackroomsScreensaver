        XMFLOAT3 handSupportUp{0.0f, 0.0f, 0.0f};
        int handSupportCount = 0;
        for (const MonsterLimbAnchor& anchor : monsterPresentation_.limbAnchors) {
            XMFLOAT3 n = anchor.planted ? anchor.anchorNormal : anchor.swingToNormal;
            if (Length3(n) <= 0.001f) continue;
            n = Normalize3(n, up);
            handSupportUp = Add3(handSupportUp, n);
            ++handSupportCount;
        }
        bool hasHandSupportUp = handSupportCount >= 3 && Length3(handSupportUp) > 0.20f;
        if (hasHandSupportUp) {
            handSupportUp = Normalize3(handSupportUp, up);
            handSupportUp = Normalize3(Add3(handSupportUp, Scale3(up, 0.05f)), handSupportUp);
        }
