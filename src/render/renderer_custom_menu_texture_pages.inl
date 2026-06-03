            text(L"Custom Game", {30, 22, 482, 68}, titleFont, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            markerLine(72, 68, 438, 62, inkPen);

            if (menuRuntime_.customSelectedScare == -2) {
#include "renderer_custom_menu_texture_environment_page.inl"
            } else if (menuRuntime_.customSelectedScare >= 0) {
#include "renderer_custom_menu_texture_scare_page.inl"
            } else {
#include "renderer_custom_menu_texture_root_page.inl"
            }
        }
