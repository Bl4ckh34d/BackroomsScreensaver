    static int MergeWaterMode(int a, int b) {
        if (a == 3 && b == 3) return 3;
        if (a == 3) a = 0;
        if (b == 3) b = 0;
        bool central = a == 0 || a == 1 || b == 0 || b == 1;
        bool edge = a == 1 || a == 2 || b == 1 || b == 2;
        if (central && edge) return 1;
        if (edge) return 2;
        return 0;
    }
