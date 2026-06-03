#pragma once

#include <windows.h>

#include <array>
#include <cstddef>
#include <string>

// Game input action metadata and default bindings.

enum class GameInputAction {
    MoveForward,
    MoveBackward,
    MoveLeft,
    MoveRight,
    Sprint,
    Crouch,
    Interact,
    Flashlight,
    Pause,
    Count
};

constexpr int kGameInputActionCount = static_cast<int>(GameInputAction::Count);

struct GameInputBindingDef {
    GameInputAction action;
    const wchar_t* label;
    const wchar_t* iniKey;
    int defaultVk;
};

const GameInputBindingDef kGameInputBindings[] = {
    {GameInputAction::MoveForward, L"Move forward", L"KeyMoveForward", 'W'},
    {GameInputAction::MoveBackward, L"Move backward", L"KeyMoveBackward", 'S'},
    {GameInputAction::MoveLeft, L"Move left", L"KeyMoveLeft", 'A'},
    {GameInputAction::MoveRight, L"Move right", L"KeyMoveRight", 'D'},
    {GameInputAction::Sprint, L"Sprint", L"KeySprint", VK_SHIFT},
    {GameInputAction::Crouch, L"Crouch", L"KeyCrouch", VK_CONTROL},
    {GameInputAction::Interact, L"Interact", L"KeyInteract", 'E'},
    {GameInputAction::Flashlight, L"Flashlight", L"KeyFlashlight", 'F'},
    {GameInputAction::Pause, L"Pause / menu", L"KeyPause", VK_ESCAPE}
};

inline std::array<int, kGameInputActionCount> DefaultGameKeyBindings() {
    std::array<int, kGameInputActionCount> keys{};
    for (const GameInputBindingDef& binding : kGameInputBindings) {
        keys[static_cast<size_t>(binding.action)] = binding.defaultVk;
    }
    return keys;
}

inline const GameInputBindingDef& GameInputBinding(GameInputAction action) {
    return kGameInputBindings[static_cast<size_t>(action)];
}

struct Settings;
int GameActionKey(const Settings& settings, GameInputAction action);
void SetGameActionKey(Settings& settings, GameInputAction action, int vk);
void AssignGameActionKey(Settings& settings, GameInputAction action, int vk);
std::wstring KeyDisplayName(int vk);
