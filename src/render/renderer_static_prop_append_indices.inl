        for (uint32_t n = 0; n < static_cast<uint32_t>(mesh.vertices.size()); ++n) {
            uint32_t idx = base + n;
            indices.push_back(idx);
            if (shadowIndices) shadowIndices->push_back(idx);
        }
        return true;
    }
