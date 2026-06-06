                text(L"Settings", {30, 22, 482, 68}, titleFont, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                markerLine(104, 68, 408, 62, inkPen);

                auto settingsHover = [&](SettingsBoardControl control) {
                    return menuRuntime_.settingsHoverControl == static_cast<int>(control);
                };
                auto sbutton = [&](SettingsBoardControl control, const wchar_t* label) {
                    RECT r{};
                    if (!SettingsBoardControlPixelRect(control, r)) return;
                    if (settingsHover(control)) fillYellow(r, 1);
                    SelectObject(dc, settingsHover(control) ? hoverPen : inkPen);
                    RoundRect(dc, r.left, r.top, r.right, r.bottom, 10, 10);
                    text(label, r, bodyFont, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                };
                auto scheck = [&](SettingsBoardControl control, const wchar_t* label, bool checked) {
                    RECT r{};
                    if (!SettingsBoardControlPixelRect(control, r)) return;
                    if (settingsHover(control)) fillYellow(r, 1);
                    RECT box{r.left + 10, r.top + 6, r.left + 26, r.top + 22};
                    if (checked) fillYellow({box.left - 3, box.top - 3, box.right + 3, box.bottom + 3}, 0);
                    SelectObject(dc, inkPen);
                    Rectangle(dc, box.left, box.top, box.right, box.bottom);
                    if (checked) {
                        HGDIOBJ previousPen = SelectObject(dc, boldInkPen);
                        MoveToEx(dc, box.left + 3, box.top + 8, nullptr);
                        LineTo(dc, box.left + 7, box.bottom - 4);
                        LineTo(dc, box.right - 3, box.top + 3);
                        SelectObject(dc, previousPen);
                    }
                    text(label, {r.left + 36, r.top, r.right - 10, r.bottom}, bodyFont);
                };
                auto sstep = [&](SettingsBoardControl minusControl, SettingsBoardControl plusControl, int y, const wchar_t* label, const wchar_t* value) {
                    text(label, {52, y, 176, y + 28}, bodyFont);
                    sbutton(minusControl, L"-");
                    text(value, {220, y, 424, y + 28}, bodyFont, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                    sbutton(plusControl, L"+");
                };
                auto percentValue = [&](int value) {
                    static wchar_t buffer[32]{};
                    swprintf_s(buffer, L"%d%%", value);
                    return buffer;
                };
                auto intValue = [&](int value) {
                    static wchar_t buffer[32]{};
                    swprintf_s(buffer, L"%d", value);
                    return buffer;
                };
                auto floatValue = [&](float value) {
                    static wchar_t buffer[32]{};
                    swprintf_s(buffer, L"%.2f", value);
                    return buffer;
                };
                auto resolutionValue = [&](int width, int height) {
                    static wchar_t buffer[64]{};
                    swprintf_s(buffer, L"%d x %d", width, height);
                    return buffer;
                };
                auto aaValue = [&](int value) {
                    static wchar_t buffer[32]{};
                    switch (NormalizeAntiAliasingMode(value)) {
                    case 0: wcscpy_s(buffer, L"Off"); break;
                    case 1: wcscpy_s(buffer, L"FXAA"); break;
                    case 2: wcscpy_s(buffer, L"MSAA 2x"); break;
                    case 4: wcscpy_s(buffer, L"MSAA 4x"); break;
                    case 8: wcscpy_s(buffer, L"MSAA 8x"); break;
                    case 16: wcscpy_s(buffer, L"MSAA 16x"); break;
                    default: wcscpy_s(buffer, L"FXAA"); break;
                    }
                    return buffer;
                };
                auto anisoValue = [&](int value) {
                    static wchar_t buffer[32]{};
                    value = NormalizeTextureAnisotropy(value);
                    if (value <= 1) wcscpy_s(buffer, L"Off");
                    else swprintf_s(buffer, L"%dx", value);
                    return buffer;
                };
                auto tabButton = [&](SettingsBoardControl control, const wchar_t* label, int tab) {
                    RECT r{};
                    if (!SettingsBoardControlPixelRect(control, r)) return;
                    if (menuRuntime_.settingsBoardTab == tab || settingsHover(control)) fillYellow(r, 1);
                    SelectObject(dc, settingsHover(control) ? hoverPen : inkPen);
                    RoundRect(dc, r.left, r.top, r.right, r.bottom, 10, 10);
                    text(label, r, bodyFont, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                };
                auto keyControl = [&](SettingsBoardControl control, int actionIndex, int y) {
                    if (actionIndex < 0 || actionIndex >= kGameInputActionCount) return;
                    const GameInputBindingDef& binding = kGameInputBindings[static_cast<size_t>(actionIndex)];
                    text(binding.label, {52, y, 236, y + 22}, bodyFont);
                    RECT r{};
                    if (!SettingsBoardControlPixelRect(control, r)) return;
                    bool capture = menuRuntime_.settingsCaptureAction == actionIndex;
                    if (settingsHover(control) || capture) fillYellow(r, 1);
                    SelectObject(dc, capture ? hoverPen : inkPen);
                    RoundRect(dc, r.left, r.top, r.right, r.bottom, 8, 8);
                    std::wstring display = capture ? L"Press a key..." :
                        KeyDisplayName(GameActionKey(menuRuntime_.settingsBoardSettings, binding.action));
                    text(display.c_str(), r, bodyFont, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                };

                tabButton(SettingsBoardControl::TabSystem, L"System", 0);
                tabButton(SettingsBoardControl::TabGraphics, L"Gfx", 1);
                tabButton(SettingsBoardControl::TabGame, L"Game", 2);
                tabButton(SettingsBoardControl::TabControls, L"Controls", 3);
                tabButton(SettingsBoardControl::TabAudio, L"Audio", 4);
                markerLine(42, 122, 470, 116, dimInkPen);

                const Settings& board = menuRuntime_.settingsBoardSettings;
                if (menuRuntime_.settingsBoardTab == 0) {
                    scheck(SettingsBoardControl::Fullscreen, L"Fullscreen", board.gameFullscreen);
                    sstep(SettingsBoardControl::ResolutionMinus, SettingsBoardControl::ResolutionPlus, 180, L"Resolution",
                        resolutionValue(board.gameResolutionWidth, board.gameResolutionHeight));
                    wchar_t fps[32]{};
                    swprintf_s(fps, L"%d fps", board.gameFrameRateLimit);
                    sstep(SettingsBoardControl::FrameRateMinus, SettingsBoardControl::FrameRatePlus, 218, L"Frame cap", fps);
                    scheck(SettingsBoardControl::Warp, L"Allow WARP fallback", board.allowWarpFallback);
                } else if (menuRuntime_.settingsBoardTab == 1) {
                    sstep(SettingsBoardControl::RenderScaleMinus, SettingsBoardControl::RenderScalePlus, 142, L"Render scale", percentValue(board.renderScalePercent));
                    sstep(SettingsBoardControl::AntiAliasingMinus, SettingsBoardControl::AntiAliasingPlus, 180, L"AA", aaValue(board.antiAliasing));
                    sstep(SettingsBoardControl::AnisotropyMinus, SettingsBoardControl::AnisotropyPlus, 218, L"Aniso", anisoValue(board.textureAnisotropy));
                    sstep(SettingsBoardControl::ExposureMinus, SettingsBoardControl::ExposurePlus, 256, L"Exposure", floatValue(board.exposure));
                    sstep(SettingsBoardControl::BloomMinus, SettingsBoardControl::BloomPlus, 294, L"Bloom", floatValue(board.bloomAmount));
                    sstep(SettingsBoardControl::MotionBlurMinus, SettingsBoardControl::MotionBlurPlus, 332, L"Motion blur", floatValue(board.motionBlurAmount));
                    sstep(SettingsBoardControl::AirDensityMinus, SettingsBoardControl::AirDensityPlus, 370, L"Air motes", floatValue(board.airParticleDensity));
                } else if (menuRuntime_.settingsBoardTab == 2) {
                    scheck(SettingsBoardControl::MonsterIgnorePlayer, L"Monster ignores player", board.monsterIgnorePlayer);
                    scheck(SettingsBoardControl::InfiniteStamina, L"Infinite stamina", board.debugInfiniteStamina);
                    scheck(SettingsBoardControl::Invincible, L"Invincible", board.debugInvincible);
                } else if (menuRuntime_.settingsBoardTab == 3) {
                    sstep(SettingsBoardControl::MouseSensitivityMinus, SettingsBoardControl::MouseSensitivityPlus, 142, L"Mouse sens", floatValue(board.mouseSensitivity));
                    scheck(SettingsBoardControl::InvertMouseY, L"Invert Y axis", board.invertMouseY);
                    keyControl(SettingsBoardControl::KeyMoveForward, static_cast<int>(GameInputAction::MoveForward), 204);
                    keyControl(SettingsBoardControl::KeyMoveBackward, static_cast<int>(GameInputAction::MoveBackward), 226);
                    keyControl(SettingsBoardControl::KeyMoveLeft, static_cast<int>(GameInputAction::MoveLeft), 248);
                    keyControl(SettingsBoardControl::KeyMoveRight, static_cast<int>(GameInputAction::MoveRight), 270);
                    keyControl(SettingsBoardControl::KeySprint, static_cast<int>(GameInputAction::Sprint), 292);
                    keyControl(SettingsBoardControl::KeyCrouch, static_cast<int>(GameInputAction::Crouch), 314);
                    keyControl(SettingsBoardControl::KeyInteract, static_cast<int>(GameInputAction::Interact), 336);
                    keyControl(SettingsBoardControl::KeyFlashlight, static_cast<int>(GameInputAction::Flashlight), 358);
                    keyControl(SettingsBoardControl::KeyPause, static_cast<int>(GameInputAction::Pause), 380);
                } else {
                    scheck(SettingsBoardControl::MuteAudio, L"Mute audio", board.audioMuted);
                    sstep(SettingsBoardControl::MasterMinus, SettingsBoardControl::MasterPlus, 194, L"Master", floatValue(board.audioMasterVolume));
                    sstep(SettingsBoardControl::MusicMinus, SettingsBoardControl::MusicPlus, 232, L"Music", floatValue(board.audioMusicVolume));
                    sstep(SettingsBoardControl::EffectsMinus, SettingsBoardControl::EffectsPlus, 270, L"Effects", floatValue(board.audioEffectsVolume));
                    sstep(SettingsBoardControl::AmbienceMinus, SettingsBoardControl::AmbiencePlus, 308, L"Ambience", floatValue(board.audioAmbienceVolume));
                    sstep(SettingsBoardControl::MonsterMinus, SettingsBoardControl::MonsterPlus, 346, L"Monster", floatValue(board.audioMonsterVolume));
                }

                sbutton(SettingsBoardControl::Back, L"Back");
                sbutton(SettingsBoardControl::Save, L"Save");
                finishGdiDraw();
