#include "therm.h"

#include <math.h>

// Inicializa el termistor con los parámetros proporcionados y configura el ADC
esp_err_t therm_init(therm_t* thermistor, adc_unit_t adc_unit, adc_channel_t channel, float series_resistance, float nominal_resistance, float nominal_temperature, float beta_coefficient) {
    thermistor->adc_channel = channel;
    thermistor->series_resistance = series_resistance;
    thermistor->nominal_resistance = nominal_resistance;
    thermistor->nominal_temperature = nominal_temperature;
    thermistor->beta_coefficient = beta_coefficient;

    // Configuración del ADC
    adc_oneshot_unit_init_cfg_t unit_cfg = {
        .unit_id = adc_unit,
        .clk_src = ADC_RTC_CLK_SRC_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&unit_cfg, &thermistor->adc_hdlr));

    // Configuración del canal del ADC
    adc_oneshot_chan_cfg_t channel_cfg = {
        .atten = ADC_ATTEN_DB_11,     // Rango de 0 a 3.3V
        .bitwidth = ADC_BITWIDTH_12,  // Resolución de 12 bits
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(thermistor->adc_hdlr, channel, &channel_cfg));

    return ESP_OK;
}

// Lee la temperatura del termistor
float therm_read_temperature(therm_t thermistor) {
    uint16_t lsb = therm_read_lsb(thermistor);
    float voltage = therm_lsb_to_voltage(lsb);
    return therm_voltage_to_temperature(voltage, thermistor.series_resistance, thermistor.nominal_resistance, thermistor.nominal_temperature, thermistor.beta_coefficient);
}

// Lee el voltaje del termistor
float therm_read_voltage(therm_t thermistor) {
    uint16_t lsb = therm_read_lsb(thermistor);
    return therm_lsb_to_voltage(lsb);
}

// Lee el valor LSB del termistor
uint16_t therm_read_lsb(therm_t thermistor) {
    int raw_value = 0;
    ESP_ERROR_CHECK(adc_oneshot_read(thermistor.adc_hdlr, thermistor.adc_channel, &raw_value));
    return raw_value;
}

// Convierte el voltaje a temperatura en grados Celsius
float therm_voltage_to_temperature(float voltage, float series_resistance, float nominal_resistance, float nominal_temperature, float beta_coefficient) {
    float r_ntc = series_resistance * (3.3 - voltage) / voltage;
    float t_kelvin = 1.0f / (1.0f / nominal_temperature + (1.0f / beta_coefficient) * log(r_ntc / nominal_resistance));
    return t_kelvin - 273.15f;
}

// Convierte el valor LSB a voltaje
float therm_lsb_to_voltage(uint16_t lsb) {
    return (float)(lsb * 3.3f / 4095.0f);
}