#pragma once
#include "gd32e23x.h"

enum class ADC_CHANNEL {
    CHANNEL_0 = 0,
    CHANNEL_1 = 1,
    // 根据需要添加更多通道
};

class Adc {
private:
    ADC_CHANNEL m_channel;

    void init();
public:
    Adc(ADC_CHANNEL channel) : m_channel(channel) {
        init();
    }
    ~Adc() {}

    // ADC 相关函数

    uint16_t readChannel(void) const;
};