                if (!staticSceneGeometry_.instancedPropShadowChunks.empty() && renderBuffers_.instancedVertexBuffer && renderBuffers_.instancedIndexBuffer && renderBuffers_.instancedInstanceBuffer) {
                    ID3D11Buffer* instBuffers[] = {renderBuffers_.instancedVertexBuffer.Get(), renderBuffers_.instancedInstanceBuffer.Get()};
                    UINT instStrides[] = {sizeof(Vertex), sizeof(StaticInstanceData)};
                    UINT instOffsets[] = {0, 0};
                    d3dRuntime_.context->IASetInputLayout(inputLayouts_.instancedInputLayout.Get());
                    d3dRuntime_.context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
                    d3dRuntime_.context->IASetVertexBuffers(0, 2, instBuffers, instStrides, instOffsets);
                    d3dRuntime_.context->IASetIndexBuffer(renderBuffers_.instancedIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
                    d3dRuntime_.context->VSSetShader(shaders_.instancedVertexShader.Get(), nullptr, 0);
                    d3dRuntime_.context->HSSetShader(nullptr, nullptr, 0);
                    d3dRuntime_.context->DSSetShader(nullptr, nullptr, 0);
                    d3dRuntime_.context->PSSetShader(nullptr, nullptr, 0);
                    d3dRuntime_.context->VSSetConstantBuffers(0, 1, renderBuffers_.constantBuffer.GetAddressOf());
                    StartupProfileLine(L"Render before InstancedPropShadow DrawIndexedInstanced");
                    drawVisibleInstancedChunks(staticSceneGeometry_.instancedPropShadowChunks, shadowOrigin, shadowDirection, shadowRange, shadowConeCos,
                        shadowTileAverage * 1.4f, false, 1);
                    renderProfile.Mark(L"InstancedPropShadow");
                    d3dRuntime_.context->PSSetShader(shaders_.shadowPixelShader.Get(), nullptr, 0);
                }
            }
