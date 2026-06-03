    struct FloorFootprint {
        float x = 0.0f;
        float z = 0.0f;
        float hx = 0.0f;
        float hz = 0.0f;
        float c = 1.0f;
        float s = 0.0f;
    };

    struct CandidatePlacement {
        float x = 0.0f;
        float z = 0.0f;
        float yaw = 0.0f;
        float score = -1.0e9f;
    };
