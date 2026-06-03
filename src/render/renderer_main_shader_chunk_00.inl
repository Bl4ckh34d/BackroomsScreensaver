R"(
#define BRM_ENABLE_HIGH_BLOOD 0
cbuffer SceneConstants : register(b0)
{
    row_major float4x4 gViewProj;
    row_major float4x4 gLightViewProj;
    row_major float4x4 gFixtureLightViewProj;
    float4 gCameraPosTime;
    float4 gCameraDirAspect;
    float4 gLighting0;
    float4 gLighting1;
    float4 gFog0;
float4 gAO0;
float4 gPost0;
float4 gPost1;
float4 gPost2;
    float4 gShadow0;
    float4 gShadow1;
    float4 gShadow2;
    float4 gFixtureShadow0;
    float4 gFixtureShadow1;
    float4 gMaze0;
float4 gMaze1;
float4 gTexture0;
float4 gTransition0;
    float4 gHorror0;
    float4 gSparkLight0;
float4 gSparkLight1;
    float4 gBlood0;
    float4 gBlood1;
    float4 gBlood2;
    float4 gBlood3;
    float4 gBlood4;
    float4 gBlood5;
    float4 gBlood6;
    float4 gBlood7;
    float4 gBlood8;
    float4 gAir0;
    float4 gExitLight0;
    float4 gExitLight1;
    float4 gExitLight2;
    float4 gExitLight3;
    float4 gMonsterFog0;
};

Texture2DArray gAlbedo : register(t0);
Texture2DArray gNormalHeight : register(t1);
Texture2D gShadowMap : register(t2);
Texture2D gMazeOpen : register(t3);
Texture2DArray gMaterialProps : register(t4);
Texture2D gFlashlightPattern : register(t5);
Texture2D gLampDamage : register(t6);
Texture2DArray gLoosePages : register(t9);
Texture2D gFixtureShadowMap : register(t10);
Texture2D gCeilingAlbedo : register(t11);
Texture2D gCeilingNormalHeight : register(t12);
Texture2D gCeilingProps : register(t13);
Texture2D gCustomMenu : register(t14);
Texture2D gDoorAlbedo : register(t15);
Texture2D gDoorNormalHeight : register(t16);
Texture2D gDoorProps : register(t17);
Texture2D gDoorFrameAlbedo : register(t18);
Texture2D gDoorFrameNormalHeight : register(t19);
Texture2D gDoorFrameProps : register(t20);
SamplerState gSampler : register(s0);
SamplerComparisonState gShadowSampler : register(s1);

struct VSIn
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float2 uv : TEXCOORD0;
    float material : TEXCOORD1;
};

struct VSInstIn
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float2 uv : TEXCOORD0;
    float material : TEXCOORD1;
    float4 instX : TEXCOORD5;
    float4 instY : TEXCOORD6;
    float4 instZ : TEXCOORD7;
    float4 instMaterial : TEXCOORD8;
};

struct VSOut
{
    float4 pos : SV_POSITION;
    float3 worldPos : TEXCOORD0;
    float3 normal : TEXCOORD1;
    float3 tangent : TEXCOORD2;
    float2 uv : TEXCOORD3;
)"
