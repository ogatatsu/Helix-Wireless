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

#include "blinkLED.h"
#include "config.h"
#include <Arduino.h>

struct BlinkData {
    uint8_t pin;
    bool isBlink;
};

static QueueHandle_t blinkLedQueue;

// ピンの点滅状態をビットで管理する
static uint8_t buf[(PINS_COUNT / 8) + !!(PINS_COUNT % 8)] = {};

static inline bool readState(uint pin) {
    return bitRead(buf[pin / 8], pin % 8);
}

static inline void writeState(uint pin, bool isBlink) {
    bitWrite(buf[pin / 8], pin % 8, isBlink);
}

static bool isAllStateZero() {
    for (int i = 0; i < arrcount(buf); i++) {
        if (buf[i] != 0) {
            return false;
        }
    }
    return true;
}

static void blinkLedTask(void *arg) {
    BlinkData data;
    while (1) {
        xQueueReceive(blinkLedQueue, &data, portMAX_DELAY);
        writeState(data.pin, data.isBlink);
        while (isAllStateZero() == false) {
            for (int i = 0; i < PINS_COUNT; i++) {
                if (readState(i)) {
                    digitalWrite(i, HIGH);
                }
            }
            delay(TURN_ON_INTERVAL);
            for (int i = 0; i < PINS_COUNT; i++) {
                if (readState(i)) {
                    digitalWrite(i, LOW);
                }
            }
            delay(TURN_OFF_INTERVAL);
            while (xQueueReceive(blinkLedQueue, &data, 0)) {
                writeState(data.pin, data.isBlink);
            }
        }
    }
}

void initLED(UBaseType_t priority) {
    pinMode(PIN_LED1, OUTPUT);
    pinMode(PIN_LED2, OUTPUT);

    blinkLedQueue = xQueueCreate(4, sizeof(BlinkData));
    xTaskCreate(blinkLedTask, "blinkLED", 64, NULL, priority, NULL);
}

static void led(uint8_t pin, bool isBlink) {
    if (pin >= PINS_COUNT) {
        return;
    }
    BlinkData data = {
        .pin = pin,
        .isBlink = isBlink,
    };

    xQueueSend(blinkLedQueue, &data, portMAX_DELAY);
}

void blinkLED1() {
    led(PIN_LED1, true);
}

void blinkLED2() {
    led(PIN_LED2, true);
}

void turnOffLED1() {
    led(PIN_LED1, false);
}

void turnOffLED2() {
    led(PIN_LED2, false);
}
