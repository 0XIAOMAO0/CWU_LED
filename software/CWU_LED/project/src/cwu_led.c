#include "cwu_led.h"
#include "cct_to_cie.h"
#include <string.h>

#define CWU_LED_ADC_CHANNEL_NUM 5

#define WARM_CIE 3000 // 6500K
#define COLD_CIE 6250 // 6000K
#define COLOR_RANGE (COLD_CIE - WARM_CIE)

#define ADC_TO_VOLTAGE(x) (x * 3.3 / 4096.0f)

#define ALARM_RES 3000 // 过温阈值电阻
#define DIVIDED_RES 10000

#define TEMP_ALARM_VOLTAGE 3.3 * ALARM_RES / (ALARM_RES + DIVIDED_RES) // 温度报警电压
#define BAT_ALARM_VOLTAGE 7.2                                          //   报警电压 7.2V

float lum_parameter;
float color_parameter;
uint8_t mode = 0;

CWU_ADC_Data_t cwu_adc_data;

void cwu_adc_push(uint16_t *adc_buff)
{
    memcpy(&cwu_adc_data, adc_buff, sizeof(CWU_ADC_Data_t));
}

void cwu_parameter_count(void)
{
    uint16_t cct_vlaue;
    uint16_t cie_index;
    float Xm;
    float Xw = CIE_ARRY[(WARM_CIE - CCT_MIN) / 10][0];
    float Yw = CIE_ARRY[(WARM_CIE - CCT_MIN) / 10][1];
    float Xc = CIE_ARRY[(COLD_CIE - CCT_MIN) / 10][0];
    float Yc = CIE_ARRY[(COLD_CIE - CCT_MIN) / 10][1];

    cct_vlaue = WARM_CIE + (cwu_adc_data.Color_temp_value_adc / 4095.0f) * COLOR_RANGE;
    cie_index = (uint16_t)(cct_vlaue - CCT_MIN) / 10.0f;
    Xm = CIE_ARRY[cie_index][0];

    lum_parameter = 1 - (cwu_adc_data.Luminance_adc / 4095.0f);
    color_parameter = (Yc * (Xw - Xm)) / (Yw * (Xm - Xc) + Yc * (Xw - Xm));
}

void cwu_mode_set(uint8_t mode_num)
{
    mode = mode_num;
}

void cwu_pwm_set(void)
{
    uint32_t cold_pwm_value;
    uint32_t white_pwm_value;
    cwu_parameter_count();
    if (mode == 0)
    {
        cold_pwm_value = (lum_parameter * color_parameter * 999);
        white_pwm_value = (lum_parameter * (1 - color_parameter) * 999);
        tmr_channel_value_set(COLD_LIGHT_LED_PORT, COLD_LIGHT_LED_CHNANEL, cold_pwm_value);
        tmr_channel_value_set(WARM_LIGHT_LED_PORT, WARM_LIGHT_LED_CHNANEL, white_pwm_value);
        tmr_channel_value_set(UV_LIGHT_LED_PORT, UV_LIGHT_LED_CHNANEL, 0);
    }
    else if (mode == 1)
    {
        tmr_channel_value_set(COLD_LIGHT_LED_PORT, COLD_LIGHT_LED_CHNANEL, 0);
        tmr_channel_value_set(WARM_LIGHT_LED_PORT, WARM_LIGHT_LED_CHNANEL, 0);
        tmr_channel_value_set(UV_LIGHT_LED_PORT, UV_LIGHT_LED_CHNANEL, lum_parameter * 1000);
    }
}

void cwu_vbat_monitor(void)
{
    volatile float vbat_value = 0;
    float last_vbat_value;
    vbat_value = (float)cwu_adc_data.vbat_adc * 4.28 * 2.6 / 4096.0f;
    vbat_value = 0.2 * vbat_value + 0.8 * last_vbat_value;
    last_vbat_value = vbat_value;
    vbat_value = (float)cwu_adc_data.vbat_adc * 4.28 * 2.6 / 4096.0f;
    if(vbat_value < BAT_ALARM_VOLTAGE)
    {
        tmr_channel_value_set(COLD_LIGHT_LED_PORT, COLD_LIGHT_LED_CHNANEL, 499);
        tmr_channel_value_set(WARM_LIGHT_LED_PORT, WARM_LIGHT_LED_CHNANEL, 000);
        wk_delay_ms(500);
        tmr_channel_value_set(COLD_LIGHT_LED_PORT, COLD_LIGHT_LED_CHNANEL, 000);
        tmr_channel_value_set(WARM_LIGHT_LED_PORT, WARM_LIGHT_LED_CHNANEL, 499);
        wk_delay_ms(500);
    }
}

void cwu_temperature_monitor(void)
{
    float temp1_value;
    float temp2_value;
    temp1_value = ADC_TO_VOLTAGE(cwu_adc_data.ntc1_adc);
    temp2_value = ADC_TO_VOLTAGE(cwu_adc_data.ntc2_adc);
    if ((temp1_value < TEMP_ALARM_VOLTAGE) || (temp2_value < TEMP_ALARM_VOLTAGE))
    {
        tmr_channel_value_set(COLD_LIGHT_LED_PORT, COLD_LIGHT_LED_CHNANEL, 499);
        tmr_channel_value_set(WARM_LIGHT_LED_PORT, WARM_LIGHT_LED_CHNANEL, 000);
        wk_delay_ms(500);
        tmr_channel_value_set(COLD_LIGHT_LED_PORT, COLD_LIGHT_LED_CHNANEL, 000);
        wk_delay_ms(500);
    }
}
