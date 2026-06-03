// Omukade body and presentation geometry.
// Included inside Renderer private section from renderer_dynamic_geometry.inl.

    void AppendOmukadeGeometry(std::vector<Vertex>& solidVerts, std::vector<Vertex>& transparentVerts) {
#include "renderer_omukade_geometry_visibility_setup.inl"
#include "renderer_omukade_geometry_surface_helpers.inl"
#include "renderer_omukade_geometry_contact_helpers.inl"
#include "renderer_omukade_geometry_body_chain.inl"
#include "renderer_omukade_geometry_body_render.inl"
#include "renderer_omukade_geometry_limbs.inl"
#include "renderer_omukade_geometry_ambient_effects.inl"
#include "renderer_omukade_geometry_head.inl"
    }
