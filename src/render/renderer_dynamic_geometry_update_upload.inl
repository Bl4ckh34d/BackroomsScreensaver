        D3D11_MAPPED_SUBRESOURCE mapped{};
        if (renderBuffers_.dynamicBuffer && dynamicGeometry_.vertexCount > 0 &&
            SUCCEEDED(d3dRuntime_.context->Map(renderBuffers_.dynamicBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
            auto* dst = static_cast<uint8_t*>(mapped.pData);
            size_t opaqueBytes = opaqueVerts.size() * sizeof(Vertex);
            size_t transparentBytes = transparentVerts.size() * sizeof(Vertex);
            if (opaqueBytes > 0) {
                std::memcpy(dst, opaqueVerts.data(), opaqueBytes);
            }
            if (transparentBytes > 0) {
                std::memcpy(dst + opaqueBytes, transparentVerts.data(), transparentBytes);
            }
            d3dRuntime_.context->Unmap(renderBuffers_.dynamicBuffer.Get(), 0);
        }
        dynamicProfile.Mark(L"Upload");
