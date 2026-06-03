#include "loading_overlay_internal.h"

void DrawLoadingSpinner(HDC hdc, int cx, int cy, int radius, int frame) {
    constexpr int dots = 12;
    for (int i = 0; i < dots; ++i) {
        float angle = (static_cast<float>(i) / static_cast<float>(dots)) * kPi * 2.0f;
        int age = (i - frame) % dots;
        if (age < 0) age += dots;
        int intensity = 72 + (dots - age) * 13;
        intensity = std::clamp(intensity, 72, 228);
        int x = cx + static_cast<int>(std::cos(angle) * radius);
        int y = cy + static_cast<int>(std::sin(angle) * radius);
        int dotRadius = std::max(2, radius / 6);
        HBRUSH brush = CreateSolidBrush(RGB(intensity, intensity - 8, intensity / 2));
        HGDIOBJ oldBrush = SelectObject(hdc, brush);
        HPEN pen = CreatePen(PS_NULL, 0, RGB(0, 0, 0));
        HGDIOBJ oldPen = SelectObject(hdc, pen);
        Ellipse(hdc, x - dotRadius, y - dotRadius, x + dotRadius + 1, y + dotRadius + 1);
        SelectObject(hdc, oldPen);
        SelectObject(hdc, oldBrush);
        DeleteObject(pen);
        DeleteObject(brush);
    }
}
