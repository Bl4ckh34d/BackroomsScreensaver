
        int meshId = RegisterInstancedStaticMesh(mesh, instancedVertices, instancedIndices, instancedMeshRanges);
        if (meshId < 0) return false;

        float c = std::cos(yaw);
        float s = std::sin(yaw);
        XMFLOAT3 right{c, 0.0f, -s};
        XMFLOAT3 worldUp{0.0f, 1.0f, 0.0f};
        XMFLOAT3 forward{s, 0.0f, c};
        float cp = std::cos(pitch);
        float sp = std::sin(pitch);
        XMFLOAT3 up = Normalize3(Add3(Scale3(worldUp, cp), Scale3(forward, sp)), worldUp);
        forward = Normalize3(Add3(Scale3(forward, cp), Scale3(worldUp, -sp)), forward);
        XMFLOAT3 axisX = Scale3(right, scaleX);
        XMFLOAT3 axisY = Scale3(up, scaleY);
        XMFLOAT3 axisZ = Scale3(forward, scaleZ);
