// Renderer shader source strings, compilation, shader object creation, and input layouts.
// Included inside Renderer private section before CreateStates().

    bool CreateShaders() {
        static const char* shader = R"(
#define BRM_ENABLE_HIGH_BLOOD 0
cbuffer SceneConstants : register(b0)
{
    row_major float4x4 gViewProj;
    row_major float4x4 gLightViewProj;
    float4 gCameraPosTime;
    float4 gCameraDirAspect;
    float4 gLighting0;
    float4 gLighting1;
    float4 gFog0;
float4 gAO0;
float4 gPost0;
float4 gPost1;
float4 gShadow0;
float4 gShadow1;
float4 gShadow2;
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
    float4 gMonsterFog0;
};

Texture2DArray gAlbedo : register(t0);
Texture2DArray gNormalHeight : register(t1);
Texture2D gShadowMap : register(t2);
Texture2D gMazeOpen : register(t3);
Texture2DArray gMaterialProps : register(t4);
Texture2D gFlashlightPattern : register(t5);
Texture2D gLampDamage : register(t6);
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

struct VSOut
{
    float4 pos : SV_POSITION;
    float3 worldPos : TEXCOORD0;
    float3 normal : TEXCOORD1;
    float3 tangent : TEXCOORD2;
    float2 uv : TEXCOORD3;
    nointerpolation float material : TEXCOORD4;
};

VSOut VSMain(VSIn input)
{
    VSOut o;
    o.worldPos = input.pos;
    o.normal = input.normal;
    o.tangent = input.tangent;
    o.uv = input.uv;
    o.material = input.material;
    if (input.material > 11.05 && input.material < 11.45)
    {
        float seed = frac(input.material * 17.37);
        float time = gCameraPosTime.w;
        float vertical = saturate(input.uv.y);
        float edge = abs(input.uv.x - 0.5) * 2.0;
        float lowSpread = smoothstep(0.18, 1.0, vertical);
        float wave = sin(time * (0.62 + seed * 0.45) + input.pos.y * 2.7 + seed * 19.0);
        float flutter = sin(time * (1.41 + seed * 0.51) + input.pos.x * 1.8 + input.pos.z * 2.1);
        o.worldPos += input.normal * (wave * (0.055 + lowSpread * 0.115) + (1.0 - edge) * flutter * 0.038);
        o.worldPos += input.tangent * (flutter * (0.035 + lowSpread * 0.070));
        o.worldPos.y += sin(time * 0.52 + seed * 11.0 + input.uv.x * 4.2) * 0.028 * (1.0 - vertical * 0.35);
    }
    o.pos = mul(float4(o.worldPos, 1.0), gViewProj);
    return o;
}

float MaterialId(float material)
{
    return clamp(floor(material), 0.0, 25.0);
}

void ShadowPS(VSOut input)
{
    float materialId = MaterialId(input.material);
    bool transparentPaper = materialId > 8.5 && materialId < 9.5 && frac(input.material) < 0.5;
    if ((materialId > 0.5 && materialId < 2.5) ||
        transparentPaper ||
        (materialId > 10.5 && materialId < 11.5) ||
        (materialId > 11.5 && materialId < 12.5) ||
        (materialId > 12.5 && materialId < 13.5) ||
        (materialId > 13.5 && materialId < 14.5) ||
        (materialId > 14.5 && materialId < 15.5) ||
        (materialId > 17.5 && materialId < 18.5) ||
        (materialId > 18.5 && materialId < 19.5) ||
        (materialId > 24.5 && materialId < 25.5))
    {
        discard;
    }
}

float3 MaterialUV(float2 uv, float material)
{
    float slice = MaterialId(material);
    return float3(uv, slice);
}

float3 BackroomsBaseColor(float3 base, float materialId)
{
    float luma = dot(base, float3(0.299, 0.587, 0.114));
    if (materialId < 0.5)
    {
        float3 classicWall = float3(0.94, 0.74, 0.29) * (0.68 + luma * 0.68);
        return saturate(lerp(base * 1.08, classicWall, 0.14));
    }
    if (materialId > 0.5 && materialId < 1.5)
    {
        float3 classicCarpet = float3(0.74, 0.62, 0.34) * (0.70 + luma * 0.58);
        return saturate(lerp(base * 1.10, classicCarpet, 0.34));
    }
    if (materialId > 1.5 && materialId < 2.5)
    {
        float3 classicCeiling = float3(0.92, 0.76, 0.30) * (0.78 + luma * 0.42);
        return saturate(lerp(base * float3(1.06, 1.00, 0.76), classicCeiling, 0.72));
    }
    return base;
}

void UnderlyingSurface(float3 worldPos, float3 normal, out float materialId, out float2 uv)
{
    float wallScale = max(gTexture0.x, 0.20);
    float floorScale = max(gTexture0.y, 0.20);
    float explicitCeilingScale = gTexture0.z;
    float2 ceilingScale = explicitCeilingScale > 0.001
        ? max(float2(explicitCeilingScale, explicitCeilingScale), float2(0.20, 0.20))
        : max(gMaze0.zw, float2(0.20, 0.20));
    if (normal.y > 0.55)
    {
        materialId = 1.0;
        uv = worldPos.xz / floorScale;
    }
    else if (normal.y < -0.55)
    {
        materialId = 2.0;
        uv = (worldPos.xz - gMaze0.xy) / ceilingScale;
    }
    else if (abs(normal.z) >= abs(normal.x))
    {
        materialId = 0.0;
        uv = float2(worldPos.x, worldPos.y) / wallScale;
    }
    else
    {
        materialId = 0.0;
        uv = float2(worldPos.z, worldPos.y) / wallScale;
    }
}

float3 UnderlyingSurfaceColor(float3 worldPos, float3 normal, out float aoMap)
{
    float materialId = 0.0;
    float2 surfaceUv = float2(0.0, 0.0);
    UnderlyingSurface(worldPos, normal, materialId, surfaceUv);
    float mipBias = materialId > 0.5 && materialId < 1.5 ? 1.15 : 0.45;
    float3 materialUv = float3(surfaceUv, materialId);
    float4 base = gAlbedo.SampleBias(gSampler, materialUv, mipBias);
    float4 pbr = gMaterialProps.SampleBias(gSampler, materialUv, mipBias);
    aoMap = saturate(pbr.r);
    return BackroomsBaseColor(base.rgb, materialId);
}

float2 AspectSoftenedBloodUv(float2 uv, float3 worldPos)
{
    float2 duvDx = ddx(uv);
    float2 duvDy = ddy(uv);
    float det = duvDx.x * duvDy.y - duvDx.y * duvDy.x;
    if (abs(det) < 0.00001)
    {
        return uv;
    }
    float3 dpDx = ddx(worldPos);
    float3 dpDy = ddy(worldPos);
    float3 dpDu = (dpDx * duvDy.y - dpDy * duvDx.y) / det;
    float3 dpDv = (-dpDx * duvDy.x + dpDy * duvDx.x) / det;
    float uMeters = length(dpDu);
    float vMeters = length(dpDv);
    float minMeters = max(min(uMeters, vMeters), 0.05);
    float2 aspect = min(float2(uMeters, vMeters) / minMeters, float2(2.65, 2.65));
    aspect = lerp(float2(1.0, 1.0), aspect, 0.42);
    return (uv - 0.5) * aspect + 0.5;
}

float2 BloodUvWorldMeters(float2 uv, float3 worldPos)
{
    float2 duvDx = ddx(uv);
    float2 duvDy = ddy(uv);
    float3 dpDx = ddx(worldPos);
    float3 dpDy = ddy(worldPos);
    float det = duvDx.x * duvDy.y - duvDx.y * duvDy.x;
    if (abs(det) < 0.00000008)
    {
        float m0 = length(dpDx) / max(length(duvDx), 0.00001);
        float m1 = length(dpDy) / max(length(duvDy), 0.00001);
        float fallback = clamp(max(m0, m1), 0.08, 4.0);
        return float2(fallback, fallback);
    }
    float3 dpDu = (dpDx * duvDy.y - dpDy * duvDx.y) / det;
    float3 dpDv = (-dpDx * duvDy.x + dpDy * duvDx.x) / det;
    return clamp(float2(length(dpDu), length(dpDv)), float2(0.05, 0.05), float2(4.0, 4.0));
}

float Hash21(float2 p)
{
    p = frac(p * float2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return frac(p.x * p.y);
}

float Hash31(float3 p)
{
    p = frac(p * float3(127.1, 311.7, 74.7));
    p += dot(p, p.yzx + 19.19);
    return frac((p.x + p.y) * p.z);
}

float Len2Inf(float2 v)
{
    v = abs(v);
    return max(v.x, v.y);
}

float Noise3(float3 p)
{
    float3 i = floor(p);
    float3 f = frac(p);
    float3 u = f * f * (3.0 - 2.0 * f);
    float n000 = Hash31(i + float3(0.0, 0.0, 0.0));
    float n100 = Hash31(i + float3(1.0, 0.0, 0.0));
    float n010 = Hash31(i + float3(0.0, 1.0, 0.0));
    float n110 = Hash31(i + float3(1.0, 1.0, 0.0));
    float n001 = Hash31(i + float3(0.0, 0.0, 1.0));
    float n101 = Hash31(i + float3(1.0, 0.0, 1.0));
    float n011 = Hash31(i + float3(0.0, 1.0, 1.0));
    float n111 = Hash31(i + float3(1.0, 1.0, 1.0));
    float nx00 = lerp(n000, n100, u.x);
    float nx10 = lerp(n010, n110, u.x);
    float nx01 = lerp(n001, n101, u.x);
    float nx11 = lerp(n011, n111, u.x);
    float nxy0 = lerp(nx00, nx10, u.y);
    float nxy1 = lerp(nx01, nx11, u.y);
    return lerp(nxy0, nxy1, u.z);
}

float Fbm3(float3 p)
{
    float v = 0.0;
    float a = 0.52;
    [loop]
    for (int i = 0; i < 5; ++i)
    {
        v += Noise3(p) * a;
        p = p * 2.07 + float3(17.1, 31.7, 11.3);
        a *= 0.50;
    }
    return v;
}

float SmokeFbm(float3 p, float seed, float time)
{
    p = p * 0.60 + float3(seed * 17.0, seed * 7.0, seed * 13.0);
    float v = Noise3(p);
    p *= 0.30;
    v = lerp(v, Noise3(p + time * float3(0.11, -0.31, 0.07)), 0.70);
    p *= 0.30;
    v = lerp(v, Noise3(p - time * float3(0.07, 0.23, 0.13)), 0.70);
    return v;
}

float BlackSmokeDensity(float3 p, float seed, float time)
{
    float3 uvw = p;
    float3 lmn = (p + 1.0) * 63.5;
    float t = time * (0.055 + seed * 0.010) + 32.0;

    float d2 = SmokeFbm(float3(0.6, 0.3, 0.6) * lmn + float3(0.0, 8.0 * t, 0.0), seed, time);
    float phase = d2 * 6.2831853;
    float d1 = SmokeFbm(0.3 * lmn + float3(0.0, 4.0 * t, 0.0) +
        5.0 * float3(cos(phase), 2.0 * d2, sin(phase)), seed + 0.37, time);
    d1 = pow(max(d1, 0.0001), lerp(4.0, 12.0, smoothstep(0.56, 1.0, Len2Inf(uvw.xz))));

    float density = 0.18 * smoothstep(0.0, 0.02, d1) +
        0.50 * smoothstep(0.02, 0.08, d1) +
        0.18 * smoothstep(0.08, 1.0, d1);
    float radialFade = 1.0 - smoothstep(0.68, 1.18, length(p.xz * float2(0.92, 1.04)));
    float verticalFade = smoothstep(-1.10, -0.74, p.y) * (1.0 - smoothstep(0.86, 1.18, p.y));
    float boxFade = 1.0 - smoothstep(0.90, 1.20, max(Len2Inf(p.xz), abs(p.y) * 0.82));
    return saturate(density * radialFade * verticalFade * boxFade);
}

float2 Rotate2(float2 p, float a)
{
    float c = cos(a);
    float s = sin(a);
    return float2(p.x * c - p.y * s, p.x * s + p.y * c);
}

)" R"(
float BloodShape(float2 uv, float3 worldPos, float3 normal, float seed, out float drips, out float thickness)
{
    drips = 0.0;
    thickness = 0.0;
    if (uv.x <= 0.001 || uv.x >= 0.999 || uv.y <= 0.001 || uv.y >= 0.999)
    {
        return 0.0;
    }

    float wallMask = saturate(1.0 - abs(normal.y));
    float ceilingMask = smoothstep(0.45, 0.90, -normal.y);
    float2 local = uv * 2.0 - 1.0;
    float angle = seed * 6.2831853 + Hash21(floor(worldPos.xz * 0.19 + seed * 17.0)) * 0.75;
    float2 q = Rotate2(local, angle);
    float3 volumeP = float3(q * (2.75 + seed * 1.20), seed * 21.7 + dot(worldPos, float3(0.071, 0.137, 0.093)));
    float largeNoise = Fbm3(volumeP * 1.95);
    float midNoise = Fbm3(volumeP * 6.40 + 9.1);
    float fineNoise = Fbm3(volumeP * 18.5 + 31.0);

    float2 axis = float2(0.74 + Hash21(float2(seed, 2.1)) * 0.30, 0.48 + Hash21(float2(seed, 5.7)) * 0.22);
    float radial = length(q / axis);
    float volumeField = 0.70 - radial * 1.55 + (largeNoise - 0.5) * 0.42 + (midNoise - 0.5) * 0.28;
    float impact = smoothstep(0.20, 0.48, volumeField) * (1.0 - smoothstep(0.15, 0.70, radial));
    float tornHoles = smoothstep(0.48, 0.78, fineNoise + radial * 0.30);
    impact *= lerp(0.72, 0.18, tornHoles);

    float2 sprayDir = normalize(float2(cos(angle + Hash21(float2(seed, 12.7)) * 1.65),
                                      sin(angle + Hash21(float2(seed, 14.9)) * 1.65)));
    float satellites = 0.0;
    float tinyMist = 0.0;
    [loop]
    for (int i = 0; i < 76; ++i)
    {
        float fi = (float)i;
        float chooseDir = Hash21(float2(seed * 29.0 + fi, 11.0));
        float a = fi * 2.39996 + seed * 8.3 + Hash21(float2(fi, seed * 17.0)) * 1.1;
        float2 radialDir = float2(cos(a), sin(a));
        float2 dir = normalize(lerp(radialDir, sprayDir, step(0.54, chooseDir) * (0.30 + Hash21(float2(fi, seed * 43.0)) * 0.55)));
        float r = 0.10 + pow(Hash21(float2(seed * 23.0 + fi, 3.0)), 0.52) * 1.02;
        float2 c = dir * r;
        c += (float2(Hash21(float2(fi, seed * 41.0)), Hash21(float2(seed * 53.0, fi))) - 0.5) * 0.22;
        float2 p = local - c;
        float dropletRand = Hash21(c + seed);
        float sizeRand = pow(dropletRand, 2.65);
        float dotRadius = 0.0038 + sizeRand * 0.040;
        dotRadius *= lerp(0.62, 1.0, step(0.88, Hash21(float2(fi, seed + 88.0))));
        float stretch = 0.78 + Hash21(c + 12.0) * 1.18;
        stretch = lerp(stretch, 1.65 + Hash21(c + 51.0) * 2.10, step(0.74, chooseDir));
        float2 pr = Rotate2(p, atan2(dir.y, dir.x));
        pr.x /= stretch;
        float spot = exp(-dot(pr, pr) / max(0.00012, dotRadius * dotRadius));
        float breakup = smoothstep(0.22, 0.62, Fbm3(float3(pr * 28.0, seed * 14.0 + fi)));
        float satellite = spot * lerp(0.36, 0.92, breakup);
        satellites = max(satellites, satellite);
        tinyMist = max(tinyMist, spot * (1.0 - smoothstep(0.011, 0.046, dotRadius)) * lerp(0.46, 1.0, breakup));
    }

    float streaks = 0.0;
    [loop]
    for (int k = 0; k < 16; ++k)
    {
        float fk = (float)k;
        float armAngle = atan2(sprayDir.y, sprayDir.x) + (Hash21(float2(fk, seed * 9.0)) - 0.5) * (1.05 + Hash21(float2(seed, fk + 3.0)) * 1.7);
        float2 dir = float2(cos(armAngle), sin(armAngle));
        float2 origin = (float2(Hash21(float2(seed * 13.0, fk)), Hash21(float2(fk, seed * 19.0))) - 0.5) * 0.28;
        float2 p = local - origin;
        float along = dot(p, dir);
        float across = abs(dot(local, float2(-dir.y, dir.x)));
        float len = 0.22 + Hash21(float2(seed + 4.0, fk)) * 0.72;
        float width = 0.0035 + Hash21(float2(seed * 7.0, fk + 5.0)) * 0.013;
        float gate = smoothstep(0.010, 0.060, along) * (1.0 - smoothstep(len, len + 0.11, along));
        float strand = exp(-(across * across) / max(0.00005, width * width)) * gate;
        float breakup = smoothstep(0.20, 0.68, Fbm3(float3(p * (12.0 + fk), seed * 19.0 + fk)));
        float2 tip = p - dir * len;
        float tipDrop = exp(-dot(tip, tip) / max(0.00018, width * width * 7.5));
        streaks = max(streaks, max(strand * breakup, tipDrop * 0.76));
    }

    [loop]
    for (int j = 0; j < 18; ++j)
    {
        float fj = (float)j;
        float x = 0.14 + Hash21(float2(seed * 41.0 + fj, 91.0)) * 0.72;
        float top = 0.13 + Hash21(float2(seed * 13.0, fj)) * 0.42;
        float len = 0.16 + Hash21(float2(seed * 7.0, fj + 2.0)) * 0.76;
        float width = 0.0025 + Hash21(float2(seed * 31.0, fj + 4.0)) * 0.014;
        float yRel = uv.y - top;
        float fall = smoothstep(0.0, 0.026, yRel) * (1.0 - smoothstep(len, len + 0.075, yRel));
        float wander = (Fbm3(float3(fj * 3.1, yRel * 6.8, seed * 18.0)) - 0.5) * 0.060;
        float taper = lerp(0.80, 0.20, saturate(yRel / max(0.001, len)));
        float xDelta = uv.x - x - wander;
        float trail = exp(-(xDelta * xDelta) / max(0.00002, width * width * taper * taper * 5.5)) * fall;
        float broken = smoothstep(0.17, 0.56, Fbm3(float3(uv * 21.0 + fj, seed * 29.0)));
        float bead = exp(-((uv.x - x - wander) * (uv.x - x - wander)) / max(0.00015, width * width * 8.0)) *
                     exp(-((uv.y - (top + len)) * (uv.y - (top + len))) / max(0.00020, width * width * 38.0));
        float nose = max(bead * 1.38, trail * smoothstep(0.55, 1.0, saturate(yRel / max(0.001, len))));
        drips = max(drips, max(trail * broken, nose));
    }
    drips *= wallMask;

    float ceilingBlebs = ceilingMask * smoothstep(0.62, 0.88, Fbm3(float3(local * 13.0, seed * 33.0))) *
        (1.0 - smoothstep(0.24, 1.05, radial));
    float pepper = smoothstep(0.78, 0.965, Fbm3(float3(local * 73.0 + seed * 3.1, seed * 61.0))) *
        smoothstep(1.14, 0.20, radial);
    float merged = max(max(impact * 0.50, satellites * 0.96), max(streaks * 0.82, max(drips, max(ceilingBlebs * 0.54, max(tinyMist * 1.12, pepper * 0.70)))));
    float edgeNoise = Fbm3(float3(local * 13.0, seed * 39.0));
    float alpha = smoothstep(0.20, 0.48, merged + (edgeNoise - 0.5) * 0.16);
    float fullBorder = min(min(uv.x, 1.0 - uv.x), min(uv.y, 1.0 - uv.y));
    float wallBorder = min(min(uv.x, 1.0 - uv.x), uv.y);
    float border = lerp(fullBorder, wallBorder, wallMask);
    alpha *= smoothstep(0.006, 0.052 + edgeNoise * 0.035, border);
    thickness = saturate(impact * 0.24 + satellites * 0.42 + tinyMist * 0.22 + pepper * 0.10 + streaks * 0.28 + drips * 0.96 + ceilingBlebs * 0.34 + alpha * 0.32);
    return alpha;
}

float CenterSeepPool(float2 uv, float3 worldPos, float seed, float age, float speed, float maxRadius, out float thickness)
{
    float grow = smoothstep(0.0, 1.0, saturate(age * speed));
    float2 p = uv * 2.0 - 1.0;
    float darkCore = 0.0;
    float soakedLayer = 0.0;
    float sourceField = 0.0;
    [unroll]
    for (int i = 0; i < 3; ++i)
    {
        float fi = (float)i;
        float enabled = i == 0 ? 1.0 : step(i == 1 ? 0.54 : 0.82, Hash21(float2(seed * 61.0 + fi, 17.0)));
        float localGrow = grow * enabled;
        float2 offset = float2(0.0, 0.0);
        if (i > 0)
        {
            float a = seed * 6.2831853 + fi * 2.43 + Hash21(float2(seed * 71.0, fi + 19.0)) * 1.25;
            float r = 0.22 + Hash21(float2(seed * 73.0 + fi, 23.0)) * 0.26;
            offset = float2(cos(a), sin(a)) * r;
        }
        float2 q = p - offset;
        float angle = seed * 6.2831853 + Hash21(float2(seed * 41.0 + fi, 19.0)) * 1.2;
        q = Rotate2(q, angle);
        q *= float2(0.94 + Hash21(float2(seed * 47.0 + fi, 23.0)) * 0.14,
                    0.92 + Hash21(float2(seed * 53.0, fi + 29.0)) * 0.16);
        float radial = length(q);
        float broad = Fbm3(float3(worldPos.xz * (3.4 + seed * 1.1 + fi * 0.35) + seed * 11.0 + fi, seed * 31.0));
        float fine = Noise3(float3(worldPos.xz * (15.0 + seed * 4.0 + fi * 1.7) + seed * 7.0, seed * 71.0 + fi));
        float cellular = Fbm3(float3(worldPos.xz * (8.4 + fi * 1.5) + seed * 19.0, seed * 89.0 + fi));
        float edgeNoise = (broad - 0.5) * (0.18 + localGrow * 0.22) + (fine - 0.5) * 0.055;
        float spotScale = i == 0 ? 1.0 : (0.54 + Hash21(float2(seed * 79.0, fi + 31.0)) * 0.20);
        float radius = lerp(0.022, maxRadius * spotScale, localGrow);
        float coreEdge = (broad - 0.5) * 0.028 + (fine - 0.5) * 0.012;
        float coreGrow = smoothstep(0.0, 1.0, saturate(age * speed * (0.58 + fi * 0.08)));
        float coreNoise = (cellular - 0.5) * (0.020 + coreGrow * 0.035) + coreEdge;
        float core = 1.0 - smoothstep(radius * (0.070 + coreGrow * 0.040) + coreNoise,
            radius * (0.18 + coreGrow * 0.145) + coreNoise, radial);
        float front = 1.0 - smoothstep(radius * 0.36 + edgeNoise,
            radius + 0.34 + edgeNoise, radial);
        float soakMask = smoothstep(0.20, 0.82, broad + (cellular - 0.5) * 0.44 + (fine - 0.5) * 0.14);
        float feather = smoothstep(0.03, 0.82, front);
        float capillary = smoothstep(0.64, 0.93, cellular + fine * 0.10) *
            (1.0 - smoothstep(radius * 0.70, radius + 0.48, radial + (broad - 0.5) * 0.12));
        darkCore = max(darkCore, core * enabled);
        sourceField = max(sourceField, front * enabled);
        soakedLayer = max(soakedLayer, max(feather * soakMask, capillary * 0.45) * enabled);
    }
    float fibers = smoothstep(0.46, 0.90, Fbm3(float3(worldPos.xz * 18.0 + seed * 13.0, seed * 79.0)));
    float dryBreak = smoothstep(0.58, 0.91, Fbm3(float3(worldPos.xz * 10.5 + seed * 23.0, seed * 101.0)));
    float mottledSoak = sourceField * (0.24 + soakedLayer * 0.36) + soakedLayer * (0.88 + fibers * 0.20);
    float sharedSoak = smoothstep(0.13, 0.64, mottledSoak);
    sharedSoak *= 1.0 - dryBreak * (0.28 + grow * 0.18);
    float shape = max(darkCore * 0.94, sharedSoak * (0.30 + fibers * 0.16));
    float border = min(min(uv.x, 1.0 - uv.x), min(uv.y, 1.0 - uv.y));
    shape *= smoothstep(0.004, 0.032, border) * smoothstep(0.02, 0.55, grow);
    thickness = saturate(darkCore * (0.78 + grow * 0.14) + sharedSoak * 0.12);
    return shape;
}

)" R"(
float FleshMaterialEligible(float materialId)
{
    return (gHorror0.x > 0.01 && materialId < 2.5) ? 1.0 : 0.0;
}

float LampDamageAtWorld(float2 worldXZ)
{
    int2 tile = (int2)floor((worldXZ - gMaze0.xy) / gMaze0.zw);
    if (tile.x < 0 || tile.y < 0 || tile.x >= (int)gMaze1.x || tile.y >= (int)gMaze1.y)
    {
        return 0.0;
    }
    return gLampDamage.Load(int3(tile, 0)).r;
}

float LampFailureMultiplier(float damage, float seed, float time)
{
    if (damage >= 0.995)
    {
        return 0.0;
    }
    float fail = smoothstep(0.06, 0.96, damage);
    float tickRate = lerp(2.2, 48.0, fail);
    float tick = floor(time * tickRate + seed * 19.0);
    float eventChance = lerp(0.98, 0.48, fail);
    float dropoutEvent = step(eventChance, Hash21(float2(seed * 37.0 + tick, seed * 11.0 + 211.0)));
    float violent = 0.22 + 1.38 * saturate(sin(time * (42.0 + seed * 91.0)) * 0.5 + 0.5);
    float pulse = lerp(1.0, violent, fail);
    pulse *= lerp(1.0, 0.035, dropoutEvent * fail);
    float brownout = lerp(1.0, 0.24, fail * fail);
    return max(0.0, pulse * brownout);
}

struct HSConstData
{
    float edges[3] : SV_TessFactor;
    float inside : SV_InsideTessFactor;
};

HSConstData HSPatchConstants(InputPatch<VSOut, 3> patch, uint patchId : SV_PrimitiveID)
{
    HSConstData o;
    float materialId = MaterialId(patch[0].material);
    float eligible = FleshMaterialEligible(materialId);
    float maxTess = lerp(10.0, 24.0, saturate(gHorror0.w / 0.22));
    float tess = eligible > 0.5 ? floor(maxTess * saturate(gHorror0.x) + 0.5) : 1.0;
    tess = max(tess, 1.0);
    o.edges[0] = tess;
    o.edges[1] = tess;
    o.edges[2] = tess;
    o.inside = tess;
    return o;
}

[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("HSPatchConstants")]
VSOut HSMain(InputPatch<VSOut, 3> patch, uint i : SV_OutputControlPointID, uint patchId : SV_PrimitiveID)
{
    return patch[i];
}

[domain("tri")]
VSOut DSMain(HSConstData input, const OutputPatch<VSOut, 3> patch, float3 bary : SV_DomainLocation)
{
    VSOut o;
    float3 worldPos = patch[0].worldPos * bary.x + patch[1].worldPos * bary.y + patch[2].worldPos * bary.z;
    float3 normal = normalize(patch[0].normal * bary.x + patch[1].normal * bary.y + patch[2].normal * bary.z);
    float3 tangent = normalize(patch[0].tangent * bary.x + patch[1].tangent * bary.y + patch[2].tangent * bary.z);
    float2 rawUv = patch[0].uv * bary.x + patch[1].uv * bary.y + patch[2].uv * bary.z;
    float material = patch[0].material;
    float materialId = MaterialId(material);
    float eligible = FleshMaterialEligible(materialId);
    if (eligible > 0.5)
    {
        float2 fleshUv = rawUv * 0.72 + float2(Hash21(floor(worldPos.xz * 0.27)), Hash21(floor(worldPos.zx * 0.31))) * 0.23;
        float height = gNormalHeight.SampleLevel(gSampler, float3(fleshUv, 15.0), 0).a;
        float lowHeight = gNormalHeight.SampleLevel(gSampler, float3(fleshUv, 15.0), 4).a;
        float foldRelief = (height - lowHeight) * 2.15 + (lowHeight - 0.50) * 0.42;
        float wallEdgeFade = 1.0;
        if (abs(normal.y) < 0.35)
        {
            wallEdgeFade = smoothstep(0.035, 0.24, worldPos.y) * smoothstep(0.035, 0.24, gMaze1.z - worldPos.y);
        }
        float displacement = foldRelief * gHorror0.w * (1.35 + gHorror0.x * 0.95) * wallEdgeFade;
        displacement = clamp(displacement, -0.13, 0.22);
        worldPos += normal * displacement;
    }
    o.worldPos = worldPos;
    o.normal = normal;
    o.tangent = tangent;
    o.uv = rawUv;
    o.material = material;
    o.pos = mul(float4(worldPos, 1.0), gViewProj);
    return o;
}

float FixturePower(float3 worldPos, float time)
{
    float2 stride = gMaze0.zw;
    float2 lampOrigin = gMaze0.xy + gMaze0.zw * 0.5;
    float2 cell = floor((worldPos.xz - lampOrigin) / stride + 0.5);
    float brokenZone = step(1.0 - gLighting1.w, Hash21(floor(cell / 3.0)));
    float h = Hash21(cell);
    float variation = lerp(0.86, 1.14, Hash21(cell + 53.0));
    float isOn = step(1.0 - gLighting1.y, h);
    float flickerFixture = step(1.0 - gLighting1.z, Hash21(cell + 17.0));
    float tick = floor(time * (1.3 + Hash21(cell + 37.0) * 2.5));
    float event = step(0.86, Hash21(cell + tick + 71.0));
    float flutter = 0.18 + 0.82 * saturate(sin(time * (41.0 + h * 50.0)) * 0.5 + 0.5);
    float buzz = 0.98 + 0.02 * sin(time * (55.0 + h * 80.0));
    float basePower = (1.0 - brokenZone) * isOn * lerp(1.0, flutter, flickerFixture * event) * buzz * variation;
    return basePower * LampFailureMultiplier(LampDamageAtWorld(worldPos.xz), h, time);
}

float LampVisualPower(float material, float3 worldPos, float time)
{
    float materialSeed = frac(material);
    float visualVariation = lerp(0.96, 1.06, frac(materialSeed * 23.71 + 0.31));
    return FixturePower(worldPos, time) * visualVariation;
}

float2 MazeTile(float2 worldXZ)
{
    return floor((worldXZ - gMaze0.xy) / gMaze0.zw);
}

float MazeOpenAt(int2 tile)
{
    if (tile.x < 0 || tile.y < 0 || tile.x >= (int)gMaze1.x || tile.y >= (int)gMaze1.y)
    {
        return 0.0;
    }
    return gMazeOpen.Load(int3(tile, 0)).r;
}

float LampRayClear(float2 startXZ, float2 endXZ);

float NearestClosedCellDistance(float2 cell)
{
    int2 baseTile = (int2)floor(cell);
    float best = 4.0;

    [loop]
    for (int yy = -1; yy <= 1; ++yy)
    {
        [loop]
        for (int xx = -1; xx <= 1; ++xx)
        {
            int2 tile = baseTile + int2(xx, yy);
            if (MazeOpenAt(tile) < 0.5)
            {
                float2 lo = (float2)tile;
                float2 hi = lo + 1.0;
                float2 outside = max(max(lo - cell, cell - hi), 0.0);
                best = min(best, length(outside));
            }
        }
    }

    return best;
}

float2 LampAreaSample(int index)
{
    if (index == 0) return float2(0.0, 0.0);
    if (index == 1) return float2(0.66, 0.0);
    if (index == 2) return float2(-0.66, 0.0);
    if (index == 3) return float2(0.0, 0.58);
    if (index == 4) return float2(0.0, -0.58);
    if (index == 5) return float2(0.48, 0.42);
    if (index == 6) return float2(-0.48, 0.42);
    if (index == 7) return float2(0.48, -0.42);
    return float2(-0.48, -0.42);
}

float LampAreaRayVisibility(float2 startXZ, float2 lampXZ, float2 dir, float2 perp, float tileSize, float distFade)
{
    float sourceWidth = tileSize * lerp(0.18, 0.58, distFade);
    float sourceLength = tileSize * lerp(0.10, 0.30, distFade);
    float receiverWidth = tileSize * lerp(0.040, 0.210, distFade);
    float vis = 0.0;
    float weightSum = 0.0;

    [loop]
    for (int i = 0; i < 9; ++i)
    {
        float2 s = LampAreaSample(i);
        float diagonal = step(0.82, abs(s.x) + abs(s.y));
        float center = 1.0 - step(0.01, dot(s, s));
        float weight = lerp(1.0, 0.72, diagonal) + center * 0.38;
        float2 source = lampXZ + perp * (s.x * sourceWidth) + dir * (s.y * sourceLength);
        float2 receiver = startXZ - perp * (s.x * receiverWidth * 0.42) - dir * (s.y * receiverWidth * 0.18);
        vis += LampRayClear(receiver, source) * weight;
        weightSum += weight;
    }

    return vis / max(weightSum, 0.001);
}

float LampVisibility(float2 worldXZ, float3 worldN, float2 lampXZ)
{
    float tileSize = gMaze1.w;
    float sourceTileSize = tileSize / 3.0;
    float2 startXZ = worldXZ + worldN.xz * tileSize * 0.16;
    float2 delta = lampXZ - startXZ;
    float dist = max(length(delta), 0.001);
    float2 dir = delta / dist;
    float2 perp = float2(-dir.y, dir.x);
    float distFade = saturate(dist / max(tileSize * 4.5, 0.001));
    float2 deltaCells = abs(delta / max(tileSize, 0.001));
    float hallFill = max(exp2(-deltaCells.x * deltaCells.x * 1.95),
                         exp2(-deltaCells.y * deltaCells.y * 1.95));
    float localFill = exp2(-dot(deltaCells, deltaCells) * 0.36);
    float areaFill = saturate(max(hallFill * 0.95, localFill * 0.58));

    float2 startCell = (startXZ - gMaze0.xy) / gMaze0.zw;
    float clearance = NearestClosedCellDistance(startCell);
    float cornerFeather = smoothstep(0.045, 0.44 + distFade * 0.20, clearance);

    float rayArea = LampAreaRayVisibility(startXZ, lampXZ, dir, perp, sourceTileSize, distFade);
    float raySoft = smoothstep(-0.04, 1.04, rayArea);
    raySoft = saturate(raySoft + rayArea * (1.0 - rayArea) * (0.18 + distFade * 0.12));
    float occludedBounce = 0.10 + cornerFeather * 0.12 + distFade * 0.045;
    float occlusion = lerp(occludedBounce, 1.0, raySoft);
    return saturate(areaFill * occlusion * (0.58 + cornerFeather * 0.42));
}

float LampRayClear(float2 startXZ, float2 endXZ)
{
    float2 startCell = (startXZ - gMaze0.xy) / gMaze0.zw;
    float2 endCell = (endXZ - gMaze0.xy) / gMaze0.zw;
    int2 tile = (int2)floor(startCell);
    int2 endTile = (int2)floor(endCell);
    if (MazeOpenAt(tile) < 0.5 || MazeOpenAt(endTile) < 0.5)
    {
        return 0.0;
    }

    float2 ray = endCell - startCell;
    float2 safeRay = float2(
        abs(ray.x) < 0.0001 ? (ray.x < 0.0 ? -0.0001 : 0.0001) : ray.x,
        abs(ray.y) < 0.0001 ? (ray.y < 0.0 ? -0.0001 : 0.0001) : ray.y);
    int2 stepTile = int2(ray.x >= 0.0 ? 1 : -1, ray.y >= 0.0 ? 1 : -1);
    float2 absRay = max(abs(ray), float2(0.0001, 0.0001));
    float2 nextBoundary = float2(
        stepTile.x > 0 ? (float)tile.x + 1.0 : (float)tile.x,
        stepTile.y > 0 ? (float)tile.y + 1.0 : (float)tile.y);
    float2 tMax = abs((nextBoundary - startCell) / safeRay);
    float2 tDelta = 1.0 / absRay;

    [loop]
    for (int i = 0; i < 96; ++i)
    {
        if (tile.x == endTile.x && tile.y == endTile.y)
        {
            return 1.0;
        }
        if (tMax.x < tMax.y)
        {
            tile.x += stepTile.x;
            tMax.x += tDelta.x;
        }
        else
        {
            tile.y += stepTile.y;
            tMax.y += tDelta.y;
        }
        if (MazeOpenAt(tile) < 0.5)
        {
            return 0.0;
        }
    }
    return 0.0;
}

)" R"(
float LocalLampLight(float3 worldPos, float3 worldN, float time)
{
    float2 stride = gMaze0.zw;
    float spacing = gMaze1.w;
    float2 lampOrigin = gMaze0.xy + gMaze0.zw * 0.5;
    float2 baseCell = floor((worldPos.xz - lampOrigin) / stride + 0.5);
    float light = 0.0;

    [loop]
    for (int yy = -2; yy <= 2; ++yy)
    {
        [loop]
        for (int xx = -2; xx <= 2; ++xx)
        {
            float2 cell = baseCell + float2(xx, yy);
            float2 lampXZ = lampOrigin + cell * stride;
            if (MazeOpenAt((int2)MazeTile(lampXZ)) < 0.5)
            {
                continue;
            }
            float3 lampPos = float3(lampXZ.x, gMaze1.z - 0.09, lampXZ.y);
            float3 L = lampPos - worldPos;
            float d2 = dot(L, L);
            float distXZ = length(lampPos.xz - worldPos.xz);
            float reach = spacing * 1.18 + gMaze1.w * 0.75;
            float roomFootprint = 1.0 - smoothstep(reach * 0.48, reach, distXZ);
            float power = FixturePower(lampPos, time);
            if (roomFootprint <= 0.002 || power <= 0.002)
            {
                continue;
            }
            float visibility = LampVisibility(worldPos.xz, worldN, lampPos.xz);
            float3 Ln = normalize(L);
            float diffuse = saturate(dot(worldN, Ln) * 0.65 + 0.35);
            float falloff = visibility * roomFootprint / (1.0 + d2 * 0.035);
            light += power * falloff * diffuse;
        }
    }

    return light;
}

float CornerAO(float3 worldPos, float3 normal)
{
    float radius = max(0.02, gAO0.y);
    float floorContact = 1.0 - smoothstep(0.0, radius * 1.20, worldPos.y);
    float ceilingContact = 1.0 - smoothstep(0.0, radius * 1.20, gMaze1.z - worldPos.y);
    float verticalContact = max(floorContact, ceilingContact);
    float wallMask = saturate(1.0 - abs(normal.y));
    float ao = wallMask * verticalContact * gAO0.z;
    return saturate(ao * gAO0.x);
}

float ShadowVisibility(float3 worldPos, float3 normal)
{
    if (gShadow0.w <= 0.001)
    {
        return 1.0;
    }

    float3 samplePos = worldPos + normalize(normal) * 0.018;
    float4 lightPos = mul(float4(samplePos, 1.0), gLightViewProj);
    if (lightPos.w <= 0.0001)
    {
        return 1.0;
    }

    float3 ndc = lightPos.xyz / lightPos.w;
    float2 uv = float2(ndc.x * 0.5 + 0.5, -ndc.y * 0.5 + 0.5);
    if (uv.x <= 0.001 || uv.x >= 0.999 || uv.y <= 0.001 || uv.y >= 0.999 || ndc.z <= 0.0 || ndc.z >= 1.0)
    {
        return 1.0;
    }

    float3 lightPosWorld = gShadow0.xyz;
    float3 toLight = normalize(lightPosWorld - samplePos);
    float slope = 1.0 - saturate(dot(normalize(normal), toLight));
    float compareDepth = ndc.z - max(gShadow1.w * (1.0 + slope * 2.4), 0.00045);
    float texel = max(gShadow2.x, 0.0001);
    float receiver = saturate(length(worldPos - lightPosWorld) / max(gShadow2.y, 0.01));
    float penumbra = pow(receiver, 0.72);
    float filterRadius = lerp(0.85, 5.20, penumbra);
    float2 stableCell = floor(worldPos.xz * 3.1 + worldPos.y * 1.7);
    float2 jitter = (float2(Hash21(stableCell + 13.0), Hash21(stableCell + 71.0)) - 0.5) * texel * lerp(0.16, 0.42, penumbra);
    float visible = 0.0;
    float weightSum = 0.0;

    [loop]
    for (int y = -2; y <= 2; ++y)
    {
        [loop]
        for (int x = -2; x <= 2; ++x)
        {
            float wx = 3.0 - abs((float)x);
            float wy = 3.0 - abs((float)y);
            float weight = wx * wy;
            float2 offset = float2(x, y) * texel * filterRadius + jitter;
            visible += gShadowMap.SampleCmpLevelZero(gShadowSampler, uv + offset, compareDepth) * weight;
            weightSum += weight;
        }
    }
    visible /= max(weightSum, 0.001);
    visible = saturate(visible + (1.0 - visible) * penumbra * 0.035);
    float strength = gShadow0.w * lerp(1.0, 0.74, penumbra);
    return lerp(1.0 - strength, 1.0, visible);
}

float FlashlightAmount(float3 worldPos, float3 worldN)
{
    float3 lightPos = gShadow0.xyz;
    float3 lightDir = normalize(gShadow1.xyz);
    float3 fromLight = worldPos - lightPos;
    float dist = length(fromLight);
    if (dist <= 0.001)
    {
        return 0.0;
    }

    float3 ray = fromLight / dist;
    float3 toLight = -ray;
    float cone = smoothstep(gShadow2.z, gShadow2.w, dot(ray, lightDir));
    float attenuation = 1.0 / (1.0 + gLighting0.y * dist * dist);
    float diffuse = saturate(dot(worldN, toLight));
    float shadow = ShadowVisibility(worldPos, worldN);
    float4 lightClip = mul(float4(worldPos, 1.0), gLightViewProj);
    float2 lightUv = lightClip.xy / max(lightClip.w, 0.0001) * 0.5 + 0.5;
    float glass = gFlashlightPattern.Sample(gSampler, saturate(lightUv)).r;
    float2 centered = lightUv * 2.0 - 1.0;
    float lensFalloff = smoothstep(1.18, 0.18, dot(centered, centered));
    float pattern = lerp(0.52, 1.22, glass) * (0.72 + lensFalloff * 0.28);
    return cone * attenuation * gLighting0.x * (0.16 + diffuse * 1.12) * shadow * pattern;
}

float SparkLightOne(float3 worldPos, float3 worldN, float4 lightData)
{
    if (lightData.w <= 0.001)
    {
        return 0.0;
    }
    float3 L = lightData.xyz - worldPos;
    float d = length(L);
    if (d <= 0.001 || d > 5.0)
    {
        return 0.0;
    }
    float visibility = LampRayClear(worldPos.xz + worldN.xz * 0.05, lightData.xz);
    float3 Ln = L / d;
    float diffuse = saturate(dot(worldN, Ln) * 0.72 + 0.28);
    float falloff = pow(saturate(1.0 - d / 5.0), 2.1) / (1.0 + d * d * 0.42);
    return lightData.w * falloff * diffuse * visibility;
}

float SparkLight(float3 worldPos, float3 worldN)
{
    return SparkLightOne(worldPos, worldN, gSparkLight0) + SparkLightOne(worldPos, worldN, gSparkLight1);
}

float3 ExitSignLight(float3 worldPos, float3 worldN)
{
    float strength = gExitLight0.w * (1.0 - saturate(gTransition0.z));
    float3 result = float3(0.0, 0.0, 0.0);
    if (strength > 0.001)
    {
        float3 L = gExitLight0.xyz - worldPos;
        float d = length(L);
        if (d > 0.001 && d <= 3.65)
        {
            float visibility = LampRayClear(worldPos.xz + worldN.xz * 0.035, gExitLight0.xz);
            float3 Ln = L / d;
            float diffuse = saturate(dot(worldN, Ln) * 0.72 + 0.28);
            float falloff = pow(saturate(1.0 - d / 3.65), 2.0) / (1.0 + d * d * 0.62);
            result += float3(0.045, 0.92, 0.34) * strength * falloff * diffuse * visibility;
        }
    }

    float doorStrength = gExitLight2.w * (1.0 - saturate(gTransition0.z));
    float doorOpen = saturate(gExitLight1.w);
    if (doorStrength > 0.001 && doorOpen > 0.001)
    {
        float3 L = gExitLight2.xyz - worldPos;
        float d = length(L);
        float reach = 14.5;
        if (d > 0.001 && d <= reach)
        {
            float3 Ln = L / d;
            float diffuse = saturate(dot(worldN, Ln) * 0.56 + 0.52);
            float3 doorDir = normalize(gExitLight1.xyz + float3(0.0001, 0.0, 0.0001));
            float3 fromSource = worldPos - gExitLight2.xyz;
            float axial = dot(fromSource, doorDir);
            float side = length(fromSource - doorDir * axial);
            float cone = smoothstep(-0.08, 0.74, axial) * smoothstep(2.65 + axial * 0.46, 0.16, side);
            float floorWash = smoothstep(0.12, 3.40, axial) * smoothstep(2.35 + axial * 0.40, 0.14, side) *
                smoothstep(1.18, 0.05, worldPos.y);
            float spill = saturate(cone * 1.75 + floorWash * 1.30 + 0.04);
            float falloff = pow(saturate(1.0 - d / reach), 0.68) / (1.0 + d * d * 0.030);
            float3 warmDaylight = float3(0.94, 0.97, 1.0);
            float angelicLift = smoothstep(0.16, 2.80, axial);
            result += warmDaylight * doorStrength * falloff * diffuse * spill;
            result += warmDaylight * doorStrength * falloff * angelicLift * spill * 0.10;
        }
    }
    return result;
}

float3 ApplyPost(float3 color)
{
    color = max(color, 0.0);
    color = 1.0 - exp(-color * gPost0.x);
    color = pow(saturate(color), 1.0 / max(gPost0.y, 0.1));
    float danger = saturate(gPost0.z);
    float death = saturate(gPost0.w);
    float time = gCameraPosTime.w;
    float blinkRate = 5.0 + danger * 14.0 + death * 36.0;
    float blinkPhase = frac(time * blinkRate + sin(time * 17.13) * 0.17 + sin(time * 43.7) * 0.07);
    float blinkGate = step(0.78 - danger * 0.28 - death * 0.42, blinkPhase);
    float blackout = saturate(blinkGate * (danger * danger * 0.30 + death * 0.62));
    color *= 1.0 - blackout;
    color = lerp(color, float3(0.0, 0.0, 0.0), smoothstep(0.76, 1.0, death));
    color = lerp(color, float3(0.0, 0.0, 0.0), saturate(gTransition0.x));
    return color;
}

float DistanceFogBlock(float dist, float scale)
{
    float fog = saturate((dist - gFog0.x) / max(0.01, gFog0.y - gFog0.x));
    fog = 1.0 - exp(-fog * fog * 3.2);
    return saturate(fog * gFog0.z * scale);
}

float MonsterVicinityFog(float3 worldPos)
{
    float radius = max(gMonsterFog0.z, 0.001);
    float nearMonster = saturate(1.0 - length(worldPos.xz - gMonsterFog0.xy) / radius);
    return saturate(nearMonster * gMonsterFog0.w);
}

float SceneFogBlock(float dist, float3 worldPos, float scale)
{
    float distanceFog = DistanceFogBlock(dist, scale);
    float monsterFog = MonsterVicinityFog(worldPos);
    return saturate(distanceFog + monsterFog * (1.0 - distanceFog * 0.35));
}

void SelectBloodRevealSlot(float4 slot, float active, float2 worldXZ, float time, inout float bestMask, inout float bestAge)
{
    float enabled = saturate(active) * step(0.001, slot.w);
    if (enabled <= 0.001)
    {
        return;
    }
    float dist = length(worldXZ - slot.xy);
    float mask = 1.0 - smoothstep(slot.w * 0.72, slot.w, dist);
    float age = max(0.0, time - slot.z);
    float score = mask * smoothstep(0.0, 0.22, age) * enabled;
    if (score > bestMask)
    {
        bestMask = score;
        bestAge = age;
    }
}

)" R"(
float4 PSMain(VSOut input) : SV_TARGET
{
    float materialId = MaterialId(input.material);
    float3 cam = gCameraPosTime.xyz;
    float time = gCameraPosTime.w;
    float3 V = normalize(cam - input.worldPos);
    float3 N = normalize(input.normal);
    float3 T = normalize(input.tangent);
    float3 B = normalize(cross(N, T));
    float2 rawUv = input.uv;
    float2 uv = frac(rawUv);

    if ((materialId > 13.5 && materialId < 14.5) || (materialId > 24.5 && materialId < 25.5))
    {
        float waterLiquid = step(24.5, materialId);
        float rawSeed = frac(input.material);
        float wallLeakSurface = step(0.96, rawSeed);
        float centerSeepSurface = step(0.43, rawSeed) * (1.0 - step(0.95, rawSeed));
        float menuCenterSeepSurface = centerSeepSurface * step(0.62, rawSeed) * (1.0 - step(0.70, rawSeed));
        float seed = rawSeed;
        if (wallLeakSurface > 0.5)
        {
            seed = saturate((rawSeed - 0.965) / 0.025);
        }
        else if (centerSeepSurface > 0.5)
        {
            seed = saturate((rawSeed - 0.43) / 0.52);
        }
        else if (rawSeed >= 0.02 && rawSeed <= 0.42)
        {
            seed = saturate((rawSeed - 0.02) / 0.40);
        }
        float wet = lerp(saturate(gHorror0.y), 1.0, waterLiquid);
        float2 bloodUv = lerp(uv, rawUv, waterLiquid);
        float2 bloodUvMeters = BloodUvWorldMeters(rawUv, input.worldPos);
        float drips = 0.0;
        float thickness = 0.0;
        float wallMask = saturate(1.0 - abs(N.y));
        float floorMask = smoothstep(0.45, 0.82, N.y);
        float ceilingMask = smoothstep(0.45, 0.82, -N.y);
        float animMask = 0.0;
        float leakAge = 0.0;
        if (waterLiquid > 0.5)
        {
            float waterDebugActive = step(1.0, gTransition0.w);
            float waterDebugPhase = frac(gTransition0.w);
            if (waterDebugActive > 0.5)
            {
                animMask = 1.0;
                leakAge = waterDebugPhase * 54.0;
            }
            else
            {
                SelectBloodRevealSlot(float4(gBlood0.xy, gBlood0.z, gBlood1.y), gBlood0.w, input.worldPos.xz, time, animMask, leakAge);
                SelectBloodRevealSlot(gBlood2, 1.0, input.worldPos.xz, time, animMask, leakAge);
                SelectBloodRevealSlot(gBlood3, 1.0, input.worldPos.xz, time, animMask, leakAge);
                SelectBloodRevealSlot(gBlood4, 1.0, input.worldPos.xz, time, animMask, leakAge);
                SelectBloodRevealSlot(gBlood5, 1.0, input.worldPos.xz, time, animMask, leakAge);
                SelectBloodRevealSlot(gBlood6, 1.0, input.worldPos.xz, time, animMask, leakAge);
                SelectBloodRevealSlot(gBlood7, 1.0, input.worldPos.xz, time, animMask, leakAge);
                SelectBloodRevealSlot(gBlood8, 1.0, input.worldPos.xz, time, animMask, leakAge);
            }
        }
        else
        {
            SelectBloodRevealSlot(float4(gBlood0.xy, gBlood0.z, gBlood1.y), gBlood0.w, input.worldPos.xz, time, animMask, leakAge);
            SelectBloodRevealSlot(gBlood2, 1.0, input.worldPos.xz, time, animMask, leakAge);
            SelectBloodRevealSlot(gBlood3, 1.0, input.worldPos.xz, time, animMask, leakAge);
            SelectBloodRevealSlot(gBlood4, 1.0, input.worldPos.xz, time, animMask, leakAge);
            SelectBloodRevealSlot(gBlood5, 1.0, input.worldPos.xz, time, animMask, leakAge);
            SelectBloodRevealSlot(gBlood6, 1.0, input.worldPos.xz, time, animMask, leakAge);
            SelectBloodRevealSlot(gBlood7, 1.0, input.worldPos.xz, time, animMask, leakAge);
            SelectBloodRevealSlot(gBlood8, 1.0, input.worldPos.xz, time, animMask, leakAge);
        }
        float alpha = 0.0;
        float bloodQuality = clamp(gBlood1.w, 0.25, 1.0);
        float requestedStreams = clamp(round(gBlood1.x), 4.0, 32.0);
        float streamBudget = saturate(bloodQuality * 1.15);
        float streamCount = clamp(round(lerp(8.0, requestedStreams, streamBudget)), 6.0, 28.0);
        float floorStreamCount = wallLeakSurface > 0.5
            ? streamCount
            : clamp(round(streamCount * lerp(0.36, 0.74, bloodQuality)), 2.0, streamCount);
        float ceilingStreamCount = wallLeakSurface > 0.5
            ? streamCount
            : clamp(round(streamCount * lerp(0.32, 0.70, bloodQuality)), 2.0, streamCount);
        float streamWidthScale = max(0.10, gBlood1.z) * lerp(1.55, 1.0, bloodQuality);
        static const bool highBloodDetail = BRM_ENABLE_HIGH_BLOOD != 0;

)" R"(

        if (wallMask > 0.45)
        {
            float u = bloodUv.x;
            float y = bloodUv.y;
            float wallLeakRun = wallLeakSurface;
            float wallStreamWidthScale = streamWidthScale * 1.16;
            float streams = 0.0;
            float streamAccum = 0.0;
            float sdfWet = 0.0;
            float sdfCore = 0.0;
            float diffuseSoak = 0.0;
            float beads = 0.0;
            float topSource = 0.0;
            [loop]
            for (int i = 0; i < 32; ++i)
            {
                float fi = (float)i;
                if (fi >= streamCount) break;
                float r0 = Hash21(float2(seed * 47.0 + fi, 3.0));
                float r1 = Hash21(float2(seed * 31.0, fi + 5.0));
                float r2 = Hash21(float2(fi + 9.0, seed * 71.0));
                float clusterCount = 2.0 + floor(Hash21(float2(seed * 83.0, 19.0)) * 3.0);
                float clusterId = floor(fmod(fi + floor(seed * 31.0), clusterCount));
                float uniformCenter = 0.055 + ((fi + 0.20 + r0 * 0.60) / max(1.0, streamCount)) * 0.89;
                float clusterCenter = 0.08 + Hash21(float2(seed * 89.0 + clusterId * 5.7, 13.0)) * 0.84;
                float clusterSpread = 0.025 + Hash21(float2(seed * 97.0 + clusterId, 29.0)) * 0.080;
                float clusteredCenter = clusterCenter + (Hash21(float2(seed * 131.0 + fi, 37.0)) - 0.5) * clusterSpread;
                float center = clamp(lerp(uniformCenter, clusteredCenter, 0.58 + Hash21(float2(seed * 151.0 + fi, 43.0)) * 0.34), 0.045, 0.955);
                float densityBase = Hash21(float2(center * 17.0 + seed * 11.0, clusterId * 3.1 + 5.0));
                if (highBloodDetail)
                {
                    densityBase = smoothstep(0.24, 0.78, Fbm3(float3(center * 4.2, seed * 18.0, clusterId * 2.3)));
                }
                float densityBand = densityBase;
                float streamStrength = lerp(0.24, 1.18, densityBand) * lerp(0.58, 1.12, Hash21(float2(seed * 173.0 + fi, 47.0)));
                float streamDelay = r0 * 4.6 + fi * (0.05 + r2 * 0.18) +
                    Hash21(float2(seed * 251.0 + fi, 157.0)) * 1.7;
                streamDelay *= lerp(1.0, 0.62, waterLiquid);
                float streamAge = max(0.0, leakAge - streamDelay);
                float speedPhase = streamAge * (0.62 + r2 * 0.54) + seed * 17.0 + fi * 2.13;
                float flowAge = max(0.0, streamAge * (0.90 + r1 * 0.18) +
                    sin(speedPhase) * (0.10 + r0 * 0.08) +
                    sin(speedPhase * 2.17 + r2 * 6.0) * 0.032);
                flowAge *= lerp(1.0, 1.34, waterLiquid);
                float sourceReady = smoothstep(0.10 + r2 * 0.55, 0.90 + r1 * 1.05, flowAge);
                float streamGrow = smoothstep(0.0, 1.0,
                    saturate((flowAge - (0.20 + r2 * 0.75) * lerp(1.0, 0.68, waterLiquid)) *
                    (0.052 + r1 * 0.055) * lerp(1.0, 1.24, waterLiquid)));
                float reachRoll = Hash21(float2(seed * 197.0 + fi, 109.0));
                float partialReach = 0.30 + Hash21(float2(seed * 211.0 + fi, 113.0)) * 0.52;
                float fullReach = 1.02 + Hash21(float2(seed * 223.0 + fi, 127.0)) * 0.18;
                float stopLen = lerp(partialReach, fullReach, step(0.48 + r1 * 0.22, reachRoll));
                float initialLen = 0.006 + r0 * 0.036;
                float len = saturate(lerp(initialLen, stopLen, streamGrow));
                float width = (0.0026 + r2 * 0.0054) * wallStreamWidthScale;
                float lean = (r1 - 0.5) * y * (0.0025 + r2 * 0.0035);
                float microWobble = (Hash21(float2(fi * 5.1 + seed * 23.0, y * 3.0)) - 0.5) * (0.0007 + r2 * 0.0010);
                if (highBloodDetail)
                {
                    microWobble = (Fbm3(float3(y * 2.2, fi * 1.7, seed * 23.0)) - 0.5) * (0.0011 + r2 * 0.0018);
                }
                float wander = lean + microWobble;
                float du = u - center - wander;
                float trailGate = 1.0 - smoothstep(len, len + 0.060, y);
                float permanentTrace = smoothstep(0.02, 0.28, streamGrow);
                float flowEnd = 16.0 + r0 * 24.0 + r2 * 18.0;
                float flowFade = 7.0 + r1 * 12.0;
                float activeFlow = 1.0 - smoothstep(flowEnd, flowEnd + flowFade, flowAge);
                float wetPulse = 0.91 + 0.09 * activeFlow * sin(y * (24.0 + r1 * 18.0) - time * (0.18 + r2 * 0.16) + seed * 37.0);
                float widthNoise = 0.86 + 0.24 * Hash21(float2(fi * 13.7 + seed * 29.0, floor(y * 15.0)));
                if (highBloodDetail)
                {
                    widthNoise = 0.76 + 0.38 * Fbm3(float3(fi * 2.2 + seed * 19.0, y * 8.0, seed * 41.0));
                }
                float gravityBulge = smoothstep(0.16, 0.92, y) * (0.08 + r1 * 0.18) +
                    smoothstep(0.78, 1.0, y) * (0.16 + r2 * 0.18);
                float widthTaper = width * (0.78 + gravityBulge) * widthNoise;
                float strandBreaks = lerp(0.70, 1.0, Hash21(float2(fi * 9.0 + seed * 43.0, floor(y * 22.0))));
                if (highBloodDetail)
                {
                    strandBreaks = smoothstep(0.18, 0.50, Fbm3(float3(u * 35.0 + fi, y * 10.0, seed * 43.0)) + 0.18);
                }
                float stream = exp(-(du * du) / max(0.000008, widthTaper * widthTaper)) * trailGate * permanentTrace * wetPulse * strandBreaks * streamStrength;
                float bleedHalo = exp(-(du * du) / max(0.000018, widthTaper * widthTaper * 18.0)) *
                    trailGate * permanentTrace * streamStrength * 0.46;
                float sdfWidth = widthTaper * (5.8 + bloodQuality * 3.2);
                float field = saturate(1.0 - abs(du) / max(0.0008, sdfWidth)) *
                    trailGate * permanentTrace * streamStrength;
                sdfWet += field;
                sdfCore += field * field;
                float soakDelay = (3.2 + r2 * 10.0 + y * (7.0 + r1 * 12.0)) * lerp(1.0, 0.74, waterLiquid);
                float soakAge = max(0.0, flowAge - soakDelay);
                float soakGrow = smoothstep(0.0, 1.0, saturate(soakAge * (0.070 + r0 * 0.058) * lerp(1.0, 1.18, waterLiquid)));
                float soakNoise = 0.72 + 0.28 * Hash21(float2(fi * 17.0 + seed * 61.0, floor(y * 16.0) + floor(u * 9.0)));
                if (highBloodDetail)
                {
                    soakNoise = 0.52 + 0.48 * Fbm3(float3(u * 12.0 + fi * 0.37, y * 8.0, seed * 67.0));
                }
                float soakWidth = widthTaper * (8.0 + bloodQuality * 13.0) * (0.62 + soakGrow * 1.22);
                float soakGate = 1.0 - smoothstep(len - 0.035, len + 0.20, y);
                float localSoak = saturate(1.0 - abs(du) / max(0.0012, soakWidth)) *
                    soakGate * soakGrow * streamStrength * soakNoise;
                diffuseSoak += localSoak;
                float headY = len - 0.018;
                float bead = 0.0;
                if (highBloodDetail)
                {
                    bead = exp(-(du * du) / max(0.000018, width * width * 3.0)) *
                        exp(-((y - headY) * (y - headY)) / max(0.00005, width * width * 22.0)) * streamStrength;
                }
                float sourceWidth = width * (2.3 + r1 * 1.8) + 0.0032 * wallStreamWidthScale;
                float sourceDepth = 0.018 + r2 * 0.040;
                float sourceY = 0.010 + Hash21(float2(seed * 269.0 + fi, 167.0)) * 0.020;
                float sourceNoise = 0.72 + 0.28 * Hash21(float2(floor(u * 18.0) + fi, seed * 37.0));
                if (highBloodDetail)
                {
                    sourceNoise = 0.45 + 0.55 * Fbm3(float3(u * 12.0, seed * 37.0 + fi, y * 5.0));
                }
                float sourceEdge = (Fbm3(float3(u * 18.0 + fi, y * 22.0, seed * 53.0)) - 0.5) *
                    (0.18 + sourceReady * 0.16);
                float2 sourceQ = float2((u - center) / max(0.0012, sourceWidth),
                    (y - sourceY) / max(0.0012, sourceDepth * (0.62 + r0 * 0.35)));
                float sourceBlob = 1.0 - smoothstep(0.56, 1.12, dot(sourceQ, sourceQ) + sourceEdge);
                float sourceFeeder = exp(-((u - center) * (u - center)) / max(0.000035, sourceWidth * sourceWidth * 0.42)) *
                    (1.0 - smoothstep(sourceDepth * (0.80 + r2 * 0.40),
                        sourceDepth * (1.55 + r1 * 0.70), y)) *
                    smoothstep(0.45, 1.0, sourceReady) * 0.38;
                float source = max(sourceBlob, sourceFeeder) * sourceNoise * sourceReady * streamStrength;
                streams = max(streams, stream);
                streamAccum += saturate(stream + bleedHalo);
                beads = max(beads, bead * smoothstep(0.08, 0.98, streamGrow) * (1.0 - smoothstep(0.96, 1.0, streamGrow) * 0.30));
                topSource = max(topSource, source);
            }
            float wallFullHeightGate = smoothstep(0.0, 0.024, y) * (1.0 - smoothstep(0.994, 1.0, y));
            float wallSheet = smoothstep(0.16, 0.86, streamAccum) *
                wallFullHeightGate *
                smoothstep(8.0, 18.0, leakAge) * 0.46;
            float mergedField = smoothstep(0.70, 1.68, sdfWet) * wallFullHeightGate;
            float mergedCore = smoothstep(0.36, 1.20, sdfCore) * wallFullHeightGate;
            float wallEdgeNoise = (Fbm3(float3(u * 4.1, seed * 9.0, y * 2.3)) - 0.5) * 0.075;
            float wallTopFade = smoothstep(-0.004 + wallEdgeNoise * 0.10, 0.036 + wallEdgeNoise * 0.10, y);
            float wallVerticalFade = wallTopFade * (1.0 - smoothstep(0.992, 1.0, y));
            float wallCardFade = smoothstep(0.018 + wallEdgeNoise, 0.115 + wallEdgeNoise, u) *
                (1.0 - smoothstep(0.885 - wallEdgeNoise, 0.982 - wallEdgeNoise, u)) *
                wallVerticalFade;
            float floodNoise = Fbm3(input.worldPos * float3(4.7, 2.8, 4.7) + seed * 11.0);
            float floodFine = Noise3(input.worldPos * float3(18.0, 8.0, 18.0) + seed * 29.0);
            float organicFlood = smoothstep(0.20, 0.72, floodNoise + (floodFine - 0.5) * 0.18);
            float streamFedSoak = smoothstep(0.34, 1.30, diffuseSoak) * wallCardFade *
                organicFlood * (0.40 + smoothstep(0.74, 1.80, diffuseSoak) * 0.22);
            float wallFlood = saturate(streamFedSoak);
            float ceilingContactFeed = smoothstep(0.18, 0.52, topSource + streams * 0.18 + sdfWet * 0.10);
            float ceilingContact = (1.0 - smoothstep(0.0, 0.038, y)) *
                smoothstep(1.8, 7.5, leakAge) *
                ceilingContactFeed *
                smoothstep(0.015 + wallEdgeNoise, 0.090 + wallEdgeNoise, u) *
                (1.0 - smoothstep(0.910 - wallEdgeNoise, 0.990 - wallEdgeNoise, u));
            wallFlood = saturate(wallFlood + ceilingContact * 0.22);
            float floorContact = smoothstep(0.935, 1.0, y) *
                smoothstep(5.0, 12.0, leakAge) *
                smoothstep(0.06, 0.42, streams + sdfWet * 0.20 + streamAccum * 0.035 + wallFlood * 0.24) *
                smoothstep(0.015 + wallEdgeNoise, 0.090 + wallEdgeNoise, u) *
                (1.0 - smoothstep(0.910 - wallEdgeNoise, 0.990 - wallEdgeNoise, u));
            wallFlood = saturate(wallFlood + floorContact * 0.36);
            wallFlood = min(wallFlood, 0.58);
            streams = saturate(streams + wallSheet * (0.48 + bloodQuality * 0.22) +
                mergedField * (0.66 + bloodQuality * 0.24) + wallFlood * (0.36 + bloodQuality * 0.18));
            float bottomGather = smoothstep(0.90, 1.0, y) * (streams * 0.48 + topSource * 0.10) * smoothstep(4.5, 9.5, leakAge);
            float brokenEdges = 0.88 + 0.12 * Hash21(float2(floor(u * 34.0) + seed * 59.0, floor(y * 24.0)));
            if (highBloodDetail)
            {
                brokenEdges = 0.78 + 0.22 * Fbm3(float3(u * 26.0, y * 18.0, seed * 59.0));
            }
            alpha = max(max(streams * brokenEdges, beads * 1.14), max(max(topSource * 0.58, bottomGather), max(wallFlood * 0.70, max(ceilingContact * 0.72, floorContact * 0.72))));
            alpha = smoothstep(0.008, 0.046, alpha);
            drips = saturate(streams + beads + bottomGather * 0.65 + wallFlood * 0.36 + floorContact * 0.28);
            thickness = saturate(streams * 0.60 + mergedCore * 0.78 + wallFlood * 0.68 + beads * 0.82 + bottomGather * 0.46 + topSource * 0.34 + ceilingContact * 0.28 + floorContact * 0.32);
            thickness *= alpha;
            drips *= alpha;
        }

)" R"(

        else if (floorMask > 0.45)
        {
            if (centerSeepSurface > 0.5)
            {
                float centerThickness = 0.0;
                float centerSpeed = lerp(0.026, 0.017, waterLiquid) * lerp(1.0, 3.15, menuCenterSeepSurface);
                float centerRadius = lerp(0.56, 0.72, waterLiquid) * lerp(1.0, 1.18, menuCenterSeepSurface);
                float source = CenterSeepPool(bloodUv, input.worldPos, seed, leakAge, centerSpeed, centerRadius, centerThickness);
                alpha = source * floorMask * smoothstep(0.0, lerp(0.45, 0.82, waterLiquid) * lerp(1.0, 0.34, menuCenterSeepSurface), leakAge);
                alpha = smoothstep(lerp(0.020, 0.012, waterLiquid), lerp(0.118, 0.090, waterLiquid), alpha);
                thickness = centerThickness * alpha;
                drips = 0.0;
            }
            else
            {
                float u = bloodUv.x;
                float away = 1.0 - bloodUv.y;
                float pooled = 0.0;
                float pooledField = 0.0;
                float wetRim = 0.0;
                float earlySoakField = 0.0;
                float lobeThickness = 0.0;
                [loop]
                for (int i = 0; i < 32; ++i)
                {
                    float fi = (float)i;
                    if (fi >= floorStreamCount) break;
                    float r0 = Hash21(float2(seed * 47.0 + fi, 3.0));
                    float r1 = Hash21(float2(seed * 31.0, fi + 5.0));
                    float r2 = Hash21(float2(fi + 9.0, seed * 71.0));
                    float clusterCount = 2.0 + floor(Hash21(float2(seed * 83.0, 19.0)) * 3.0);
                    float clusterId = floor(fmod(fi + floor(seed * 31.0), clusterCount));
                    float uniformCenter = 0.055 + ((fi + 0.20 + r0 * 0.60) / max(1.0, streamCount)) * 0.89;
                    float clusterCenter = 0.08 + Hash21(float2(seed * 89.0 + clusterId * 5.7, 13.0)) * 0.84;
                    float clusterSpread = 0.025 + Hash21(float2(seed * 97.0 + clusterId, 29.0)) * 0.080;
                    float clusteredCenter = clusterCenter + (Hash21(float2(seed * 131.0 + fi, 37.0)) - 0.5) * clusterSpread;
                    float center = clamp(lerp(uniformCenter, clusteredCenter, 0.58 + Hash21(float2(seed * 151.0 + fi, 43.0)) * 0.34), 0.045, 0.955);
                    float densityBase = Hash21(float2(center * 17.0 + seed * 11.0, clusterId * 3.1 + 5.0));
                    if (highBloodDetail)
                    {
                        densityBase = smoothstep(0.24, 0.78, Fbm3(float3(center * 4.2, seed * 18.0, clusterId * 2.3)));
                    }
                    float densityBand = densityBase;
                    float streamStrength = lerp(0.24, 1.18, densityBand) * lerp(0.58, 1.12, Hash21(float2(seed * 173.0 + fi, 47.0)));
                    float streamDelay = wallLeakSurface > 0.5
                        ? (r0 * 3.4 + fi * (0.05 + r2 * 0.16) + Hash21(float2(seed * 251.0 + fi, 157.0)) * 1.1)
                        : (r0 * 2.2 + fi * 0.16);
                    float streamAge = max(0.0, leakAge - streamDelay);
                    float flowAge = streamAge;
                    if (wallLeakSurface > 0.5)
                    {
                        float speedPhase = streamAge * (0.62 + r2 * 0.54) + seed * 17.0 + fi * 2.13;
                        flowAge = max(0.0, streamAge * (0.90 + r1 * 0.18) +
                            sin(speedPhase) * (0.10 + r0 * 0.08) +
                            sin(speedPhase * 2.17 + r2 * 6.0) * 0.032);
                    }
                    float streamGrowRate = wallLeakSurface > 0.5
                        ? (0.074 + r1 * 0.062)
                        : (0.088 + r1 * 0.066);
                    float streamGrow = smoothstep(0.0, 1.0, saturate(flowAge * streamGrowRate));
                    float len = saturate(0.160 + streamGrow * (0.92 + r1 * 0.24));
                    float reachFloor = smoothstep(0.83, 0.98, len) * streamStrength;
                    float contact = reachFloor;
                    if (wallLeakSurface > 0.5)
                    {
                        float waterFloorHit = smoothstep(0.88, 1.00, len) * smoothstep(5.8, 9.8, flowAge);
                        float bloodFloorHit = smoothstep(0.985, 1.075, len) * smoothstep(12.0, 17.0, flowAge);
                        contact = lerp(bloodFloorHit, waterFloorHit, waterLiquid) * streamStrength;
                    }
                    float poolDelay = wallLeakSurface > 0.5
                        ? lerp(11.8 + r2 * 4.8, 3.8 + r2 * 2.4, waterLiquid)
                        : 4.8;
                    float extraDelay = r2 * (wallLeakSurface > 0.5 ? lerp(1.3, 0.45, waterLiquid) : 1.9);
                    float poolAge = max(0.0, flowAge - poolDelay - extraDelay);
                    float poolGrowRate = wallLeakSurface > 0.5 ? (0.034 + r1 * 0.030) : (0.112 + r1 * 0.056);
                    poolGrowRate *= wallLeakSurface > 0.5 ? lerp(1.0, 1.42, waterLiquid) : lerp(1.0, 0.56, waterLiquid);
                    float poolGrow = smoothstep(0.0, 1.0, saturate(poolAge * poolGrowRate)) * contact;
                    float leanAtFloor = (r1 - 0.5) * (0.0025 + r2 * 0.0035);
                    float bottomWobble = 0.0;
                    if (wallLeakSurface > 0.5)
                    {
                        bottomWobble = (Hash21(float2(fi * 5.1 + seed * 23.0, 3.0)) - 0.5) * (0.0007 + r2 * 0.0010);
                        if (highBloodDetail)
                        {
                            bottomWobble = (Fbm3(float3(2.2, fi * 1.7, seed * 23.0)) - 0.5) * (0.0011 + r2 * 0.0018);
                        }
                    }
                    float sourceU = center + leanAtFloor + bottomWobble;
                    float edgeNoise = Hash21(float2(floor(u * 12.0) + fi, floor(away * 10.0) + seed * 23.0)) - 0.5;
                    if (highBloodDetail)
                    {
                        edgeNoise = Fbm3(float3(u * 7.5 + fi, away * 4.0, seed * 23.0)) - 0.5;
                    }
                    float spreadAway = wallLeakSurface > 0.5
                        ? (0.070 + poolGrow * (1.02 + r2 * 0.58) + edgeNoise * 0.046)
                        : (0.070 + poolGrow * (0.78 + r2 * 0.48) + edgeNoise * 0.040);
                    float spreadSide = (wallLeakSurface > 0.5
                        ? (0.032 + poolGrow * (0.155 + r1 * 0.210))
                        : (0.030 + poolGrow * (0.115 + r1 * 0.180))) * lerp(0.78, 1.0, streamWidthScale);
                    spreadAway *= lerp(1.0, 1.34, waterLiquid);
                    spreadSide *= lerp(1.0, 1.18, waterLiquid);
                    float sideWorld = (u - sourceU) * bloodUvMeters.x;
                    float awayWorld = away * bloodUvMeters.y;
                    float soakRag = Fbm3(float3(input.worldPos.xz * (7.0 + r2 * 7.0) + fi * 0.37, seed * 31.0 + fi));
                    if (wallLeakSurface > 0.5)
                    {
                        float soakPoolDelay = lerp(9.4 + r2 * 3.8, 3.1 + r2 * 2.0, waterLiquid);
                        float soakPoolAge = max(0.0, flowAge - soakPoolDelay);
                        float waterBottomReached = smoothstep(0.86, 0.99, len) * smoothstep(4.8, 8.8, flowAge);
                        float bloodBottomReached = smoothstep(0.965, 1.055, len) * smoothstep(10.5, 15.5, flowAge);
                        float bottomReached = lerp(bloodBottomReached, waterBottomReached, waterLiquid);
                        float soakGrowRate = (0.038 + r1 * 0.032) * lerp(1.0, 1.30, waterLiquid);
                        float soakGrow = smoothstep(0.0, 1.0, saturate(soakPoolAge * soakGrowRate)) *
                            bottomReached * streamStrength;
                        float soakSpreadAway = 0.055 + soakGrow * (0.72 + r2 * 0.42) + edgeNoise * 0.042;
                        float soakSpreadSide = (0.030 + soakGrow * (0.170 + r1 * 0.150)) * lerp(0.82, 1.0, streamWidthScale);
                        soakSpreadAway *= lerp(1.0, 1.42, waterLiquid);
                        soakSpreadSide *= lerp(1.0, 1.18, waterLiquid);
                        float soakSideWorld = max(0.016, soakSpreadSide * bloodUvMeters.x);
                        float soakAwayWorld = max(0.018, soakSpreadAway * bloodUvMeters.x);
                        float2 soakQ = float2(sideWorld / soakSideWorld, awayWorld / soakAwayWorld);
                        float soakBreakup = (soakRag - 0.5) * (0.28 + soakGrow * 0.34);
                        float soakLayer = 1.0 - smoothstep(0.72, 1.14, dot(soakQ, soakQ) + soakBreakup);
                        soakLayer *= smoothstep(0.0, 0.16, soakGrow);
                        earlySoakField = max(earlySoakField, soakLayer * (0.24 + soakGrow * 0.18));
                    }
                    float spreadSideWorld = max(0.010, spreadSide * bloodUvMeters.x);
                    float spreadAwayWorld = max(0.012, spreadAway * bloodUvMeters.x);
                    float2 q = float2(sideWorld / spreadSideWorld, awayWorld / spreadAwayWorld);
                    float edgeBreakup = (soakRag - 0.5) * (0.34 + poolGrow * 0.42);
                    float lobe = 1.0 - smoothstep(0.70, 1.05, dot(q, q) + edgeBreakup);
                    lobe *= smoothstep(0.0, 0.14, poolGrow);
                    float feeder = exp(-(sideWorld * sideWorld) / max(0.000035, spreadSideWorld * spreadSideWorld * 0.22)) *
                        (1.0 - smoothstep(0.0, (0.135 + poolGrow * 0.075) * bloodUvMeters.x, awayWorld)) * contact;
                    float capillary = smoothstep(0.62, 0.92,
                        Fbm3(float3(input.worldPos.xz * (18.0 + r1 * 12.0) + fi, seed * 57.0))) *
                        (1.0 - smoothstep(0.72, 1.35, dot(q, q))) * smoothstep(0.12, 0.85, poolGrow) * contact * 0.22;
                    lobe = max(lobe, capillary);
                    pooled = max(pooled, lobe);
                    pooledField += saturate(lobe) * (0.62 + poolGrow * 0.32) + feeder * 0.22;
                    wetRim = max(wetRim, feeder);
                    lobeThickness = max(lobeThickness, lobe * (0.64 + poolGrow * 0.54) + feeder * 0.52);
                }
                float sdfMerge = smoothstep(0.42, 1.18, pooledField + wetRim * 0.40);
                float merged = saturate(max(max(pooled, sdfMerge), earlySoakField * 0.58) + wetRim * 0.72);
                float lateralNoise = (Hash21(float2(floor(u * 11.0) + seed * 11.0, floor(away * 8.0))) - 0.5) * 0.024;
                if (highBloodDetail)
                {
                    lateralNoise = (Fbm3(float3(u * 6.1, away * 3.7, seed * 11.0)) - 0.5) * 0.030;
                }
                float lateral = smoothstep(0.030 + lateralNoise, 0.070 + lateralNoise, u) *
                    (1.0 - smoothstep(0.930 - lateralNoise, 0.970 - lateralNoise, u));
                float farEdgeNoise = (Fbm3(float3(u * 4.7 + seed * 23.0, away * 2.1, seed * 67.0)) - 0.5) * 0.16 +
                    (Noise3(float3(u * 13.0, away * 8.0, seed * 97.0)) - 0.5) * 0.045;
                float waterFarFade = 1.0 - smoothstep(1.56 + farEdgeNoise, 1.94 + farEdgeNoise, away);
                float waterNearFade = smoothstep(-0.030 + farEdgeNoise * 0.24, 0.045 + farEdgeNoise * 0.24, away);
                float floorCardFade = lerp(1.0, lateral * waterFarFade * waterNearFade, waterLiquid);
                float seam = 1.0 - smoothstep(0.0, 0.040, away);
                float floodNoise = 0.70 + 0.30 * Fbm3(float3(input.worldPos.xz * 2.7 + seed * 9.0, seed * 31.0));
                float floorFrontNoise = (Fbm3(float3(u * 3.1 + seed * 17.0, away * 1.9, seed * 53.0)) - 0.5) * 0.20 +
                    (Noise3(float3(u * 10.0, away * 5.5, seed * 79.0)) - 0.5) * 0.055;
                float lateFeatherStart = lerp(14.0, 10.0, waterLiquid);
                float lateFeatherEnd = lerp(34.0, 52.0, waterLiquid);
                float lateFeather = smoothstep(lateFeatherStart, lateFeatherEnd, leakAge) *
                    smoothstep(0.22, 0.86, pooledField + wetRim * 0.70) *
                    smoothstep(0.56, 0.91, floodNoise + floorFrontNoise * 0.55) *
                    (1.0 - smoothstep(lerp(0.74, 0.88, waterLiquid), lerp(1.16, 1.42, waterLiquid), away)) *
                    lateral * lerp(0.22, 0.38, waterLiquid);
                merged = saturate(merged + lateFeather);
                merged *= floorCardFade;
                wetRim *= floorCardFade;
                earlySoakField *= floorCardFade;
                lobeThickness *= floorCardFade;
                float soakNoise = 0.78 + 0.22 * Hash21(float2(floor(input.worldPos.x * 7.0) + seed * 41.0, floor(input.worldPos.z * 7.0)));
                float fibers = 0.82 + 0.18 * Hash21(float2(floor(input.worldPos.x * 9.0) + seed * 47.0, floor(input.worldPos.z * 9.0)));
                if (highBloodDetail)
                {
                    soakNoise = 0.65 + 0.35 * Fbm3(float3(input.worldPos.xz * 10.0, seed * 41.0));
                    fibers = 0.70 + 0.30 * Fbm3(float3(input.worldPos.xz * 11.0, seed * 41.0));
                }
                float soak = saturate(merged + earlySoakField * lerp(0.42, 0.68, waterLiquid)) *
                    lateral * lerp(0.22, 0.36, waterLiquid) * soakNoise;
                alpha = max(max(merged, soak * fibers), seam * lateral * wetRim * 0.75);
                alpha = smoothstep(lerp(0.024, 0.014, waterLiquid), lerp(0.112, 0.088, waterLiquid), alpha);
                thickness = saturate(lobeThickness * 0.86 + soak * lerp(0.28, 0.48, waterLiquid) +
                    earlySoakField * lerp(0.070, 0.16, waterLiquid) + seam * wetRim * 0.46);
                drips = saturate(merged + seam * wetRim * 0.45);
                thickness *= alpha;
                drips *= alpha;
            }
        }

)" R"(

        else
        {
            if (wallLeakSurface > 0.5)
            {
                float u = bloodUv.x;
                float away = bloodUv.y;
                float edgeJitterU = (Hash21(float2(floor(away * 10.0) + seed * 31.0, seed * 17.0)) - 0.5) * 0.055;
                float edgeJitterV = (Hash21(float2(floor(u * 10.0) + seed * 37.0, seed * 23.0)) - 0.5) * 0.050;
                if (highBloodDetail)
                {
                    edgeJitterU = (Fbm3(float3(away * 5.6, seed * 9.0, u * 1.7)) - 0.5) * 0.070;
                    edgeJitterV = (Fbm3(float3(u * 5.2, seed * 11.0, away * 1.9)) - 0.5) * 0.064;
                }
                float cardEdgeFade = smoothstep(0.012 + edgeJitterU, 0.115 + edgeJitterU, u) *
                    (1.0 - smoothstep(0.885 - edgeJitterU, 0.988 - edgeJitterU, u)) *
                    smoothstep(0.010 + edgeJitterV, 0.105 + edgeJitterV, away) *
                    (1.0 - smoothstep(0.850 - edgeJitterV, 0.985 - edgeJitterV, away));
                float topSource = 0.0;
                float topThickness = 0.0;
                [loop]
                for (int i = 0; i < 32; ++i)
                {
                    float fi = (float)i;
                    if (fi >= ceilingStreamCount) break;
                    float r0 = Hash21(float2(seed * 47.0 + fi, 3.0));
                    float r1 = Hash21(float2(seed * 31.0, fi + 5.0));
                    float r2 = Hash21(float2(fi + 9.0, seed * 71.0));
                    float clusterCount = 2.0 + floor(Hash21(float2(seed * 83.0, 19.0)) * 3.0);
                    float clusterId = floor(fmod(fi + floor(seed * 31.0), clusterCount));
                    float uniformCenter = 0.055 + ((fi + 0.20 + r0 * 0.60) / max(1.0, streamCount)) * 0.89;
                    float clusterCenter = 0.08 + Hash21(float2(seed * 89.0 + clusterId * 5.7, 13.0)) * 0.84;
                    float clusterSpread = 0.025 + Hash21(float2(seed * 97.0 + clusterId, 29.0)) * 0.080;
                    float clusteredCenter = clusterCenter + (Hash21(float2(seed * 131.0 + fi, 37.0)) - 0.5) * clusterSpread;
                    float center = clamp(lerp(uniformCenter, clusteredCenter, 0.58 + Hash21(float2(seed * 151.0 + fi, 43.0)) * 0.34), 0.045, 0.955);
                    float densityBase = Hash21(float2(center * 17.0 + seed * 11.0, clusterId * 3.1 + 5.0));
                    if (highBloodDetail)
                    {
                        densityBase = smoothstep(0.24, 0.78, Fbm3(float3(center * 4.2, seed * 18.0, clusterId * 2.3)));
                    }
                    float densityBand = densityBase;
                    float streamStrength = lerp(0.24, 1.18, densityBand) * lerp(0.58, 1.12, Hash21(float2(seed * 173.0 + fi, 47.0)));
                    float streamAge = max(0.0, leakAge - r0 * 2.2 - fi * 0.16);
                    float sourceGrow = smoothstep(0.0, 1.0, saturate(streamAge * (0.065 + r1 * 0.040) * lerp(1.0, 0.72, waterLiquid)));
                    float sourceReady = smoothstep(0.25, 1.15, streamAge);
                    float spreadAway = 0.030 + sourceGrow * (0.145 + r2 * 0.095);
                    float spreadSide = (0.018 + r2 * 0.022) * streamWidthScale * (1.0 + sourceGrow * 1.10);
                    spreadAway *= lerp(1.0, 1.22, waterLiquid);
                    spreadSide *= lerp(1.0, 1.14, waterLiquid);
                    float edgeNoise = (Hash21(float2(floor(u * 12.0) + fi, floor(away * 10.0) + seed * 23.0)) - 0.5) * 0.010 * sourceGrow;
                    if (highBloodDetail)
                    {
                        edgeNoise = (Fbm3(float3(u * 8.0 + fi, away * 7.0, seed * 23.0)) - 0.5) * 0.016 * sourceGrow;
                    }
                    float sideWorld = (u - center) * bloodUvMeters.x;
                    float awayWorld = away * bloodUvMeters.y;
                    float spreadSideWorld = max(0.006, spreadSide * bloodUvMeters.x);
                    float spreadAwayWorld = max(0.010, spreadAway * bloodUvMeters.x);
                    float edgeNoiseWorld = edgeNoise * bloodUvMeters.x;
                    float skew = (Hash21(float2(seed * 19.0 + fi, 91.0)) - 0.5) * sourceGrow * 0.42;
                    float2 q = float2((sideWorld + awayWorld * skew) / spreadSideWorld, (awayWorld + edgeNoiseWorld) / spreadAwayWorld);
                    float soakRag = Fbm3(float3(input.worldPos.xz * (11.0 + r2 * 9.0) + fi * 0.29, seed * 43.0 + fi));
                    float microBreak = Noise3(float3(input.worldPos.xz * (18.0 + r2 * 14.0) + fi * 0.17, seed * 71.0 + fi));
                    float lobe = 1.0 - smoothstep(0.54, 1.03,
                        dot(q, q) + (soakRag - 0.5) * (0.36 + sourceGrow * 0.42) + (microBreak - 0.5) * 0.10);
                    float capillary = smoothstep(0.60, 0.94,
                        Fbm3(float3(input.worldPos.xz * (20.0 + r1 * 10.0) + fi, seed * 61.0))) *
                        (1.0 - smoothstep(0.72, 1.28, dot(q, q))) * sourceGrow * sourceReady * 0.18;
                    float source = max(lobe, capillary) * sourceReady * streamStrength;
                    topSource = max(topSource, source);
                    topThickness = max(topThickness, source * (0.48 + sourceGrow * 0.42));
                }
                float raggedEdge = 0.88 + 0.12 * Hash21(float2(floor(u * 26.0) + seed * 67.0, floor(away * 18.0)));
                if (highBloodDetail)
                {
                    raggedEdge = 0.80 + 0.20 * Fbm3(float3(u * 20.0, away * 14.0, seed * 67.0));
                }
                float ceilingNoise = Fbm3(float3(input.worldPos.xz * 4.8 + seed * 7.0, seed * 37.0));
                float fineNoise = Noise3(float3(input.worldPos.xz * 18.0 + seed * 5.0, seed * 83.0));
                float organicMask = smoothstep(0.22, 0.70, ceilingNoise + (fineNoise - 0.5) * 0.20);
                float rimAge = smoothstep(lerp(5.5, 4.0, waterLiquid), lerp(11.5, 17.0, waterLiquid), leakAge);
                float rimNoise = Fbm3(float3(u * 5.4 + seed * 13.0, away * 3.2, seed * 47.0));
                float rimFine = Noise3(float3(u * 19.0 + seed * 7.0, away * 9.0, seed * 71.0));
                float rimWidth = 0.026 + rimNoise * 0.034;
                float rimSideFade = smoothstep(0.010 + edgeJitterU, 0.090 + edgeJitterU, u) *
                    (1.0 - smoothstep(0.910 - edgeJitterU, 0.990 - edgeJitterU, u));
                float rimBreakup = smoothstep(0.16, 0.72, rimNoise + (rimFine - 0.5) * 0.22);
                float delayedRim = (1.0 - smoothstep(rimWidth, rimWidth + 0.070, away)) *
                    rimSideFade * rimBreakup * rimAge;
                float soakFrontNoise = (Fbm3(float3(u * 3.4 + seed * 13.0, away * 1.7, seed * 41.0)) - 0.5) * 0.22 +
                    (Noise3(float3(u * 11.0, away * 5.0, seed * 73.0)) - 0.5) * 0.06;
                float rimFeed = smoothstep(0.04, 0.30, topSource);
                float soakReach = saturate((leakAge - lerp(7.5, 5.0, waterLiquid)) / lerp(34.0, 50.0, waterLiquid));
                float unevenCeilingReach = saturate(soakReach + rimFeed * 0.22 + soakFrontNoise);
                float ceilingFront = 1.0 - smoothstep(unevenCeilingReach,
                    unevenCeilingReach + 0.17 + abs(soakFrontNoise) * 0.10,
                    away);
                float ceilingSoak = smoothstep(lerp(8.0, 5.0, waterLiquid), lerp(31.0, 52.0, waterLiquid), leakAge) *
                    ceilingFront * smoothstep(0.006, 0.058, away) *
                    (1.0 - smoothstep(1.01 + edgeJitterV, 1.11 + edgeJitterV, away)) *
                    cardEdgeFade * organicMask * (0.62 + ceilingNoise * 0.38);
                topSource = saturate(topSource * cardEdgeFade + ceilingSoak * lerp(0.52, 0.74, waterLiquid) +
                    delayedRim * (lerp(0.52, 0.68, waterLiquid) + rimFeed * 0.40));
                alpha = topSource * raggedEdge * ceilingMask * smoothstep(0.0, 0.35, leakAge);
                alpha = smoothstep(lerp(0.014, 0.008, waterLiquid), lerp(0.092, 0.072, waterLiquid), alpha);
                thickness = saturate(topThickness * 0.86 + ceilingSoak * lerp(0.54, 0.78, waterLiquid) +
                    delayedRim * lerp(0.42, 0.58, waterLiquid)) * alpha;
            }
            else
            {
                float centerThickness = 0.0;
                float centerSpeed = lerp(0.024, 0.016, waterLiquid) * lerp(1.0, 3.45, menuCenterSeepSurface);
                float centerRadius = lerp(0.58, 0.74, waterLiquid) * lerp(1.0, 1.20, menuCenterSeepSurface);
                float source = CenterSeepPool(bloodUv, input.worldPos, seed, leakAge, centerSpeed, centerRadius, centerThickness);
                alpha = source * ceilingMask * smoothstep(0.0, lerp(0.65, 1.05, waterLiquid) * lerp(1.0, 0.32, menuCenterSeepSurface), leakAge);
                alpha = smoothstep(lerp(0.024, 0.012, waterLiquid), lerp(0.116, 0.088, waterLiquid), alpha);
                thickness = centerThickness * alpha;
            }
            drips = 0.0;
        }

        float wetAlpha = alpha;
        float wetThickness = thickness;
        float wetDrips = drips;
        alpha = wetAlpha * animMask;
        thickness = wetThickness * animMask;
        drips = wetDrips * animMask;
        if (alpha < lerp(0.045, 0.026, waterLiquid)) discard;

        float2 local = bloodUv * 2.0 - 1.0;
        float2 bulgeSlope = -local * thickness * (0.08 + wet * 0.08);
        bulgeSlope.y += wallMask * drips * (0.030 + wet * 0.028);
        bulgeSlope += (float2(
            Fbm3(float3(bloodUv * 18.0, seed * 47.0)),
            Fbm3(float3(bloodUv.yx * 18.0 + 3.7, seed * 53.0))) - 0.5) * thickness * 0.010;
        float3 B2 = normalize(cross(N, T));
        float3 worldN = normalize(N + T * bulgeSlope.x + B2 * bulgeSlope.y);
        float dist = length(input.worldPos - cam);
        float flashlight = FlashlightAmount(input.worldPos, worldN);
        float overhead = LocalLampLight(input.worldPos, worldN, time) * gLighting1.x;
        float sparkLight = SparkLight(input.worldPos, worldN);
        float3 exitGreen = ExitSignLight(input.worldPos, worldN);
        float exitGlow = max(exitGreen.r, max(exitGreen.g, exitGreen.b));
        float3 toLight = normalize(gShadow0.xyz - input.worldPos);
        float3 reflectDir = reflect(-toLight, worldN);
        float facing = saturate(dot(reflectDir, V));
        float fresnel = pow(1.0 - saturate(dot(worldN, V)), 4.0);
        float specLight = saturate(flashlight + overhead * 0.34 + sparkLight * 0.72 + exitGlow * 0.48);
        float spec = (pow(facing, 168.0) * 1.85 + pow(facing, 34.0) * 0.36 + fresnel * 0.16) *
            specLight * (0.58 + wet * 1.85) * saturate(alpha + thickness * 0.35);
        float grime = Fbm3(input.worldPos * float3(8.0, 5.0, 8.0) + seed * 31.0);
        float filmAlpha = saturate(alpha * (0.58 + thickness * 0.38 + drips * 0.12 + wet * 0.035));
        if (filmAlpha < lerp(0.045, 0.026, waterLiquid)) discard;
        float lightEnergy = saturate(flashlight * 0.88 + overhead * 0.38 + sparkLight * 0.62 + exitGlow * 0.34 + gLighting0.z * 0.06);
        float3 thinBlood = float3(0.320, 0.0140, 0.0052);
        float3 pooledBlood = float3(0.092, 0.0024, 0.0012);
        float3 blood = lerp(thinBlood, pooledBlood, saturate(thickness * 0.88 + drips * 0.24));
        float3 color = blood * (0.24 + lightEnergy * 1.02) * (0.70 + grime * 0.22);
        color = lerp(color, color * float3(0.54, 0.28, 0.24), drips * 0.18);
        color += float3(0.72, 0.62, 0.48) * spec;
        color += exitGreen * (0.06 + wet * 0.16) * saturate(alpha + thickness * 0.18);
        if (waterLiquid > 0.5)
        {
            float waterCore = saturate(alpha * 0.74 + thickness * 0.26);
            float waterFresnel = pow(1.0 - saturate(dot(worldN, V)), 3.0);
            float waterSpec = spec * (0.54 + waterCore * 1.10) + waterFresnel * specLight * (0.08 + waterCore * 0.18);
            float3 thinTint = float3(0.045, 0.070, 0.076) * (0.42 + lightEnergy * 0.34);
            float3 deepTint = float3(0.025, 0.044, 0.052) * (0.36 + lightEnergy * 0.26);
            float3 clearFilm = lerp(thinTint, deepTint, saturate(thickness * 0.80 + drips * 0.20));
            float3 reflectedLamp = float3(0.42, 0.56, 0.58) * waterSpec;
            color = clearFilm * (0.48 + waterCore * 0.38) + reflectedLamp;
            color += exitGreen * (0.018 + waterCore * 0.052);
            filmAlpha = saturate(alpha * (0.20 + thickness * 0.11 + drips * 0.045));
        }
        color *= 1.0 - CornerAO(input.worldPos, worldN) * 0.45;
        float fog = saturate((dist - gFog0.x) / max(0.01, gFog0.y - gFog0.x));
        fog = 1.0 - exp(-fog * fog * 3.2);
        float fogBlock = saturate(fog * gFog0.z * 1.25);
        color = lerp(color, float3(0.0, 0.0, 0.0), fogBlock);
        filmAlpha *= 1.0 - fogBlock * 0.18;
        return float4(ApplyPost(color), filmAlpha);
    }

)" R"(

    if (input.material > 11.05 && input.material < 11.45)
    {
        float seed = frac(input.material * 23.71);
        float2 smokeUv = AspectSoftenedBloodUv(uv, input.worldPos);
        float2 plane = float2(smokeUv.x * 2.0 - 1.0, 1.0 - smokeUv.y * 2.0);
        float edgeDist = min(min(smokeUv.x, 1.0 - smokeUv.x), min(smokeUv.y, 1.0 - smokeUv.y));
        float cardFade = smoothstep(0.110, 0.330, edgeDist);
        float2 ovalP = plane * float2(1.10, 1.04);
        float radial = dot(ovalP, ovalP);
        float ovalFade = exp(-radial * 2.65) * (1.0 - smoothstep(0.62, 1.02, radial));
        float3 rdLocal = normalize(float3(dot(-V, T), dot(-V, B), dot(-V, N)));
        float3 p0 = float3(plane.x * 0.96, plane.y * 1.05, 0.0);
        float heightFade = smoothstep(0.02, 0.30, input.worldPos.y) * (1.0 - smoothstep(2.10, 2.70, input.worldPos.y));
        float transmittance = 1.0;
        float alpha = 0.0;
        [loop]
        for (int s = 0; s < 9; ++s)
        {
            float stepT = -0.95 + ((float)s + 0.5) * (1.90 / 9.0);
            float3 p = p0 + rdLocal * stepT;
            p.xz *= 0.90 + seed * 0.16;
            p.y += sin(time * (0.31 + seed * 0.10) + seed * 21.0) * 0.10;
            float density = BlackSmokeDensity(p, seed + (float)s * 0.017, time);
            float sampleAlpha = saturate(density * 0.210);
            alpha += transmittance * sampleAlpha;
            transmittance *= 1.0 - sampleAlpha;
        }
        float reform = 0.83 + 0.17 * sin(time * (0.68 + seed * 0.21) + seed * 17.0 + input.worldPos.y * 1.7);
        alpha = saturate(alpha * heightFade * cardFade * ovalFade * reform * 1.42);
        if (alpha < 0.012) discard;
        float3 color = float3(0.0, 0.0, 0.0);

        float dist = length(input.worldPos - cam);
        float fog = saturate((dist - gFog0.x) / max(0.01, gFog0.y - gFog0.x));
        fog = 1.0 - exp(-fog * fog * 3.2);
        float fogBlock = saturate(fog * gFog0.z * 1.35);
        alpha *= pow(1.0 - fogBlock, 1.65);
        return float4(color, alpha);
    }

    if (input.material > 11.005 && input.material < 11.055)
    {
        float seed = frac(input.material * 29.73);
        float encodedSide = clamp(floor(rawUv.x + 0.0001), 0.0, 3.0);
        float rawModeEncoded = floor(rawUv.y + 0.0001);
        float floorNeighborMask = floor(rawModeEncoded / 8.0);
        float rawMode = rawModeEncoded - floorNeighborMask * 8.0;
        float warpA = Fbm3(float3(uv * (2.8 + seed * 2.2) + seed * 5.1, seed * 17.0));
        float warpB = Fbm3(float3(uv.yx * (3.4 + seed * 1.7) - seed * 4.3, seed * 29.0));
        float2 warpedUv = saturate(uv + (float2(warpA, warpB) - 0.5) * (0.075 + seed * 0.035));
        float2 d = warpedUv - 0.5;
        float border = min(min(uv.x, 1.0 - uv.x), min(uv.y, 1.0 - uv.y));
        float broad = Fbm3(float3(warpedUv * 5.7 + seed * 3.1, seed * 17.0));
        float fine = Fbm3(float3(warpedUv * 18.0 - seed * 2.0, seed * 31.0));
        float floorSurface = saturate(N.y * 2.0);
        float ceilingSurface = saturate(-N.y * 2.0);
        float horizontalSurface = saturate(abs(N.y) * 2.0);
        float vertical = 1.0 - horizontalSurface;
        float floorBridge = floorSurface * step(3.5, rawMode) * (1.0 - step(4.5, rawMode));
        float wallFromCeiling = vertical * step(2.5, rawMode) * (1.0 - step(3.5, rawMode));
        float wallFromFloor = vertical * step(3.5, rawMode);
        float ceilingCompact = ceilingSurface * (1.0 - vertical) * step(2.5, rawMode) * (1.0 - step(3.5, rawMode));
        float floorTouchdown = floorSurface * step(4.5, rawMode) * (1.0 - step(5.5, rawMode));
        float encodedMode = clamp(lerp(rawMode, 0.0, saturate(ceilingCompact + floorTouchdown)), 0.0, 2.0);
        float encodedEdge = (1.0 - vertical) * step(0.5, encodedMode);
        float edgeOnly = (1.0 - vertical) * step(1.5, encodedMode);
        float materialBand = saturate((input.material - 11.006) / 0.049);
        float edgeMode = max(smoothstep(0.32, 0.60, materialBand), encodedEdge);
        float floorMergeN = step(0.5, fmod(floor(floorNeighborMask / 1.0), 2.0));
        float floorMergeS = step(0.5, fmod(floor(floorNeighborMask / 2.0), 2.0));
        float floorMergeW = step(0.5, fmod(floor(floorNeighborMask / 4.0), 2.0));
        float floorMergeE = step(0.5, fmod(floor(floorNeighborMask / 8.0), 2.0));
        float floorMergeNW = step(0.5, fmod(floor(floorNeighborMask / 16.0), 2.0)) * saturate(floorMergeN + floorMergeW);
        float floorMergeNE = step(0.5, fmod(floor(floorNeighborMask / 32.0), 2.0)) * saturate(floorMergeN + floorMergeE);
        float floorMergeSW = step(0.5, fmod(floor(floorNeighborMask / 64.0), 2.0)) * saturate(floorMergeS + floorMergeW);
        float floorMergeSE = step(0.5, fmod(floor(floorNeighborMask / 128.0), 2.0)) * saturate(floorMergeS + floorMergeE);
        float2 mergedD = d;
        mergedD.x = d.x * lerp(1.0, 0.76, saturate(floorMergeW + floorMergeE)) +
            (floorMergeW - floorMergeE) * 0.08;
        mergedD.y = d.y * lerp(1.0, 0.76, saturate(floorMergeN + floorMergeS)) +
            (floorMergeS - floorMergeN) * 0.08;

        float puddleAngle = seed * 6.28318 + Hash21(float2(seed, 3.7)) * 2.4;
        float2 puddleRot = float2(cos(puddleAngle), sin(puddleAngle));
        float2 rd = float2(dot(mergedD, puddleRot), dot(mergedD, float2(-puddleRot.y, puddleRot.x)));
        float2 puddleAspect = float2(0.82 + Hash21(float2(seed, 5.1)) * 0.62,
                                     0.74 + Hash21(float2(seed, 7.3)) * 0.72);
        float2 floorPuddleAspect = float2(0.94 + Hash21(float2(seed, 5.1)) * 0.14,
                                          0.94 + Hash21(float2(seed, 7.3)) * 0.14);
        float floorShape = smoothstep(0.70, 0.16, length(rd * floorPuddleAspect) + (broad - 0.5) * 0.105);
        float floorLobes = 0.0;
        [loop]
        for (int f = 0; f < 4; ++f)
        {
            float ff = (float)f;
            float enabled = step(0.22, Hash21(float2(seed * 67.0 + ff, 31.0)));
            float2 fc = float2(Hash21(float2(seed * 71.0 + ff, 37.0)),
                               Hash21(float2(seed * 73.0, ff + 41.0))) - 0.5;
            fc *= 0.18 + Hash21(float2(seed * 79.0 + ff, 43.0)) * 0.34;
            float fa = seed * 3.9 + ff * 1.21 + Hash21(float2(seed, ff + 47.0)) * 1.3;
            float2 fr = float2(cos(fa), sin(fa));
            float2 fq = mergedD - fc;
            fq = float2(dot(fq, fr), dot(fq, float2(-fr.y, fr.x)));
            fq *= float2(1.10 + Hash21(float2(ff, seed * 83.0)) * 0.82,
                         0.76 + Hash21(float2(seed * 89.0, ff)) * 0.58);
            float oval = smoothstep(0.34 + Hash21(float2(seed * 97.0, ff)) * 0.17, 0.09, length(fq));
            floorLobes = max(floorLobes, oval * enabled * (0.48 + Hash21(float2(seed * 101.0 + ff, 53.0)) * 0.40));
        }
        floorShape = max(floorShape, floorLobes);
        float touchdownNoise = (Fbm3(float3(warpedUv * (10.0 + seed * 3.0) + seed * 11.0, seed * 59.0)) - 0.5) * 0.095 +
            (fine - 0.5) * 0.030;
        float touchdownRadial = length(rd * float2(0.94 + seed * 0.08, 0.88 + Hash21(float2(seed, 61.0)) * 0.12));
        float touchdownCore = 1.0 - smoothstep(0.30, 0.47, touchdownRadial + touchdownNoise * 0.55);
        float touchdownBody = 1.0 - smoothstep(0.48, 0.64, touchdownRadial + touchdownNoise);
        float touchdownRim = (1.0 - smoothstep(0.62, 0.75, touchdownRadial + touchdownNoise * 1.15)) *
            smoothstep(0.34, 0.58, touchdownRadial + touchdownNoise * 0.65);
        float touchdownShape = saturate(max(touchdownCore, touchdownBody * 0.88) + touchdownRim * 0.22);
        floorShape = lerp(floorShape, max(touchdownShape, floorShape * 0.32), floorTouchdown);
        float floorSeamNoise = (Fbm3(float3(uv * 8.0 + seed * 13.0, seed * 41.0)) - 0.5) * 0.075;
        float floorSeamBreak = smoothstep(0.18, 0.72, broad + fine * 0.13);
        float floorAlongX = smoothstep(0.030, 0.185, uv.x) * (1.0 - smoothstep(0.815, 0.970, uv.x));
        float floorAlongY = smoothstep(0.030, 0.185, uv.y) * (1.0 - smoothstep(0.815, 0.970, uv.y));
        float floorWetN = floorMergeN * (1.0 - smoothstep(0.27 + floorSeamNoise, 0.62 + floorSeamNoise, 1.0 - uv.y)) * floorAlongX;
        float floorWetS = floorMergeS * (1.0 - smoothstep(0.27 - floorSeamNoise, 0.62 - floorSeamNoise, uv.y)) * floorAlongX;
        float floorWetW = floorMergeW * (1.0 - smoothstep(0.27 - floorSeamNoise, 0.62 - floorSeamNoise, uv.x)) * floorAlongY;
        float floorWetE = floorMergeE * (1.0 - smoothstep(0.27 + floorSeamNoise, 0.62 + floorSeamNoise, 1.0 - uv.x)) * floorAlongY;
        float cornerNW = floorMergeNW * (1.0 - smoothstep(0.24, 0.62, length(float2(uv.x, 1.0 - uv.y) * float2(1.08, 1.08)) + floorSeamNoise));
        float cornerNE = floorMergeNE * (1.0 - smoothstep(0.24, 0.62, length(float2(1.0 - uv.x, 1.0 - uv.y) * float2(1.08, 1.08)) - floorSeamNoise));
        float cornerSW = floorMergeSW * (1.0 - smoothstep(0.24, 0.62, length(float2(uv.x, uv.y) * float2(1.08, 1.08)) - floorSeamNoise));
        float cornerSE = floorMergeSE * (1.0 - smoothstep(0.24, 0.62, length(float2(1.0 - uv.x, uv.y) * float2(1.08, 1.08)) + floorSeamNoise));
        float floorSeamShape = max(max(floorWetN, floorWetS), max(floorWetW, floorWetE));
        float floorCornerShape = max(max(cornerNW, cornerNE), max(cornerSW, cornerSE));
        floorShape = max(floorShape, max(floorSeamShape * (0.34 + floorSeamBreak * 0.18),
            floorCornerShape * (0.74 + floorSeamBreak * 0.26)));

)" R"(

        floorShape *= 1.0 - smoothstep(0.78, 0.96,
            Fbm3(float3(warpedUv * (8.0 + seed * 3.0) + seed * 9.0, seed * 37.0))) * 0.075;

        float ceilingShape = smoothstep(0.66, 0.15, length(rd * puddleAspect) + (broad - 0.5) * 0.25);
        float ceilingSdf = 10.0;
        float ceilingField = 0.0;
        [loop]
        for (int l = 0; l < 9; ++l)
        {
            float fl = (float)l;
            float2 lc = float2(Hash21(float2(seed * 17.0 + fl, 11.0)),
                               Hash21(float2(seed * 23.0, fl + 13.0))) - 0.5;
            lc *= 0.18 + Hash21(float2(seed * 31.0 + fl, 17.0)) * 0.72;
            float la = seed * 5.1 + fl * 1.37 + Hash21(float2(seed, fl + 19.0)) * 1.9;
            float2 lr = float2(cos(la), sin(la));
            float2 q = mergedD - lc;
            q = float2(dot(q, lr), dot(q, float2(-lr.y, lr.x)));
            q *= float2(1.05 + Hash21(float2(fl, seed * 37.0)) * 1.15,
                        0.76 + Hash21(float2(seed * 41.0, fl)) * 0.95);
            float radius = 0.16 + Hash21(float2(seed * 43.0, fl)) * 0.27;
            float edgeRough = (Fbm3(float3((q + lc) * (8.0 + fl * 0.7), seed * 47.0 + fl)) - 0.5) * 0.070;
            float sdf = length(q) - radius + edgeRough;
            ceilingSdf = min(ceilingSdf, sdf);
            float lobe = 1.0 - smoothstep(-0.035, 0.125 + Hash21(float2(seed * 53.0, fl)) * 0.08, sdf);
            ceilingField += saturate(lobe) * (0.45 + Hash21(float2(seed * 47.0 + fl, 23.0)) * 0.48);
        }
        float islands = smoothstep(0.66, 0.94, fine) * smoothstep(0.84, 0.22, length(mergedD * (1.12 + seed * 0.32)));
        float mergedCeiling = max(1.0 - smoothstep(-0.015, 0.110, ceilingSdf),
            smoothstep(0.62, 1.42, ceilingField));
        float dryBreak = smoothstep(0.72, 0.94,
            Fbm3(float3(warpedUv * (9.0 + seed * 5.0) + seed * 13.0, seed * 53.0))) *
            smoothstep(0.12, 0.72, max(ceilingShape, mergedCeiling));
        float ceilingNoiseEdge = (Fbm3(float3(warpedUv * 15.0 + seed * 19.0, seed * 61.0)) - 0.5) * 0.18;
        ceilingShape = max(max(ceilingShape * 0.52, mergedCeiling), islands * 0.74) * lerp(1.0, 0.22, edgeOnly);
        ceilingShape *= 1.0 - dryBreak * 0.26;
        ceilingShape = saturate(ceilingShape + ceilingNoiseEdge * smoothstep(0.18, 0.78, ceilingShape));
        float ceilingSeamNoise = (Fbm3(float3(uv * 6.5 + seed * 17.0, seed * 73.0)) - 0.5) * 0.105;
        float ceilingSeamBreak = smoothstep(0.16, 0.76, broad + fine * 0.16);
        float ceilingAlongX = smoothstep(0.020, 0.155, uv.x) * (1.0 - smoothstep(0.845, 0.985, uv.x));
        float ceilingAlongY = smoothstep(0.020, 0.155, uv.y) * (1.0 - smoothstep(0.845, 0.985, uv.y));
        float ceilingWetN = floorMergeN * (1.0 - smoothstep(0.20 + ceilingSeamNoise, 0.58 + ceilingSeamNoise, 1.0 - uv.y)) * ceilingAlongX;
        float ceilingWetS = floorMergeS * (1.0 - smoothstep(0.20 - ceilingSeamNoise, 0.58 - ceilingSeamNoise, uv.y)) * ceilingAlongX;
        float ceilingWetW = floorMergeW * (1.0 - smoothstep(0.20 - ceilingSeamNoise, 0.58 - ceilingSeamNoise, uv.x)) * ceilingAlongY;
        float ceilingWetE = floorMergeE * (1.0 - smoothstep(0.20 + ceilingSeamNoise, 0.58 + ceilingSeamNoise, 1.0 - uv.x)) * ceilingAlongY;
        float ceilingCornerNW = floorMergeNW * (1.0 - smoothstep(0.18, 0.58, length(float2(uv.x, 1.0 - uv.y) * float2(1.0, 1.0)) + ceilingSeamNoise));
        float ceilingCornerNE = floorMergeNE * (1.0 - smoothstep(0.18, 0.58, length(float2(1.0 - uv.x, 1.0 - uv.y) * float2(1.0, 1.0)) - ceilingSeamNoise));
        float ceilingCornerSW = floorMergeSW * (1.0 - smoothstep(0.18, 0.58, length(float2(uv.x, uv.y) * float2(1.0, 1.0)) - ceilingSeamNoise));
        float ceilingCornerSE = floorMergeSE * (1.0 - smoothstep(0.18, 0.58, length(float2(1.0 - uv.x, uv.y) * float2(1.0, 1.0)) + ceilingSeamNoise));
        float ceilingSeamShape = max(max(ceilingWetN, ceilingWetS), max(ceilingWetW, ceilingWetE));
        float ceilingCornerShape = max(max(ceilingCornerNW, ceilingCornerNE), max(ceilingCornerSW, ceilingCornerSE));
        float ceilingNeighborShape = max(ceilingSeamShape * (0.30 + ceilingSeamBreak * 0.16),
            ceilingCornerShape * (0.72 + ceilingSeamBreak * 0.26));
        ceilingNeighborShape *= 1.0 - smoothstep(0.82, 0.98,
            Fbm3(float3(uv * 13.0 + seed * 23.0, seed * 79.0))) * 0.18;
        ceilingShape = max(ceilingShape, ceilingNeighborShape);
        float2 compactOffset = float2(Hash21(float2(seed * 109.0, 71.0)),
                                      Hash21(float2(seed * 113.0, 73.0))) - 0.5;
        compactOffset *= 0.22;
        float2 compactQ = mergedD - compactOffset;
        float compactAngle = seed * 4.7 + Hash21(float2(seed * 127.0, 79.0)) * 2.2;
        float2 compactRot = float2(cos(compactAngle), sin(compactAngle));
        compactQ = float2(dot(compactQ, compactRot), dot(compactQ, float2(-compactRot.y, compactRot.x)));
        compactQ *= float2(1.04 + Hash21(float2(seed * 131.0, 83.0)) * 0.90,
                           0.82 + Hash21(float2(seed * 137.0, 89.0)) * 0.56);
        float compactRadius = 0.18 + Hash21(float2(seed * 139.0, 97.0)) * 0.17;
        float compactNoise = (Fbm3(float3((warpedUv + compactOffset) * 11.0 + seed * 7.0, seed * 101.0)) - 0.5) * 0.105;
        float compactMask = 1.0 - smoothstep(compactRadius, compactRadius + 0.145, length(compactQ) + compactNoise);
        float compactCore = 1.0 - smoothstep(compactRadius * 0.58, compactRadius + 0.090, length(compactQ) + compactNoise * 0.62);
        float compactBreak = 1.0 - smoothstep(0.76, 0.96,
            Fbm3(float3(warpedUv * (12.0 + seed * 3.0) + seed * 17.0, seed * 107.0))) * 0.16;
        ceilingShape = lerp(ceilingShape, max(ceilingShape * compactMask, compactCore * 0.68) * compactBreak, ceilingCompact);
        float sidePick = lerp(floor(seed * 4.0), encodedSide, encodedEdge);
        float edgeAway = uv.y;
        float edgeAlong = uv.x;
        if (sidePick >= 1.0 && sidePick < 2.0)
        {
            edgeAway = 1.0 - uv.y;
            edgeAlong = 1.0 - uv.x;
        }
        else if (sidePick >= 2.0 && sidePick < 3.0)
        {
            edgeAway = uv.x;
            edgeAlong = 1.0 - uv.y;
        }
        else if (sidePick >= 3.0)
        {
            edgeAway = 1.0 - uv.x;
            edgeAlong = uv.y;
        }
        float edgeNoise = Fbm3(float3(edgeAlong * 5.2 + seed * 4.3, edgeAway * 3.4 - seed * 2.1, seed * 43.0));
        float edgeFine = Fbm3(float3(edgeAlong * 19.0 - seed * 7.0, edgeAway * 14.0 + seed * 3.0, seed * 71.0));
        float edgeReach = 0.28 + seed * 0.30 + (edgeNoise - 0.5) * 0.18;
        float edgeFront = 1.0 - smoothstep(edgeReach, edgeReach + 0.13 + abs(edgeNoise - 0.5) * 0.10, edgeAway);
        float edgeTaper = smoothstep(0.02, 0.18, edgeAlong) * smoothstep(0.98, 0.74, edgeAlong);
        float edgeBreakup = smoothstep(0.18, 0.76, edgeNoise + (edgeFine - 0.5) * 0.24);
        float edgeThreads = smoothstep(0.80, 0.98, edgeFine) *
            (1.0 - smoothstep(edgeReach * 0.80, edgeReach + 0.26, edgeAway)) * edgeTaper;
        float edgeShape = max(edgeFront * edgeTaper * edgeBreakup, edgeThreads * 0.72);
        ceilingShape = max(ceilingShape, edgeShape * edgeMode);
        float horizontal = lerp(ceilingShape, floorShape, floorSurface);
        float wallFlowY = uv.y;
        float wallSource = wallFromCeiling;
        float wallWaterCore = 0.0;
        float wallWaterHalo = 0.0;
        float wallWaterSoak = 0.0;
        float wallBottomSoak = 0.0;
        float wallCardSideFade = smoothstep(0.014, 0.095, uv.x) * (1.0 - smoothstep(0.905, 0.990, uv.x));
        float wallEndFade = 1.0 - smoothstep(0.995, 1.012, wallFlowY);
        float wallWaterDebugActive = step(1.0, gTransition0.w);
        float wallWaterDebugPhase = frac(gTransition0.w);
        float wallWaterBaseAge = lerp(9.0, wallWaterDebugPhase * 8.5, wallWaterDebugActive);
        [loop]
        for (int wf = 0; wf < 11; ++wf)
        {
            float fi = (float)wf;
            float r0 = Hash21(float2(seed * 47.0 + fi, 3.0));
            float r1 = Hash21(float2(seed * 31.0, fi + 5.0));
            float r2 = Hash21(float2(fi + 9.0, seed * 71.0));
            float clusterCount = 2.0 + floor(Hash21(float2(seed * 83.0, 19.0)) * 2.0);
            float clusterId = floor(fmod(fi + floor(seed * 19.0), clusterCount));
            float uniformCenter = 0.070 + ((fi + 0.25 + r0 * 0.50) / 11.0) * 0.86;
            float clusterCenter = 0.08 + Hash21(float2(seed * 89.0 + clusterId * 5.7, 13.0)) * 0.84;
            float center = clamp(lerp(uniformCenter, clusterCenter + (r1 - 0.5) * 0.14, 0.46 + r2 * 0.22), 0.045, 0.955);
            float enabled = step(0.18, Hash21(float2(seed * 109.0 + fi, 41.0)));
            float len = 1.035 + r1 * 0.105;
            float flowDelay = r0 * 1.30 + fi * (0.045 + r2 * 0.030);
            float flowAge = max(0.0, wallWaterBaseAge - flowDelay);
            float speedPhase = flowAge * (0.58 + r2 * 0.74) + seed * 19.0 + fi * 1.73;
            float flowClock = max(0.0, flowAge * (1.08 + r1 * 0.28) +
                sin(speedPhase) * (0.10 + r0 * 0.09) +
                sin(speedPhase * 2.11 + r2 * 5.0) * 0.030);
            float flowGrow = smoothstep(0.0, 1.0, saturate(flowClock * (0.42 + r1 * 0.14)));
            float dynamicLen = lerp(0.075 + r2 * 0.055, len, flowGrow);
            float flowReady = smoothstep(0.02, 0.34 + r0 * 0.24, flowAge);
            float width = 0.0040 + r2 * 0.0075;
            float wander = (r1 - 0.5) * wallFlowY * (0.020 + r2 * 0.028) +
                (Fbm3(float3(wallFlowY * 5.2 + fi, seed * 11.0, 2.0)) - 0.5) * (0.007 + r0 * 0.010);
            float du = uv.x - center - wander;
            float trailGate = (1.0 - smoothstep(dynamicLen, dynamicLen + 0.045, wallFlowY)) *
                smoothstep(0.000, 0.035 + r0 * 0.025, wallFlowY);
            float breakNoise = smoothstep(0.16, 0.70,
                Fbm3(float3(uv.x * 28.0 + fi * 1.7, wallFlowY * 16.0, seed * 47.0)) + r2 * 0.18);
            float continuousFlow = lerp(breakNoise, max(breakNoise, 0.58 + r2 * 0.18),
                smoothstep(0.34, 0.96, wallFlowY) * flowGrow);
            float gravityWidth = width * (0.72 + smoothstep(0.22, 0.92, wallFlowY) * (0.36 + r1 * 0.44));
            float coreTrail = exp(-(du * du) / max(0.000012, gravityWidth * gravityWidth)) *
                trailGate * continuousFlow * enabled * wallFromCeiling * flowReady * (0.62 + r0 * 0.55);
            float haloTrail = exp(-(du * du) / max(0.000035, gravityWidth * gravityWidth * 18.0)) *
                trailGate * enabled * wallFromCeiling * flowReady * (0.35 + r1 * 0.30);
            float floorContact = exp(-(du * du) / max(0.000035, gravityWidth * gravityWidth * 22.0)) *
                smoothstep(0.82, 0.998, wallFlowY) * flowGrow * enabled * wallFromCeiling * flowReady;
            float sourceWidth = width * (3.8 + r1 * 2.5);
            float sourcePool = exp(-(du * du) / max(0.00005, sourceWidth * sourceWidth)) *
                (1.0 - smoothstep(0.060 + r2 * 0.045, 0.185 + r2 * 0.060, wallFlowY)) *
                enabled * wallFromCeiling * flowReady * (0.32 + r0 * 0.38);
            wallWaterCore = max(wallWaterCore, max(coreTrail, floorContact * 0.32));
            wallWaterHalo = max(wallWaterHalo, max(max(haloTrail * 0.72, sourcePool), floorContact * 0.62));
            wallWaterSoak += saturate(haloTrail * 0.36 + sourcePool * 0.24 + floorContact * 0.12);
        }
        wallWaterSoak = smoothstep(0.18, 1.05, wallWaterSoak) *
            smoothstep(0.10, 0.82, broad + wallWaterHalo * 0.38) *
            wallCardSideFade * wallEndFade;
        wallWaterHalo = saturate(max(wallWaterHalo * wallCardSideFade * wallEndFade, wallWaterSoak * 0.58));
        wallWaterCore *= wallCardSideFade * wallEndFade;
        float bottomDist = 1.0 - uv.y;
        float bottomNoise = (Fbm3(float3(uv.x * 9.0 + seed * 7.0, bottomDist * 5.0, seed * 91.0)) - 0.5) * 0.055;
        wallBottomSoak = wallFromFloor * wallCardSideFade *
            (1.0 - smoothstep(0.10 + bottomNoise, 0.34 + bottomNoise, bottomDist)) *
            smoothstep(0.18, 0.78, broad + fine * 0.16);
        float wallWetShape = saturate(max(max(wallWaterHalo * 0.72, wallWaterCore * 1.15) * wallSource,
            wallBottomSoak * 0.72));
        float shape = lerp(horizontal, wallWetShape, vertical);

)" R"(
        float bridgeBorder = min(min(uv.x, 1.0 - uv.x), min(uv.y, 1.0 - uv.y));
        float bridgeFilm = smoothstep(0.010, 0.075, bridgeBorder) *
            (0.86 + broad * 0.14);
        shape = max(shape, bridgeFilm * floorBridge);
        float edgeAwareBorder = border;
        if (edgeMode > 0.001)
        {
            if (sidePick < 1.0)
            {
                edgeAwareBorder = min(min(uv.x, 1.0 - uv.x), 1.0 - uv.y);
            }
            else if (sidePick < 2.0)
            {
                edgeAwareBorder = min(min(uv.x, 1.0 - uv.x), uv.y);
            }
            else if (sidePick < 3.0)
            {
                edgeAwareBorder = min(min(uv.y, 1.0 - uv.y), 1.0 - uv.x);
            }
            else
            {
                edgeAwareBorder = min(min(uv.y, 1.0 - uv.y), uv.x);
            }
        }
        float mergedFloorBorder = min(
            min(lerp(uv.x, 1.0, floorMergeW), lerp(1.0 - uv.x, 1.0, floorMergeE)),
            min(lerp(uv.y, 1.0, floorMergeS), lerp(1.0 - uv.y, 1.0, floorMergeN)));
        float floorMergeCoverage = saturate(max(floorSeamShape, floorCornerShape) * 1.65);
        float floorEdgeBorder = lerp(edgeAwareBorder, max(edgeAwareBorder, mergedFloorBorder), floorSurface * floorMergeCoverage);
        float borderNoise = (Fbm3(float3(uv * 11.0 + seed * 23.0, seed * 67.0)) - 0.5) * 0.032;
        float floorBorder = smoothstep(0.030 + borderNoise * 0.65, 0.155 + borderNoise * 1.15, floorEdgeBorder);
        float ceilingMergeCoverage = saturate(max(ceilingSeamShape, ceilingCornerShape) * 1.55);
        float ceilingEdgeAwareBorder = lerp(edgeAwareBorder, max(edgeAwareBorder, mergedFloorBorder), ceilingSurface * ceilingMergeCoverage);
        float ceilingSoftBorder = smoothstep(0.014 + borderNoise, 0.165 + borderNoise, ceilingEdgeAwareBorder);
        float ceilingEdgeBorder = smoothstep(0.006 + borderNoise * 0.45, 0.095 + borderNoise * 0.60, ceilingEdgeAwareBorder);
        float ceilingBorder = lerp(ceilingSoftBorder, ceilingEdgeBorder, edgeMode);
        float cardBorder = lerp(ceilingBorder, floorBorder, floorSurface);
        float verticalBorder = smoothstep(0.006, 0.14, border);
        float wallSideBorder = smoothstep(0.006, 0.14, min(uv.x, 1.0 - uv.x));
        verticalBorder = lerp(verticalBorder, wallSideBorder, saturate(wallFromCeiling + wallFromFloor));
        shape *= lerp(lerp(cardBorder, 1.0, floorBridge), verticalBorder, vertical);
        float debugLoopActive = step(1.0, gTransition0.w);
        float debugPhase = frac(gTransition0.w);
        float debugSpread = saturate((debugPhase - 0.05) / 0.70);
        float debugFade = 1.0 - smoothstep(0.88, 0.98, debugPhase);
        float2 debugCenterTile = floor(gMaze1.xy * 0.5) + 0.5;
        float2 debugCenterXZ = gMaze0.xy + debugCenterTile * gMaze0.zw;
        float2 debugTileDelta = (input.worldPos.xz - debugCenterXZ) / max(gMaze0.zw, float2(0.001, 0.001));
        float debugRadius = max(0.80, (max(gMaze1.x, gMaze1.y) - 2.0) * 0.58);
        float debugDist = lerp(length(debugTileDelta * float2(0.82, 0.82)), lerp(uv.y, 1.0 - uv.y, wallFromFloor), vertical);
        float debugEdgeNoise = (fine - 0.5) * 0.13 + (broad - 0.5) * 0.07;
        float debugReveal = (1.0 - smoothstep(debugSpread * debugRadius, debugSpread * debugRadius + 0.42, debugDist + debugEdgeNoise)) * debugFade;
        shape *= lerp(1.0, debugReveal, debugLoopActive);
        float touchdownGrow = smoothstep(0.46, 0.66, debugPhase);
        shape *= lerp(1.0, touchdownGrow, debugLoopActive * floorTouchdown);
        if (shape < 0.045) discard;

        float core = smoothstep(0.13, 0.82, shape);
        core = max(core, wallWaterCore * vertical);
        float rim = smoothstep(0.035, 0.18, shape) * (1.0 - smoothstep(0.38, 0.74, shape));
        float3 wetN = normalize(N + T * (fine - 0.5) * 0.035 + B * (broad - 0.5) * 0.026);
        float flashlight = FlashlightAmount(input.worldPos, wetN);
        float overhead = LocalLampLight(input.worldPos, wetN, time) * gLighting1.x;
        float sparkLight = SparkLight(input.worldPos, wetN);
        float3 exitGreen = ExitSignLight(input.worldPos, wetN);
        float exitGlow = max(exitGreen.r, max(exitGreen.g, exitGreen.b));
        float3 toLight = normalize(gShadow0.xyz - input.worldPos);
        float facing = saturate(dot(reflect(-toLight, wetN), V));
        float fresnel = pow(1.0 - saturate(dot(wetN, V)), 5.0);
        float specLight = saturate(flashlight + overhead * 0.58 + sparkLight * 0.42 + exitGlow * 0.54);
        float spec = (pow(facing, 110.0) * 0.30 + pow(facing, 30.0) * 0.052 + fresnel * 0.014) * specLight * core;
        float filmAlpha = saturate(core * (0.40 + broad * 0.145) + rim * (0.105 + fine * 0.050) +
            vertical * (wallWaterCore * 0.16 + wallWaterSoak * 0.07 + wallBottomSoak * 0.10));
        filmAlpha *= lerp(1.12, 0.82, vertical);
        if (filmAlpha < 0.035) discard;
        float3 wetFilm = float3(0.0018, 0.0024, 0.0022) * (0.26 + specLight * 0.13);
        float3 color = wetFilm + float3(0.36, 0.42, 0.39) * spec;
        float wallFlow = vertical * saturate(wallWaterCore + wallWaterSoak * 0.42);
        float wallDamp = vertical * wallBottomSoak;
        color = lerp(color, float3(0.0007, 0.0012, 0.0010) * (0.50 + specLight * 0.24), wallFlow * 0.76);
        color = lerp(color, float3(0.0008, 0.0011, 0.0010) * (0.38 + specLight * 0.16), wallDamp * 0.42);
        color += float3(0.28, 0.36, 0.33) * spec * wallFlow * 0.72;
        color += exitGreen * (0.055 + core * 0.12 + spec * 0.55);
        float dist = length(input.worldPos - cam);
        float fog = saturate((dist - gFog0.x) / max(0.01, gFog0.y - gFog0.x));
        fog = 1.0 - exp(-fog * fog * 3.2);
        float fogBlock = saturate(fog * gFog0.z * 1.28);
        color = lerp(color, float3(0.0, 0.0, 0.0), fogBlock);
        filmAlpha *= pow(1.0 - fogBlock, 1.35);
        return float4(ApplyPost(color), filmAlpha);
    }

)" R"(

    if (gHorror0.x > 0.01 && materialId < 11.5 && !(materialId > 3.5 && materialId < 4.5))
    {
        float flesh = saturate(gHorror0.x);
        float3 viewTS = float3(dot(V, T), dot(V, B), max(dot(V, N), 0.12));
        float2 fleshUv = rawUv * 0.72 + float2(Hash21(floor(input.worldPos.xz * 0.27)), Hash21(floor(input.worldPos.zx * 0.31))) * 0.23;
        float parallaxScale = gHorror0.w * (0.55 + flesh * 0.45);
        float layers = lerp(18.0, 9.0, saturate(viewTS.z));
        float2 stepUv = (viewTS.xy / max(viewTS.z, 0.12)) * (parallaxScale / layers);
        float2 pomUv = fleshUv;
        float layerDepth = 0.0;
        float currentDepth = 1.0 - gNormalHeight.Sample(gSampler, float3(pomUv, 15.0)).a;
        [loop]
        for (int p = 0; p < 18; ++p)
        {
            if ((float)p >= layers || layerDepth >= currentDepth) break;
            pomUv -= stepUv;
            layerDepth += 1.0 / layers;
            currentDepth = 1.0 - gNormalHeight.Sample(gSampler, float3(pomUv, 15.0)).a;
        }
        fleshUv = lerp(fleshUv, pomUv, saturate(parallaxScale * 18.0));
        float3 materialUv = float3(fleshUv, 15.0);
        float4 base = gAlbedo.Sample(gSampler, materialUv);
        float4 nh = gNormalHeight.Sample(gSampler, materialUv);
        float4 pbr = gMaterialProps.Sample(gSampler, materialUv);
        float aoMap = saturate(pbr.r);
        float sourceRoughness = saturate(pbr.g);
        float3 nTex = normalize(nh.xyz * 2.0 - 1.0);
        nTex = normalize(float3(nTex.xy * (1.28 + flesh * 0.42), nTex.z));
        float3 worldN = normalize(nTex.x * T + nTex.y * B + nTex.z * N);
        float dist = length(input.worldPos - cam);
        float flashlight = FlashlightAmount(input.worldPos, worldN);
        float3 toLight = normalize(gShadow0.xyz - input.worldPos);
        float3 reflectDir = reflect(-toLight, worldN);
        float wet = saturate(gHorror0.z);
        float ndv = saturate(dot(worldN, V));
        float fresnel = pow(1.0 - ndv, 4.0);
        float facing = saturate(dot(reflectDir, V));
        float effectiveRoughness = lerp(sourceRoughness, max(0.18, sourceRoughness * 0.42), wet);
        effectiveRoughness = saturate(effectiveRoughness - wet * 0.055);
        float gloss = 1.0 - effectiveRoughness;
        float specSharp = pow(facing, lerp(24.0, 150.0, gloss)) * lerp(0.42, 2.7, gloss);
        float specBroad = pow(facing, lerp(5.0, 22.0, gloss)) * lerp(0.48, 0.16, gloss);
        float poreSparkle = smoothstep(0.58, 0.92, Fbm3(float3(fleshUv * 42.0, 17.0))) * wet * gloss;
        float spec = (specSharp + specBroad + fresnel * (0.20 + wet * 0.36) + poreSparkle * 0.36) *
            flashlight * (0.75 + wet * 3.3 + flesh * 0.85);
        float cavity = saturate((0.58 - nh.a) * 1.9);
        float ridge = saturate((nh.a - 0.48) * 1.7);
        float3 fleshColor = base.rgb * (0.48 + flesh * 0.22) + float3(0.10, 0.005, 0.003) * flesh;
        float aoShadow = lerp(0.34, 1.0, aoMap);
        fleshColor = lerp(fleshColor, fleshColor * float3(0.28, 0.08, 0.07), saturate(cavity * 0.62 + (1.0 - aoMap) * 0.54));
        fleshColor += float3(0.085, 0.014, 0.008) * ridge * wet;
        float3 color = fleshColor * flashlight * 0.94 * aoShadow;
        color += float3(1.0, 0.21, 0.085) * spec * lerp(0.62, 0.98, aoMap);
        color *= 1.0 - CornerAO(input.worldPos, worldN) * 0.65;
        color = lerp(color, float3(0.0, 0.0, 0.0), SceneFogBlock(dist, input.worldPos, 1.0));
        return float4(ApplyPost(color), 1.0);
    }

    if ((materialId > 2.5 && materialId < 3.5) || (materialId > 4.5 && materialId < 5.5))
    {
        float edge = max(smoothstep(0.055, 0.0, min(uv.x, 1.0 - uv.x)),
                         smoothstep(0.055, 0.0, min(uv.y, 1.0 - uv.y)));
        float lens = smoothstep(0.42, 0.0, abs(uv.y - 0.5)) * smoothstep(0.46, 0.0, abs(uv.x - 0.5));
        float3 lampBase = float3(0.72 + lens * 0.24 - edge * 0.18,
                                 0.76 + lens * 0.23 - edge * 0.16,
                                 0.70 + lens * 0.20 - edge * 0.12);
        float3 offBase = float3(0.91 + lens * 0.07 - edge * 0.055,
                                0.92 + lens * 0.07 - edge * 0.050,
                                0.86 + lens * 0.06 - edge * 0.045);
        float3 base = materialId < 3.5 ? lampBase : offBase;
        float emit = materialId < 3.5 ? LampVisualPower(input.material, input.worldPos, time) * 2.6 * (1.0 - saturate(gTransition0.z)) : 0.0;
        float passiveLight = materialId < 3.5
            ? gLighting0.z
            : gLighting0.z * 0.48 +
              FlashlightAmount(input.worldPos, N) * 0.86 +
              LocalLampLight(input.worldPos, N, time) * gLighting1.x * 0.22;
        float3 color = base * (passiveLight + emit);
        float fog = saturate((length(input.worldPos - cam) - gFog0.x) / max(0.01, gFog0.y - gFog0.x));
        fog = 1.0 - exp(-fog * fog * 3.2);
        color = lerp(color, float3(0.0, 0.0, 0.0), fog * gFog0.z);
        return float4(ApplyPost(color), 1.0);
    }

    if (materialId > 11.5 && materialId < 12.5)
    {
        float dist = length(input.worldPos - cam);
        float fog = saturate((dist - gFog0.x) / max(0.01, gFog0.y - gFog0.x));
        float variant = frac(input.material);
        if (variant > 0.55)
        {
            float2 d = uv - 0.5;
            float radial = exp(-dot(d, d) * 5.4);
            float curl = Hash21(floor((uv + time * float2(0.05, -0.09)) * 9.0 + input.material * 23.0));
            float life = saturate((variant - 0.55) / 0.40);
            float alpha = radial * (0.18 + curl * 0.16) * life;
            if (alpha < 0.018) discard;
            float flashlight = FlashlightAmount(input.worldPos, N);
            float nonFleshLight = 1.0 - saturate(gTransition0.z);
            float3 color = float3(0.46, 0.50, 0.47) * (flashlight * 0.14 + (0.18 + gLighting0.z) * nonFleshLight);
            color = lerp(color, float3(0.0, 0.0, 0.0), fog * gFog0.z * 0.75);
            return float4(ApplyPost(color), alpha);
        }

        float3 materialUv = MaterialUV(rawUv, input.material);
        float4 base = gAlbedo.Sample(gSampler, materialUv);
        if (base.a < 0.03) discard;
        fog = 1.0 - exp(-fog * fog * 1.4);
        float pulse = 0.96 + 0.04 * sin(time * 7.1 + input.material * 17.0);
        float nonFleshLight = 1.0 - saturate(gTransition0.z);
        if (nonFleshLight < 0.001) discard;
        float3 color = (float3(13.0, 0.35, 0.10) * base.a * pulse + base.rgb * 3.2) * nonFleshLight;
        float fogBlock = saturate(fog * gFog0.z * 1.55);
        float fogVisibility = pow(1.0 - fogBlock, 2.7);
        color *= fogVisibility;
        return float4(saturate(ApplyPost(color) + color * 0.35 * fogVisibility), saturate(base.a * 1.55 * fogVisibility));
    }

    if (materialId > 12.5 && materialId < 13.5)
    {
        float2 d = uv - 0.5;
        float r = length(d);
        float r2 = dot(d, d);
        float variant = frac(input.material);
        float core = exp(-r2 * 86.0);
        float halo = exp(-r2 * 28.0);
        float roundMask = smoothstep(0.47, 0.18, r);
        float alpha = saturate(core * 1.18 + halo * 0.44) * roundMask * (0.70 + variant * 0.62);
        if (alpha < 0.016) discard;
        float nonFleshLight = 1.0 - saturate(gTransition0.z);
        if (nonFleshLight < 0.001) discard;
        float3 color = (float3(8.4, 3.7, 0.72) * core + float3(2.8, 0.78, 0.12) * halo) * nonFleshLight;
        float dist = length(input.worldPos - cam);
        float fog = saturate((dist - gFog0.x) / max(0.01, gFog0.y - gFog0.x));
        fog = 1.0 - exp(-fog * fog * 1.4);
        color = lerp(color, float3(0.0, 0.0, 0.0), fog * gFog0.z * 0.24);
        return float4(saturate(ApplyPost(color) + color * 0.18), saturate(alpha));
    }

    if (materialId > 14.5 && materialId < 15.5)
    {
        float variant = frac(input.material);
        float particleFade = smoothstep(0.055, 0.24, variant);
        float3 toLight = input.worldPos - gShadow0.xyz;
        float lightDist = length(toLight);
        float focus = max(0.45, gAir0.x);
        float blur = saturate(abs(lightDist - focus) / (0.62 + lightDist * 0.18)) * saturate(gAir0.y);
        float2 p = uv * 2.0 - 1.0;
        float angle = atan2(p.y, p.x);
        float3 stable = floor(input.worldPos * 2.7 + variant * 31.0);
        float h0 = Hash31(stable + 3.0);
        float h1 = Hash31(stable.yzx + 17.0);
        float h2 = Hash31(stable.zxy + 41.0);
        float strandAngle = h0 * 6.2831853;
        float2 strandDir = float2(cos(strandAngle), sin(strandAngle));
        float2 strandPerp = float2(-strandDir.y, strandDir.x);
        float strandX = dot(p, strandDir);
        float strandY = dot(p, strandPerp);
        float strandTaper = 1.0 - smoothstep(0.42 + h1 * 0.22, 1.05 + h2 * 0.16, abs(strandX));
        float waviness = sin(strandX * (10.0 + h0 * 11.0) + variant * 31.0 + time * 0.05) * (0.020 + h1 * 0.026);
        float hairA = exp(-pow(abs(strandY - waviness), 1.35) * (74.0 + h2 * 86.0)) * strandTaper;
        float hairB = exp(-pow(abs(strandY - 0.075 - waviness * 0.55), 1.28) * (96.0 + h0 * 72.0)) *
            (1.0 - smoothstep(0.25 + h2 * 0.16, 0.92, abs(strandX + 0.14)));
        float hairC = exp(-pow(abs(strandY + 0.060 + waviness * 0.70), 1.32) * (110.0 + h1 * 60.0)) *
            (1.0 - smoothstep(0.18 + h0 * 0.22, 0.86, abs(strandX - 0.10)));
        float clumpBreak = smoothstep(0.18, 0.72, Fbm3(float3(p * (8.0 + h2 * 7.0) + variant * 17.0, variant * 53.0)));
        float hairClump = max(hairA, max(hairB * 0.76, hairC * 0.62)) * lerp(0.54, 1.0, clumpBreak);
        float2 q = p + strandPerp * waviness * 0.45;
        float lobesA = sin(angle * (5.0 + floor(h0 * 4.0)) + variant * 38.0 + time * 0.035);
        float lobesB = sin(angle * (9.0 + floor(h1 * 5.0)) + variant * 71.0 - time * 0.026);
        float corner = sin(angle * (13.0 + floor(h2 * 4.0)) + h1 * 19.0);
        float sides = 5.0 + floor(h0 * 6.0);
        float sector = floor((angle + 3.14159265 + variant * 6.2831853) / (6.2831853 / sides));
        float faceted = (Hash21(float2(sector, variant * 43.0)) - 0.5) * 0.22;
        float edge = 0.58 + faceted + lobesA * 0.13 + lobesB * 0.08 + corner * 0.045;
        float r = length(q);
        float blob = smoothstep(edge + 0.13 + blur * 0.24, edge - 0.06 - blur * 0.10, r);
        float shell = smoothstep(edge + 0.24 + blur * 0.26, edge + 0.02, r) * (1.0 - blob);
        float holes = step(0.60 + blur * 0.18, Hash21(floor((q + variant) * (7.0 + h0 * 8.0))));
        blob *= lerp(1.0, 0.52, holes * (1.0 - blur * 0.45));
        float shape = max(blob * 0.82, max(shell * 0.24, hairClump * (0.46 + h1 * 0.42)));
        float flecks = lerp(0.78, 1.10, Hash21(floor(q * (10.0 + h1 * 9.0)) + variant * 23.0));
        shape *= flecks * smoothstep(1.22, 0.35, length(p));
        if (shape < 0.018) discard;

        float flashlight = FlashlightAmount(input.worldPos, N);
        float3 lightDir = normalize(gShadow1.xyz);
        float axisDist = max(0.0, dot(toLight, lightDir));
        float outerCos = clamp(gShadow2.z, 0.04, 0.98);
        float coneRadius = max(0.018, axisDist * sqrt(max(0.0, 1.0 - outerCos * outerCos)) / outerCos);
        float radialDist = length(toLight - lightDir * axisDist);
        float centerLine = 1.0 - smoothstep(0.08, 0.84, radialDist / coneRadius);
        float flashlightScale = sqrt(max(0.0, gLighting0.x));
        float centerBoost = (0.76 + centerLine * 0.70) * lerp(0.72, 1.24, saturate(flashlightScale * 0.78));
        float lightFade = smoothstep(0.35, 1.10, lightDist) * (1.0 - smoothstep(gShadow2.y * 0.46, gShadow2.y * 0.82, lightDist));
        float depthFade = 1.0 - smoothstep(gFog0.y * 0.72, gFog0.y, length(input.worldPos - cam));
        float focusAlpha = lerp(1.0, 0.46, blur);
        float alpha = shape * flashlight * centerBoost * lightFade * depthFade * focusAlpha * (0.20 + variant * 0.12) * particleFade * (1.0 - saturate(gTransition0.z));
        if (alpha < 0.010) discard;
        float3 color = float3(0.72, 0.78, 0.72) * (0.20 + flashlight * (1.08 + centerLine * 0.82));
        color += float3(0.42, 0.48, 0.44) * shell * (0.08 + blur * 0.08 + centerLine * 0.08) * flashlightScale;
        float fog = saturate((length(input.worldPos - cam) - gFog0.x) / max(0.01, gFog0.y - gFog0.x));
        fog = 1.0 - exp(-fog * fog * 2.2);
        color = lerp(color, float3(0.0, 0.0, 0.0), fog * gFog0.z * 0.65);
        return float4(ApplyPost(color), saturate(alpha));
    }

    if (materialId > 17.5 && materialId < 18.5)
    {
        float4 label = gAlbedo.Sample(gSampler, float3(uv, 18.0));
        if (label.a < 0.025) discard;
        float hover = smoothstep(0.35, 0.60, frac(input.material));
        float pulse = lerp(0.72, 0.92 + 0.08 * sin(time * 4.8 + input.worldPos.x * 2.1), hover);
        float3 glow = label.rgb * lerp(1.35 + label.a * 0.80, 4.2 + label.a * 5.8, hover) * pulse;
        float dist = length(input.worldPos - cam);
        float fog = saturate((dist - gFog0.x) / max(0.01, gFog0.y - gFog0.x));
        float fogVisibility = pow(1.0 - saturate(fog * gFog0.z), 2.2);
        glow *= fogVisibility;
        return float4(saturate(ApplyPost(glow) + glow * lerp(0.02, 0.18, hover)), saturate(label.a * lerp(0.95, 1.55, hover) * fogVisibility));
    }

    if (materialId > 18.5 && materialId < 19.5)
    {
        float strength = smoothstep(0.58, 0.94, frac(input.material));
        float2 p = uv * 2.0 - 1.0;
        float edge = smoothstep(1.20, 0.10, abs(p.x)) * smoothstep(1.20, 0.02, abs(p.y));
        float haze = Fbm3(float3(input.worldPos.xz * 1.35 + p * 0.34, time * 0.045));
        float streak = smoothstep(1.02, 0.16, abs(p.x + (haze - 0.5) * 0.34));
        float dist = length(input.worldPos - cam);
        float fogVisibility = pow(1.0 - SceneFogBlock(dist, input.worldPos, 0.22), 1.12);
        float alpha = edge * lerp(0.16, 0.42, strength) * (0.70 + streak * 0.62) * fogVisibility;
        if (alpha < 0.008) discard;
        float3 color = float3(0.94, 0.97, 1.0) * (3.6 + strength * 7.8) * (0.82 + haze * 0.30);
        return float4(saturate(ApplyPost(color) + color * 0.14), saturate(alpha));
    }

    if (input.material > 10.50 && input.material < 10.90)
    {
        float ndv = saturate(dot(N, V));
        float rim = pow(1.0 - ndv, 2.2);
        float hot = 0.82 + pow(ndv, 0.45) * 0.95 + rim * 0.34;
        float flutter = 0.96 + 0.04 * sin(time * 6.2 + input.material * 19.0);
        float bloodMaterial = smoothstep(0.79, 0.83, frac(input.material));
        float3 hotBase = lerp(float3(8.8, 0.28, 0.075), float3(5.6, 0.025, 0.012), bloodMaterial);
        float3 rimBase = lerp(float3(3.8, 0.065, 0.018), float3(1.9, 0.006, 0.003), bloodMaterial);
        float3 color = hotBase * hot * flutter;
        color += rimBase * rim;
        float dist = length(input.worldPos - cam);
        float fog = saturate((dist - gFog0.x) / max(0.01, gFog0.y - gFog0.x));
        fog = 1.0 - exp(-fog * fog * 1.8);
        float fogBlock = saturate(fog * gFog0.z * 1.55);
        float fogVisibility = pow(1.0 - fogBlock, 2.7);
        color *= fogVisibility;
        return float4(saturate(ApplyPost(color) + color * 0.32 * fogVisibility), fogVisibility);
    }

)" R"(
    if (materialId > 8.5 && materialId < 9.5 && frac(input.material) > 0.5)
    {
        float grain = Fbm3(input.worldPos * float3(12.0, 18.0, 12.0) + 2.3);
        float stain = Fbm3(input.worldPos * float3(4.0, 7.0, 4.0) + 18.0);
        float ridge = Fbm3(input.worldPos * float3(38.0, 24.0, 38.0) + 41.0);
        float3 worldN = normalize(N + T * (grain - 0.5) * 0.10 + B * (ridge - 0.5) * 0.055);
        float flashlight = FlashlightAmount(input.worldPos, worldN);
        float overhead = LocalLampLight(input.worldPos, worldN, time) * gLighting1.x;
        float sparkLight = SparkLight(input.worldPos, worldN);
        float3 bone = float3(0.62, 0.58, 0.48);
        bone += (grain - 0.5) * float3(0.10, 0.085, 0.055);
        bone -= smoothstep(0.54, 0.88, stain) * float3(0.13, 0.12, 0.090);
        float3 toLight = normalize(gShadow0.xyz - input.worldPos);
        float facing = saturate(dot(reflect(-toLight, worldN), V));
        float spec = pow(facing, 34.0) * 0.22 * (flashlight + sparkLight * 0.45);
        float dist = length(input.worldPos - cam);
        float3 color = bone * (gLighting0.z * 0.34 + flashlight * 1.12 + overhead * 0.26 + sparkLight * 0.80);
        color += float3(0.78, 0.68, 0.48) * spec;
        color = lerp(color, float3(0.0, 0.0, 0.0), SceneFogBlock(dist, input.worldPos, 1.0));
        return float4(ApplyPost(color), 1.0);
    }

    if (materialId > 23.5 && materialId < 24.5)
    {
        float softness = saturate(frac(input.material));
        float2 p = uv * 2.0 - 1.0;
        float edgeNoise = (Fbm3(float3(input.worldPos.xz * (2.1 + softness * 1.4), softness * 17.0)) - 0.5) * (0.055 + softness * 0.070);
        float fineBreakup = Fbm3(float3(input.worldPos.xz * 8.5 + softness * 13.0, softness * 31.0));
        float r = length(p * float2(0.84, 1.16));
        float contact = 1.0 - smoothstep(0.16, 0.68 + softness * 0.24, r + edgeNoise * 0.28);
        float feather = 1.0 - smoothstep(0.26 + softness * 0.10, 1.16 + softness * 0.44, r + edgeNoise);
        float shape = saturate(max(contact * (0.26 - softness * 0.075), feather * 0.86));
        shape *= 0.82 + fineBreakup * 0.18;
        float localFixturePower = FixturePower(input.worldPos, time) * gLighting1.x;
        float flickerLinkedShadow = saturate(localFixturePower * lerp(0.42, 0.32, softness));
        float alpha = shape * flickerLinkedShadow * lerp(0.125, 0.070, softness) * (1.0 - saturate(gTransition0.z));
        if (alpha < 0.006) discard;
        return float4(0.0, 0.0, 0.0, alpha);
    }

    if ((materialId > 0.5 && materialId < 1.5) || (materialId > 1.5 && materialId < 2.5))
    {
        float floorMaterial = materialId < 1.5;
        float mipBias = floorMaterial > 0.5 ? 1.15 : 0.75;
        float3 materialUv = MaterialUV(rawUv, input.material);
        float4 base = gAlbedo.SampleBias(gSampler, materialUv, mipBias);
        base.rgb = BackroomsBaseColor(base.rgb, materialId);
        float4 pbr = gMaterialProps.SampleBias(gSampler, materialUv, mipBias);
        float4 nh = gNormalHeight.SampleBias(gSampler, materialUv, mipBias + 0.55);
        float3 nTex = normalize(nh.xyz * 2.0 - 1.0);
        float normalStrength = floorMaterial > 0.5 ? 0.055 : 0.135;
        nTex = normalize(float3(nTex.xy * normalStrength, nTex.z));
        float3 worldN = normalize(nTex.x * T + nTex.y * B + nTex.z * N);
        float dist = length(input.worldPos - cam);
        float flashlight = FlashlightAmount(input.worldPos, worldN);
        float overhead = LocalLampLight(input.worldPos, worldN, time) * gLighting1.x;
        float sparkLight = SparkLight(input.worldPos, worldN);
        float3 exitGreen = ExitSignLight(input.worldPos, worldN);
        float exitGlow = max(exitGreen.r, max(exitGreen.g, exitGreen.b));
        float lift = gLighting0.z * (floorMaterial > 0.5 ? 0.018 : 0.040) * (1.0 - saturate(gTransition0.z));
        float aoMap = saturate(pbr.r);
        float roughness = saturate(pbr.g);
        float3 color = base.rgb * (gLighting0.z + overhead + flashlight + sparkLight + lift) * lerp(0.55, 1.0, aoMap);
        color += base.rgb * exitGreen * lerp(0.55, 1.0, aoMap);
        float3 toLight = normalize(gShadow0.xyz - input.worldPos);
        float specFacing = saturate(dot(reflect(-toLight, worldN), V));
        float gloss = 1.0 - roughness;
        float specSharpness = lerp(12.0, 92.0, gloss);
        float specLight = flashlight + sparkLight * 0.55 + exitGlow * 0.42 + overhead * (floorMaterial > 0.5 ? 0.18 : 0.62);
        float ceilingSheen = floorMaterial > 0.5 ? 0.16 : 0.54;
        float surfaceSpec = pow(specFacing, specSharpness) * gloss * gloss * ceilingSheen * specLight;
        color += float3(1.0, 0.92, 0.76) * surfaceSpec * lerp(0.72, 1.0, aoMap);
        color *= 1.0 - CornerAO(input.worldPos, N);
        float fog = saturate((dist - gFog0.x) / max(0.01, gFog0.y - gFog0.x));
        fog = 1.0 - exp(-fog * fog * 3.2);
        color = lerp(color, float3(0.0, 0.0, 0.0), fog * gFog0.z);
        return float4(ApplyPost(color), 1.0);
    }

    if (materialId > 5.5 && materialId < 6.5)
    {
        float2 p = uv;
        float outerBevel = max(max(1.0 - smoothstep(0.0, 0.055, p.x),
                                   1.0 - smoothstep(0.0, 0.055, 1.0 - p.x)),
                               max(1.0 - smoothstep(0.0, 0.055, p.y),
                                   1.0 - smoothstep(0.0, 0.055, 1.0 - p.y)));
        float panelInset = max(1.0 - smoothstep(0.018, 0.050, abs(p.x - 0.18)),
                               1.0 - smoothstep(0.018, 0.050, abs(p.x - 0.82)));
        panelInset = max(panelInset, max(1.0 - smoothstep(0.018, 0.050, abs(p.y - 0.24)),
                                         1.0 - smoothstep(0.018, 0.050, abs(p.y - 0.76))));
        panelInset *= 0.35;
        float grain = Fbm3(float3(p * float2(3.4, 5.2), 4.7));
        float grime = Fbm3(float3(input.worldPos.xz * 0.7 + p * 0.55, 21.0));
        float3 base = float3(0.27, 0.205, 0.135);
        base += (grain - 0.5) * float3(0.018, 0.014, 0.010);
        base -= (outerBevel * 0.38 + panelInset) * float3(0.038, 0.030, 0.020);
        base -= smoothstep(0.72, 0.97, grime) * float3(0.026, 0.023, 0.017);
        float3 worldN = normalize(N + T * (grain - 0.5) * 0.012);
        float dist = length(input.worldPos - cam);
        float flashlight = FlashlightAmount(input.worldPos, worldN);
        float overhead = LocalLampLight(input.worldPos, worldN, time) * gLighting1.x;
        float sparkLight = SparkLight(input.worldPos, worldN);
        float3 exitGreen = ExitSignLight(input.worldPos, worldN);
        float3 color = base * (gLighting0.z + overhead * 0.88 + flashlight + sparkLight);
        color += base * exitGreen * 0.55;
        float3 toLight = normalize(gShadow0.xyz - input.worldPos);
        float facing = saturate(dot(reflect(-toLight, worldN), V));
        color += float3(0.75, 0.56, 0.34) * pow(facing, 54.0) * 0.075 * (flashlight + sparkLight * 0.45);
        color *= 1.0 - CornerAO(input.worldPos, worldN) * 0.55;
        color = lerp(color, float3(0.0, 0.0, 0.0), SceneFogBlock(dist, input.worldPos, 1.0));
        return float4(ApplyPost(color), 1.0);
    }

    float3 viewTS = float3(dot(V, T), dot(V, B), max(dot(V, N), 0.18));
    float parallaxScale = 0.0;
    if (materialId < 0.5) parallaxScale = 0.018;
    else if (materialId < 1.5) parallaxScale = 0.0;
    else if (materialId < 2.5) parallaxScale = 0.003;
    float2 sampledRawUv = rawUv;
    if (materialId > 6.5 && materialId < 7.5)
    {
        sampledRawUv.y = 1.0 - sampledRawUv.y;
    }
    float3 firstUv = MaterialUV(sampledRawUv, input.material);
    float height = parallaxScale > 0.0 ? gNormalHeight.Sample(gSampler, firstUv).a : 0.48;
    sampledRawUv += (height - 0.48) * parallaxScale * viewTS.xy / viewTS.z;

    float3 materialUv = MaterialUV(sampledRawUv, input.material);
    float floorMipBias = (materialId > 0.5 && materialId < 1.5) ? 1.75 : 0.0;
    float4 base = gAlbedo.SampleBias(gSampler, materialUv, floorMipBias);
    base.rgb = BackroomsBaseColor(base.rgb, materialId);
    float4 pbr = gMaterialProps.SampleBias(gSampler, materialUv, floorMipBias);
    if (materialId > 3.5 && base.a < 0.08) discard;

    float4 nh = gNormalHeight.SampleBias(gSampler, materialUv, floorMipBias);
    float3 nTex = normalize(nh.xyz * 2.0 - 1.0);
    float normalStrength = 0.55;
    if (materialId > 0.5 && materialId < 1.5) normalStrength = 0.0;
    else if (materialId > 1.5 && materialId < 2.5) normalStrength = 0.35;
    nTex = normalize(float3(nTex.xy * normalStrength, nTex.z));
    float3 worldN = normalize(nTex.x * T + nTex.y * B + nTex.z * N);

    float dist = length(input.worldPos - cam);
    float flashlight = FlashlightAmount(input.worldPos, worldN);
    float sparkLight = SparkLight(input.worldPos, worldN);
    float3 exitGreen = ExitSignLight(input.worldPos, worldN);
    float exitGlow = max(exitGreen.r, max(exitGreen.g, exitGreen.b));

    float fixture = FixturePower(input.worldPos, time);
    float overhead = LocalLampLight(input.worldPos, worldN, time) * gLighting1.x;

    float ambient = gLighting0.z;
    float aoMap = saturate(pbr.r);
    float roughness = saturate(pbr.g);
    float3 color = base.rgb * (ambient + overhead + flashlight + sparkLight) * lerp(0.58, 1.0, aoMap);
    color += base.rgb * exitGreen * lerp(0.58, 1.0, aoMap);
    float3 toLight = normalize(gShadow0.xyz - input.worldPos);
    float specFacing = saturate(dot(reflect(-toLight, worldN), V));
    float gloss = 1.0 - roughness;
    float surfaceSpec = pow(specFacing, lerp(18.0, 95.0, gloss)) * gloss * 0.18 * (flashlight + sparkLight * 0.5 + exitGlow * 0.45);
    color += float3(1.0, 0.92, 0.78) * surfaceSpec;
    if (materialId > 1.5 && materialId < 2.5)
    {
        color += base.rgb * 0.035 * (1.0 - saturate(gTransition0.z));
    }
    if (materialId > 6.5 && materialId < 7.5)
    {
        float nonFleshLight = 1.0 - saturate(gTransition0.z);
        color = base.rgb * ((0.28 + flashlight * 0.2) * nonFleshLight + flashlight * 0.12) + base.rgb * 1.7 * nonFleshLight;
    }
    color *= 1.0 - CornerAO(input.worldPos, worldN);
    float eyeGlow = 0.0;
    if (materialId > 3.5 && materialId < 4.5)
    {
        eyeGlow = saturate((base.r - max(base.g, base.b)) * 3.5);
        color += base.rgb * eyeGlow * 2.4;
    }

    float fogBlock = SceneFogBlock(dist, input.worldPos, 1.0);
    color = lerp(color, float3(0.0, 0.0, 0.0), fogBlock);
    float3 posted = ApplyPost(color);
    if (materialId > 3.5 && materialId < 4.5)
    {
        float eyeFogVisibility = pow(1.0 - saturate(fogBlock * 1.55), 2.7);
        eyeGlow *= eyeFogVisibility;
        float death = saturate(gPost0.w);
        float eyeHold = smoothstep(0.48, 0.72, death) * (1.0 - smoothstep(0.92, 1.0, death));
        posted += float3(eyeGlow * 1.7, eyeGlow * 0.06, eyeGlow * 0.025);
        posted += float3(eyeGlow * 4.2, eyeGlow * 0.12, eyeGlow * 0.04) * eyeHold;
    }
    return float4(saturate(posted), base.a);
}
)";

        ComPtr<ID3DBlob> vs;
        ComPtr<ID3DBlob> hs;
        ComPtr<ID3DBlob> ds;
        ComPtr<ID3DBlob> ps;
        ComPtr<ID3DBlob> shadowPs;
        static const char* overlayShader = R"(
struct OverlayVSIn
{
    float2 pos : POSITION;
    float4 color : COLOR0;
};

struct OverlayVSOut
{
    float4 pos : SV_POSITION;
    float4 color : COLOR0;
};

OverlayVSOut OverlayVS(OverlayVSIn input)
{
    OverlayVSOut o;
    o.pos = float4(input.pos, 0.0, 1.0);
    o.color = input.color;
    return o;
}

float4 OverlayPS(OverlayVSOut input) : SV_TARGET
{
    return input.color;
}
)";
        static const char* postShader = R"(
