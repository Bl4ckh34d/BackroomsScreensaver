            auto addTri = [&](int ia, int ib, int ic) {
                int ca = clusterIndex(ia);
                int cb = clusterIndex(ib);
                int cc = clusterIndex(ic);
                if (ca == cb || cb == cc || ca == cc) return;
                int sa = ca;
                int sb = cb;
                int sc = cc;
                if (sa > sb) std::swap(sa, sb);
                if (sb > sc) std::swap(sb, sc);
                if (sa > sb) std::swap(sa, sb);
                if (sc >= (1 << 21)) return;
                uint64_t key = static_cast<uint64_t>(sa) |
                    (static_cast<uint64_t>(sb) << 21) |
                    (static_cast<uint64_t>(sc) << 42);
                if (!seen.insert(key).second) return;

                XMFLOAT3 a = localPos(ia);
                XMFLOAT3 b = localPos(ib);
                XMFLOAT3 c = localPos(ic);
                XMFLOAT3 rawNormal = Cross3(Sub3(b, a), Sub3(c, a));
                XMFLOAT3 n = Normalize3(rawNormal, {0.0f, 1.0f, 0.0f});
                XMFLOAT3 faceCenter = Scale3(Add3(Add3(a, b), c), 1.0f / 3.0f);
                if (Dot3(n, faceCenter) < 0.0f) rawNormal = Scale3(rawNormal, -1.0f);
                clusters[static_cast<size_t>(ca)].normal = Add3(clusters[static_cast<size_t>(ca)].normal, rawNormal);
                clusters[static_cast<size_t>(cb)].normal = Add3(clusters[static_cast<size_t>(cb)].normal, rawNormal);
                clusters[static_cast<size_t>(cc)].normal = Add3(clusters[static_cast<size_t>(cc)].normal, rawNormal);
                tris.push_back({ca, cb, cc});
            };
