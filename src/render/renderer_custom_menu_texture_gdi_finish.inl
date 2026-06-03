            auto finishGdiDraw = [&]() {
                SelectObject(dc, oldBrush);
                SelectObject(dc, oldPen);
                SelectObject(dc, oldBitmap);
                DeleteObject(titleFont);
                DeleteObject(bodyFont);
                DeleteObject(smallFont);
                DeleteObject(inkPen);
                DeleteObject(boldInkPen);
                DeleteObject(hoverPen);
                DeleteObject(dimInkPen);
                DeleteObject(yellowBrush);
                std::memcpy(dib.data(), bits, dib.size());
            };
