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

#include "HidWrapper.h"

void HidWrapper::init(BLEHidAdafruit &blehid) {
    _blehid = &blehid;
}

void HidWrapper::setKey(uint8_t keycode) {
    _keyCount[keycode]++;
    if (_keyCount[keycode] == 1) {
        addKey(keycode);
    }
}

void HidWrapper::unsetKey(uint8_t keycode) {
    _keyCount[keycode]--;
    if (_keyCount[keycode] == 0) {
        removeKey(keycode);
    }
}

void HidWrapper::setModifier(Modifier modifier) {
    for (int i = 0; i < 8; i++) {
        if (bitRead(static_cast<uint8_t>(modifier), i)) {
            _modifierCount[i]++;
        }
    }
}

void HidWrapper::unsetModifier(Modifier modifier) {
    for (int i = 0; i < 8; i++) {
        if (bitRead(static_cast<uint8_t>(modifier), i)) {
            _modifierCount[i]--;
        }
    }
}

void HidWrapper::oneShotModifier(Modifier modifier) {
    _oneShotModifier = _oneShotModifier | modifier;
}

void HidWrapper::sendKeyReportIfChanged() {
    bool isChanged = false;

    // normal key check
    if (memcmp(_prevSentKeys, _pressedKeys, sizeof(_prevSentKeys)) != 0) {
        memcpy(_prevSentKeys, _pressedKeys, sizeof(_prevSentKeys));
        isChanged = true;
    }

    // modifier Key check
    Modifier modifier = _oneShotModifier;
    _oneShotModifier = static_cast<Modifier>(0);

    for (int i = 0; i < 8; i++) {
        if (_modifierCount[i] != 0) {
            modifier = static_cast<Modifier>(static_cast<uint8_t>(modifier) | bit(i));
        }
    }
    if (modifier != _prevSentModifier) {
        _prevSentModifier = modifier;
        isChanged = true;
    }

    // send KeyboardReport
    if (isChanged) {
        _blehid->keyboardReport(static_cast<uint8_t>(modifier), _pressedKeys);
    }
}

void HidWrapper::consumerKeyPress(UsageCode usageCode) {
    _blehid->consumerKeyPress(static_cast<uint16_t>(usageCode));
}

void HidWrapper::consumerKeyRelease() {
    _blehid->consumerKeyRelease();
}

void HidWrapper::mouseMove(int8_t x, int8_t y) {
    _blehid->mouseMove(x, y);
}

void HidWrapper::mouseScroll(int8_t scroll) {
    _blehid->mouseScroll(scroll);
}

void HidWrapper::mousePan(int8_t pan) {
    _blehid->mousePan(pan);
}

void HidWrapper::mouseButtonPress(MouseButton button) {
    for (int i = 0; i < 5; i++) {
        if (bitRead(static_cast<uint8_t>(button), i)) {
            _buttonCount[i]++;
        }
    }
    sendMouseButtonReportIfChanged();
}

void HidWrapper::mouseButtonRelease(MouseButton button) {
    for (int i = 0; i < 5; i++) {
        if (bitRead(static_cast<uint8_t>(button), i)) {
            _buttonCount[i]--;
        }
    }
    sendMouseButtonReportIfChanged();
}

void HidWrapper::addKey(uint8_t keycode) {
    // すでに入ってるなら追加しない
    for (int i = 0; i < 6; i++) {
        if (_pressedKeys[i] == keycode) {
            return;
        }
    }
    // 開いているスペースを探して追加
    for (int i = 0; i < 6; i++) {
        if (_pressedKeys[i] == 0) {
            _pressedKeys[i] = keycode;
            return;
        }
    }
    // 満杯ならずらして末尾に追加
    memmove(_pressedKeys, _pressedKeys + 1, 5);
    _pressedKeys[5] = keycode;
}

void HidWrapper::removeKey(uint8_t keycode) {
    int i = 0;
    for (; i < 6; i++) {
        if (_pressedKeys[i] == keycode) {
            _pressedKeys[i] = 0;
            break;
        }
    }
    // 削除したスペースを埋めるためにずらしていく
    // _pressedKeys[6]は常に0が入っているので末尾には0が補充される
    for (; i < 6; i++) {
        _pressedKeys[i] = _pressedKeys[i + 1];
    }
}

void HidWrapper::sendMouseButtonReportIfChanged() {
    MouseButton button = static_cast<MouseButton>(0);

    for (int i = 0; i < 5; i++) {
        if (_buttonCount[i] != 0) {
            button = static_cast<MouseButton>(static_cast<uint8_t>(button) | bit(i));
        }
    }
    if (button != _prevSentButton) {
        _prevSentButton = button;
        _blehid->mouseButtonPress(static_cast<uint8_t>(button));
    }
}
