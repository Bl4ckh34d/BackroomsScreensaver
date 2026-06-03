        frame.issued = false;
            StartupProfileLine(L"GPU profile frame dropped: disjoint query read failed.");
            return;
        }
        if (disjoint.Disjoint || disjoint.Frequency == 0) {
            frame.issued = false;
            StartupProfileLine(L"GPU profile frame dropped: timestamp frequency was disjoint.");
            return;
        }
