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
