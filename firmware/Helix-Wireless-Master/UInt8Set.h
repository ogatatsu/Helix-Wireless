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

#include <Arduino.h>

// 符号なし8ビット型専用の集合コンテナクラス
class UInt8Set {

    friend bool operator==(const UInt8Set &a, const UInt8Set &b);

  public:
    void add(uint8_t val);

    void addAll(const uint8_t vals[], uint len);

    UInt8Set &operator|=(const UInt8Set &rhs);

    void remove(uint8_t val);

    void removeAll(const uint8_t vals[], uint len);

    UInt8Set &operator-=(const UInt8Set &rhs);

    bool contains(uint8_t val) const;

    bool containsAll(const uint8_t vals[], uint len) const;

    bool containsAny(const uint8_t vals[], uint len) const;

    void toArray(uint8_t buf[]) const;

    uint16_t count() const;

  private:
    // 8 * 32 = 256
    uint8_t _data[32] = {};
    mutable uint16_t _count = 0;
    mutable bool _needsRecount = false;
};

// 比較
bool operator==(const UInt8Set &a, const UInt8Set &b);

bool operator!=(const UInt8Set &a, const UInt8Set &b);

// 和集合
UInt8Set operator|(const UInt8Set &a, const UInt8Set &b);
// 差集合
UInt8Set operator-(const UInt8Set &a, const UInt8Set &b);
