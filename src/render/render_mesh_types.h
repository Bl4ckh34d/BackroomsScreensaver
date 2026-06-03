#pragma once

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
