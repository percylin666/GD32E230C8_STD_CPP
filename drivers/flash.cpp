#include "flash.hpp"

//FLASH 相关操作函数的实现（写使能、等待忙、扇区擦除、页编程、读数据）可以根据具体的 SPI Flash 芯片命令集来实现，以下是一个示例：
uint32_t Flash::flash_read_jedec_id(void) {
    uint32_t id = 0;
    uint8_t id_parts[3];

    m_spi.nss_control(NSS_LEVEL::LOW); // 拉低 NSS 开始通信
    m_spi.ReadWriteByte(0x9F); // 发送读取 JEDEC ID 的命令
    id_parts[0] = m_spi.ReadWriteByte(0xFF); // 读取厂商 ID
    id_parts[1] = m_spi.ReadWriteByte(0xFF); // 读取设备 ID 高字节
    id_parts[2] = m_spi.ReadWriteByte(0xFF); // 读取设备 ID 低字节
    m_spi.nss_control(NSS_LEVEL::HIGH); // 拉高 NSS 结束通信

    id = (id_parts[0] << 16) | (id_parts[1] << 8) | id_parts[2];
    return id;
}

void Flash::flash_write_enable(void) {
    m_spi.nss_control(NSS_LEVEL::LOW);
    m_spi.ReadWriteByte(0x06); // 写使能命令
    m_spi.nss_control(NSS_LEVEL::HIGH);
}

void Flash::flash_wait_for_busy(void) {
    uint8_t status = 0;
    do {
        m_spi.nss_control(NSS_LEVEL::LOW);
        m_spi.ReadWriteByte(0x05); // 读取状态寄存器命令
        status = m_spi.ReadWriteByte(0xFF); // 读取状态寄存器值
        m_spi.nss_control(NSS_LEVEL::HIGH);
    } while (status & 0x01); // 等待 BUSY 位清零
}

void Flash::flash_sector_erase(uint32_t address) {
    flash_write_enable();
    m_spi.nss_control(NSS_LEVEL::LOW);
    m_spi.ReadWriteByte(0x20); // 扇区擦除命令
    m_spi.ReadWriteByte((address >> 16) & 0xFF); // 地址高字节
    m_spi.ReadWriteByte((address >> 8) & 0xFF);  // 地址中间字节
    m_spi.ReadWriteByte(address & 0xFF);         // 地址低字节
    m_spi.nss_control(NSS_LEVEL::HIGH);
    flash_wait_for_busy();
}

void Flash::flash_page_program(uint32_t address, uint8_t* data, uint16_t length) {
    flash_write_enable();
    m_spi.nss_control(NSS_LEVEL::LOW);
    m_spi.ReadWriteByte(0x02); // 页编程命令
    m_spi.ReadWriteByte((address >> 16) & 0xFF); // 地址高字节
    m_spi.ReadWriteByte((address >> 8) & 0xFF);  // 地址中间字节
    m_spi.ReadWriteByte(address & 0xFF);         // 地址低字节
    for (uint16_t i = 0; i < length; i++) {
        m_spi.ReadWriteByte(data[i]); // 写入数据
    }
    m_spi.nss_control(NSS_LEVEL::HIGH);
    flash_wait_for_busy();
}

void Flash::flash_read_data(uint32_t addr, uint8_t *data, uint16_t length) {
    m_spi.nss_control(NSS_LEVEL::LOW);
    m_spi.ReadWriteByte(0x03); // 读取数据命令
    m_spi.ReadWriteByte((addr >> 16) & 0xFF); // 地址高字节
    m_spi.ReadWriteByte((addr >> 8) & 0xFF);  // 地址中间字节
    m_spi.ReadWriteByte(addr & 0xFF);         // 地址低字节
    for (uint16_t i = 0; i < length; i++) {
        data[i] = m_spi.ReadWriteByte(0xFF); // 读取数据
    }
    m_spi.nss_control(NSS_LEVEL::HIGH);
}


uint8_t Flash::flash_test(uint16_t *read_data, uint16_t write_data) {
    //先读后写，如果有数据，就不进行写入
    //返回值：0表示失败 1表示读成功 2表示写成功
    flash_read_data(0x000000, (uint8_t*)read_data, 2); // 读取前 2 字节数据
    if (*read_data != 0xFFFF) {
        return 1;
    }

    flash_page_program(0x000000, (uint8_t*)&write_data, 2); // 写入 2 字节数据
    flash_read_data(0x000000, (uint8_t*)read_data, 2); // 再次读取验证
    if (*read_data == write_data) {
        return 2;
    }

    return 0; // 读写都失败
}