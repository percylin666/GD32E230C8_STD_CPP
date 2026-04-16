#include "spi.hpp"

SpiDevice::SpiDevice(uint32_t spi_periph, uint32_t speed)
    : m_spi_periph(spi_periph), m_speed(speed) {
    init();
}


void SpiDevice::init() const {
    // 1. 开启 SPI 时钟
    if (m_spi_periph == SPI0) {
        rcu_periph_clock_enable(RCU_SPI0);
    } else if (m_spi_periph == SPI1) {
        rcu_periph_clock_enable(RCU_SPI1);
    }

    // 2. 配置 GPIO 引脚为 SPI 模式
    // 注意：SPI0 通常使用 PA5 (SCK), PA6 (MISO), PA7 (MOSI)，SPI1 使用 PB3 (SCK), PB4 (MISO), PB5 (MOSI)
    if (m_spi_periph == SPI0) {
        rcu_periph_clock_enable(RCU_GPIOA);

        // SCK, MOSI 需要配置为复用推挽输出
        gpio_af_set(GPIOA, GPIO_AF_0, GPIO_PIN_5 | GPIO_PIN_7); // AF0 对应 SPI0
        gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_5 | GPIO_PIN_7);
        gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_5 | GPIO_PIN_7);
        // MISO 配置为复用输入
        gpio_af_set(GPIOA, GPIO_AF_0, GPIO_PIN_6); // AF0 对应 SPI0
        gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_6);
        // NSS 为手动控制，配置为普通 GPIO 输出
        gpio_mode_set(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_4); // NSS
        gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_4);
        gpio_bit_set(GPIOA, GPIO_PIN_4); // 默认拉高 NSS

    } else if (m_spi_periph == SPI1) {
        rcu_periph_clock_enable(RCU_GPIOB);

        // SCK, MOSI 需要配置为复用推挽输出
        gpio_af_set(GPIOB, GPIO_AF_0, GPIO_PIN_3 | GPIO_PIN_5); // AF0 对应 SPI1
        gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_3 | GPIO_PIN_5);
        gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_3 | GPIO_PIN_5);
        // MISO 配置为复用输入
        gpio_af_set(GPIOB, GPIO_AF_0, GPIO_PIN_4); // AF0 对应 SPI1
        gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_4);
        // NSS 为手动控制，配置为普通 GPIO 输出
        gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_12); // NSS
        gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_12);
        gpio_bit_set(GPIOB, GPIO_PIN_12); // 默认拉高 NSS
    }

    // 3. 配置 SPI 参数（时钟频率等）
    spi_parameter_struct spi_initpara;
    spi_struct_para_init(&spi_initpara);
    spi_initpara.trans_mode = SPI_TRANSMODE_FULLDUPLEX; // 全双工
    spi_initpara.device_mode = SPI_MASTER; // 主机模式
    spi_initpara.frame_size = SPI_FRAMESIZE_8BIT; // 8 位数据帧
    spi_initpara.nss = SPI_NSS_SOFT; // 软件控制 NSS
    spi_initpara.endian = SPI_ENDIAN_MSB; // 大端模式
    spi_initpara.clock_polarity_phase = SPI_CK_PL_LOW_PH_1EDGE; // 时钟空闲低电平，第1个时钟边沿采样
    spi_initpara.prescale = SPI_PSC_8; // 预分频器，具体值需要根据 m_speed 和系统时钟计算
    spi_init(m_spi_periph, &spi_initpara);

    spi_enable(m_spi_periph);
}

void SpiDevice::nss_control(NSS_LEVEL level) const {
    // 根据 m_spi_periph 控制对应的 NSS 引脚
    if (m_spi_periph == SPI0) {
        gpio_bit_write(GPIOA, GPIO_PIN_4, level == NSS_LEVEL::HIGH ? SET : RESET); // SPI0 的 NSS 在 PA4
    } else if (m_spi_periph == SPI1) {
        gpio_bit_write(GPIOB, GPIO_PIN_12, level == NSS_LEVEL::HIGH ? SET : RESET); // SPI1 的 NSS 在 PB12
    }
}

uint8_t SpiDevice::ReadWriteByte(uint8_t byte) const {
    while (spi_i2s_flag_get(m_spi_periph, SPI_FLAG_TBE) == RESET);
    spi_i2s_data_transmit(m_spi_periph, byte);
    while (spi_i2s_flag_get(m_spi_periph, SPI_FLAG_RBNE) == RESET);
    uint8_t received = spi_i2s_data_receive(m_spi_periph);
    return received;
}
