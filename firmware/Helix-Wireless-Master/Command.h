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

#include "HidWrapper.h"
#include "LayerController.h"
#include "SpeedController.h"
#include "Timer.h"
#include "config.h"
#include "keycode.h"

/*------------------------------------------------------------------*/
class Command {
  public:
    static void init(BLEHidAdafruit &blehid);

    void apply(bool pressed);
    virtual void onPress() {}
    virtual void onRelease() {}

  protected:
    static Command *_lastPressedCommand;
    static HidWrapper _hid;
    static LayerController _layerController;
    static SpeedController _speedController;

  private:
    bool _prevPressed = false;
};
/*------------------------------------------------------------------*/
class NormalKey : public Command {
  public:
    NormalKey(uint8_t keycode);
    void onPress() override;
    void onRelease() override;

  private:
    const uint8_t _keycode;
};
/*------------------------------------------------------------------*/
class ModifierKey : public Command {
  public:
    ModifierKey(Modifier modifier);
    void onPress() override;
    void onRelease() override;

  private:
    const Modifier _modifier;
};
/*------------------------------------------------------------------*/
class CombinationKey : public Command {
  public:
    CombinationKey(Modifier modifier, uint8_t keycode);
    void onPress() override;
    void onRelease() override;

  private:
    const Modifier _modifier;
    const uint8_t _keycode;
};
/*------------------------------------------------------------------*/
class ModifierTap : public Command, public Timer {
  public:
    ModifierTap(Modifier modifier, uint8_t keycode);
    void onPress() override;
    void onRelease() override;
    void onTimer() override;

  private:
    const Modifier _modifier;
    const uint8_t _keycode;
};
/*------------------------------------------------------------------*/
class OneShotModifier : public Command {
  public:
    OneShotModifier(Modifier modifier);
    void onPress() override;
    void onRelease() override;

  private:
    const Modifier _modifier;
};
/*------------------------------------------------------------------*/
class DivideLayer : public Command {
  public:
    DivideLayer(Command *commands[LAYER_SIZE]);
    void onPress() override;
    void onRelease() override;

  private:
    Command **_commands;
    Command *_executingCommand;
};
/*------------------------------------------------------------------*/
class LayerTap : public Command, public Timer {
  public:
    LayerTap(uint8_t layerNumber, uint8_t keycode);
    void onPress() override;
    void onRelease() override;
    void onTimer() override;

  private:
    const uint8_t _layerNumber;
    const uint8_t _keycode;
};
/*------------------------------------------------------------------*/
class ToggleLayer : public Command {
  public:
    ToggleLayer(uint8_t layerNumber);
    void onPress() override;

  private:
    const uint8_t _layerNumber;
};
/*------------------------------------------------------------------*/
class SwitchLayer : public Command {
  public:
    SwitchLayer(uint8_t layerNumber);
    void onPress() override;
    void onRelease() override;

  private:
    const uint8_t _layerNumber;
};
/*------------------------------------------------------------------*/
class OneShotLayer : public Command {
  public:
    OneShotLayer(uint8_t layerNumber);
    void onPress() override;
    void onRelease() override;

  private:
    const uint8_t _layerNumber;
};
/*------------------------------------------------------------------*/
class DetectMultiPress : public Command {
  public:
    DetectMultiPress(Command *commands[], uint len);
    void onPress() override;
    void onRelease() override;

  private:
    Command **_commands;
    Command *_executingCommand;
    const uint _len;
    uint _state = 0;
    unsigned long _lastPressMillis = 0;
};
/*------------------------------------------------------------------*/
class TapOrPress : public Command, public Timer {
  public:
    TapOrPress(uint ms, Command *tapCommand, Command *pressCommand);
    void onPress() override;
    void onRelease() override;
    void onTimer() override;

  private:
    const uint _ms;
    uint8_t _state = 0;
    Command *_tapCommand;
    Command *_pressCommand;
};
/*------------------------------------------------------------------*/
class ConsumerControll : public Command {
  public:
    ConsumerControll(UsageCode usageCode);
    void onPress() override;
    void onRelease() override;

  private:
    UsageCode _usageCode;
};
/*------------------------------------------------------------------*/
class MouseMove : public Command {
  public:
    MouseMove(int8_t x, int8_t y);
    void onPress() override;
    void onRelease() override;

    class Mover : public Timer {
      public:
        Mover();
        void setXY(int8_t x, int8_t y);
        void unsetXY(int8_t x, int8_t y);
        void onTimer() override;

      private:
        void calcXY(int8_t &x, int8_t &y);

        int _x = 0;
        int _y = 0;
        uint8_t _count = 0;
        bool _isInitialMove;
    };

    static Mover _mover;

  private:
    int8_t _x;
    int8_t _y;
};
/*------------------------------------------------------------------*/
class MouseSpeed : public Command {
  public:
    MouseSpeed(int16_t percent);
    void onPress() override;
    void onRelease() override;

  private:
    int16_t _percent;
};
/*------------------------------------------------------------------*/
class MouseScroll : public Command {
  public:
    MouseScroll(int8_t scroll);
    void onPress() override;

  private:
    const int8_t _scroll;
};
/*------------------------------------------------------------------*/
class MousePan : public Command {
  public:
    MousePan(int8_t pan);
    void onPress() override;

  private:
    const int8_t _pan;
};
/*------------------------------------------------------------------*/
class MouseClick : public Command {
  public:
    MouseClick(MouseButton button);
    void onPress() override;
    void onRelease() override;

  private:
    const MouseButton _button;
};
/*------------------------------------------------------------------*/
extern Command *const NoOperation;
extern Command *const Transparent;
extern Command *const SwitchSequenceMode;
extern Command *const ResetConnection;

enum SequenceModeState {
    SEQ_MODE_DISABLE,
    SEQ_MODE_START,
    SEQ_MODE_MATCH,
    SEQ_MODE_KEY_RELEASE,
};

extern enum SequenceModeState sequenceModeState;

/*------------------------------------------------------------------*/
class Macro : public Command, public Timer {
  public:
    class MacroCommand {
      public:
        virtual uint apply() = 0;
    };

    class DownKey : public MacroCommand {
      public:
        DownKey(uint8_t keycode);
        uint apply() override;

      private:
        uint8_t _keycode;
    };

    class UpKey : public MacroCommand {
      public:
        UpKey(uint8_t keycode);
        uint apply() override;

      private:
        uint8_t _keycode;
    };

    class DownModifier : public MacroCommand {
      public:
        DownModifier(Modifier modifier);
        uint apply() override;

      private:
        Modifier _modifier;
    };

    class UpModifier : public MacroCommand {
      public:
        UpModifier(Modifier modifier);
        uint apply() override;

      private:
        Modifier _modifier;
    };

    class Wait : public MacroCommand {
      public:
        Wait(uint delay);
        uint apply() override;

      private:
        uint _delay;
    };

    Macro(MacroCommand **mcommands, uint len);
    void onPress() override;
    void onTimer() override;

  private:
    MacroCommand **_mcommands;
    const uint _len;
    uint _state;
    bool _isRunning = false;
};
