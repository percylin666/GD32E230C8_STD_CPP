#include "rgb_led.hpp"

void RGBLed::setColor(uint16_t r, uint16_t g, uint16_t b) {
    // 将 RGB 颜色值转换为 PWM 占空比 (1-500)
    m_r_value = r;
    m_g_value = g;
    m_b_value = b;
    // 更新 TIMER0 CH0 的比较值以调整红色亮度
    // Timer timer0(TIMER0, 1, TIMER_MODE::PWM);
    m_timer_r.setCompareValue(m_r_value);
    // 更新 TIMER2 CH0 的比较值以调整绿色亮度
    // Timer timer2(TIMER2, 1, TIMER_MODE::PWM);
    m_timer_g.setCompareValue(m_g_value);
    // 更新 TIMER14 CH0 的比较值以调整蓝色亮度
    // Timer timer14(TIMER14, 1, TIMER_MODE::PWM);
    m_timer_b.setCompareValue(m_b_value);
}