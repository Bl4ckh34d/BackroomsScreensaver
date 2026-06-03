        XMFLOAT3 right{std::cos(yaw), 0.0f, -std::sin(yaw)};
                XMFLOAT3 up{0.0f, 1.0f, 0.0f};
                XMFLOAT3 forward{std::sin(yaw), 0.0f, std::cos(yaw)};
                XMFLOAT3 emitterPos = Add3(center, OrientedOffset(right, up, forward, 0.0f, lowVent ? 0.02f : -0.02f, 0.090f));
