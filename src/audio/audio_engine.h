#pragma once

#include "audio_engine_types.h"

class AudioEngine {
public:
#include "audio_engine_public_api.inl"
#include "audio_engine_update.inl"
#include "audio_engine_public_playback_api.inl"

private:
#include "audio_engine_private_api.inl"
#include "audio_engine_fields.inl"
};