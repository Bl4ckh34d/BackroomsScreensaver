#pragma once

struct MonsterPreviewRuntimeState {
    bool active = false;
    MonsterPreviewView view = MonsterPreviewView::Orbit;
    bool manualOrbit = false;
    float orbitYaw = 0.0f;
    float orbitPitch = -0.18f;
    float orbitDistance = 3.15f;
};
