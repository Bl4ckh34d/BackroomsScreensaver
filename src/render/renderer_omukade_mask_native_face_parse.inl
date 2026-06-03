            struct FaceV {
                int p;
                int t;
                int n;
            };
            std::vector<std::array<FaceV, 3>> faceTris;
            faceTris.reserve(80000);
            std::ifstream faces(path);
            if (!faces) return false;
            std::string faceLine;
            while (std::getline(faces, faceLine)) {
                if (faceLine.size() < 2 || faceLine[0] != 'f' || faceLine[1] != ' ') continue;
                std::vector<ObjFaceVertex> poly;
                poly.reserve(8);
                const char* p = faceLine.c_str() + 2;
                while (*p) {
                    ObjFaceVertex v = ParseObjFaceVertex(p, static_cast<int>(positions.size()),
                        static_cast<int>(texcoords.size()), static_cast<int>(normals.size()));
                    if (v.vertex >= 0 && v.vertex < static_cast<int>(positions.size())) poly.push_back(v);
                }
                if (poly.size() < 3) continue;
                for (size_t i = 1; i + 1 < poly.size(); ++i) {
                    faceTris.push_back({FaceV{poly[0].vertex, poly[0].texcoord, poly[0].normal},
                        FaceV{poly[i].vertex, poly[i].texcoord, poly[i].normal},
                        FaceV{poly[i + 1].vertex, poly[i + 1].texcoord, poly[i + 1].normal}});
                }
            }
            if (faceTris.empty()) return false;
