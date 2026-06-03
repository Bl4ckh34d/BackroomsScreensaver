                auto percentText = [](int value, wchar_t* out, size_t count) {
                    swprintf_s(out, count, L"%d%%", std::clamp(value, 0, 400));
                };
                auto meterText = [](int value, wchar_t* out, size_t count) {
                    swprintf_s(out, count, L"%dm", std::max(0, value));
                };
                text(L"Environment", {40, 80, 472, 116}, bodyFont, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                markerLine(132, 118, 380, 114, dimInkPen);
                wchar_t valueText[24]{};
                percentText(menuRuntime_.customSpec.mapDirtPercent, valueText, std::size(valueText));
                timingStepper(CustomGameMenuControl::EnvDirtMinus, CustomGameMenuControl::EnvDirtPlus, 116, L"Grime", valueText);
                percentText(menuRuntime_.customSpec.paperDensityPercent, valueText, std::size(valueText));
                timingStepper(CustomGameMenuControl::EnvPaperMinus, CustomGameMenuControl::EnvPaperPlus, 146, L"Papers", valueText);
                percentText(menuRuntime_.customSpec.propDensityPercent, valueText, std::size(valueText));
                timingStepper(CustomGameMenuControl::EnvPropMinus, CustomGameMenuControl::EnvPropPlus, 176, L"Props", valueText);
                percentText(menuRuntime_.customSpec.lampOnPercent, valueText, std::size(valueText));
                timingStepper(CustomGameMenuControl::EnvLampOnMinus, CustomGameMenuControl::EnvLampOnPlus, 216, L"Lamps", valueText);
                percentText(menuRuntime_.customSpec.lampFlickerPercent, valueText, std::size(valueText));
                timingStepper(CustomGameMenuControl::EnvLampFlickerMinus, CustomGameMenuControl::EnvLampFlickerPlus, 246, L"Flicker", valueText);
                percentText(menuRuntime_.customSpec.lampSparkPercent, valueText, std::size(valueText));
                timingStepper(CustomGameMenuControl::EnvLampSparkMinus, CustomGameMenuControl::EnvLampSparkPlus, 276, L"Sparks", valueText);
                meterText(menuRuntime_.customSpec.fogStartMeters, valueText, std::size(valueText));
                timingStepper(CustomGameMenuControl::EnvFogStartMinus, CustomGameMenuControl::EnvFogStartPlus, 316, L"Fog start", valueText);
                meterText(menuRuntime_.customSpec.fogEndMeters, valueText, std::size(valueText));
                timingStepper(CustomGameMenuControl::EnvFogEndMinus, CustomGameMenuControl::EnvFogEndPlus, 346, L"Fog end", valueText);
                percentText(menuRuntime_.customSpec.fogDarknessPercent, valueText, std::size(valueText));
                timingStepper(CustomGameMenuControl::EnvFogDarkMinus, CustomGameMenuControl::EnvFogDarkPlus, 376, L"Fog dark", valueText);
                button(CustomGameMenuControl::ScareDetailBack, L"Back");
                finishGdiDraw();
