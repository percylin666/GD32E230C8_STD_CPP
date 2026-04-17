#pragma once

#include <type_traits>

namespace Conv {

// 专门用于标记十六进制输出的包装器
template <typename T>
struct HexWrapper {
    T value;
};
// 辅助函数，方便调用
template <typename T>
HexWrapper<T> to_hex(T val) { return {val}; }

/**
 * @brief 模板化 itoa 函数
 * @tparam T 整数类型 (int, long, uint32_t 等)
 */
template <typename T>
void itoa(T value, char* str, int base) {
    if (base < 2 || base > 36) {
        *str = '\0';
        return;
    }

    char* ptr = str;
    char* ptr1 = str;
    char tmp_char;
    
    // 使用无符号版本处理数值，避免负数溢出问题
    typedef typename std::make_unsigned<T>::type unsigned_T;
    unsigned_T uvalue = static_cast<unsigned_T>(value);

    // 仅在 10 进制下处理负数符号
    if (value < 0 && base == 10) {
        *ptr++ = '-';
        uvalue = static_cast<unsigned_T>(-value);
        ptr1++; // 反转时跳过负号
    }

    // 转换逻辑
    do {
        unsigned_T remainder = uvalue % base;
        uvalue /= base;
        *ptr++ = "0123456789abcdefghijklmnopqrstuvwxyz"[remainder];
    } while (uvalue);

    *ptr-- = '\0';

    // 反转字符串
    while (ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }
}

inline void ftoa(float value, char* str, int precision = 4) {
    if (precision < 0) precision = 0;
    if (precision > 6) precision = 6; // 限制精度范围

    if (value < 0) {
        *str++ = '-';
        value = -value;
    }

    uint32_t ipart = (uint32_t)value;
    itoa(ipart, str, 10);

    if (precision > 0) {
        char* ptr = str + strlen(str);
        *ptr++ = '.'; // 添加小数点

        float fpart = value - (float)ipart;
        for (int i = 0; i < precision; ++i) {
            fpart *= 10;
            uint32_t digit = (uint32_t)fpart;
            *ptr++ = char('0' + digit);
            fpart -= (float)digit;
        }
        *ptr = '\0';
    }
}

} // namespace Conv
