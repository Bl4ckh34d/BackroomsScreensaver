        } else {
            if (header.vertexStride != sizeof(PackedStaticPropVertex)) return false;
            std::vector<PackedStaticPropVertex> packed(header.vertexCount);
            in.read(reinterpret_cast<char*>(packed.data()),
                static_cast<std::streamsize>(packed.size() * sizeof(PackedStaticPropVertex)));
            if (!in) {
                out = {};
                return false;
            }
            XMFLOAT3 minP{header.min[0], header.min[1], header.min[2]};
            XMFLOAT3 extent{
                header.max[0] - header.min[0],
                header.max[1] - header.min[1],
                header.max[2] - header.min[2]
            };
            float uvMinU = header.extra[0];
            float uvMinV = header.extra[1];
            float uvExtentU = header.extra[2] - header.extra[0];
            float uvExtentV = header.extra[3] - header.extra[1];
            auto unpackUnit = [](uint16_t v) {
                return static_cast<float>(v) * (1.0f / 65535.0f);
            };
            auto unpackSnorm = [](int16_t v) {
                return std::clamp(static_cast<float>(v) * (1.0f / 32767.0f), -1.0f, 1.0f);
            };
            out.vertices.reserve(header.vertexCount);
            for (const PackedStaticPropVertex& src : packed) {
                XMFLOAT3 pos{
                    minP.x + extent.x * unpackUnit(src.px),
                    minP.y + extent.y * unpackUnit(src.py),
                    minP.z + extent.z * unpackUnit(src.pz)
                };
                XMFLOAT3 normal = Normalize3({unpackSnorm(src.nx), unpackSnorm(src.ny), unpackSnorm(src.nz)}, {0.0f, 1.0f, 0.0f});
                XMFLOAT3 tangent = Normalize3({unpackSnorm(src.tx), unpackSnorm(src.ty), unpackSnorm(src.tz)}, {1.0f, 0.0f, 0.0f});
                XMFLOAT2 uv{
                    uvMinU + uvExtentU * unpackUnit(src.u),
                    uvMinV + uvExtentV * unpackUnit(src.v)
                };
                out.vertices.push_back({pos, normal, tangent, uv, static_cast<float>(src.material)});
            }
        }
