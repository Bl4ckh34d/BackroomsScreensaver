        if (isRawVertexFile) {
            if (header.vertexStride != sizeof(Vertex)) return false;
            out.vertices.resize(header.vertexCount);
            in.read(reinterpret_cast<char*>(out.vertices.data()),
                static_cast<std::streamsize>(out.vertices.size() * sizeof(Vertex)));
            if (!in) {
                out = {};
                return false;
            }
