const int kDebugPropCount = 16;

int WrapDebugPropIndex(int index) {
    int count = std::max(1, kDebugPropCount);
    index %= count;
    if (index < 0) index += count;
    return index;
}

const wchar_t* DebugPropName(int index) {
    switch (WrapDebugPropIndex(index)) {
    case 0: return L"Office chair modern";
    case 1: return L"Office chair classic";
    case 2: return L"Office chair task";
    case 3: return L"Office chair tipped";
    case 4: return L"Filing cabinet";
    case 5: return L"Office desk";
    case 6: return L"Trash bin upright";
    case 7: return L"Trash bin tipped";
    case 8: return L"Desk lamp";
    case 9: return L"Audio cassette";
    case 10: return L"Air vent";
    case 11: return L"Exit sign";
    case 12: return L"Ceiling lamp 01";
    case 13: return L"Ceiling lamp 02";
    case 14: return L"Ceiling lamp 03";
    case 15: return L"Ceiling lamp 04";
    default: return L"Prop";
    }
}
