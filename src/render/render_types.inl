// Shared render, runtime, particle, and navigation data structures.
// Included from main.cpp before math, settings, maze, and renderer code.

struct Vertex {
    XMFLOAT3 pos;
    XMFLOAT3 normal;
    XMFLOAT3 tangent;
    XMFLOAT2 uv;
    float material;
};
static_assert(sizeof(Vertex) == sizeof(float) * 12, "Vertex binary layout must stay packed for runtime mesh files.");

#pragma pack(push, 1)
struct PackedStaticPropVertexV2 {
    uint16_t px;
    uint16_t py;
    uint16_t pz;
    int16_t nx;
    int16_t ny;
    int16_t nz;
    int16_t tx;
    int16_t ty;
    int16_t tz;
    float u;
    float v;
    uint16_t material;
};

struct PackedStaticPropVertex {
    uint16_t px;
    uint16_t py;
    uint16_t pz;
    int16_t nx;
    int16_t ny;
    int16_t nz;
    int16_t tx;
    int16_t ty;
    int16_t tz;
    uint16_t u;
    uint16_t v;
    uint16_t material;
};
#pragma pack(pop)
static_assert(sizeof(PackedStaticPropVertexV2) == 28, "Packed V2 static prop binary layout changed.");
static_assert(sizeof(PackedStaticPropVertex) == 24, "Packed static prop binary layout changed.");

struct OverlayVertex {
    XMFLOAT2 pos;
    XMFLOAT4 color;
};

enum class RendererRuntimeMode {
    ScreensaverAutopilot,
    PlayableGame,
    DebugViewer,
    Preview
};

struct GameInputSnapshot {
    float moveX = 0.0f;
    float moveZ = 0.0f;
    float lookDeltaX = 0.0f;
    float lookDeltaY = 0.0f;
    bool jump = false;
    bool sprint = false;
    bool crouch = false;
    bool interact = false;
    bool pause = false;
};

struct SceneConstants {
    XMFLOAT4X4 viewProj;
    XMFLOAT4X4 lightViewProj;
    XMFLOAT4 cameraPosTime;
    XMFLOAT4 cameraDirAspect;
    XMFLOAT4 lighting0;
    XMFLOAT4 lighting1;
    XMFLOAT4 fog0;
    XMFLOAT4 ao0;
    XMFLOAT4 post0;
    XMFLOAT4 post1;
    XMFLOAT4 shadow0;
    XMFLOAT4 shadow1;
    XMFLOAT4 shadow2;
    XMFLOAT4 maze0;
    XMFLOAT4 maze1;
    XMFLOAT4 texture0;
    XMFLOAT4 transition0;
    XMFLOAT4 horror0;
    XMFLOAT4 sparkLight0;
    XMFLOAT4 sparkLight1;
    XMFLOAT4 blood0;
    XMFLOAT4 blood1;
    XMFLOAT4 blood2;
    XMFLOAT4 blood3;
    XMFLOAT4 blood4;
    XMFLOAT4 blood5;
    XMFLOAT4 blood6;
    XMFLOAT4 blood7;
    XMFLOAT4 blood8;
    XMFLOAT4 air0;
    XMFLOAT4 exitLight0;
    XMFLOAT4 monsterFog0;
};

struct Tile {
    int x = 0;
    int y = 0;
};

struct SparkEmitter {
    XMFLOAT3 pos{};
    float cooldown = 0.0f;
    bool triggered = false;
};

struct SparkParticle {
    XMFLOAT3 pos{};
    XMFLOAT3 vel{};
    float age = 0.0f;
    float life = 1.0f;
    float size = 0.025f;
};

struct SparkFlash {
    XMFLOAT3 pos{};
    float age = 0.0f;
    float life = 0.18f;
    float intensity = 1.0f;
};

struct SparkChain {
    XMFLOAT3 pos{};
    float timer = 0.0f;
    float intensity = 1.0f;
    int remaining = 0;
};

struct RuntimeLampState {
    Tile tile{};
    XMFLOAT3 pos{};
    float damage = 0.0f;
    float sparkTimer = 0.0f;
    bool broken = false;
};

struct SteamEmitter {
    XMFLOAT3 pos{};
    XMFLOAT3 dir{0.0f, 0.0f, 1.0f};
    float cooldown = 0.0f;
    bool panelDropped = false;
    bool triggered = false;
};

struct SteamParticle {
    XMFLOAT3 pos{};
    XMFLOAT3 vel{};
    float age = 0.0f;
    float life = 1.0f;
    float size = 0.25f;
};

struct VentDrop {
    XMFLOAT3 pos{};
    XMFLOAT3 vel{};
    float yaw = 0.0f;
    float roll = 0.0f;
    float angular = 0.0f;
    float age = 0.0f;
    float life = 3.0f;
    float halfW = 0.46f;
    float halfH = 0.27f;
    bool landed = false;
};

struct AirParticle {
    XMFLOAT3 pos{};
    XMFLOAT3 vel{};
    float age = 0.0f;
    float life = 32.0f;
    float size = 0.018f;
    float seed = 0.0f;
    float angle = 0.0f;
    float spin = 0.0f;
    float aspect = 1.0f;
    float nearLayer = 0.0f;
};

struct BloodScarePoint {
    XMFLOAT3 pos{};
    XMFLOAT3 source{};
    XMFLOAT3 normal{0.0f, 0.0f, 1.0f};
    float radius = 2.2f;
    float activationTime = -1000000.0f;
    float focusDelaySeconds = 1.20f;
    float dreadScale = 1.0f;
    bool triggered = false;
    bool focusTaken = false;
    bool requireFacing = false;
    bool revealBlood = true;
    bool waterLiquid = false;
};

struct BloodRevealRegion {
    XMFLOAT3 center{};
    float radius = 0.0f;
    float activationTime = -1000000.0f;
    bool waterLiquid = false;
};
