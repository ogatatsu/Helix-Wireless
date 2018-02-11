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

// デバイス情報
#define DEVICE_NAME "Helix Wireless Slave"

// バッテリーの最大電圧、最小電圧
#define MAX_BATTERY_VOLTAGE 3.0
#define MIN_BATTERY_VOLTAGE 2.0

// バッテリーの電圧を測る間隔 (ms)
#define BATTERY_SAMPLING_INTERVAL 60000

// BLEの送信電波強度: -40, -30, -20, -16, -12, -8, -4, 0, 4
#define TX_POWER -12

// LEDの点滅間隔、省電力のためにOFFの方を長めにしてある
#define TURN_ON_INTERVAL 100
#define TURN_OFF_INTERVAL 800

// cherry mx bounce time is <= 5ms
#define DEBOUNCE_DELAY 6
