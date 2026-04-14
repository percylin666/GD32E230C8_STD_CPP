#include "i2c.hpp"

I2CDevice::I2CDevice(uint32_t i2c_periph, uint32_t speed)
    : m_i2c_periph(i2c_periph), m_speed(speed) {
    init();
}

void I2CDevice::init() {
    // 1. 开启 I2C 时钟
    if (m_i2c_periph == I2C0) {
        rcu_periph_clock_enable(RCU_I2C0);
    } else if (m_i2c_periph == I2C1) {
        rcu_periph_clock_enable(RCU_I2C1);
    }

    // 2. 配置 GPIO 引脚为 I2C 模式
    // 注意：I2C0 通常使用 PB6 (SCL) 和 PB7 (SDA)，I2C1 使用 PB10 (SCL) 和 PB11 (SDA)
    if (m_i2c_periph == I2C0) {
        rcu_periph_clock_enable(RCU_GPIOB);
        gpio_af_set(GPIOB, GPIO_AF_1, GPIO_PIN_6 | GPIO_PIN_7); // AF1 对应 I2C0
        gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_6 | GPIO_PIN_7);
        gpio_output_options_set(GPIOB, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, GPIO_PIN_6 | GPIO_PIN_7);
    } else if (m_i2c_periph == I2C1) {
        rcu_periph_clock_enable(RCU_GPIOB);
        gpio_af_set(GPIOB, GPIO_AF_1, GPIO_PIN_10 | GPIO_PIN_11); // AF1 对应 I2C1
        gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_10 | GPIO_PIN_11);
        gpio_output_options_set(GPIOB, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, GPIO_PIN_10 | GPIO_PIN_11);
    }

    // 3. 配置 I2C 参数（时钟频率等）
    i2c_clock_config(m_i2c_periph, m_speed, I2C_DTCY_2);
    i2c_mode_addr_config(m_i2c_periph, I2C_I2CMODE_ENABLE, I2C_ADDFORMAT_7BITS, 0x00); // 从机地址暂时设置为 0x00
    i2c_enable(m_i2c_periph);

    i2c_ack_config(m_i2c_periph, I2C_ACK_ENABLE); // 使能 ACK
}

void I2CDevice::write(uint8_t device_addr, uint8_t reg_addr, uint8_t data) {
    // 1. 发送起始条件
    i2c_start_on_bus(m_i2c_periph);
    while (!i2c_flag_get(m_i2c_periph, I2C_FLAG_SBSEND));

    // 2. 发送设备地址（写模式）
    i2c_master_addressing(m_i2c_periph, device_addr, I2C_TRANSMITTER);
    while (!i2c_flag_get(m_i2c_periph, I2C_FLAG_ADDSEND));
    i2c_flag_clear(m_i2c_periph, I2C_FLAG_ADDSEND);

    // 3. 发送寄存器地址
    i2c_data_transmit(m_i2c_periph, reg_addr);
    while (!i2c_flag_get(m_i2c_periph, I2C_FLAG_TBE));

    // 4. 发送数据
    i2c_data_transmit(m_i2c_periph, data);
    while (!i2c_flag_get(m_i2c_periph, I2C_FLAG_TBE));

    // 5. 发送停止条件
    i2c_stop_on_bus(m_i2c_periph);
}

uint8_t I2CDevice::read(uint8_t device_addr, uint8_t reg_addr) {
    uint8_t data = 0;

    // 1. 发送起始条件
    i2c_start_on_bus(m_i2c_periph);
    while (!i2c_flag_get(m_i2c_periph, I2C_FLAG_SBSEND));

    // 2. 发送设备地址（写模式）以设置寄存器地址
    i2c_master_addressing(m_i2c_periph, device_addr, I2C_TRANSMITTER);
    while (!i2c_flag_get(m_i2c_periph, I2C_FLAG_ADDSEND));
    i2c_flag_clear(m_i2c_periph, I2C_FLAG_ADDSEND);

    // 3. 发送寄存器地址
    i2c_data_transmit(m_i2c_periph, reg_addr);
    while (!i2c_flag_get(m_i2c_periph, I2C_FLAG_TBE));

    // 4. 发送重复起始条件
    i2c_start_on_bus(m_i2c_periph);
    while (!i2c_flag_get(m_i2c_periph, I2C_FLAG_SBSEND));

    // 5. 发送设备地址（读模式）
    i2c_master_addressing(m_i2c_periph, device_addr, I2C_RECEIVER);
    while (!i2c_flag_get(m_i2c_periph, I2C_FLAG_ADDSEND));
    i2c_flag_clear(m_i2c_periph, I2C_FLAG_ADDSEND);

    // 6. 读取数据
    while (!i2c_flag_get(m_i2c_periph, I2C_FLAG_RBNE));
    data = i2c_data_receive(m_i2c_periph);

    // 7. 发送停止条件
    i2c_stop_on_bus(m_i2c_periph);

    return data;
}