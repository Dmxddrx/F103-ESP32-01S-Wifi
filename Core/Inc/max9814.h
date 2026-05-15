#ifndef MAX9814_H
#define MAX9814_H

#include "stm32f1xx_hal.h"
#include <math.h>

// This is an arbitrary offset. To get True dB SPL, you must calibrate
// this against a known sound level meter in the real world.
#define DB_OFFSET 50.0

void MAX9814_Init(ADC_HandleTypeDef *hadc);
uint16_t MAX9814_Get_Amplitude(ADC_HandleTypeDef *hadc);
float MAX9814_Get_Decibels(ADC_HandleTypeDef *hadc);

#endif /* MAX9814_H */
