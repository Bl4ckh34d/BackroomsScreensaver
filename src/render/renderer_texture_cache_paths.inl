    static std::wstring TextureCacheFileName() {
#if defined(BACKROOMS_GAME_EXE)
        return L"BackroomsMazeGame_textures.bin";
#else
        return L"BackroomsMaze_textures.bin";
#endif
    }

    static std::filesystem::path TextureCachePath() {
        return CacheDirectory() / TextureCacheFileName();
    }
