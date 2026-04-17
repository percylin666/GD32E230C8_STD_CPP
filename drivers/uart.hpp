#pragma once
#include "gd32e23x.h"

#include <cstdio>
#include <cstring>
#include <cstdlib>

#include "conver.hpp"

namespace Raw {
    struct RawData {
        const uint8_t* data;
        size_t size;
    };
    
    // 辅助函数，方便写成 uart << Raw::makeRaw(buf, 8);
    inline RawData makeRaw(const uint8_t* d, size_t s) {
        return RawData{d, s};
    }
    // 同时也支持 char* 类型的重载
    inline RawData makeRaw(const char* d, size_t s) {
        return RawData{(const uint8_t*)d, s};
    }
} // namespace Raw

class UartDevice {
private:
    uint32_t m_periph; // 存储 USART0, USART1 等基地址
    uint32_t m_baud_rate;

    void write(const char *data, size_t size) const;

public:
    // 构造函数
    UartDevice(uint32_t usart_periph, uint32_t baud = 115200);
    ~UartDevice() {}

    void init() const;    


    // 重载 字符的 << 运算符
    const UartDevice& operator<<(char c) const;

    // 重载 字符串的 << 运算符
    const UartDevice& operator<<(const char* str) const;

    // 原有的十进制模板重载保持不变
    template <typename T>
    typename std::enable_if<std::is_integral<T>::value, const UartDevice&>::type
    operator<<(T value) const;

    // 专门处理十六进制包装器的重载
    template <typename T>
    const UartDevice& operator<<(Conv::HexWrapper<T> hex) const;

    // 处理原始二进制数据的 << 运算符重载, ascii 字符可以通过这个接口发送
    const UartDevice& operator<<(const Raw::RawData& raw) const;

    // 处理float类型的 << 运算符重载
    const UartDevice& operator<<(float value) const;

    // 处理double类型的 << 运算符重载
    const UartDevice& operator<<(double value) const;
};

// 使用模板实现的函数，只能放在头文件中，否则会出现链接错误
// 原有的十进制模板重载保持不变
template <typename T>
typename std::enable_if<std::is_integral<T>::value, const UartDevice&>::type UartDevice::operator<<(T value) const {
    char buf[20];
    Conv::itoa(value, buf, 10);
    write(buf, strlen(buf));
    return *this;
}

// 新增：专门处理十六进制包装器的重载
template <typename T>
const UartDevice& UartDevice::operator<<(Conv::HexWrapper<T> hex) const {
    char buf[20];
    write("0x", 2); // 自动补齐前缀
    Conv::itoa(hex.value, buf, 16); // 强制 16 进制
    write(buf, strlen(buf));
    return *this;
}