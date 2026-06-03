    static XMFLOAT3 Add3(XMFLOAT3 a, XMFLOAT3 b) {
        return {a.x + b.x, a.y + b.y, a.z + b.z};
    }

    static XMFLOAT3 Scale3(XMFLOAT3 a, float s) {
        return {a.x * s, a.y * s, a.z * s};
    }

    static XMFLOAT3 Sub3(XMFLOAT3 a, XMFLOAT3 b) {
        return {a.x - b.x, a.y - b.y, a.z - b.z};
    }

    static float Dot3(XMFLOAT3 a, XMFLOAT3 b) {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    static XMFLOAT3 Cross3(XMFLOAT3 a, XMFLOAT3 b) {
        return {
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        };
    }

    static float Length3(XMFLOAT3 a) {
        return std::sqrt(std::max(0.0f, Dot3(a, a)));
    }

    static XMFLOAT3 Normalize3(XMFLOAT3 a, XMFLOAT3 fallback = {0.0f, 1.0f, 0.0f}) {
        float len = Length3(a);
        if (len <= 0.0001f) return fallback;
        return Scale3(a, 1.0f / len);
    }

    static XMFLOAT3 Lerp3(XMFLOAT3 a, XMFLOAT3 b, float t) {
        return {Lerp(a.x, b.x, t), Lerp(a.y, b.y, t), Lerp(a.z, b.z, t)};
    }

    static XMFLOAT3 RotateYVec(XMFLOAT3 a, float angle) {
        float c = std::cos(angle);
        float s = std::sin(angle);
        return {a.x * c + a.z * s, a.y, -a.x * s + a.z * c};
    }

    static XMFLOAT3 OrientedOffset(XMFLOAT3 right, XMFLOAT3 up, XMFLOAT3 forward, float x, float y, float z) {
        return Add3(Add3(Scale3(right, x), Scale3(up, y)), Scale3(forward, z));
    }
