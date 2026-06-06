R"(
        float floorReceiver = smoothstep(0.42, 0.82, worldN.y);
        float ceilingReceiver = smoothstep(0.42, 0.82, -worldN.y);
        float verticalReceiver = (1.0 - floorReceiver) * (1.0 - ceilingReceiver);
        float wallFacing = smoothstep(-0.18, 0.54, max(dot(worldN, portalDir) * 0.88, abs(dot(worldN, portalRight)) * 0.86));
        float receiver = saturate(floorReceiver * 0.42 + ceilingReceiver * 0.28 + verticalReceiver * wallFacing);
        float signNear = 1.0 - smoothstep(0.40, 3.20, length(worldPos - gExitLight0.xyz));
        float doorOrTrim = saturate(step(5.5, materialId) * (1.0 - step(6.5, materialId)) +
            step(9.5, materialId) * (1.0 - step(10.5, materialId)));
        float spill = portalEnabled * roomDepth * width * height * receiver;
        result += float3(0.035, 0.32, 0.13) * strength * spill * (0.08 + signNear * 0.12) * (0.84 + doorOrTrim * 0.50);

        float hallDepth = -axial;
        float maxHallDepth = max(gMaze1.w * 14.0, 14.0);
        float hallLength = smoothstep(0.08, 0.42, hallDepth) *
            (1.0 - smoothstep(maxHallDepth - gMaze1.w * 1.20, maxHallDepth + gMaze1.w * 0.35, hallDepth));
        float hallWidth = smoothstep(doorHalfW * 2.35, doorHalfW * 0.30, lateral);
        float hallHeight = smoothstep(-0.18, 0.04, worldPos.y) * (1.0 - smoothstep(gMaze1.z + 0.04, gMaze1.z + 0.24, worldPos.y));
        float floorReceiver2 = smoothstep(0.42, 0.82, worldN.y);
        float ceilingReceiver2 = smoothstep(0.42, 0.82, -worldN.y);
        float wallReceiver2 = (1.0 - floorReceiver2) * (1.0 - ceilingReceiver2) *
            smoothstep(-0.14, 0.58, max(abs(dot(worldN, portalRight)) * 0.92, abs(dot(worldN, portalDir)) * 0.38));
        float receiver2 = saturate(floorReceiver2 * 0.92 + wallReceiver2 * 0.56 + ceilingReceiver2 * 0.74);
        float lampSpacing = max(gMaze1.w * 1.75, 2.2);
        float lampBand = pow(saturate(sin(hallDepth / lampSpacing * 6.2831853) * 0.5 + 0.5), 0.55);
        float lampBands = 0.74 + 0.34 * lampBand;
        float hallFade = 1.0 - smoothstep(maxHallDepth - gMaze1.w * 2.15, maxHallDepth + gMaze1.w * 0.45, hallDepth);
        float vestibuleStrength = max(saturate(strength * 0.16), gExitLight2.w * max(saturate(gExitLight1.w), 0.18));
        result += float3(1.0, 0.90, 0.58) * vestibuleStrength * hallLength * hallWidth * hallHeight * hallFade *
            (receiver2 * (0.72 + lampBands * 0.34) + 0.08);

    }

    float doorStrength = gExitLight2.w * (1.0 - saturate(gTransition0.z));
    float doorOpen = saturate(gExitLight1.w);
    if (doorStrength > 0.001 && doorOpen > 0.001)
    {
            float3 doorDir = normalize(gExitLight1.xyz + float3(0.0001, 0.0, 0.0001));
            float3 doorRight = normalize(float3(doorDir.z, 0.0, -doorDir.x) + float3(0.0001, 0.0, 0.0001));
            float doorHalfW = max(0.12, gExitLight3.w);
            float axial = dot(worldPos - gExitLight3.xyz, doorDir);
            float lateral = dot(worldPos - gExitLight3.xyz, doorRight);
            float doorPanelMaterial = step(5.5, materialId) * (1.0 - step(6.5, materialId));
            float corridorExtraWidth = lerp(0.28, 1.08, doorPanelMaterial);
            float corridorWidth = smoothstep(doorHalfW + corridorExtraWidth, doorHalfW - 0.02, abs(lateral));
            float doorHalfH = 1.18;
            float corridorHeight = smoothstep(doorHalfH + 0.38, doorHalfH - 0.18, abs(worldPos.y - doorHalfH));
            float corridorSide = (1.0 - smoothstep(-0.10, 0.08, axial)) * corridorWidth * corridorHeight;
            float3 warmDaylight = float3(1.0, 0.88, 0.58);
            float floorReceiver = smoothstep(0.42, 0.82, worldN.y);
            float roomDistance = max(0.0, axial);
            float beamHalfWidth = lerp(doorHalfW * 0.72, doorHalfW * 3.75, saturate(roomDistance / 4.80));
            float sideSoft = lerp(0.12, 0.58, saturate(roomDistance / 4.80));
            float sideMask = smoothstep(beamHalfWidth, beamHalfWidth - sideSoft, abs(lateral));
            float lengthMask = smoothstep(0.04, 0.38, roomDistance) * (1.0 - smoothstep(5.10, 6.75, roomDistance));
            float sourceCut = smoothstep(doorHalfW * 1.28, doorHalfW * 0.08, abs(lateral));
            float floorLane = floorReceiver * sideMask * lengthMask * lerp(sourceCut, 1.0, saturate(roomDistance / 1.25));
            float floorFalloff = exp(-roomDistance * 0.145);
            result += warmDaylight * doorStrength * floorLane * floorFalloff * 0.54;
            float ceilingReceiver = smoothstep(0.42, 0.82, -worldN.y);
            float verticalReceiver = (1.0 - floorReceiver) * (1.0 - ceilingReceiver);
            float wallFacing = smoothstep(-0.08, 0.46, max(dot(worldN, -doorDir) * 0.92, abs(dot(worldN, doorRight)) * 0.86));
            float wallHeight = smoothstep(0.08, 0.34, worldPos.y) * (1.0 - smoothstep(2.74, 3.04, worldPos.y));
            float wallWidth = smoothstep(doorHalfW * 5.35, doorHalfW * 0.64, abs(lateral));
            float wallLane = verticalReceiver * wallFacing * wallWidth * smoothstep(0.50, 0.92, roomDistance) *
                (1.0 - smoothstep(5.20, 6.60, roomDistance)) * wallHeight;
            result += warmDaylight * doorStrength * wallLane * floorFalloff * 0.315;
            float roomWash = (floorLane * 0.16 + wallLane * 0.34) * (1.0 - smoothstep(6.0, 8.2, roomDistance));
            result += warmDaylight * doorStrength * roomWash * 0.145;
            result += warmDaylight * doorStrength * corridorSide * 0.92;
    }
    return result;
}

