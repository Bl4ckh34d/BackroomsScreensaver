    bool CreateHighResCeilingTextures() {
        return CreateHighResPbrTextures(settingsRuntime_.live.ceilingStem, materialTextures_.ceilingAlbedoSrv, materialTextures_.ceilingNormalSrv, materialTextures_.ceilingPropsSrv);
    }

    bool CreateHighResDoorTextures() {
        return CreateHighResPbrTextures(
                L"assets\\PBRs\\downloads\\door001\\extracted\\Door001_4K-JPG",
                materialTextures_.doorAlbedoSrv, materialTextures_.doorNormalSrv, materialTextures_.doorPropsSrv) &&
            CreateHighResPbrTextures(
                L"assets\\PBRs\\downloads\\painted_wood_009c\\extracted\\PaintedWood009C_4K-JPG",
                materialTextures_.doorFrameAlbedoSrv, materialTextures_.doorFrameNormalSrv, materialTextures_.doorFramePropsSrv);
    }
