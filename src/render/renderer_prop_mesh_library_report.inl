        StartupProfileLine(L"Loaded prop meshes: " + std::to_wstring(loaded) + L"/14");
        {
            auto tris = [](const StaticPropMesh& mesh) {
                return static_cast<uint64_t>(mesh.vertices.size() / 3);
            };
            uint64_t chairTris = 0;
            for (const StaticPropMesh& mesh : renderAssets_.chairPropMeshes) chairTris += tris(mesh);
            uint64_t ceilingLampTris = 0;
            for (const StaticPropMesh& mesh : renderAssets_.ceilingLampPropMeshes) ceilingLampTris += tris(mesh);
            std::wstringstream counts;
            counts << L"Prop mesh triangle library: chairs=" << chairTris
                << L", ceilingLamps=" << ceilingLampTris
                << L", cabinet=" << tris(renderAssets_.cabinetPropMesh)
                << L", desk=" << tris(renderAssets_.deskPropMesh)
                << L", trashBin=" << tris(renderAssets_.trashBinPropMesh)
                << L", deskLamp=" << tris(renderAssets_.deskLampPropMesh)
                << L", cassette=" << tris(renderAssets_.cassettePropMesh)
                << L", airVent=" << tris(renderAssets_.airVentPropMesh)
                << L", exitSign=" << tris(renderAssets_.exitSignPropMesh);
            StartupProfileLine(counts.str());
        }
