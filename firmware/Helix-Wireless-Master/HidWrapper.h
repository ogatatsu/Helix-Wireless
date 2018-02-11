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

#include "UInt8Set.h"
#include "keycode.h"
#include <bluefruit.h>

// BLEHidAdafruitをラップしたクラス
class HidWrapper {
  public:
    void init(BLEHidAdafruit &blehid);

    // Keyboard API
    // setKeyをした後でsendReportIfKeyChangedを呼び出すことでキーを送る。
    // 何回キーをsetしたかを覚えてるので複数回同じキーコードでsetKeyを呼び出したら、
    // 同じ回数unsetKeyを呼び出すまではそのキーコードはsetされ続ける。
    // これにより別のスイッチに同じキーコードを割り当てたとしても正しく動作する。
    void setKey(uint8_t keycode);

    void unsetKey(uint8_t keycode);

    void setModifier(Modifier modifier);

    void unsetModifier(Modifier modifier);

    void oneShotModifier(Modifier modifier);

    void sendKeyReportIfChanged();

    // Consumer API
    // BLEHidAdafruitクラスの同じ名前のメソッドを呼び出すだけ。
    // 同時押しは非対応
    void consumerKeyPress(UsageCode usageCode);

    void consumerKeyRelease();

    // Mouse API
    // mouseButtonPress,Releaseは複数スイッチでの同時押しに対応
    // 他のAPIはBLEHidAdafruitクラスの同じ名前のメソッドを呼び出すだけ。
    void mouseMove(int8_t x, int8_t y);

    void mouseScroll(int8_t scroll);

    void mousePan(int8_t pan);

    void mouseButtonPress(MouseButton button);

    void mouseButtonRelease(MouseButton button);

  private:
    void addKey(uint8_t keycode);
    void removeKey(uint8_t keycode);

    void sendMouseButtonReportIfChanged();

    BLEHidAdafruit *_blehid;

    uint8_t _pressedKeys[7] = {};
    uint8_t _prevSentKeys[6] = {};
    uint8_t _keyCount[256] = {};

    uint8_t _modifierCount[8] = {};
    Modifier _prevSentModifier = static_cast<Modifier>(0);
    Modifier _oneShotModifier = static_cast<Modifier>(0);

    MouseButton _prevSentButton = static_cast<MouseButton>(0);
    uint8_t _buttonCount[5] = {};
};
