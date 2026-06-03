    s.assetFolder = IniString(L"Textures", L"AssetFolder", s.assetFolder.c_str());
    s.wallStem = IniString(L"Textures", L"WallStem", s.wallStem.c_str());
    s.floorStem = IniString(L"Textures", L"FloorStem", s.floorStem.c_str());
    s.ceilingStem = IniString(L"Textures", L"CeilingStem", s.ceilingStem.c_str());
    s.fleshStem = IniString(L"Textures", L"FleshStem", s.fleshStem.c_str());
    s.wallTextureMeters = std::max(0.2f, IniFloat(L"Textures", L"WallScaleMeters", s.wallTextureMeters));
    s.floorTextureMeters = std::max(0.2f, IniFloat(L"Textures", L"FloorScaleMeters", s.floorTextureMeters));
    s.ceilingTextureMeters = std::max(0.0f, IniFloat(L"Textures", L"CeilingScaleMeters", s.ceilingTextureMeters));
    s.useExternalNormals = IniInt(L"Textures", L"UseExternalNormals", s.useExternalNormals ? 1 : 0) != 0;
    s.maxNormalMapMB = std::clamp(IniInt(L"Textures", L"MaxNormalMapMB", s.maxNormalMapMB), 0, 1024);

