// Screensaver run-mode, debug-global, and launch-window setup helpers.
// Included from screensaver_app.inl before RunScreensaver.

struct ScreensaverRunConfig {
    bool monsterPreviewMode = false;
    bool bloodDebugMode = false;
    bool diagnosticWindowMode = false;
    MonsterPreviewView monsterPreviewView = MonsterPreviewView::Orbit;
};

ScreensaverRunConfig BuildScreensaverRunConfig(RunMode mode) {
    ScreensaverRunConfig config{};
    config.monsterPreviewMode =
        mode == RunMode::MonsterPreview ||
        mode == RunMode::MonsterPreviewFront ||
        mode == RunMode::MonsterPreviewSide ||
        mode == RunMode::MonsterPreviewLeftSide ||
        mode == RunMode::MonsterPreviewTop;
    config.bloodDebugMode = mode == RunMode::BloodDebug;
    config.diagnosticWindowMode = config.monsterPreviewMode || config.bloodDebugMode;
    if (mode == RunMode::MonsterPreviewFront) config.monsterPreviewView = MonsterPreviewView::Front;
    else if (mode == RunMode::MonsterPreviewSide) config.monsterPreviewView = MonsterPreviewView::Side;
    else if (mode == RunMode::MonsterPreviewLeftSide) config.monsterPreviewView = MonsterPreviewView::LeftSide;
    else if (mode == RunMode::MonsterPreviewTop) config.monsterPreviewView = MonsterPreviewView::Top;
    return config;
}
