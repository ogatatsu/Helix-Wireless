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

#include "Command.h"
#include "Timer.h"
#include "config.h"
#include "util.h"
#include <Arduino.h>

/*------------------------------------------------------------------*/
/* Command
 *------------------------------------------------------------------*/
// static method
void Command::init(BLEHidAdafruit &blehid) {
    _hid.init(blehid);
}

// static member
Command *Command::_lastPressedCommand = nullptr;
HidWrapper Command::_hid;
LayerController Command::_layerController;
SpeedController Command::_speedController;

// instance method
void Command::apply(bool pressed) {
    if (_prevPressed == false && pressed == true) { //FALL
        _lastPressedCommand = this;
        onPress();
    } else if (_prevPressed == true && pressed == false) { //RISE
        onRelease();
    }
    _prevPressed = pressed;
}

/*------------------------------------------------------------------*/
/* NormalKey
 *------------------------------------------------------------------*/
NormalKey::NormalKey(uint8_t keycode)
    : _keycode(keycode) {
}

void NormalKey::onPress() {
    _hid.setKey(_keycode);
    _hid.sendKeyReportIfChanged();
}

void NormalKey::onRelease() {
    _hid.unsetKey(_keycode);
    _hid.sendKeyReportIfChanged();
}

/*------------------------------------------------------------------*/
/* ModifierKey
 *------------------------------------------------------------------*/
ModifierKey::ModifierKey(Modifier modifier)
    : _modifier(modifier) {
}

void ModifierKey::onPress() {
    _hid.setModifier(_modifier);
    _hid.sendKeyReportIfChanged();
}

void ModifierKey::onRelease() {
    _hid.unsetModifier(_modifier);
    _hid.sendKeyReportIfChanged();
}

/*------------------------------------------------------------------*/
/* CombinationKey
 *------------------------------------------------------------------*/
CombinationKey::CombinationKey(Modifier modifier, uint8_t keycode)
    : _modifier(modifier), _keycode(keycode) {
}

void CombinationKey::onPress() {
    _hid.setKey(_keycode);
    _hid.setModifier(_modifier);
    _hid.sendKeyReportIfChanged();
}

void CombinationKey::onRelease() {
    _hid.unsetKey(_keycode);
    _hid.unsetModifier(_modifier);
    _hid.sendKeyReportIfChanged();
}

/*------------------------------------------------------------------*/
/* ModifierTap
 *------------------------------------------------------------------*/
ModifierTap::ModifierTap(Modifier modifier, uint8_t keycode)
    : Timer(TAP_SPEED, false), _modifier(modifier), _keycode(keycode) {
}

void ModifierTap::onPress() {
    _hid.setModifier(_modifier);
    _hid.sendKeyReportIfChanged();
}

void ModifierTap::onRelease() {
    _hid.unsetModifier(_modifier);
    if (_lastPressedCommand == this) {
        _hid.setKey(_keycode);
        _hid.sendKeyReportIfChanged();
        startTimer();
    } else {
        _hid.sendKeyReportIfChanged();
    }
}

void ModifierTap::onTimer() {
    _hid.unsetKey(_keycode);
    _hid.sendKeyReportIfChanged();
}

/*------------------------------------------------------------------*/
/* OneShotModifier
 *------------------------------------------------------------------*/
OneShotModifier::OneShotModifier(Modifier modifier)
    : _modifier(modifier) {
}

void OneShotModifier::onPress() {
    _hid.setModifier(_modifier);
    _hid.sendKeyReportIfChanged();
}

void OneShotModifier::onRelease() {
    _hid.unsetModifier(_modifier);

    if (_lastPressedCommand == this) {
        _hid.oneShotModifier(_modifier);
    } else {
        _hid.sendKeyReportIfChanged();
    }
}

/*------------------------------------------------------------------*/
/* DivideLayer
 *------------------------------------------------------------------*/
DivideLayer::DivideLayer(Command *commands[LAYER_SIZE])
    : _commands(commands) {
    for (int i = 0; i < LAYER_SIZE; i++) {
        if (_commands[i] == nullptr) {
            _commands[i] = Transparent;
        }
    }
}

void DivideLayer::onPress() {

    // 現在のレイヤーの状態を取得
    bool layer[LAYER_SIZE];
    _layerController.getState(layer);

    // 適切なコマンドが見つからなかったらNopが実行されるようにしておく
    _executingCommand = NoOperation;

    // layerを上から舐めていってonのlayerを探す
    int i = LAYER_SIZE - 1;
    for (; i >= 0; i--) {
        if (layer[i] == true) {
            break;
        }
    }
    // onのレイヤーでもそのlayerのコマンドがTransparentならさらに下を探していく
    for (; i >= 0; i--) {
        if (_commands[i] == Transparent) {
            continue;
        }
        // 見つかった
        _executingCommand = _commands[i];
        break;
    }
    // 委託する
    _lastPressedCommand = _executingCommand;
    _executingCommand->onPress();
}

