            if (dynamicGeometry_.opaqueVertexCount > 0 && sessionRuntime_.mode != RendererRuntimeMode::MainMenu) {
                d3dRuntime_.context->HSSetShader(nullptr, nullptr, 0);
                d3dRuntime_.context->DSSetShader(nullptr, nullptr, 0);
                d3dRuntime_.context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
                d3dRuntime_.context->IASetVertexBuffers(0, 1, renderBuffers_.dynamicBuffer.GetAddressOf(), &stride, &offset);
                StartupProfileLine(L"Render before ShadowDynamic Draw");
                d3dRuntime_.context->Draw(dynamicGeometry_.opaqueVertexCount, 0);
                renderProfile.Mark(L"ShadowDynamic");
            }
