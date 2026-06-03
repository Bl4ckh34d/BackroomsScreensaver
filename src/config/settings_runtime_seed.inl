uint32_t ResolveRuntimeSeed(uint32_t configuredSeed) {
    if (configuredSeed != 0) return configuredSeed;

    LARGE_INTEGER counter{};
    QueryPerformanceCounter(&counter);
    uint64_t mix = static_cast<uint64_t>(counter.QuadPart);
    mix ^= GetTickCount64() + 0x9e3779b97f4a7c15ull;
    mix ^= static_cast<uint64_t>(GetCurrentProcessId()) << 32;
    mix ^= static_cast<uint64_t>(GetCurrentThreadId()) << 16;
    mix ^= static_cast<uint64_t>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    mix ^= static_cast<uint64_t>(reinterpret_cast<uintptr_t>(&mix));

    uint64_t osRandom[2]{};
    if (SystemFunction036(osRandom, sizeof(osRandom))) {
        mix ^= osRandom[0];
        mix ^= osRandom[1] + 0x9e3779b97f4a7c15ull + (mix << 6) + (mix >> 2);
    }

    std::random_device rd;
    mix ^= static_cast<uint64_t>(rd()) << 1;
    mix ^= static_cast<uint64_t>(rd()) << 33;

    mix ^= mix >> 33;
    mix *= 0xff51afd7ed558ccdull;
    mix ^= mix >> 33;
    mix *= 0xc4ceb9fe1a85ec53ull;
    mix ^= mix >> 33;
    uint32_t seed = static_cast<uint32_t>(mix ^ (mix >> 32));
    return seed != 0 ? seed : 1u;
}
