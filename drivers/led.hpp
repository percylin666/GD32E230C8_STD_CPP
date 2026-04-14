#pragma once

#include "gd32e23x.h"

enum class LED_INIT_STATUS {
    OFF = 0,
    ON = 1,
};

class Led {
private:
    //时钟
    rcu_periph_enum m_periph_clock;
    //GPIO Port
    uint32_t m_gpio_port;
    //GPIO Pin
    uint32_t m_gpio_pin;
    //status
    LED_INIT_STATUS m_LedInitStatus;
    bool m_LedStatus;

    void init();

public:
    Led(rcu_periph_enum periph_clock, uint32_t gpio_port, uint32_t gpio_pin, LED_INIT_STATUS initial_status = LED_INIT_STATUS::OFF);
    ~Led() {}

    void write(bool ledStatus);
    void toggle();
    bool& read();
};