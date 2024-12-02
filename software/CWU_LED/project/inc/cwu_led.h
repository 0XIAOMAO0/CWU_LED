#ifndef __CWU_LED_H__
#define __CWU_LED_H__

#include "at32f421_wk_config.h"

#define COLD_LIGHT_LED_PORT TMR3
#define COLD_LIGHT_LED_CHNANEL TMR_SELECT_CHANNEL_1
#define WARM_LIGHT_LED_PORT TMR1
#define WARM_LIGHT_LED_CHNANEL TMR_SELECT_CHANNEL_1
#define UV_LIGHT_LED_PORT TMR3
#define UV_LIGHT_LED_CHNANEL TMR_SELECT_CHANNEL_2

typedef struct {
    uint16_t Color_temp_value_adc;
    uint16_t Luminance_adc;
    uint16_t vbat_adc;
    uint16_t ntc1_adc;
    uint16_t ntc2_adc;
}CWU_ADC_Data_t;

void cwu_mode_set(uint8_t mode_num);
void cwu_pwm_set(void);

extern CWU_ADC_Data_t cuw_adc_data;
void cwu_adc_push(uint16_t *adc_buff);
void cwu_vbat_monitor(void);
void cwu_temperature_monitor(void);

#endif
