        XMFLOAT3 attentionSource = sourceFromCeiling
            ? XMFLOAT3{center.x, wallH - 0.010f, center.z}
            : XMFLOAT3{center.x, 0.035f, center.z};
        AddWaterAttentionPoint(center, attentionSource, normal,
            std::max(w, h) * 0.82f, seed + (sourceFromCeiling ? 0.57f : 0.29f), true);
        return true;
