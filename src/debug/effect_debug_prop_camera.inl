float DebugPropCameraDistanceScale(int index) {
    switch (WrapDebugPropIndex(index)) {
    case 4:
    case 5:
        return 2.15f;
    case 8:
    case 9:
        return 1.12f;
    case 10:
    case 11:
        return 1.30f;
    default:
        return 1.62f;
    }
}

float DebugPropCameraTargetY(int index) {
    switch (WrapDebugPropIndex(index)) {
    case 8:
    case 9:
        return 0.22f;
    case 4:
        return 0.76f;
    case 5:
        return 0.58f;
    case 10:
    case 11:
        return 0.46f;
    default:
        return 0.52f;
    }
}
