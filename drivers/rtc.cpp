#include "rtc.hpp"

inline uint8_t DEC2BCD(uint8_t val) {
    return (((val) / 10) << 4) | ((val) % 10);
}

inline uint8_t BCD2DEC(uint8_t val) {
    return ((val >> 4) * 10) + (val & 0x0F);
}

void Rtc::init() {
    // 1. 开启 PMU 和 BKPI (备份接口) 时钟
    rcu_periph_clock_enable(RCU_PMU);
    // 2. 允许访问备份寄存器
    pmu_backup_write_enable();

    // 3. 重置备份域
    rcu_bkp_reset_enable();
    rcu_bkp_reset_disable();

    //4. 配置 RTC 时钟源，外部低速时钟 LXTAL
    rcu_osci_on(RCU_LXTAL);

    //5. 等待晶振稳定
    rcu_periph_clock_enable(RCU_PMU);
    pmu_backup_write_enable();

    rcu_osci_on(RCU_LXTAL);
    
    // 如果这里等了很久，说明硬件起振有问题
    if (ERROR == rcu_osci_stab_wait(RCU_LXTAL)) {
        // 硬件 LXTAL 失败，临时切换到内部 IRC40K 测试
        rcu_osci_on(RCU_IRC40K);
        rcu_osci_stab_wait(RCU_IRC40K);
        rcu_rtc_clock_config(RCU_RTCSRC_IRC40K);
        
        // 注意：IRC40K 分频通常设为 asyn=124, syn=319 (125*320=40000)
    } else {
        rcu_rtc_clock_config(RCU_RTCSRC_LXTAL);
    }

    rcu_periph_clock_enable(RCU_RTC);
    rtc_register_sync_wait();

    // 9. 配置 RTC 分频器，设置 RTC 时钟为 1Hz
    // 由于 LXTAL 是 32768Hz，预分频器设置为 32767，分频器设置为 0，这样 RTC 时钟就是 32768 / (32767 + 1) = 1Hz
    rtc_parameter_struct rtc_init_struct;
    /* 异步分频 = 127, 同步分频 = 255 */
    /* (127+1) * (255+1) = 128 * 256 = 32768 */
    rtc_init_struct.rtc_year = 0x26;  // 2026年
    rtc_init_struct.rtc_month = RTC_MAR;
    rtc_init_struct.rtc_date = 0x13;
    rtc_init_struct.rtc_day_of_week = RTC_FRIDAY;
    rtc_init_struct.rtc_hour = 0x12;
    rtc_init_struct.rtc_minute = 0x00;
    rtc_init_struct.rtc_second = 0x00;
    rtc_init_struct.rtc_display_format = RTC_24HOUR;
    rtc_init_struct.rtc_am_pm = RTC_AM;
    /* 配置分频系数 */
    rtc_init_struct.rtc_factor_asyn = 0x7F; // 127
    rtc_init_struct.rtc_factor_syn = 0xFF;  // 255

    rtc_init(&rtc_init_struct);
}

void Rtc::set(RtcTime &set_time) {
    rtc_parameter_struct rtc_set_struct;

    // 1. 核心转换
    rtc_set_struct.rtc_year   = DEC2BCD(set_time.year % 100); 
    rtc_set_struct.rtc_month  = DEC2BCD(set_time.month);
    rtc_set_struct.rtc_date   = DEC2BCD(set_time.date);
    rtc_set_struct.rtc_hour   = DEC2BCD(set_time.hour);
    rtc_set_struct.rtc_minute = DEC2BCD(set_time.minute);
    rtc_set_struct.rtc_second = DEC2BCD(set_time.second);
    
    // 2. 星期直接赋值
    rtc_set_struct.rtc_day_of_week = set_time.day_of_week;

    // 3. 格式配置
    rtc_set_struct.rtc_display_format = RTC_24HOUR;
    rtc_set_struct.rtc_am_pm = RTC_AM;
    rtc_set_struct.rtc_factor_asyn = 0x7F;
    rtc_set_struct.rtc_factor_syn = 0xFF;

    // 4. 写入硬件
    if (ERROR == rtc_init(&rtc_set_struct)) {
        // 如果返回 ERROR，说明当前 RTC 处于写保护或初始化模式未退出
        // 可以在这里增加打印调试信息
    }
}

RtcTime& Rtc::get(void) {
    static RtcTime current_time;
    rtc_parameter_struct rtc_get_struct;
    rtc_current_time_get(&rtc_get_struct);
    
    current_time.year = BCD2DEC(rtc_get_struct.rtc_year);
    current_time.month = BCD2DEC(rtc_get_struct.rtc_month);
    current_time.date = BCD2DEC(rtc_get_struct.rtc_date);
    current_time.day_of_week = rtc_get_struct.rtc_day_of_week;
    current_time.hour = BCD2DEC(rtc_get_struct.rtc_hour);
    current_time.minute = BCD2DEC(rtc_get_struct.rtc_minute);
    current_time.second = BCD2DEC(rtc_get_struct.rtc_second);

    return current_time;
}