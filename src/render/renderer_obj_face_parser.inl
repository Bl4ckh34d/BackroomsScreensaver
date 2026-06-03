    static int ParseObjFaceIndex(const char*& p, int vertexCount) {
        while (*p && std::isspace(static_cast<unsigned char>(*p))) ++p;
        if (!*p) return std::numeric_limits<int>::min();
        char* end = nullptr;
        long raw = std::strtol(p, &end, 10);
        if (end == p) {
            while (*p && !std::isspace(static_cast<unsigned char>(*p))) ++p;
            return std::numeric_limits<int>::min();
        }
        while (*end && !std::isspace(static_cast<unsigned char>(*end))) ++end;
        p = end;
        if (raw > 0) return static_cast<int>(raw - 1);
        if (raw < 0) return vertexCount + static_cast<int>(raw);
        return std::numeric_limits<int>::min();
    }

    struct ObjFaceVertex {
        int vertex = std::numeric_limits<int>::min();
        int texcoord = std::numeric_limits<int>::min();
        int normal = std::numeric_limits<int>::min();
    };

    static int ResolveObjIndex(long raw, int count) {
        if (raw > 0) return static_cast<int>(raw - 1);
        if (raw < 0) return count + static_cast<int>(raw);
        return std::numeric_limits<int>::min();
    }

    static ObjFaceVertex ParseObjFaceVertex(const char*& p, int vertexCount, int texcoordCount, int normalCount = 0) {
        ObjFaceVertex result{};
        while (*p && std::isspace(static_cast<unsigned char>(*p))) ++p;
        if (!*p) return result;

        char* end = nullptr;
        long rawVertex = std::strtol(p, &end, 10);
        if (end == p) {
            while (*p && !std::isspace(static_cast<unsigned char>(*p))) ++p;
            return result;
        }
        result.vertex = ResolveObjIndex(rawVertex, vertexCount);

        const char* q = end;
        if (*q == '/') {
            ++q;
            if (*q && *q != '/') {
                char* uvEnd = nullptr;
                long rawUv = std::strtol(q, &uvEnd, 10);
                if (uvEnd != q) {
                    result.texcoord = ResolveObjIndex(rawUv, texcoordCount);
                    q = uvEnd;
                }
            }
            if (*q == '/') {
                ++q;
                if (*q) {
                    char* normalEnd = nullptr;
                    long rawNormal = std::strtol(q, &normalEnd, 10);
                    if (normalEnd != q) {
                        result.normal = ResolveObjIndex(rawNormal, normalCount);
                        q = normalEnd;
                    }
                }
            }
        }
        while (*q && !std::isspace(static_cast<unsigned char>(*q))) ++q;
        p = q;
        return result;
    }
