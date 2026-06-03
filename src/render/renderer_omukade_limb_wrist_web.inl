                XMFLOAT3 wristWeb = Add3(palmCenter, Scale3(contactNormal, -limb * 0.34f));
                organicSegment(knee, wristWeb, limb * 0.78f, limb * 0.32f, limbMaterial + 0.035f, 7);
            }
        }