cbuffer SceneConstants : register(b0)
{
    row_major float4x4 gViewProj;
    row_major float4x4 gLightViewProj;
    float4 gCameraPosTime;
    float4 gCameraDirAspect;
    float4 gLighting0;
    float4 gLighting1;
    float4 gFog0;
    float4 gAO0;
    float4 gPost0;
    float4 gPost1;
};

Texture2D gSceneColor : register(t0);
SamplerState gPostSampler : register(s0);

struct PostVSOut
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};

PostVSOut PostVS(uint vertexId : SV_VertexID)
{
    PostVSOut o;
    float2 p = float2((vertexId << 1) & 2, vertexId & 2);
    o.uv = p;
    o.pos = float4(p * float2(2.0, -2.0) + float2(-1.0, 1.0), 0.0, 1.0);
    return o;
}

float DirtHash(float2 p)
{
    p = frac(p * float2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return frac(p.x * p.y);
}

float LensDirt(float2 uv)
{
    float2 centered = uv - 0.5;
    float vignette = smoothstep(0.18, 0.78, length(centered));
    float fine = DirtHash(floor(uv * 52.0)) * 0.55 + DirtHash(floor(uv * 137.0 + 19.0)) * 0.45;
    float smudgeA = smoothstep(0.36, 0.0, length((uv - float2(0.32, 0.42)) * float2(1.0, 1.8)));
    float smudgeB = smoothstep(0.42, 0.0, length((uv - float2(0.68, 0.58)) * float2(1.5, 0.9)));
    return saturate(vignette * 0.38 + pow(fine, 8.0) * 0.42 + smudgeA * 0.22 + smudgeB * 0.18);
}

float3 BrightPart(float3 c)
{
    float luma = dot(c, float3(0.299, 0.587, 0.114));
    float gate = smoothstep(0.58, 1.0, luma);
    return c * gate;
}

float4 PostPS(PostVSOut input) : SV_TARGET
{
    uint w;
    uint h;
    gSceneColor.GetDimensions(w, h);
    float2 texel = 1.0 / max(float2(w, h), float2(1.0, 1.0));
    float2 uv = input.uv;
    float danger = saturate(gPost0.z);
    float death = saturate(gPost0.w);
    float bloomAmount = saturate(gPost1.z);
    float dirtAmount = saturate(gPost1.w);
    float2 motion = clamp(gPost1.xy, float2(-0.045, -0.045), float2(0.045, 0.045));

    float3 color = gSceneColor.Sample(gPostSampler, uv).rgb;
    float3 blur = color * 0.36;
    blur += gSceneColor.Sample(gPostSampler, saturate(uv - motion * 0.35)).rgb * 0.22;
    blur += gSceneColor.Sample(gPostSampler, saturate(uv - motion * 0.72)).rgb * 0.17;
    blur += gSceneColor.Sample(gPostSampler, saturate(uv - motion * 1.08)).rgb * 0.11;
    blur += gSceneColor.Sample(gPostSampler, saturate(uv + motion * 0.30)).rgb * 0.14;
    float motionWeight = saturate(length(motion) * 42.0);
    color = lerp(color, blur, motionWeight * (0.34 + danger * 0.22));

    float2 bloomStep = texel * (2.0 + danger * 2.0);
    float3 bloom = BrightPart(color) * 0.42;
    bloom += BrightPart(gSceneColor.Sample(gPostSampler, uv + bloomStep * float2( 1.0,  0.0)).rgb) * 0.12;
    bloom += BrightPart(gSceneColor.Sample(gPostSampler, uv + bloomStep * float2(-1.0,  0.0)).rgb) * 0.12;
    bloom += BrightPart(gSceneColor.Sample(gPostSampler, uv + bloomStep * float2( 0.0,  1.0)).rgb) * 0.12;
    bloom += BrightPart(gSceneColor.Sample(gPostSampler, uv + bloomStep * float2( 0.0, -1.0)).rgb) * 0.12;
    bloom += BrightPart(gSceneColor.Sample(gPostSampler, uv + bloomStep * float2( 1.0,  1.0)).rgb) * 0.08;
    bloom += BrightPart(gSceneColor.Sample(gPostSampler, uv + bloomStep * float2(-1.0,  1.0)).rgb) * 0.08;
    bloom += BrightPart(gSceneColor.Sample(gPostSampler, uv + bloomStep * float2( 1.0, -1.0)).rgb) * 0.08;
    bloom += BrightPart(gSceneColor.Sample(gPostSampler, uv + bloomStep * float2(-1.0, -1.0)).rgb) * 0.08;
    float dirt = LensDirt(uv) * dirtAmount;
    color += bloom * bloomAmount * (0.12 + dirt * 0.55);
    color += float3(1.0, 0.92, 0.72) * dirt * bloomAmount * 0.018;
    color *= 1.0 - smoothstep(0.58, 1.04, length((uv - 0.5) * float2(gCameraDirAspect.w, 1.0))) * (0.055 + dirtAmount * 0.035);
    color = lerp(color, float3(0.0, 0.0, 0.0), smoothstep(0.82, 1.0, death));
    return float4(saturate(color), 1.0);
}
)";
        ComPtr<ID3DBlob> overlayVs;
        ComPtr<ID3DBlob> overlayPs;
        ComPtr<ID3DBlob> postVs;
        ComPtr<ID3DBlob> postPs;
        if (!CompileShader(shader, "VSMain", "vs_4_0", vs)) return false;
        if (featureLevel_ >= D3D_FEATURE_LEVEL_11_0) {
            if (!CompileShader(shader, "HSMain", "hs_5_0", hs)) return false;
            if (!CompileShader(shader, "DSMain", "ds_5_0", ds)) return false;
        }
        const char* mainPsProfile = featureLevel_ >= D3D_FEATURE_LEVEL_11_0 ? "ps_5_0" : "ps_4_0";
        if (!CompileShader(shader, "PSMain", mainPsProfile, ps)) return false;
        if (!CompileShader(shader, "ShadowPS", "ps_4_0", shadowPs)) return false;
        if (!CompileShader(overlayShader, "OverlayVS", "vs_4_0", overlayVs)) return false;
        if (!CompileShader(overlayShader, "OverlayPS", "ps_4_0", overlayPs)) return false;
        if (!CompileShader(postShader, "PostVS", "vs_4_0", postVs)) return false;
        if (!CompileShader(postShader, "PostPS", "ps_4_0", postPs)) return false;
        HRESULT hr = device_->CreateVertexShader(vs->GetBufferPointer(), vs->GetBufferSize(), nullptr, &vertexShader_);
        if (FAILED(hr)) return false;
        if (featureLevel_ >= D3D_FEATURE_LEVEL_11_0) {
            hr = device_->CreateHullShader(hs->GetBufferPointer(), hs->GetBufferSize(), nullptr, &hullShader_);
            if (FAILED(hr)) return false;
            hr = device_->CreateDomainShader(ds->GetBufferPointer(), ds->GetBufferSize(), nullptr, &domainShader_);
            if (FAILED(hr)) return false;
        }
        hr = device_->CreatePixelShader(ps->GetBufferPointer(), ps->GetBufferSize(), nullptr, &pixelShader_);
        if (FAILED(hr)) return false;
        hr = device_->CreatePixelShader(shadowPs->GetBufferPointer(), shadowPs->GetBufferSize(), nullptr, &shadowPixelShader_);
        if (FAILED(hr)) return false;
        hr = device_->CreateVertexShader(overlayVs->GetBufferPointer(), overlayVs->GetBufferSize(), nullptr, &overlayVertexShader_);
        if (FAILED(hr)) return false;
        hr = device_->CreatePixelShader(overlayPs->GetBufferPointer(), overlayPs->GetBufferSize(), nullptr, &overlayPixelShader_);
        if (FAILED(hr)) return false;
        hr = device_->CreateVertexShader(postVs->GetBufferPointer(), postVs->GetBufferSize(), nullptr, &postVertexShader_);
        if (FAILED(hr)) return false;
        hr = device_->CreatePixelShader(postPs->GetBufferPointer(), postPs->GetBufferSize(), nullptr, &postPixelShader_);
        if (FAILED(hr)) return false;

        D3D11_INPUT_ELEMENT_DESC desc[] = {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, pos), D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, normal), D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, tangent), D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Vertex, uv), D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"TEXCOORD", 1, DXGI_FORMAT_R32_FLOAT, 0, offsetof(Vertex, material), D3D11_INPUT_PER_VERTEX_DATA, 0}
        };
        hr = device_->CreateInputLayout(desc, ARRAYSIZE(desc), vs->GetBufferPointer(), vs->GetBufferSize(), &inputLayout_);
        if (FAILED(hr)) return false;
        D3D11_INPUT_ELEMENT_DESC overlayDesc[] = {
            {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(OverlayVertex, pos), D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(OverlayVertex, color), D3D11_INPUT_PER_VERTEX_DATA, 0}
        };
        hr = device_->CreateInputLayout(overlayDesc, ARRAYSIZE(overlayDesc), overlayVs->GetBufferPointer(), overlayVs->GetBufferSize(), &overlayInputLayout_);
        return SUCCEEDED(hr);
    }
