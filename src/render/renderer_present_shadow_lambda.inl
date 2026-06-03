// Flashlight and fixture depth shadow passes.

        auto renderDepthShadow = [&](ID3D11DepthStencilView* shadowDsv, UINT shadowSize, const XMMATRIX& shadowViewProj,
                                     XMFLOAT3 shadowOrigin, XMFLOAT3 shadowDirection, float shadowRange, float shadowConeCos) {
#include "renderer_present_shadow_setup.inl"
#include "renderer_present_shadow_static_scene.inl"
#include "renderer_present_shadow_static_props.inl"
#include "renderer_present_shadow_instanced_props.inl"
#include "renderer_present_shadow_dynamic_geometry.inl"
        };
