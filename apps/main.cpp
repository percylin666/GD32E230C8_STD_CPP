#include "gd32e23x.h"
#include "system.h"

#include "led.hpp"
#include "key.hpp"
#include "uart.hpp"
#include "timer.hpp"
#include "rgb_led.hpp"
#include "exti.hpp"
#include "i2c.hpp"
#include "flash.hpp"
#include "rtc.hpp"
#include "adc.hpp"
#include "fwd.hpp"
#include "gd32e23x_it.h"

int main(void) {
    uint32_t systick_cnt = 0;
    uint8_t flag_100ms = 0;
    uint8_t flag_500ms = 0;
    uint8_t flag_10s = 0;
    uint16_t rgb_val = 0;
    
    uint8_t task_cnt = 0;
    uint16_t rgb_tab[3] = {1}; // RGB 颜色值，范围 0-500
    RGB_LED_STATUS rgb_led_status = RGB_LED_STATUS::RED; // 当前颜色状态

    Exti exti1(RCU_GPIOA, GPIOA, GPIO_PIN_0, EXTI_TRIG_FALLING);

    RGBLed rgbLed; // 创建一个 RGB LED 对象，连接到 TIMER0_CH0、TIMER2_CH0 和 TIMER14_CH0 引脚

    //LED PF6 PF7
    Led led1(RCU_GPIOF, GPIOF, GPIO_PIN_6, LED_INIT_STATUS::ON);
    Led led2(RCU_GPIOF, GPIOF, GPIO_PIN_7, LED_INIT_STATUS::OFF);

    //KEY PB10 PB11
    Key key1(RCU_GPIOB, GPIOB, GPIO_PIN_10);
    Key key2(RCU_GPIOB, GPIOB, GPIO_PIN_11);

    //UART
    UartDevice uart0(USART0, 115200);

    //I2C0
    I2CDevice eeprom(I2C0, 100000); // 创建一个 I2C0 对象，通信速度为 100kHz

    //SPI
    Flash flash; // 创建一个 SPI0 对象，通信速度为 1MHz

    //ADC
    Adc adc(ADC_CHANNEL::CHANNEL_1);

    //RTC
    Rtc rtc;

    RtcTime set_time;
    set_time.year   = 26;  // 写入 26，DEC2BCD 会转为 0x26
    set_time.month  = 3;   // 写入 3，DEC2BCD 会转为 0x03 (即 RTC_MAR)
    set_time.date   = 13;  // 写入 13，DEC2BCD 会转为 0x13
    set_time.hour   = 17;  // 写入 16，DEC2BCD 会转为 0x16
    set_time.minute = 14;  // 写入 8，DEC2BCD 会转为 0x08
    set_time.second = 0;
    // 注意：星期的处理比较特殊
    set_time.day_of_week = RTC_FRIDAY; // 建议直接使用库定义的宏 (RTC_FRIDAY = 0x05)
    rtc.set(set_time);

    //systick 初始化
    // SystemCoreClock 是系统主频 (例如 72000000)
    if (SysTick_Config(SystemCoreClock / 1000)) {
        /* Capture error */
        while (1);
    }

    // // 1. 开启 GPIOF 时钟 (LED 在 PF6 PF7)
    // rcu_periph_clock_enable(RCU_GPIOF);

    // // 2. 配置 PF6 PF7 为输出模式
    // gpio_mode_set(GPIOF, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, GPIO_PIN_6 | GPIO_PIN_7);
    // gpio_output_options_set(GPIOF, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_6 | GPIO_PIN_7);

    // while(1) {
    //     gpio_bit_write(GPIOF, GPIO_PIN_6, SET);
    //     gpio_bit_write(GPIOF, GPIO_PIN_7, SET);
    //     sys_delay_ms(2000);
    //     gpio_bit_write(GPIOF, GPIO_PIN_6, RESET);
    //     gpio_bit_write(GPIOF, GPIO_PIN_7, RESET);
    //     sys_delay_ms(2000);

    //     led_task("a"); // 调用 LED 任务
    // }

    // I2C 设备测试 先读后写，写入的数据比读出的大 1，方便观察
    uint8_t device_addr = 0xA0; // 从机地址为 0xA0
    uint8_t reg_addr = 0x01;    // 要访问的寄存器地址为 0x01
    uint8_t data = eeprom.read(device_addr + 1, reg_addr); // 先读出当前寄存器值
    uart0 << "EEPROM Read: Device " << Conv::to_hex(device_addr) << ", Reg " << Conv::to_hex(reg_addr) << " = " << Conv::to_hex(data) << "\r\n";
    eeprom.write(device_addr, reg_addr, data + 1); // 写入比读出值大 1 的数据
    uart0 << "EEPROM Write: Device " << Conv::to_hex(device_addr) << ", Reg " << Conv::to_hex(reg_addr) << " = " << Conv::to_hex(data + 1) << "\r\n";


    // FLASH 设备测试，发送一个字节并读取回来的数据
    uint32_t jedec_id = flash.flash_read_jedec_id();
    uart0 << "SPI Flash JEDEC ID: " << Conv::to_hex(jedec_id) << "\r\n";

    // FLASH 读写测试
    uint16_t read_data = 0; // 要写入的数据
    uint8_t ret = flash.flash_test(&read_data, 0xBEEF); // 写入测试数据 0xBEEF

    if (ret == 0) {
        // 失败
        uart0 << "FLASH Test: Failed\r\n";
    }
    else if (ret == 1) {
        // 读成功
        uart0 << "FLASH Test: Read Success, Data = " << Conv::to_hex(read_data) << "\r\n";
    }
    else if (ret == 2) {
        // 写成功
        uart0 << "FLASH Test: Write Success\r\n";
    }

    // 独立看门狗
    FWatchdog watchdog(1000); // 创建一个看门狗对象，喂狗周期为 1000ms

    while (1) {

        if (g_systick_flag) {
            g_systick_flag = 0; // 清除标志位

            systick_cnt++;

            if (systick_cnt % 100 == 0) { // 每 100ms 设置一次标志位
                flag_100ms = 1;
            }

            if (systick_cnt % 500 == 0) { // 每 500ms 设置一次标志位
                flag_500ms = 1;
            }

            if (systick_cnt % 10000 == 0) { // 每 10s 设置一次标志位
                flag_10s = 1;
            }

            if (systick_cnt >= 10000) { // 防止溢出，10秒后重置计数器
                systick_cnt = 0;
            }

            if (flag_100ms) {
                flag_100ms = 0; // 清除标志位

                // 100ms 定时任务
                static uint8_t is_fading_up = 1; // 1 表示正在变亮，0 表示正在变暗
                switch (rgb_led_status) {
                case RGB_LED_STATUS::RED:

                    rgb_tab[1] = 0;
                    rgb_tab[2] = 0;

                    if (is_fading_up) {
                        // --- 上升阶段 ---
                        rgb_tab[0] += rgb_val++; 
                        if (rgb_tab[0] >= 400) {
                            is_fading_up = 0; // 触顶，切换到下降模式
                            // 注意：此时 rgb_val 保持当前最大值，不重置
                        }
                    } else {
                        // --- 下降阶段 (原路返回) ---
                        rgb_val--;               // 步进值先递减
                        rgb_tab[0] -= rgb_val;   // 亮度随之递减
                        
                        // 当步进值回到 0 或 亮度降到底时切换
                        if (rgb_val <= 0 || rgb_tab[0] <= 0 || rgb_tab[0] > 512) { 
                            rgb_tab[0] = 0;
                            rgb_val = 0;
                            is_fading_up = 1;  // 重置为上升模式，供下一个灯使用
                            rgb_led_status = RGB_LED_STATUS::GREEN; 
                        }
                    }
                    
                    break;

                case RGB_LED_STATUS::GREEN:
                    rgb_tab[0] = 0;
                    rgb_tab[2] = 0;

                    if (is_fading_up) {
                        // --- 上升阶段 ---
                        rgb_tab[1] += rgb_val++; 
                        if (rgb_tab[1] >= 400) {
                            is_fading_up = 0; // 触顶，切换到下降模式
                            // 注意：此时 rgb_val 保持当前最大值，不重置
                        }
                    } else {
                        // --- 下降阶段 (原路返回) ---
                        rgb_val--;               // 步进值先递减
                        rgb_tab[1] -= rgb_val;   // 亮度随之递减
                        
                        // 当步进值回到 0 或 亮度降到底时切换
                        if (rgb_val <= 0 || rgb_tab[1] <= 0 || rgb_tab[1] > 512) { 
                            rgb_tab[1] = 0;
                            rgb_val = 0;
                            is_fading_up = 1;  // 重置为上升模式，供下一个灯使用
                            rgb_led_status = RGB_LED_STATUS::BLUE; 
                        }
                    }

                    break;

                case RGB_LED_STATUS::BLUE:
                    rgb_tab[0] = 0;
                    rgb_tab[1] = 0;

                    if (is_fading_up) {
                        // --- 上升阶段 ---
                        rgb_tab[2] += rgb_val++; 
                        if (rgb_tab[2] >= 400) {
                            is_fading_up = 0; // 触顶，切换到下降模式
                            // 注意：此时 rgb_val 保持当前最大值，不重置
                        }
                    } else {
                        // --- 下降阶段 (原路返回) ---
                        rgb_val--;               // 步进值先递减
                        rgb_tab[2] -= rgb_val;   // 亮度随之递减
                        
                        // 当步进值回到 0 或 亮度降到底时切换
                        if (rgb_val <= 0 || rgb_tab[2] <= 0 || rgb_tab[2] > 512) { 
                            rgb_tab[2] = 0;
                            rgb_val = 0;
                            is_fading_up = 1;  // 重置为上升模式，供下一个灯使用
                            rgb_led_status = RGB_LED_STATUS::RED; 
                        }
                    }
                    break;

                default:
                    break;
                }

                rgbLed.setColor(rgb_tab[0], rgb_tab[1], rgb_tab[2]);
            }

            if (flag_500ms) {
                flag_500ms = 0; // 清除标志位

                led1.toggle();
                led2.toggle();

                task_cnt++;
                uart0 << "Hello, GD32E23x! count: " << (int)task_cnt << " | " << Conv::to_hex(task_cnt) << " | " << Raw::makeRaw((const uint8_t*)&task_cnt, sizeof(task_cnt)) << "\r\n"; // 输出计数器值和一个十六进制数
                uart0 << 0xFF << " " << Conv::to_hex(0xFE) << " " << 0xFD << " " << Conv::to_hex(0xFC) << "\r\n"; // 直接输出十六进制数测试
                double test_float = 3.141592;
                uart0 << test_float << "\r\n"; // 浮点数测试
                
                watchdog.feed_dog(); // 喂狗
            }
    
            // 10 秒定时
            if (flag_10s) {
                flag_10s = 0; // 清除标志位
                uint16_t adc_val = adc.readChannel();
                uart0 << "ADC Val: " << (int)adc_val << "\r\n";

                RtcTime current_time = rtc.get();
                uart0 << "Current RTC Time: " 
                      << (int)current_time.year << "-" 
                      << (int)current_time.month << "-" 
                      << (int)current_time.date << " " 
                      << (int)current_time.hour << ":" 
                      << (int)current_time.minute << ":" 
                      << (int)current_time.second << "\r\n";
            }

            if (key1.scan() == true) {
                uart0 << "Key1 Pressed\r\n";
            }
            if (key2.scan() == true) {
                uart0 << "Key2 Pressed\r\n";
            }

            if (g_ext0_flag) {
                g_ext0_flag = 0; // 清除标志位
                uart0 << "EXTI Line 0 Interrupt Triggered\r\n";
            }
        }

        __NOP(); // 占位，防止编译器优化掉循环
    }
}