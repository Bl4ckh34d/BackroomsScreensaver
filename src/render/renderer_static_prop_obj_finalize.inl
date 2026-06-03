        if (out.vertices.empty()) return false;
        out.min = minP;
        out.max = maxP;
        out.generatedUvFallback = StaticPropNeedsGeneratedUv(out);
        return true;
