    bool LoadStaticPropObj(const std::wstring& configuredPath, float material, StaticPropMesh& out) const {
        out = {};
        std::filesystem::path path = ResolveConfiguredAssetPath(configuredPath);
        if (path.empty()) {
            std::filesystem::path fallbackConfigured(configuredPath);
            std::wstring ext = fallbackConfigured.extension().wstring();
            std::transform(ext.begin(), ext.end(), ext.begin(), [](wchar_t c) { return static_cast<wchar_t>(towlower(c)); });
            if (ext == L".brmesh") {
                fallbackConfigured.replace_extension(L".obj");
                path = ResolveConfiguredAssetPath(fallbackConfigured.wstring());
            }
        }
        if (path.empty()) {
            StartupProfileLine(L"Prop mesh not found: " + configuredPath);
            return false;
        }

        std::wstring ext = path.extension().wstring();
        std::transform(ext.begin(), ext.end(), ext.begin(), [](wchar_t c) { return static_cast<wchar_t>(towlower(c)); });
        auto applyDefaultMaterialIfUnassigned = [&]() {
            bool hasAssignedMaterial = false;
            for (const Vertex& v : out.vertices) {
                if (std::floor(v.material) > 0.5f) {
                    hasAssignedMaterial = true;
                    break;
                }
            }
            if (!hasAssignedMaterial) {
                for (Vertex& v : out.vertices) {
                    v.material = material;
                }
            }
        };
        if (ext == L".brmesh") {
            if (LoadStaticPropBinary(path, out)) {
                applyDefaultMaterialIfUnassigned();
                return true;
            }

            std::filesystem::path fallbackPath = path;
            fallbackPath.replace_extension(L".obj");
            std::error_code ec;
            if (std::filesystem::exists(fallbackPath, ec)) {
                out = {};
                path = fallbackPath;
            } else {
                StartupProfileLine(L"Could not load prop mesh: " + path.wstring());
                return false;
            }
        } else if (ext == L".bin") {
            if (LoadStaticPropBinary(path, out)) {
                applyDefaultMaterialIfUnassigned();
                return true;
            }
            StartupProfileLine(L"Could not load prop mesh: " + path.wstring());
            return false;
        }

        std::ifstream in(path);
        if (!in) return false;

        std::vector<XMFLOAT3> positions;
        std::vector<XMFLOAT2> texcoords;
        positions.reserve(20000);
        texcoords.reserve(20000);
        XMFLOAT3 minP{std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
        XMFLOAT3 maxP{-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max()};
        std::string line;
        while (std::getline(in, line)) {
            if (line.size() >= 3 && line[0] == 'v' && line[1] == 't' && std::isspace(static_cast<unsigned char>(line[2]))) {
                const char* p = line.c_str() + 3;
                char* end = nullptr;
                float u = std::strtof(p, &end);
                if (end == p) continue;
                p = end;
                float v = std::strtof(p, &end);
                if (end == p) continue;
                texcoords.push_back({u, v});
                continue;
            }
            if (line.size() < 2 || line[0] != 'v' || line[1] != ' ') continue;
            const char* p = line.c_str() + 2;
            char* end = nullptr;
            float x = std::strtof(p, &end);
            if (end == p) continue;
            p = end;
            float y = std::strtof(p, &end);
            if (end == p) continue;
            p = end;
            float z = std::strtof(p, &end);
            if (end == p) continue;
            positions.push_back({x, y, z});
            minP.x = std::min(minP.x, x);
            minP.y = std::min(minP.y, y);
            minP.z = std::min(minP.z, z);
            maxP.x = std::max(maxP.x, x);
            maxP.y = std::max(maxP.y, y);
            maxP.z = std::max(maxP.z, z);
        }
        if (positions.empty()) return false;

        auto uvFor = [&](XMFLOAT3 p, XMFLOAT3 n, int texcoord) {
            if (texcoord >= 0 && texcoord < static_cast<int>(texcoords.size())) {
                return texcoords[static_cast<size_t>(texcoord)];
            }
            if (std::abs(n.y) > 0.62f) return XMFLOAT2{p.x * 1.7f + 0.5f, p.z * 1.7f + 0.5f};
            if (std::abs(n.x) > std::abs(n.z)) return XMFLOAT2{p.z * 1.7f + 0.5f, p.y * 1.7f};
            return XMFLOAT2{p.x * 1.7f + 0.5f, p.y * 1.7f};
        };
        auto addTri = [&](ObjFaceVertex va, ObjFaceVertex vb, ObjFaceVertex vc, float faceMaterial) {
            XMFLOAT3 a = positions[static_cast<size_t>(va.vertex)];
            XMFLOAT3 b = positions[static_cast<size_t>(vb.vertex)];
            XMFLOAT3 c = positions[static_cast<size_t>(vc.vertex)];
            XMFLOAT3 n = Normalize3(Cross3(Sub3(b, a), Sub3(c, a)), {0.0f, 1.0f, 0.0f});
            XMFLOAT3 tangent = Normalize3(Sub3(b, a), {1.0f, 0.0f, 0.0f});
            if (std::abs(Dot3(tangent, n)) > 0.92f) {
                tangent = Normalize3(Cross3({0.0f, 1.0f, 0.0f}, n), {1.0f, 0.0f, 0.0f});
            }
            out.vertices.push_back({a, n, tangent, uvFor(a, n, va.texcoord), faceMaterial});
            out.vertices.push_back({b, n, tangent, uvFor(b, n, vb.texcoord), faceMaterial});
            out.vertices.push_back({c, n, tangent, uvFor(c, n, vc.texcoord), faceMaterial});
        };

        std::ifstream faces(path);
        if (!faces) return false;
        float currentMaterial = material;
        while (std::getline(faces, line)) {
            if (line.rfind("usemtl ", 0) == 0) {
                const char* p = line.c_str() + 7;
                while (*p && std::isspace(static_cast<unsigned char>(*p))) ++p;
                currentMaterial = material;
                if (*p == 'p' || *p == 'P') {
                    char* end = nullptr;
                    long id = std::strtol(p + 1, &end, 10);
                    if (end != p + 1 && id >= 0 && id < kMaterialCount) {
                        currentMaterial = static_cast<float>(id);
                    }
                }
                continue;
            }
            if (line.size() < 2 || line[0] != 'f' || line[1] != ' ') continue;
            std::vector<ObjFaceVertex> poly;
            poly.reserve(8);
            const char* p = line.c_str() + 2;
            while (*p) {
                ObjFaceVertex fv = ParseObjFaceVertex(p, static_cast<int>(positions.size()), static_cast<int>(texcoords.size()));
                if (fv.vertex >= 0 && fv.vertex < static_cast<int>(positions.size())) poly.push_back(fv);
            }
            if (poly.size() < 3) continue;
            for (size_t i = 1; i + 1 < poly.size(); ++i) {
                addTri(poly[0], poly[i], poly[i + 1], currentMaterial);
            }
        }

        if (out.vertices.empty()) return false;
        out.min = minP;
        out.max = maxP;
        out.generatedUvFallback = StaticPropNeedsGeneratedUv(out);
        return true;
    }
