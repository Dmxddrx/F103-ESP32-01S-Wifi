#ifndef GENERAL_H_
#define GENERAL_H_

#include "stm32f1xx_hal.h"
#include "wifi.h"
#include "oled.h"
#include "max9814.h"

// Configuration
#define WIFI_SSID "Dialog 4G 208"
#define WIFI_PASS "Hasith2001"

// Function Prototypes
void General_Init(I2C_HandleTypeDef *hi2c);
void General_Run(void);

#endif /* GENERAL_H_ */
