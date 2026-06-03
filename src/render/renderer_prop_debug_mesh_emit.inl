        if (mesh && !mesh->vertices.empty()) {
            float spanX = PropMeshSpan(*mesh, 0);
            float spanY = PropMeshSpan(*mesh, 1);
            float spanZ = PropMeshSpan(*mesh, 2);
            float spanMax = std::max(spanX, std::max(spanY, spanZ));
            float scale = std::clamp(targetMax / std::max(0.001f, spanMax), 0.035f, 12.0f);
            bool wallMounted = propIndex == 10 || propIndex == 11;
            bool suspendedLamp = propIndex >= 12 && propIndex <= 15;
            if (wallMounted || suspendedLamp) {
                float c = std::cos(yaw);
                float s = std::sin(yaw);
                XMFLOAT3 right{c, 0.0f, -s};
                XMFLOAT3 up{0.0f, 1.0f, 0.0f};
                XMFLOAT3 forward{s, 0.0f, c};
                XMFLOAT3 meshCenter{
                    (mesh->min.x + mesh->max.x) * 0.5f,
                    (mesh->min.y + mesh->max.y) * 0.5f,
                    (mesh->min.z + mesh->max.z) * 0.5f
                };
                XMFLOAT3 desiredCenter{
                    centerX,
                    wallMounted ? ctx.wallH * 0.54f : 1.08f,
                    wallMounted ? centerZ - ctx.tileD * 0.34f : centerZ
                };
                XMFLOAT3 centeredOffset = Add3(Scale3(right, meshCenter.x * scale),
                    Add3(Scale3(up, meshCenter.y * scale), Scale3(forward, meshCenter.z * scale)));
                XMFLOAT3 origin = Add3(desiredCenter, Scale3(centeredOffset, -1.0f));
                AppendStaticPropMesh(vertices, indices, *mesh, origin, yaw, scale, scale, scale,
                    0.0f, -1.0f, &propShadowIndices);
                cameraRuntime_.propLookPoints.push_back(desiredCenter);
                return;
            }
            AppendStaticPropMeshGrounded(vertices, indices, *mesh, {centerX, 0.0f, centerZ},
                yaw, scale, scale, scale, pitch, -1.0f, &propShadowIndices);
            float lookY = std::clamp((mesh->max.y - mesh->min.y) * scale * 0.55f, 0.16f, 1.15f);
            cameraRuntime_.propLookPoints.push_back({centerX, lookY, centerZ});
            return;
        }
