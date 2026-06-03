    static uint64_t Fnv1aAppend(uint64_t hash, const void* data, size_t size) {
        const uint8_t* bytes = static_cast<const uint8_t*>(data);
        for (size_t i = 0; i < size; ++i) {
            hash ^= bytes[i];
            hash *= 1099511628211ull;
        }
        return hash;
    }

    static uint64_t ShaderCacheHash(const char* src, const char* entry, const char* profile, UINT flags) {
        uint64_t hash = 1469598103934665603ull;
        const char* version = "BackroomsMazeShaderCacheV4";
        hash = Fnv1aAppend(hash, version, std::strlen(version));
        hash = Fnv1aAppend(hash, entry, std::strlen(entry) + 1);
        hash = Fnv1aAppend(hash, profile, std::strlen(profile) + 1);
        hash = Fnv1aAppend(hash, &flags, sizeof(flags));
        hash = Fnv1aAppend(hash, src, std::strlen(src));
        return hash;
    }
