#pragma once
#include "gd32e23x.h"

enum class NSS_LEVEL {
    LOW = 0,
    HIGH = 1
};

class SpiDevice {
private:
    uint32_t m_spi_periph; // 存储 SPI0, SPI1 等基地址
    uint32_t m_speed;      // SPI 通信速度 (单位: Hz)

    void init() const;

public:
    SpiDevice(uint32_t spi_periph, uint32_t speed);
    ~SpiDevice() {}

    void nss_control(NSS_LEVEL level) const;
    uint8_t ReadWriteByte(uint8_t byte) const;
};
