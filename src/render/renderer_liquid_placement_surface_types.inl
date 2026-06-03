// Liquid, water, and decal placement helpers.
// Included inside Renderer private section before maze mesh construction.

    struct WaterTileSurface {
        bool active = false;
        bool suppressCard = false;
        int side = 0;
        int mode = 0;
        float seed = 0.0f;
        float score = -1.0f;
    };
