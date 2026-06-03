                float panelW = ctx.tileW * (0.94f / 3.0f);
                float panelD = ctx.tileD * (0.94f / 3.0f);
                float material = lampOn ? 3.0f + seed * 0.49f : 5.0f;
                AddCeilingCard(vertices, indices, {lampCenter.x, 0.0f, lampCenter.z},
                    panelW, panelD, 0.0f, ctx.wallH - 0.004f, material);
