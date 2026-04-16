#pragma once
#include "gd32e23x.h"

class FWatchdog {
private:
    uint32_t m_feed_dog_period_ms; // 喂狗周期，单位为毫秒

    void init();
public:
    FWatchdog(uint32_t feed_dog_period_ms = 1000) : m_feed_dog_period_ms(feed_dog_period_ms) {
        init();
    };
    ~FWatchdog() {}

    void feed_dog(void) const;
};