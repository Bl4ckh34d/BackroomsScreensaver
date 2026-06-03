    bool MarkLiquidCanvasArea(LiquidCanvasBuildContext& build,
                              float px,
                              float pz,
                              float width,
                              float depth,
                              float yaw,
                              bool water,
                              bool ceiling,
                              uint32_t sourceMask,
                              bool centerSource,
                              float seed,
                              float score,
                              bool downstream = false,
                              float sourceX = std::numeric_limits<float>::quiet_NaN(),
                              float sourceZ = std::numeric_limits<float>::quiet_NaN()) {
        #include "renderer_liquid_canvas_area_setup.inl"
        #include "renderer_liquid_canvas_area_tile_bounds.inl"
        #include "renderer_liquid_canvas_area_tile_loop.inl"

    }
