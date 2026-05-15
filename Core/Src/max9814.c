#include "max9814.h"

// We create a buffer of 2000 samples.
// At ~47kHz sampling rate, this holds about 42 milliseconds of audio history.
// 2000 * 16-bits = 4 Kilobytes of RAM (The Bluepill has 20KB, so this is very safe).
#define ADC_BUFFER_SIZE 2000
uint16_t adc_buffer[ADC_BUFFER_SIZE];

void MAX9814_Init(ADC_HandleTypeDef *hadc) {
    // ADC hardware init is usually handled by CubeMX in main.c
    // You can run a calibration here if needed for the STM32F103
    HAL_ADCEx_Calibration_Start(hadc);

    // Start the ADC in DMA mode. It will continuously fill adc_buffer in the background forever.
	HAL_ADC_Start_DMA(hadc, (uint32_t*)adc_buffer, ADC_BUFFER_SIZE);
}

uint16_t MAX9814_Get_Amplitude(ADC_HandleTypeDef *hadc) {
    uint16_t max_val = 0;
    uint16_t min_val = 4095;

    // Instead of waiting 50ms for the ADC to read, we just rapidly scan the array
    // that the DMA has been secretly filling in the background.
    // At 72MHz, scanning 2000 elements takes a fraction of a millisecond!
    for (int i = 0; i < ADC_BUFFER_SIZE; i++) {
        if (adc_buffer[i] > max_val) {
            max_val = adc_buffer[i];
        }
        if (adc_buffer[i] < min_val) {
            min_val = adc_buffer[i];
        }
    }

    // Return the peak-to-peak amplitude
    return (max_val - min_val);
}

float MAX9814_Get_Decibels(ADC_HandleTypeDef *hadc) {
    uint16_t amplitude = MAX9814_Get_Amplitude(hadc);

    if (amplitude == 0) {
        return 0.0f;
    }

    // Calculate volts peak-to-peak (Assuming 3.3V reference and 12-bit ADC)
    float voltage = (amplitude * 3.3f) / 4095.0f;

    // Standard audio formula: 20 * log10(V/V0) + offset
    float db = 20.0f * log10f(voltage) + DB_OFFSET;

    if (db < 0.0f) db = 0.0f;

    return db;
}
