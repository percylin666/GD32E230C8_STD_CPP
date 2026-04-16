#include "wwd.hpp"

void WWatchdog::init() const {
    /* 1 开启时钟 */
    rcu_periph_clock_enable(RCU_WWDGT);

    /* 2 配置看门狗 
        - counter_value: 0x7F 最大值
        - window_value: 0x5F 窗口值，必须小于counter_value，否则会一直复位，当计数器减到 0x5F 以下, 0x3F 以上时才允许喂狗
        - prescaler: WWDGT_CFG_PSC_DIV8 预分频，时钟频率为PCLK1/4096/8
    */
    
    wwdgt_config(m_counter_value, m_window_value, WWDGT_CFG_PSC_DIV8);

    /* 3 启动看门狗 */
    wwdgt_enable();

    /* 4 程序说明 
        - wwdgt_config的参数一般是0x7F、0x5F、WWDGT_CFG_PSC_DIV8
        - 这组参数的功能是使能后必须在+14.5 ~ +29.5ms内喂狗，否则会复位
        - 第一次喂狗后，后续喂狗必须在+14.5 ~ +29.5ms内
        - 需要精准估算程序的执行时间，确保在窗口期内喂狗
    */
}

void WWatchdog::feed_dog(void) const {
    /* 喂狗，重装载计数器 */
    wwdgt_counter_update(m_counter_value);
}