        bool allowBodySurfaceClimb = monsterPreview_.active || debugEffectMonster;
        bool allowCeilingLimbContacts = monsterPreview_.active || debugEffectMonster;

        struct SurfaceContact {
            XMFLOAT3 point;
            XMFLOAT3 normal;
        };
