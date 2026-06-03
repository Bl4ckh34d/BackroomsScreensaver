R"(
    return clamp(floor(material), 0.0, 35.0);
}

void ShadowPS(VSOut input)
{
    float materialId = MaterialId(input.material);
    bool transparentPaper = materialId > 8.5 && materialId < 9.5 && frac(input.material) < 0.5;
    bool randomLoosePage = materialId > 34.5 && materialId < 35.5;
    if ((materialId > 0.5 && materialId < 2.5) ||
        transparentPaper ||
        randomLoosePage ||
        (materialId > 10.5 && materialId < 11.5) ||
        (materialId > 11.5 && materialId < 12.5) ||
        (materialId > 12.5 && materialId < 13.5) ||
        (materialId > 13.5 && materialId < 14.5) ||
        (materialId > 14.5 && materialId < 15.5) ||
        (materialId > 17.5 && materialId < 18.5) ||
        (materialId > 18.5 && materialId < 19.5) ||
        (materialId > 24.5 && materialId < 25.5) ||
        (materialId > 25.5 && materialId < 26.5))
    {
        discard;
    }
}

float3 MaterialUV(float2 uv, float material)
{
    float slice = MaterialId(material);
    return float3(uv, slice);
}

bool IsCeilingMaterial(float material)
{
    float materialId = MaterialId(material);
    return materialId > 1.5 && materialId < 2.5;
}

bool IsDoorMaterial(float material)
{
    float materialId = MaterialId(material);
    return materialId > 5.5 && materialId < 6.5;
}

bool IsDoorFrameMaterial(float material)
{
    float materialId = MaterialId(material);
    return materialId > 20.5 && materialId < 21.5 && frac(material) > 0.30;
}

float4 SampleMaterialAlbedo(float2 uv, float material, float mipBias)
{
    if (IsCeilingMaterial(material))
    {
        return gCeilingAlbedo.SampleBias(gSampler, uv, mipBias);
    }
    if (IsDoorMaterial(material))
    {
        return gDoorAlbedo.SampleBias(gSampler, uv, mipBias);
    }
    if (IsDoorFrameMaterial(material))
    {
        return gDoorFrameAlbedo.SampleBias(gSampler, uv, mipBias);
    }
    return gAlbedo.SampleBias(gSampler, MaterialUV(uv, material), mipBias);
}

float4 SampleMaterialProps(float2 uv, float material, float mipBias)
{
    if (IsCeilingMaterial(material))
    {
        return gCeilingProps.SampleBias(gSampler, uv, mipBias);
    }
    if (IsDoorMaterial(material))
    {
        return gDoorProps.SampleBias(gSampler, uv, mipBias);
    }
    if (IsDoorFrameMaterial(material))
    {
        return gDoorFrameProps.SampleBias(gSampler, uv, mipBias);
    }
    return gMaterialProps.SampleBias(gSampler, MaterialUV(uv, material), mipBias);
}

float4 SampleMaterialNormalHeight(float2 uv, float material, float mipBias)
{
    if (IsCeilingMaterial(material))
    {
        return gCeilingNormalHeight.SampleBias(gSampler, uv, mipBias);
    }
    if (IsDoorMaterial(material))
    {
        return gDoorNormalHeight.SampleBias(gSampler, uv, mipBias);
    }
    if (IsDoorFrameMaterial(material))
    {
)"
