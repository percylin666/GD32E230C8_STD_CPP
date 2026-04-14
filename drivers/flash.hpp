#pragma once
#include "gd32e23x.h"
#include "spi.hpp"

class Flash {
private:
    SpiDevice m_spi = SpiDevice(SPI0, 1000000); // 创建一个 SPI0 对象，通信速度为 1MHz
public:
    Flash() {}
    ~Flash() {}

    //与 FLASH 相关
    uint32_t flash_read_jedec_id(void);
    void flash_write_enable(void);
    void flash_wait_for_busy(void);
    void flash_sector_erase(uint32_t address);
    void flash_page_program(uint32_t address, uint8_t* data, uint16_t length);
    void flash_read_data(uint32_t addr, uint8_t *data, uint16_t length);

    uint8_t flash_test(uint16_t *read_data, uint16_t write_data); //返回值：0表示失败 1表示读成功 2表示写成功
};