void DivideLayer::onRelease() {
    _executingCommand->onRelease();
}

/*------------------------------------------------------------------*/
/* LayerTap
 *------------------------------------------------------------------*/
LayerTap::LayerTap(uint8_t layerNumber, uint8_t keycode)
    : Timer(TAP_SPEED, false), _layerNumber(layerNumber), _keycode(keycode) {
}

void LayerTap::onPress() {
    _layerController.on(_layerNumber);
}

void LayerTap::onRelease() {
    _layerController.off(_layerNumber);
    if (_lastPressedCommand == this) {
        _hid.setKey(_keycode);
        _hid.sendKeyReportIfChanged();
        startTimer();
    }
}

void LayerTap::onTimer() {
    _hid.unsetKey(_keycode);
    _hid.sendKeyReportIfChanged();
}

/*------------------------------------------------------------------*/
/* ToggleLayer
 *------------------------------------------------------------------*/
ToggleLayer::ToggleLayer(uint8_t layerNumber)
    : _layerNumber(layerNumber) {
}

void ToggleLayer::onPress() {
    _layerController.toggle(_layerNumber);
}

/*------------------------------------------------------------------*/
/* SwitchLayer
 *------------------------------------------------------------------*/
SwitchLayer::SwitchLayer(uint8_t layerNumber)
    : _layerNumber(layerNumber) {
}

void SwitchLayer::onPress() {
    _layerController.on(_layerNumber);
}

void SwitchLayer::onRelease() {
    _layerController.off(_layerNumber);
}

/*------------------------------------------------------------------*/
/* OneShotLayer
 *------------------------------------------------------------------*/
OneShotLayer::OneShotLayer(uint8_t layerNumber)
    : _layerNumber(layerNumber) {
}

void OneShotLayer::onPress() {
    _layerController.on(_layerNumber);
}

void OneShotLayer::onRelease() {
    _layerController.off(_layerNumber);

    if (_lastPressedCommand == this) {
        _layerController.oneShot(_layerNumber);
    }
}

/*------------------------------------------------------------------*/
/* DetectMultiPress
 *------------------------------------------------------------------*/
DetectMultiPress::DetectMultiPress(Command *commands[], uint len)
    : _commands(commands), _len(len) {
}

void DetectMultiPress::onPress() {
    unsigned long currentMillis = millis();
    if ((unsigned long)(currentMillis - _lastPressMillis) <= MULTI_PRESS_TERM) {
        _state++;
    } else {
        _state = 0;
    }
    _lastPressMillis = currentMillis;

    if (_state == _len) {
        _state = 0;
    }

    _lastPressedCommand = _executingCommand = _commands[_state];
    _executingCommand->onPress();
}

void DetectMultiPress::onRelease() {
    _executingCommand->onRelease();
}

/*------------------------------------------------------------------*/
/* TapOrPress
 *------------------------------------------------------------------*/
TapOrPress::TapOrPress(uint ms, Command *tapCommand, Command *pressCommand)
    : Timer(1, false), _ms(ms), _tapCommand(tapCommand), _pressCommand(pressCommand) {
}
// _state = 0 未実行
// _state = 1 ms待ち
// _state = 2 tap確定
// _state = 3 press確定
void TapOrPress::onPress() {
    if (_state == 0) {
        _state = 1;
        changePeriod(_ms);
        startTimer();
    }
}

void TapOrPress::onRelease() {
    if (_state == 1) {
        _state = 2;
        _lastPressedCommand = _tapCommand;
        _tapCommand->onPress();
        changePeriod(TAP_SPEED);
    } else if (_state == 3) {
        _pressCommand->onRelease();
        _state = 0;
    }
}

void TapOrPress::onTimer() {
    if (_state == 1) {
        _state = 3;
        _lastPressedCommand = _pressCommand;
        _pressCommand->onPress();
    } else if (_state == 2) {
        _tapCommand->onRelease();
        _state = 0;
    }
}

/*------------------------------------------------------------------*/
/* ConsumerControll
 *------------------------------------------------------------------*/
ConsumerControll::ConsumerControll(UsageCode usageCode)
    : _usageCode(usageCode) {
}

void ConsumerControll::onPress() {
    _hid.consumerKeyPress(_usageCode);
}
void ConsumerControll::onRelease() {
    _hid.consumerKeyRelease();
}

/*------------------------------------------------------------------*/
/* MouseMove
 *------------------------------------------------------------------*/
MouseMove::Mover::Mover() : Timer(1, true) {
}

void MouseMove::Mover::setXY(int8_t x, int8_t y) {
    _count++;
    _x += x;
    _y += y;
    calcXY(x, y);
    _hid.mouseMove(x, y);
    if (_count == 1) {
        _isInitialMove = true;
        changePeriod(MOUSEKEY_DELAY);
        startTimer();
    }
}

void MouseMove::Mover::unsetXY(int8_t x, int8_t y) {
    _count--;
    _x -= x;
    _y -= y;
    if (_count == 0) {
        stopTimer();
    }
}

