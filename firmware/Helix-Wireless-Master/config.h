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
#define DEVICE_NAME "Helix Wireless"
#define MANUFACTURER_NAME "..."
#define MODEL_NUMBER "..."

// バッテリーの最大電圧、最小電圧
#define MAX_BATTERY_VOLTAGE 3.0
#define MIN_BATTERY_VOLTAGE 2.0

// バッテリーの電圧を測る間隔 (ms)
#define BATTERY_SAMPLING_INTERVAL 60000

// BLEの送信電波強度: -40, -30, -20, -16, -12, -8, -4, 0, 4
#define TX_POWER -4

// LEDの点滅間隔、省電力のためにOFFの方を長めにしてある (ms)
#define TURN_ON_INTERVAL 100
#define TURN_OFF_INTERVAL 800

// cherry mx bounce time is <= 5ms
#define DEBOUNCE_DELAY 6

// レイヤーのサイズ
#define LAYER_SIZE 8

// ModTap,LayerTapコマンドでタップが発動した時の入力速度 (ms)
#define TAP_SPEED 30

// 複数回押し判定時間 (ms)
#define MULTI_PRESS_TERM 500

// 同時押しキーマップの最大同時押し数
#define MAX_SIMULTANEOUS_PRESS_COUNT 5

// シーケンスキーマップの最大シーケンス数
#define MAX_SEQUENCE_COUNT 5

// MouseMoveコマンドの最初のキープレス時のディレイ
#define MOUSEKEY_DELAY 200

// MouseMoveのマウスカーソルの動く間隔 (ms)
// 下げすぎると不安定になるかも
#define MOUSEKEY_INTERVAL 30
