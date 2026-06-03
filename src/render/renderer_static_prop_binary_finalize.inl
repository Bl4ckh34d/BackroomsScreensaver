        out.min = {header.min[0], header.min[1], header.min[2]};
        out.max = {header.max[0], header.max[1], header.max[2]};
        out.generatedUvFallback = StaticPropNeedsGeneratedUv(out);
        return true;
