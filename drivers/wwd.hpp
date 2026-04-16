#pragma once

#include "gd32e23x.h"

class WWatchdog {
private:
    uint8_t m_counter_value;
    uint8_t m_window_value;

    void init() const;

public:
    WWatchdog(uint8_t counter_value = 0x7F, uint8_t window_value = 0x5F)
        : m_counter_value(counter_value), m_window_value(window_value) {
        init();
    };
    ~WWatchdog() {}

    void feed_dog(void) const;
};