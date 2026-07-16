#pragma once

#include <Core/Types.h>
#include <Core/Container.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace grom
{

enum class EKeyCode : u32
{
    Backspace = VK_BACK,
    Tab = VK_TAB,
    Enter = VK_RETURN,
    Shift = VK_SHIFT,
    Control = VK_CONTROL,
    Alt = VK_MENU,
    Pause = VK_PAUSE,
    Capital = VK_CAPITAL,
    Escape = VK_ESCAPE,
    Space = VK_SPACE,
    End = VK_END,
    Home = VK_HOME,
    Left = VK_LEFT,
    Up = VK_UP,
    Right = VK_RIGHT,
    Down = VK_DOWN,
    Insert = VK_INSERT,
    Delete = VK_DELETE,
    Key0 = '0',
    Key1 = '1',
    Key2 = '2',
    Key3 = '3',
    Key4 = '4',
    Key5 = '5',
    Key6 = '6',
    Key7 = '7',
    Key8 = '8',
    Key9 = '9',
    KeyA = 'A',
    KeyB = 'B',
    KeyC = 'C',
    KeyD = 'D',
    KeyE = 'E',
    KeyF = 'F',
    KeyG = 'G',
    KeyH = 'H',
    KeyI = 'I',
    KeyJ = 'J',
    KeyK = 'K',
    KeyL = 'L',
    KeyM = 'M',
    KeyN = 'N',
    KeyO = 'O',
    KeyP = 'P',
    KeyQ = 'Q',
    KeyR = 'R',
    KeyS = 'S',
    KeyT = 'T',
    KeyU = 'U',
    KeyV = 'V',
    KeyW = 'W',
    KeyX = 'X',
    KeyY = 'Y',
    KeyZ = 'Z',
    LWin = VK_LWIN,
    RWin = VK_RWIN,
    Numpad0 = VK_NUMPAD0,
    Numpad1 = VK_NUMPAD1,
    Numpad2 = VK_NUMPAD2,
    Numpad3 = VK_NUMPAD3,
    Numpad4 = VK_NUMPAD4,
    Numpad5 = VK_NUMPAD5,
    Numpad6 = VK_NUMPAD6,
    Numpad7 = VK_NUMPAD7,
    Numpad8 = VK_NUMPAD8,
    Numpad9 = VK_NUMPAD9,
    Multiply = VK_MULTIPLY,
    Add = VK_ADD,
    Subtract = VK_SUBTRACT,
    Decimal = VK_DECIMAL,
    Divide = VK_DIVIDE,
    F1 = VK_F1,
    F2 = VK_F2,
    F3 = VK_F3,
    F4 = VK_F4,
    F5 = VK_F5,
    F6 = VK_F6,
    F7 = VK_F7,
    F8 = VK_F8,
    F9 = VK_F9,
    F10 = VK_F10,
    F11 = VK_F11,
    F12 = VK_F12,
    NumLock = VK_NUMLOCK,
    ScrollLock = VK_SCROLL,
    LShift = VK_LSHIFT,
    RShift = VK_RSHIFT,
    LControl = VK_LCONTROL,
    RControl = VK_RCONTROL,
    LAlt = VK_LMENU,
    RAlt = VK_RMENU
};

enum class EMouseButton : u32
{
    Left = 0,
    Right = 1,
    Middle = 2,
    X1 = 3,
    X2 = 4
};

struct InputState
{
    TStaticArray<bool, 256> Keys{};
    TStaticArray<bool, 5> MouseButtons{};
    i32 MouseX = 0;
    i32 MouseY = 0;
    i32 MouseDeltaX = 0;
    i32 MouseDeltaY = 0;
    i32 ScrollDelta = 0;
};

void PollInput(HWND hWnd, InputState& state);

} // namespace grom
