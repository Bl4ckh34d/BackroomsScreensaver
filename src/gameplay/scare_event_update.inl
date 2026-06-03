// Scare event trigger update coordinator.
// Included inside Renderer's private section from scare_effect_events.inl.

#include "scare_event_vision_flash.inl"

    void UpdateScareEvents(float dt) {
#include "scare_event_update_gates.inl"
#include "scare_event_world_flicker.inl"
#include "scare_event_broken_lamps.inl"
#include "scare_event_air_vents.inl"
    }