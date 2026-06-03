// Exit portal helpers.
// Included inside Renderer private section before maze mesh construction.

    struct ExitPortal {
        Tile tile{};
        int dx = 0;
        int dy = 1;
        float yaw = kPi;
        XMFLOAT3 inward{0.0f, 0.0f, -1.0f};
        XMFLOAT3 right{-1.0f, 0.0f, 0.0f};
        XMFLOAT3 wallCenter{};
        float halfSpan = kTile * 0.5f;
        bool valid = false;
    };

    ExitPortal BuildExitPortal(float tileW, float tileD) const {
        ExitPortal portal{};
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        if (!world.maze) return portal;
        const Maze& maze = *world.maze;
        portal.tile = maze.exit;
        struct DoorSide { int dx; int dy; };
        std::vector<DoorSide> sides;
        Tile e = maze.exit;
        if (sessionRuntime_.mode == RendererRuntimeMode::MainMenu) {
            sides.push_back({-1, 0});
        } else {
            if (e.y == maze.h - 2) sides.push_back({0, 1});
            if (e.x == maze.w - 2) sides.push_back({1, 0});
            if (e.y == 1) sides.push_back({0, -1});
            if (e.x == 1) sides.push_back({-1, 0});
        }
        const DoorSide fallbackSides[] = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};
        sides.insert(sides.end(), std::begin(fallbackSides), std::end(fallbackSides));

        for (const DoorSide& side : sides) {
            if (sessionRuntime_.mode != RendererRuntimeMode::MainMenu && maze.IsOpen(e.x + side.dx, e.y + side.dy)) continue;
            XMFLOAT3 c = maze.WorldCenter(e, 0.0f);
            portal.dx = side.dx;
            portal.dy = side.dy;
            portal.inward = {-static_cast<float>(side.dx), 0.0f, -static_cast<float>(side.dy)};
            portal.yaw = std::atan2(portal.inward.x, portal.inward.z);
            XMVECTOR rightVec = XMVector3Normalize(XMVector3Cross(XMVectorSet(0, 1, 0, 0), XMLoadFloat3(&portal.inward)));
            XMStoreFloat3(&portal.right, rightVec);
            portal.wallCenter = {c.x + side.dx * tileW * 0.5f, 0.0f, c.z + side.dy * tileD * 0.5f};
            portal.halfSpan = (side.dy != 0 ? tileW : tileD) * 0.5f;
            portal.valid = true;
            break;
        }
        return portal;
    }
