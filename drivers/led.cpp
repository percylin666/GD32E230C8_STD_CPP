#include "led.hpp"

Led::Led(rcu_periph_enum periph_clock, uint32_t gpio_port, uint32_t gpio_pin, LED_INIT_STATUS initial_status) 
    : m_periph_clock(periph_clock), m_gpio_port(gpio_port), m_gpio_pin(gpio_pin), m_LedInitStatus(initial_status) {
    init();
}

void Led::init() {
    // 1. 开启 GPIO 时钟
    rcu_periph_clock_enable(m_periph_clock);

    // 2. 配置 GPIO 为输出模式，上拉，推挽输出，50MHz
    gpio_mode_set(m_gpio_port, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, m_gpio_pin);
    gpio_output_options_set(m_gpio_port, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, m_gpio_pin);

    m_LedStatus = (m_LedInitStatus == LED_INIT_STATUS::ON) ? true : false;

    write(m_LedStatus);
}

void Led::write(bool ledStatus) {
    m_LedStatus = ledStatus;
    gpio_bit_write(m_gpio_port, m_gpio_pin, m_LedStatus ? SET : RESET);
}

void Led::toggle() {
    gpio_bit_write(m_gpio_port, m_gpio_pin, m_LedStatus ? RESET : SET);
    m_LedStatus = !m_LedStatus;
}

bool& Led::read() {
    return m_LedStatus;
}