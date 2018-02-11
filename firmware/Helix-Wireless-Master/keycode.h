/*
  The MIT License (MIT)

  Copyright (c) 2018 ogatatsu.

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#pragma once

#include <bluefruit.h>

#define _NONE               0

#define _A                  4
#define _B                  5
#define _C                  6
#define _D                  7
#define _E                  8
#define _F                  9
#define _G                  10
#define _H                  11
#define _I                  12
#define _J                  13
#define _K                  14
#define _L                  15
#define _M                  16
#define _N                  17
#define _O                  18
#define _P                  19
#define _Q                  20
#define _R                  21
#define _S                  22
#define _T                  23
#define _U                  24
#define _V                  25
#define _W                  26
#define _X                  27
#define _Y                  28
#define _Z                  29

#define _1                  30
#define _2                  31
#define _3                  32
#define _4                  33
#define _5                  34
#define _6                  35
#define _7                  36
#define _8                  37
#define _9                  38
#define _0                  39

#define _ENTER              40
#define _ESCAPE             41
#define _BACKSPACE          42
#define _TAB                43
#define _SPACE              44
#define _MINUS              45
#define _EQUAL              46
#define _BRACKET_LEFT       47
#define _BRACKET_RIGHT      48
#define _BACKSLASH          49
#define _NON_US_NUMBER_SIGN 50
#define _SEMICOLON          51
#define _QUOTE              52
#define _GRAVE              53
#define _COMMA              54
#define _PERIOD             55
#define _SLASH              56
#define _CAPS_LOCK          57

#define _F1                 58
#define _F2                 59
#define _F3                 60
#define _F4                 61
#define _F5                 62
#define _F6                 63
#define _F7                 64
#define _F8                 65
#define _F9                 66
#define _F10                67
#define _F11                68
#define _F12                69

#define _PRINT_SCREEN       70
#define _SCROLL_LOCK        71
#define _PAUSE              72
#define _INSERT             73
#define _HOME               74
#define _PAGE_UP            75
#define _DELETE             76
#define _END                77
#define _PAGE_DOWN          78
#define _ARROW_RIGHT        79
#define _ARROW_LEFT         80
#define _ARROW_DOWN         81
#define _ARROW_UP           82
#define _NUM_LOCK           83
#define _KEYPAD_DIVIDE      84
#define _KEYPAD_MULTIPLY    85
#define _KEYPAD_SUBTRACT    86
#define _KEYPAD_ADD         87
#define _KEYPAD_ENTER       88
#define _KEYPAD_1           89
#define _KEYPAD_2           90
#define _KEYPAD_3           91
#define _KEYPAD_4           92
#define _KEYPAD_5           93
#define _KEYPAD_6           94
#define _KEYPAD_7           95
#define _KEYPAD_8           96
#define _KEYPAD_9           97
#define _KEYPAD_0           98
#define _KEYPAD_PERIOD      99
#define _NON_US_BACKSLASH   100
#define _APPLICATION        101
#define _POWER              102
#define _KEYPAD_EQUAL       103
#define _F13                104
#define _F14                105
#define _F15                106
#define _F16                107
#define _F17                108
#define _F18                109
#define _F19                110
#define _F20                111
#define _F21                112
#define _F22                113
#define _F23                114
#define _F24                115

#define _INT1               135
#define _INT2               136
#define _INT3               137
#define _INT4               138
#define _INT5               139
#define _INT6               140
#define _INT7               141
#define _INT8               142
#define _INT9               143
#define _LANG1              144
#define _LANG2              145
#define _LANG3              146
#define _LANG4              147
#define _LANG5              148
#define _LANG6              149
#define _LANG7              150
#define _LANG8              151
#define _LANG9              152

// ModifierKeys
enum class Modifier : uint8_t {
    LEFTCTRL   = 1,
    LEFTSHIFT  = 2,
    LEFTALT    = 4,
    LEFTGUI    = 8,
    RIGHTCTRL  = 16,
    RIGHTSHIFT = 32,
    RIGHTALT   = 64,
    RIGHTGUI   = 128,
};

inline Modifier operator|(const Modifier &a, const Modifier &b) {
    return static_cast<Modifier>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

#define _CTRL               Modifier::LEFTCTRL
#define _SHIFT              Modifier::LEFTSHIFT
#define _ALT                Modifier::LEFTALT
#define _GUI                Modifier::LEFTGUI
#define _LEFT_CTRL          Modifier::LEFTCTRL
#define _LEFT_SHIFT         Modifier::LEFTSHIFT
#define _LEFT_ALT           Modifier::LEFTALT
#define _LEFT_GUI           Modifier::LEFTGUI
#define _RIGHT_CTRL         Modifier::RIGHTCTRL
#define _RIGHT_SHIFT        Modifier::RIGHTSHIFT
#define _RIGHT_ALT          Modifier::RIGHTALT
#define _RIGHT_GUI          Modifier::RIGHTGUI

// Mouse
enum class MouseButton : uint8_t {
    LEFT     = 1,
    RIGHT    = 2,
    MIDDLE   = 4,
    BACKWARD = 8,
    FORWARD  = 16,
};

inline MouseButton operator|(const MouseButton &a, const MouseButton &b) {
    return static_cast<MouseButton>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

#define _LEFT_BUTTON        MouseButton::LEFT
#define _RIGHT_BUTTON       MouseButton::RIGHT
#define _MIDDLE_BUTTON      MouseButton::MIDDLE
#define _BACKWARD_BUTTON    MouseButton::BACKWARD
#define _FORWARD_BUTTON     MouseButton::FORWARD

// Consumer Controll
enum class UsageCode : uint16_t {
    PLAY_PAUSE   = 205,
    FAST_FORWARD = 179,
    REWIND       = 180,
    NEXT         = 181,
    PREV         = 182,
    MUTE         = 226,
    VOLUME_UP    = 233,
    VOLUME_DOWN  = 234,
};

#define _PLAY_PAUSE         UsageCode::PLAY_PAUSE
#define _FAST_FORWARD       UsageCode::FAST_FORWARD
#define _REWIND             UsageCode::REWIND
#define _NEXT               UsageCode::NEXT
#define _PREV               UsageCode::PREV
#define _MUTE               UsageCode::MUTE
#define _VOLUME_UP          UsageCode::VOLUME_UP
#define _VOLUME_DOWN        UsageCode::VOLUME_DOWN
