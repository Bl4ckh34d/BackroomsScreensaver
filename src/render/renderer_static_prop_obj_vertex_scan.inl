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
