#include "fwd.hpp"

void FWatchdog::init() {
    /* 1. 解锁寄存器 */
    fwdgt_write_enable();

    /* 2. 配置看门狗
        FWDGT_RLD；设置重装载寄存器的值，决定了看门狗的超时时间
        FWDGT_PSC_DIV64；对40kHz的时钟源进行分频，得到625Hz的时钟频率
    */

    // 先通过喂狗周期计算出重装载寄存器的值
    uint16_t reload_value = (625 * m_feed_dog_period_ms) / 1000; // 计算重装载寄存器的值，单位为毫秒
    fwdgt_config(reload_value, FWDGT_PSC_DIV64);

    /* 3. 启动看门狗 */
    fwdgt_enable();
}

void FWatchdog::feed_dog(void) const{
    fwdgt_counter_reload(); // 重装载寄存器重新加载，喂狗
}