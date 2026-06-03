    bool CreateHighResPbrTextures(const std::wstring& base,
                                  ComPtr<ID3D11ShaderResourceView>& albedoSrv,
                                  ComPtr<ID3D11ShaderResourceView>& normalSrv,
                                  ComPtr<ID3D11ShaderResourceView>& propsSrv) {
#include "renderer_high_res_pbr_arrays.inl"
#include "renderer_high_res_pbr_resolve_path.inl"
#include "renderer_high_res_pbr_copy_image.inl"
#include "renderer_high_res_pbr_copy_scalar.inl"
#include "renderer_high_res_pbr_copy_normal.inl"
#include "renderer_high_res_pbr_load_assets.inl"
#include "renderer_high_res_pbr_upload.inl"
    }
