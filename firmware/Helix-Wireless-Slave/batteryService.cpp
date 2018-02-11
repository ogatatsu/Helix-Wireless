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

#include "batteryService.h"
#include "Timer.h"
#include "analogReadVdd.h"
#include "config.h"
#include "util.h"

const static long MIN_ANALOG_VALUE = MIN_BATTERY_VOLTAGE / 3.6 * 16383;
const static long MAX_ANALOG_VALUE = MAX_BATTERY_VOLTAGE / 3.6 * 16383;

class _BatteryService : public Timer {
  public:
    _BatteryService()
        : Timer(BATTERY_SAMPLING_INTERVAL, true) {
    }

    void init(BLEBas &blebas) {
        _blebas = &blebas;
        _level = readLevel();
        _blebas->write(_level);
        startTimer();
    }

    void onTimer() override {
        uint8_t level = readLevel();
        if (_level != level) {
            _level = level;
            _blebas->write(level);
        }
    }

  private:
    uint8_t readLevel() {
        uint16_t val = analogReadVdd();
        int result = map(val, MIN_ANALOG_VALUE, MAX_ANALOG_VALUE, 0, 100);
        result = clamp(result, 0, 100);
        return result;
    }

    BLEBas *_blebas;
    uint8_t _level;
};

static _BatteryService bas;

void startBatteryService(BLEBas &blebas) {
    bas.init(blebas);
}
