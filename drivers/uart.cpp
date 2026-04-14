#include "uart.hpp"

void UartDevice::init() {
    if (m_periph == USART0) {
        rcu_periph_clock_enable(RCU_USART0);
        rcu_periph_clock_enable(RCU_GPIOA); // USART0 的 TX/RX PA9/PA10

        // 配置 PA9 PA10 为复用功能
        gpio_af_set(GPIOA, GPIO_AF_1, GPIO_PIN_9);
        gpio_af_set(GPIOA, GPIO_AF_1, GPIO_PIN_10);
        // 配置 PA9 PA10 为复用推挽输出
        gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_9);
        gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);
        gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_10);
        gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_10);
    } else if (m_periph == USART1) {
        rcu_periph_clock_enable(RCU_USART1);
        rcu_periph_clock_enable(RCU_GPIOA); // USART1 的 TX/RX PA2/PA3
        // 配置 PA2 (TX), PA3 (RX)
        gpio_af_set(GPIOA, GPIO_AF_1, GPIO_PIN_2);
        gpio_af_set(GPIOA, GPIO_AF_1, GPIO_PIN_3);
        gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_2);
        gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_2);
        gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_3);
        gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_3);
    }

    usart_deinit(m_periph);
    usart_baudrate_set(m_periph, m_baud_rate);
    usart_word_length_set(m_periph, USART_WL_8BIT);
    usart_stop_bit_set(m_periph, USART_STB_1BIT);
    usart_parity_config(m_periph, USART_PM_NONE);
    
    usart_transmit_config(m_periph, USART_TRANSMIT_ENABLE);
    usart_receive_config(m_periph, USART_RECEIVE_ENABLE);

    usart_enable(m_periph);
}


UartDevice::UartDevice(uint32_t usart_periph, uint32_t baud) : m_periph(usart_periph), m_baud_rate(baud) {
    init();
}

void UartDevice::write(const char *data, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        // 等待发送缓冲区空
        while (usart_flag_get(m_periph, USART_FLAG_TBE) == RESET);
        usart_data_transmit(m_periph, data[i]);
    }
}

UartDevice& UartDevice::operator<<(char c) {
    write(&c, 1);
    return *this;
}

UartDevice& UartDevice::operator<<(const char* str) {
    write(str, strlen(str));
    return *this;
}
