// Maze mesh construction and placement.
// Included inside Renderer private section after static mesh helpers.

    void CreateMazeMesh() {
#include "renderer_maze_mesh_body.inl"
        if (sessionRuntime_.IsPlayableGame() && !gEffectDebugViewer && !gBloodDebugEveryWall && !BenchmarkDemoEnabled()) {
            mazeRespawnState_ = std::make_unique<MazeRespawnState>(MazeRespawnState{
                effectRuntime_.sparkEmitters, effectRuntime_.steamEmitters,
                effectRuntime_.wetDripEmitters, scareRuntime_.bloodScarePoints
            });
        } else {
            mazeRespawnState_.reset();
        }
    }
