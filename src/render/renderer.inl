#include "renderer_dependencies.h"

class Renderer {
public:
    #include "renderer_lifecycle.inl"

private:
    static constexpr int kStartupProgressPreShaderSteps = 3;
    static constexpr int kStartupProgressPostShaderSteps = 10;
    static constexpr int kStartupProgressUnitsPerStep = 4;

    const Maze& RenderMazeView() const {
        return gameWorld_.MazeView();
    }

    #include "renderer_private_modules.inl"
};
