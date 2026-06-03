        auto elapsedMs = [&](size_t from, size_t to) {
            if (timestamps[to] < timestamps[from]) return 0.0;
            return static_cast<double>(timestamps[to] - timestamps[from]) * 1000.0 /
                static_cast<double>(disjoint.Frequency);
        };
