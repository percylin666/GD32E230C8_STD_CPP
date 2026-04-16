#include "adc.hpp"


void Adc::init() {
    /* 1. 开启 GPIOA 和 ADC 时钟 */
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_ADC);
    
    /* 2. 配置 ADC 时钟分频 (假设 APB2 为 72MHz, 6分频为 12MHz) */
    rcu_adc_clock_config(RCU_ADCCK_APB2_DIV6);

    /* 3. 配置 PA1 为模拟输入 */
    gpio_mode_set(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO_PIN_1);

    /* 4. 复位 ADC */
    adc_deinit();

    /* 5. 使用你的函数原型进行模式配置 */
    adc_special_function_config(ADC_SCAN_MODE, DISABLE);
    adc_special_function_config(ADC_CONTINUOUS_MODE, DISABLE);
    
    /* 6. 数据右对齐 */
    adc_data_alignment_config(ADC_DATAALIGN_RIGHT);

    /* 7. 配置常规通道组长度为 1 (只读 PA1 一个通道) */
    adc_channel_length_config(ADC_REGULAR_CHANNEL, 1);

    /* 8. 配置 PA1 (Channel 1) 的采样顺序和时间 */
    // 第一个参数 0 代表它是常规组里的第 1 个转换通道
    switch (m_channel) {
    case ADC_CHANNEL::CHANNEL_0:
        break;

    case ADC_CHANNEL::CHANNEL_1:
        adc_regular_channel_config(0, ADC_CHANNEL_1, ADC_SAMPLETIME_28POINT5);
        break;

    default:
        break;
    }

    /* 9. 配置触发源：软件触发 */
    adc_external_trigger_source_config(ADC_REGULAR_CHANNEL, ADC_EXTTRIG_REGULAR_NONE);
    adc_external_trigger_config(ADC_REGULAR_CHANNEL, ENABLE);

    /* 10. 使能 ADC 并校准 */
    adc_enable();

    adc_calibration_enable();
}

uint16_t Adc::readChannel(void) const{
    /* 1. 软件触发转换 */
    adc_software_trigger_enable(ADC_REGULAR_CHANNEL);
    
    /* 2. 等待转换完成标志 (EOC) */
    while (SET != adc_flag_get(ADC_FLAG_EOC));
    
    /* 3. 清除标志位并读取数据 */
    adc_flag_clear(ADC_FLAG_EOC);
    return adc_regular_data_read();
}