#pragma once
#include "gd32e23x.h"

class I2CDevice {
private:
    uint32_t m_i2c_periph; // 存储 I2C0, I2C1 等基地址
    uint32_t m_speed;      // I2C 通信速度 (单位: Hz)

    void init();

public:
    I2CDevice(uint32_t i2c_periph, uint32_t speed);
    ~I2CDevice() {}

    void write(uint8_t device_addr, uint8_t reg_addr, uint8_t data);
    uint8_t read(uint8_t device_addr, uint8_t reg_addr);
};