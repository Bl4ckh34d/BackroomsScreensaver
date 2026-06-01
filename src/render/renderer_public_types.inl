enum class MonsterPreviewView {
    Orbit,
    Front,
    Side,
    LeftSide,
    Top
};

struct MenuPlaquePlacement {
    XMFLOAT3 center{};
    XMFLOAT3 right{1.0f, 0.0f, 0.0f};
    XMFLOAT3 inward{0.0f, 0.0f, -1.0f};
    float halfW = 0.72f;
    float halfH = 0.122f;
};

enum class CustomGameMenuControl : int {
    None = 0,
    BrokenLampScares,
    AirVentScares,
    WaterScares,
    BloodWorldScares,
    FleshWorldScares,
    BrokenLampScareDetails,
    AirVentScareDetails,
    WaterScareDetails,
    BloodWorldScareDetails,
    FleshWorldScareDetails,
    OmukadeBoss,
    SizeXMinus,
    SizeXPlus,
    SizeYMinus,
    SizeYPlus,
    RoomCountMinus,
    RoomCountPlus,
    EnvironmentDetails,
    EnvDirtMinus,
    EnvDirtPlus,
    EnvPaperMinus,
    EnvPaperPlus,
    EnvPropMinus,
    EnvPropPlus,
    EnvLampOnMinus,
    EnvLampOnPlus,
    EnvLampFlickerMinus,
    EnvLampFlickerPlus,
    EnvLampSparkMinus,
    EnvLampSparkPlus,
    EnvFogStartMinus,
    EnvFogStartPlus,
    EnvFogEndMinus,
    EnvFogEndPlus,
    EnvFogDarkMinus,
    EnvFogDarkPlus,
    ScareChanceMinus,
    ScareChancePlus,
    ScareStartMinMinus,
    ScareStartMinPlus,
    ScareStartMaxMinus,
    ScareStartMaxPlus,
    EightPages,
    ScareDetailBack,
    Start,
    Back
};
