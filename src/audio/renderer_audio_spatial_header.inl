// Renderer audio occlusion, emitter setup, and per-frame update coordinator. 
// Included inside Renderer's private section from renderer_audio.inl.

    bool AudioRayClear(XMFLOAT3 from, XMFLOAT3 to) const {
        return AudioWallBlocksBetween(from, to) == 0;
    }
