    bool CompileShader(const char* src, const char* entry, const char* profile, ComPtr<ID3DBlob>& blob) {
        UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(_DEBUG)
        flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
        flags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
        uint64_t cacheHash = ShaderCacheHash(src, entry, profile, flags);
        ReportShaderActivity(L"Checking cache for", entry, profile);
        if (LoadShaderCache(entry, cacheHash, blob)) {
            SaveShaderCache(entry, cacheHash, blob.Get());
            ReportShaderComplete(L"Loaded cached shader", entry, profile, false);
            return true;
        }

        ReportShaderActivity(L"Compiling", entry, profile);
        ComPtr<ID3DBlob> errors;
        HRESULT hr = D3DCompile(src, std::strlen(src), nullptr, nullptr, nullptr, entry, profile, flags, 0, &blob, &errors);
        if (FAILED(hr) && errors) {
            const char* errorText = static_cast<const char*>(errors->GetBufferPointer());
            OutputDebugStringA(errorText);
            std::string bytes(errorText, errorText + errors->GetBufferSize());
            StartupProfileLine(L"Shader compile failed for " + std::wstring(entry, entry + std::strlen(entry)) + L": " + std::wstring(bytes.begin(), bytes.end()));
        }
        if (SUCCEEDED(hr)) {
            SaveShaderCache(entry, cacheHash, blob.Get());
            ReportShaderComplete(L"Compiled shader", entry, profile, true);
        }
        return SUCCEEDED(hr);
    }
