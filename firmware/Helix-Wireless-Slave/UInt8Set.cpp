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

#include "UInt8Set.h"

void UInt8Set::add(uint8_t val) {
    bitSet(_data[val / 8], val % 8);
    _needsRecount = true;
}

void UInt8Set::addAll(const uint8_t vals[], uint len) {
    for (int i = 0; i < len; i++) {
        bitSet(_data[vals[i] / 8], vals[i] % 8);
    }
    _needsRecount = true;
}

UInt8Set &UInt8Set::operator|=(const UInt8Set &rhs) {
    uint64_t *_data_64 = reinterpret_cast<uint64_t *>(_data);
    uint64_t *rhs_data_64 = reinterpret_cast<uint64_t *>(const_cast<uint8_t *>(rhs._data));
    for (int i = 0; i < 4; i++) {
        _data_64[i] |= rhs_data_64[i];
    }
    _needsRecount = true;
    return *this;
}

void UInt8Set::remove(uint8_t val) {
    bitClear(_data[val / 8], val % 8);
    _needsRecount = true;
}

void UInt8Set::removeAll(const uint8_t vals[], uint len) {
    for (int i = 0; i < len; i++) {
        bitClear(_data[vals[i] / 8], vals[i] % 8);
    }
    _needsRecount = true;
}

UInt8Set &UInt8Set::operator-=(const UInt8Set &rhs) {
    uint64_t *_data_64 = reinterpret_cast<uint64_t *>(_data);
    uint64_t *rhs_data_64 = reinterpret_cast<uint64_t *>(const_cast<uint8_t *>(rhs._data));
    for (int i = 0; i < 4; i++) {
        _data_64[i] &= ~(rhs_data_64[i]);
    }
    _needsRecount = true;
    return *this;
}

bool UInt8Set::contains(uint8_t val) const {
    return bitRead(_data[val / 8], val % 8);
}

bool UInt8Set::containsAll(const uint8_t vals[], uint len) const {
    for (int i = 0; i < len; i++) {
        if (contains(vals[i]) == false) {
            return false;
        }
    }
    return true;
}

bool UInt8Set::containsAny(const uint8_t vals[], uint len) const {
    for (int i = 0; i < len; i++) {
        if (contains(vals[i])) {
            return true;
        }
    }
    return false;
}

void UInt8Set::toArray(uint8_t buf[]) const {
    uint size = this->count();
    uint count = 0;
    for (int i = 0;; i++) {
        if (count == size) {
            return;
        }
        if (contains(i)) {
            buf[count++] = i;
        }
    }
}

uint16_t UInt8Set::count() const {
    if (_needsRecount) {
        _count = 0;
        uint64_t *_data_64 = reinterpret_cast<uint64_t *>(const_cast<uint8_t *>(_data));
        for (int i = 0; i < 4; i++) {
            _count += __builtin_popcountll(_data_64[i]);
        }
        _needsRecount = false;
    }
    return _count;
}

bool operator==(const UInt8Set &a, const UInt8Set &b) {
    if (a.count() != b.count()) {
        return false;
    }
    return memcmp(a._data, b._data, 32) ? false : true;
}

bool operator!=(const UInt8Set &a, const UInt8Set &b) {
    return !(a == b);
}

UInt8Set operator|(const UInt8Set &a, const UInt8Set &b) {
    UInt8Set result;
    result |= a;
    result |= b;
    return result;
}

UInt8Set operator-(const UInt8Set &a, const UInt8Set &b) {
    UInt8Set result;
    result |= a;
    result -= b;
    return result;
}
