#pragma once

#include <cstdint>
#include <type_traits>

/**
 * @brief 泛型环形缓冲区模板
 * @tparam T 存储的数据类型 (如 uint8_t, int, 自定义结构体)
 * @tparam Size 缓冲区大小 (必须是 2 的幂，例如 128, 256, 1024)
 */
template <typename T, uint32_t Size>
class RingBuffer {
    static_assert((Size > 0) && ((Size & (Size - 1)) == 0), "Size must be a power of 2");

private:
    T m_buffer[Size];
    volatile uint32_t m_head = 0;
    volatile uint32_t m_tail = 0;

    // 位运算掩码：Size 为 1024 时，m_mask 为 1023 (0x3FF)
    static constexpr uint32_t m_mask = Size - 1;

public:
    RingBuffer() = default;
    ~RingBuffer() = default;

    bool write(T data) {
        uint32_t nextHead = (m_head + 1) & m_mask;
        if (nextHead == m_tail) {
            return false; // 缓冲区满
        }
        m_buffer[m_head] = data;
        m_head = nextHead;
        return true;
    }

    bool read(T& data) {
        if (m_head == m_tail) {
            return false; // 缓冲区空
        }
        data = m_buffer[m_tail];
        m_tail = (m_tail + 1) & m_mask;
        return true;
    }

    uint32_t available() const {
        return (m_head - m_tail) & m_mask;
    }

    bool isEmpty() const {
        return m_head == m_tail;
    }

    bool isFull() const {
        return ((m_head + 1) & m_mask) == m_tail;
    }

    void clear() {
        m_head = m_tail = 0;
    }
};