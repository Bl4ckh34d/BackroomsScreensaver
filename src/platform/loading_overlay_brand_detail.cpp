#include "loading_overlay_internal.h"

void DrawLoadingBrandFallbackDetail(
    HDC hdc,
    const RECT& rc,
    const LoadingOverlaySnapshot* state,
    int x,
    int y,
    int logoSize) {
    HFONT detailFont = CreateFontW(-14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
    HGDIOBJ oldFont = SelectObject(hdc, detailFont);
    SetTextColor(hdc, RGB(126, 119, 91));
    std::wstring detail = state && !state->detail.empty() ? state->detail : L"Preparing renderer";
    RECT detailRect{rc.left + 24, y + logoSize + 18, rc.right - 24, y + logoSize + 44};
    DrawTextW(hdc, detail.c_str(), -1, &detailRect, DT_CENTER | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);
    SelectObject(hdc, oldFont);
    DeleteObject(detailFont);
}
