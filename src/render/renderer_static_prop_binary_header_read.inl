        struct Header {
            char magic[8];
            uint32_t vertexCount;
            uint32_t vertexStride;
            float min[3];
            float max[3];
            float extra[4];
        };

        Header header{};
        std::ifstream in(path, std::ios::binary);
        if (!in) return false;
        in.read(reinterpret_cast<char*>(&header), sizeof(header));
        constexpr uint32_t kMaxStaticPropVertices = 1000000;
        bool isRawVertexFile = std::memcmp(header.magic, "BRMPRP1", 7) == 0;
        bool isPackedVertexFileV2 = std::memcmp(header.magic, "BRMPRP2", 7) == 0;
        bool isPackedVertexFile = std::memcmp(header.magic, "BRMPRP3", 7) == 0;
        if (!in || (!isRawVertexFile && !isPackedVertexFileV2 && !isPackedVertexFile) ||
            header.vertexCount == 0 ||
            header.vertexCount > kMaxStaticPropVertices) {
            return false;
        }
