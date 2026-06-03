            std::ifstream faces(path);
            if (!faces) return;
            std::string faceLine;
            while (std::getline(faces, faceLine)) {
                if (faceLine.size() < 2 || faceLine[0] != 'f' || faceLine[1] != ' ') continue;
                std::vector<int> poly;
                poly.reserve(8);
                const char* p = faceLine.c_str() + 2;
                while (*p) {
                    int idx = ParseObjFaceIndex(p, static_cast<int>(positions.size()));
                    if (idx >= 0 && idx < static_cast<int>(positions.size())) poly.push_back(idx);
                }
                if (poly.size() < 3) continue;
                for (size_t i = 1; i + 1 < poly.size(); ++i) {
                    addTri(poly[0], poly[i], poly[i + 1]);
                }
            }
        };
