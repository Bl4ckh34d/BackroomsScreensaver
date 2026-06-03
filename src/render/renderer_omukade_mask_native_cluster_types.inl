            struct FaceCluster {
                XMFLOAT3 pos{0.0f, 0.0f, 0.0f};
                XMFLOAT3 normal{0.0f, 0.0f, 0.0f};
                XMFLOAT2 uv{0.0f, 0.0f};
                int count = 0;
            };
            struct FaceClusterTri {
                int a;
                int b;
                int c;
            };