void MouseMove::Mover::onTimer() {
    int8_t x, y;
    calcXY(x, y);
    _hid.mouseMove(x, y);
    if (_isInitialMove == true) {
        _isInitialMove = false;
        changePeriod(MOUSEKEY_INTERVAL);
    }
}

void MouseMove::Mover::calcXY(int8_t &x, int8_t &y) {
    double factor = _speedController.getFactor();
    int ix, iy;
    ix = round(_x * factor);
    ix = clamp(ix, -127, 127);
    iy = round(_y * factor);
    iy = clamp(iy, -127, 127);
    x = static_cast<int8_t>(ix);
    y = static_cast<int8_t>(iy);
}

MouseMove::Mover MouseMove::_mover;

MouseMove::MouseMove(int8_t x, int8_t y)
    : _x(x), _y(y) {
}

void MouseMove::onPress() {
    _mover.setXY(_x, _y);
}

void MouseMove::onRelease() {
    _mover.unsetXY(_x, _y);
}

/*------------------------------------------------------------------*/
/* MouseSpeed
 *------------------------------------------------------------------*/
MouseSpeed::MouseSpeed(int16_t percent)
    : _percent(percent) {
}

void MouseSpeed::onPress() {
    _speedController.set(_percent);
}

void MouseSpeed::onRelease() {
    _speedController.unset(_percent);
}

/*------------------------------------------------------------------*/
/* MouseScroll
 *------------------------------------------------------------------*/
MouseScroll::MouseScroll(int8_t scroll)
    : _scroll(scroll) {
}

void MouseScroll::onPress() {
    _hid.mouseScroll(_scroll);
}

/*------------------------------------------------------------------*/
/* MousePan
 *------------------------------------------------------------------*/
MousePan::MousePan(int8_t pan)
    : _pan(pan) {
}

void MousePan::onPress() {
    _hid.mousePan(_pan);
}

/*------------------------------------------------------------------*/
/* MouseClick
 *------------------------------------------------------------------*/
MouseClick::MouseClick(MouseButton button)
    : _button(button) {
}

void MouseClick::onPress() {
    _hid.mouseButtonPress(_button);
}

void MouseClick::onRelease() {
    _hid.mouseButtonRelease(_button);
}

/*------------------------------------------------------------------*/
/* SwitchSequenceMode
 *------------------------------------------------------------------*/
enum SequenceModeState sequenceModeState = SEQ_MODE_DISABLE;

class _SwitchSequenceMode : public Command {
  public:
    void onPress() {
        if (sequenceModeState == SEQ_MODE_DISABLE) {
            sequenceModeState = SEQ_MODE_START;
        }
    }
};

Command *const SwitchSequenceMode = new _SwitchSequenceMode;

/*------------------------------------------------------------------*/
/* ResetConnection
 *------------------------------------------------------------------*/
class _ResetConnection : public Command {
  public:
    void onPress() {
        Bluefruit.clearBonds();
        NVIC_SystemReset();
    }
};

Command *const ResetConnection = new _ResetConnection;

/*------------------------------------------------------------------*/
/* Literal
 *------------------------------------------------------------------*/
Command *const NoOperation = new Command;
Command *const Transparent = new Command;

/*------------------------------------------------------------------*/
/* Macro
 *------------------------------------------------------------------*/
Macro::DownKey::DownKey(uint8_t keycode)
    : _keycode(keycode) {
}

uint Macro::DownKey::apply() {
    _hid.setKey(_keycode);
    return 0;
}

Macro::UpKey::UpKey(uint8_t keycode)
    : _keycode(keycode) {
}

uint Macro::UpKey::apply() {
    _hid.unsetKey(_keycode);
    return 0;
}

Macro::DownModifier::DownModifier(Modifier modifier)
    : _modifier(modifier) {
}

uint Macro::DownModifier::apply() {
    _hid.setModifier(_modifier);
    return 0;
}

Macro::UpModifier::UpModifier(Modifier modifier)
    : _modifier(modifier) {
}

uint Macro::UpModifier::apply() {
    _hid.unsetModifier(_modifier);
    return 0;
}

Macro::Wait::Wait(uint delay)
    : _delay(delay) {
}

uint Macro::Wait::apply() {
    return _delay;
}

Macro::Macro(MacroCommand **mcommands, uint len)
    : Timer(1, true), _mcommands(mcommands), _len(len) {
}

void Macro::onPress() {
    if (_isRunning) {
        return;
    } else {
        _isRunning = true;
        _state = 0;
        startTimer();
    }
}

void Macro::onTimer() {
    while (_state < _len) {
        uint delay = _mcommands[_state++]->apply();
        if (delay != 0) {
            _hid.sendKeyReportIfChanged();
            changePeriod(delay);
            return;
        }
    }
    _hid.sendKeyReportIfChanged();
    _isRunning = false;
    stopTimer();
}
