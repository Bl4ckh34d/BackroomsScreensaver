#pragma once

// Config dialog tab labels, notes, and full settings field table.
#include "config_dialog_model_tab_text.h"

const ConfigFieldDef kConfigFields[] = {
#include "config_dialog_fields_renderer.inl"
#include "config_dialog_fields_maze.inl"
#include "config_dialog_fields_textures.inl"
#include "config_dialog_fields_lighting.inl"
#include "config_dialog_fields_camera_ai.inl"
#include "config_dialog_fields_camera_fx.inl"
#include "config_dialog_fields_atmosphere.inl"
#include "config_dialog_fields_monster.inl"
};