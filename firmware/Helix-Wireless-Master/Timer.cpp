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

#include "Timer.h"
#include "queues.h"

static void timeout(TimerHandle_t timer) {
    // Software Timersのスタックを消費しないようにstaticで宣言
    static EventData data = {
        .eventType = TIMER_EVENT,
    };
    data.timer = reinterpret_cast<Timer *>(pvTimerGetTimerID(timer));
    xQueueSend(eventQueue, &data, portMAX_DELAY);
}

Timer::Timer(uint ms, bool autoReload) {
    _th = xTimerCreate(NULL, ms2tick(ms), autoReload, this, timeout);
}

void Timer::startTimer() {
    xTimerStart(_th, 0);
}

void Timer::stopTimer() {
    xTimerStop(_th, 0);
}

void Timer::changePeriod(uint ms) {
    xTimerChangePeriod(_th, ms2tick(ms), 0);
}