float DoorRoomSideLightBlock(float3 worldPos, float3 worldN, float materialId)
{
    float doorOpen = saturate(gExitLight1.w);
    float doorStrength = gExitLight2.w * (1.0 - saturate(gTransition0.z));
    if (doorOpen <= 0.001 || doorStrength <= 0.001)
    {
        return 0.0;
    }

    float3 doorDir = normalize(gExitLight1.xyz + float3(0.0001, 0.0, 0.0001));
    float3 doorRight = normalize(float3(doorDir.z, 0.0, -doorDir.x) + float3(0.0001, 0.0, 0.0001));
    float3 rel = worldPos - gExitLight3.xyz;
    float axial = dot(rel, doorDir);
    float lateral = dot(rel, doorRight);
    float doorHalfW = max(0.12, gExitLight3.w);

    float floorReceiver = smoothstep(0.42, 0.82, worldN.y);
    float ceilingReceiver = smoothstep(0.42, 0.82, -worldN.y);
    float verticalSurface = (1.0 - floorReceiver) * (1.0 - ceilingReceiver);
    float roomSideWall = verticalSurface * smoothstep(0.52, 0.84, dot(worldN, doorDir));
    float nearWallPlane = smoothstep(0.26, 0.02, abs(axial));
    float aroundDoor = smoothstep(doorHalfW * 3.60, doorHalfW * 0.30, abs(lateral));
    float heightGate = smoothstep(0.02, 0.22, worldPos.y) * (1.0 - smoothstep(2.62, 2.88, worldPos.y));
    float wallBlock = roomSideWall * nearWallPlane * aroundDoor * heightGate;
)"
