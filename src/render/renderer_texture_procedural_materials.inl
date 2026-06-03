        ReportStartupSubStep(L"Loading textures", L"Generating procedural material atlas.", 1);
        for (int y = 0; y < kTextureSize; ++y) {
            for (int x = 0; x < kTextureSize; ++x) {
#include "renderer_texture_procedural_wall_floor.inl"
#include "renderer_texture_procedural_ceiling.inl"
#include "renderer_texture_procedural_light_glass.inl"
#include "renderer_texture_procedural_omukade_flesh.inl"
#include "renderer_texture_procedural_door.inl"
#include "renderer_texture_procedural_props.inl"
#include "renderer_texture_procedural_effects.inl"
            }
        }
        profile.Mark(L"ProceduralMaterials");
