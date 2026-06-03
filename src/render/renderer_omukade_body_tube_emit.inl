        bool renderBodyMass = true;
        if (renderBodyMass) {
            for (int i = 0; i + 1 < bodyCount; ++i) {
                for (int r = 0; r < tubeSides; ++r) {
                    TubeVertex a = tubeVertex(i, r);
                    TubeVertex b = tubeVertex(i, r + 1);
                    TubeVertex c = tubeVertex(i + 1, r + 1);
                    TubeVertex d = tubeVertex(i + 1, r);
                    pushTubeTri(a, b, c, gutMat);
                    pushTubeTri(a, c, d, gutMat);
                }
            }
        }
