#pragma once
#include "gd32e23x.h"

enum class TIMER_MODE {
    NORMAL,
    OUTPUT_COMPARE,
    PWM
};

class Timer {
private:
    uint32_t m_timer_periph; // 存储 TIMER0, TIMER1 等基地址
    uint32_t m_period;       // 定时器周期 (单位: ms)
    TIMER_MODE m_mode;       // 定时器模式

    uint32_t m_compare_value = 0; // PWM 模式下的比较值 1-1000 LED灯只需要1-500

    void init();

public:
    // 构造函数
    Timer(uint32_t timer_periph, uint32_t period_ms, TIMER_MODE mode = TIMER_MODE::NORMAL);
    ~Timer() {}

    void setCompareValue(uint32_t compare_value);

    void start();
    void stop();
    void reset();
};