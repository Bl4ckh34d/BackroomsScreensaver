
        struct Cluster {
            XMFLOAT3 sum{0.0f, 0.0f, 0.0f};
            XMFLOAT3 normal{0.0f, 0.0f, 0.0f};
            int count = 0;
        };
        struct Tri {
            int a;
            int b;
            int c;
        };
