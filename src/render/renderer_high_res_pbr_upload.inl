        return CreateTexture2DSrvRGBA(size, albedo, albedoSrv) &&
            CreateTexture2DSrvRGBA(size, normalHeight, normalSrv) &&
            CreateTexture2DSrvRGBA(size, props, propsSrv);
