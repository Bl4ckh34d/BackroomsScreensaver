            text(L"Layer", {42, 78, 132, 104}, bodyFont);
            SelectObject(dc, inkPen);
            RoundRect(dc, 142, 76, 302, 108, 8, 8);
            markerLine(278, 88, 286, 96, inkPen);
            markerLine(286, 96, 294, 88, inkPen);
            text(L"Layer 1", {154, 76, 292, 108}, bodyFont);

            text(L"Jump scares", {42, 104, 470, 126}, smallFont);
            markerLine(42, 124, 174, 119, dimInkPen);
            row(CustomGameMenuControl::BrokenLampScares, L"Broken lamps", menuRuntime_.customSpec.brokenLampScares);
            row(CustomGameMenuControl::AirVentScares, L"Air vents", menuRuntime_.customSpec.airVentScares);
            row(CustomGameMenuControl::WaterScares, L"Water damage", menuRuntime_.customSpec.waterScares);
            row(CustomGameMenuControl::BloodWorldScares, L"Blood world", menuRuntime_.customSpec.bloodWorldScares);
            row(CustomGameMenuControl::FleshWorldScares, L"Flesh world", menuRuntime_.customSpec.fleshWorldScares);
            detailButton(CustomGameMenuControl::BrokenLampScareDetails);
            detailButton(CustomGameMenuControl::AirVentScareDetails);
            detailButton(CustomGameMenuControl::WaterScareDetails);
            detailButton(CustomGameMenuControl::BloodWorldScareDetails);
            detailButton(CustomGameMenuControl::FleshWorldScareDetails);

            text(L"Bosses", {42, 232, 250, 254}, smallFont);
            markerLine(42, 252, 118, 248, dimInkPen);
            row(CustomGameMenuControl::OmukadeBoss, L"Omukade", menuRuntime_.customSpec.omukadeBoss);

            text(L"Collectibles", {274, 232, 492, 254}, smallFont);
            markerLine(274, 252, 400, 248, dimInkPen);
            row(CustomGameMenuControl::EightPages, L"8 pages collectible", menuRuntime_.customSpec.eightPages);
            button(CustomGameMenuControl::EnvironmentDetails, L"Environment");

            text(L"Level size", {42, 300, 470, 320}, smallFont);
            markerLine(42, 319, 156, 315, dimInkPen);
            stepper(CustomGameMenuControl::SizeXMinus, CustomGameMenuControl::SizeXPlus, 320, L"X", menuRuntime_.customSpec.mazeWidth);
            stepper(CustomGameMenuControl::SizeYMinus, CustomGameMenuControl::SizeYPlus, 354, L"Y", menuRuntime_.customSpec.mazeHeight);
            stepper(CustomGameMenuControl::RoomCountMinus, CustomGameMenuControl::RoomCountPlus, 388, L"Rooms", menuRuntime_.customSpec.roomCount);
            button(CustomGameMenuControl::Back, L"Back");
            button(CustomGameMenuControl::Start, L"Start");

            finishGdiDraw();
