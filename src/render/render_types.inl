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

struct MonsterLimbAnchor {
    bool planted = false;
    XMFLOAT3 anchor{};
    XMFLOAT3 anchorNormal{};
    XMFLOAT3 swingFrom{};
    XMFLOAT3 swingTo{};
    XMFLOAT3 swingFromNormal{};
    XMFLOAT3 swingToNormal{};
    float swingStart = 0.0f;
    float swingDuration = 0.34f;
    int retargetCount = 0;
};

struct MonsterHandprint {
    XMFLOAT3 pos{};
    XMFLOAT3 normal{};
    float size = 0.18f;
    float seed = 0.0f;
    float createdAt = 0.0f;
};

struct CollectiblePage {
    XMFLOAT3 center{};
    XMFLOAT3 right{1.0f, 0.0f, 0.0f};
    XMFLOAT3 up{0.0f, 1.0f, 0.0f};
    XMFLOAT3 normal{0.0f, 0.0f, 1.0f};
    int pageIndex = -1;
    bool collected = false;
};

struct SavePoint {
    XMFLOAT3 pos{};
    float yaw = 0.0f;
    bool active = false;
};

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

struct TexturedOverlayVertex {
    XMFLOAT2 pos;
    XMFLOAT2 uv;
    XMFLOAT4 color;
};

struct StaticIndexChunk {
    UINT startIndex = 0;
    UINT indexCount = 0;
    XMFLOAT3 center{};
    float radius = 0.0f;
    int minTileX = 0;
    int minTileY = 0;
    int maxTileX = 0;
    int maxTileY = 0;
};

struct StaticInstanceData {
    XMFLOAT4 axisXOriginX{};
    XMFLOAT4 axisYOriginY{};
    XMFLOAT4 axisZOriginZ{};
    XMFLOAT4 materialOverrideVariant{-1.0f, 0.0f, 0.0f, 0.0f};
};

struct StaticInstanceChunk {
    UINT startIndex = 0;
    UINT indexCount = 0;
    INT baseVertex = 0;
    UINT startInstance = 0;
    UINT instanceCount = 0;
    XMFLOAT3 center{};
    float radius = 0.0f;
    int minTileX = 0;
    int minTileY = 0;
    int maxTileX = 0;
    int maxTileY = 0;
};

enum class RendererRuntimeMode {
    ScreensaverAutopilot,
    MainMenu,
    PlayableGame,
    DebugViewer,
    Preview
};

inline bool IsPlayableSimulationMode(RendererRuntimeMode mode) {
    return mode == RendererRuntimeMode::PlayableGame ||
        mode == RendererRuntimeMode::ScreensaverAutopilot;
}

struct GameInputSnapshot {
    float moveX = 0.0f;
    float moveZ = 0.0f;
    float lookDeltaX = 0.0f;
    float lookDeltaY = 0.0f;
    bool sprint = false;
    bool crouch = false;
    bool interact = false;
    bool flashlight = false;
    bool pause = false;
};

struct SceneConstants {
    XMFLOAT4X4 viewProj;
    XMFLOAT4X4 lightViewProj;
    XMFLOAT4X4 fixtureLightViewProj;
    XMFLOAT4X4 monsterEyeViewProj0;
    XMFLOAT4X4 monsterEyeViewProj1;
    XMFLOAT4 cameraPosTime;
    XMFLOAT4 cameraDirAspect;
    XMFLOAT4 lighting0;
    XMFLOAT4 lighting1;
    XMFLOAT4 fog0;
    XMFLOAT4 ao0;
    XMFLOAT4 post0;
    XMFLOAT4 post1;
    XMFLOAT4 post2;
    XMFLOAT4 shadow0;
    XMFLOAT4 shadow1;
    XMFLOAT4 shadow2;
    XMFLOAT4 fixtureShadow0;
    XMFLOAT4 fixtureShadow1;
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
    XMFLOAT4 exitLight1;
    XMFLOAT4 exitLight2;
    XMFLOAT4 exitLight3;
    XMFLOAT4 monsterFog0;
    XMFLOAT4 monsterEye0;
    XMFLOAT4 monsterEye1;
    XMFLOAT4 monsterEye2;
    XMFLOAT4 monsterEye3;
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
    int humVariant = 0;
    bool flickerWasDim = false;
    float flickerClickCooldown = 0.0f;
};

struct LampHumCandidate {
    size_t index = 0;
    float distSq = 0.0f;
};

struct SteamEmitter {
    XMFLOAT3 pos{};
    XMFLOAT3 dir{0.0f, 0.0f, 1.0f};
    float cooldown = 0.0f;
    bool panelDropped = false;
    bool triggered = false;
};

struct WetDripEmitter {
    XMFLOAT3 pos{};
    float interval = 1.0f;
    float timer = 0.0f;
    float volume = 0.30f;
    float age = 0.0f;
    float audibleDelay = 0.0f;
};

struct WetFloorFootprint {
    XMFLOAT2 center{};
    XMFLOAT2 right{1.0f, 0.0f};
    XMFLOAT2 forward{0.0f, 1.0f};
    float halfW = 0.0f;
    float halfD = 0.0f;
    float wetDelaySeconds = 0.0f;
};

struct PlayerAudibleSoundPulse {
    XMFLOAT3 pos{};
    float radius = 0.0f;
    float age = 0.0f;
    float life = 0.90f;
    bool processedByMonster = false;
    bool heardByMonster = false;
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
