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

#include "keyScan.h"
#include "Switch.h"
#include "UInt8Set.h"
#include "queues.h"

// マトリックス回路で使うピンの定義
const static uint8_t OUT0 = 4;  // ROW0
const static uint8_t OUT1 = 11; // ROW1
const static uint8_t OUT2 = 12; // ROW2
const static uint8_t OUT3 = 14; // ROW3
const static uint8_t OUT4 = 16; // ROW4
const static uint8_t outputs[] = {OUT0, OUT1, OUT2, OUT3, OUT4};

const static uint8_t IN0 = 13; // COL0
const static uint8_t IN1 = 15; // COL1
const static uint8_t IN2 = 31; // COL2
const static uint8_t IN3 = 30; // COL3
const static uint8_t IN4 = 29; // COL4
const static uint8_t IN5 = 28; // COL5
const static uint8_t IN6 = 27; // COL6
const static uint8_t inputs[] = {IN0, IN1, IN2, IN3, IN4, IN5, IN6};

// スイッチとマトリックスの定義
// Switch(pin, id)
static Switch sw1(IN0, 1);
static Switch sw2(IN1, 2);
static Switch sw3(IN2, 3);
static Switch sw4(IN3, 4);
static Switch sw5(IN4, 5);
static Switch sw6(IN5, 6);

static Switch sw7(IN0, 13);
static Switch sw8(IN1, 14);
static Switch sw9(IN2, 15);
static Switch sw10(IN3, 16);
static Switch sw11(IN4, 17);
static Switch sw12(IN5, 18);

static Switch sw13(IN0, 25);
static Switch sw14(IN1, 26);
static Switch sw15(IN2, 27);
static Switch sw16(IN3, 28);
static Switch sw17(IN4, 29);
static Switch sw18(IN5, 30);

static Switch sw19(IN0, 37);
static Switch sw20(IN1, 38);
static Switch sw21(IN2, 39);
static Switch sw22(IN3, 40);
static Switch sw23(IN4, 41);
static Switch sw24(IN5, 42);
static Switch sw25(IN6, 43);

static Switch sw26(IN0, 51);
static Switch sw27(IN1, 52);
static Switch sw28(IN2, 53);
static Switch sw29(IN3, 54);
static Switch sw30(IN4, 55);
static Switch sw31(IN5, 56);
static Switch sw32(IN6, 57);

const static uint8_t OUTPUTS_SIZE = sizeof(outputs) / sizeof(outputs[0]);
const static uint8_t INPUTS_SIZE = sizeof(inputs) / sizeof(inputs[0]);

static Switch *switches[OUTPUTS_SIZE][INPUTS_SIZE] = {
    {&sw1,  &sw2,  &sw3,  &sw4,  &sw5,  &sw6,  nullptr},
    {&sw7,  &sw8,  &sw9,  &sw10, &sw11, &sw12, nullptr},
    {&sw13, &sw14, &sw15, &sw16, &sw17, &sw18, nullptr},
    {&sw19, &sw20, &sw21, &sw22, &sw23, &sw24, &sw25},
    {&sw26, &sw27, &sw28, &sw29, &sw30, &sw31, &sw32},
};

// 割り込み用
// 省電力のために常にポーリングはせずに、キー入力割り込みで起きて入力が無くなったら寝るを繰り返す
static uint8_t tmp;
static volatile bool isInterrupted = false;
static QueueHandle_t keyInterruptQueue;

// 起きる
static void key_interrupt_callback() {
    isInterrupted = true;
    xQueueOverwriteFromISR(keyInterruptQueue, &tmp, NULL);
}

// 出力ピンを一括設定
static inline void outputsWrite(int val) {
    for (int i = 0; i < OUTPUTS_SIZE; i++) {
        digitalWrite(outputs[i], val);
    }
}

// ピンの初期化など
static void initMatrix() {
    // サイズが1のキューを通知代わりに使用する
    keyInterruptQueue = xQueueCreate(1, sizeof(tmp));
    // ピンの入力、出力設定
    for (int i = 0; i < OUTPUTS_SIZE; i++) {
        pinMode(outputs[i], OUTPUT);
        digitalWrite(outputs[i], LOW);
    }
    // アクティブロー
    for (int i = 0; i < INPUTS_SIZE; i++) {
        pinMode(inputs[i], INPUT_PULLUP);
        attachInterrupt(inputs[i], key_interrupt_callback, FALLING);
    }
    // スイッチオブジェクトの初期化
    for (int o = 0; o < OUTPUTS_SIZE; o++) {
        for (int i = 0; i < INPUTS_SIZE; i++) {
            if (switches[o][i] == nullptr) {
                continue;
            }
            switches[o][i]->init();
        }
    }
}

// 割り込みが発生してから+(DEBOUNCE_DELAY * 3)msまでの間はキースキャンする。
// キー押し時はキースキャン中に入力ピンの電圧が変わるので押されてる限り割り込みが発生し続ける
static bool needsKeyScan() {
    static unsigned long lastInterruptMillis = 0;

    // 割り込みされたら
    if (isInterrupted) {
        lastInterruptMillis = millis();
        isInterrupted = false;
        return true;
    }
    // 最後に押されてた時間から今の時間までを計算して
    unsigned long currentMillis = millis();
    if ((unsigned long)(currentMillis - lastInterruptMillis) <= (DEBOUNCE_DELAY * 3)) {
        return true;
    }
    return false;
}

static void keyScanTask(void *arg) {
    EventData data = {
        .eventType = SCAN_KEY_EVENT,
    };
    UInt8Set previousIDs;
    UInt8Set &currentIDs = data.ids;

    while (1) {
        if (needsKeyScan()) {
            // スキャン
            outputsWrite(HIGH);
            for (int o = 0; o < OUTPUTS_SIZE; o++) {
                digitalWrite(outputs[o], LOW);
                for (int i = 0; i < INPUTS_SIZE; i++) {
                    if (switches[o][i] == nullptr) {
                        continue;
                    }
                    switches[o][i]->update(currentIDs);
                }
                digitalWrite(outputs[o], HIGH);
            }
            // 割り込みのために出力をLOWに設定
            outputsWrite(LOW);
            // 更新してたらloopに送る。
            if (currentIDs != previousIDs) {
                xQueueSend(eventQueue, &data, portMAX_DELAY);
                previousIDs = currentIDs;
            }
            // poll interval
            delay(DEBOUNCE_DELAY / 2);
        } else {
            // 起きてた時に来た通知はクリアーする
            xQueueReset(keyInterruptQueue);
            // needsKeyScan内での割り込みチェック後から再度割り込みが発生してなければ
            if (isInterrupted == false) {
                // 割り込みが発生するまで寝る
                xQueueReceive(keyInterruptQueue, &tmp, portMAX_DELAY);
            }
        }
    }
}

void startKeyScan(UBaseType_t priority) {
    initMatrix();
    xTaskCreate(keyScanTask, "keyScan", 128, NULL, priority, NULL);
}
