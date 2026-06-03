    bool BuildSpecializedPixelShaders(const char* shader,
        std::string& generalShader,
        std::string& liquidShader,
        std::wstring& shaderSplitError) {
#include "renderer_shader_specialization_markers.inl"
#include "renderer_shader_specialization_liquid_fallback.inl"
#include "renderer_shader_specialization_outputs.inl"
    }
