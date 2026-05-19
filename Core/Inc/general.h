#ifndef GENERAL_H_
#define GENERAL_H_

#include "stm32f1xx_hal.h"
#include "wifi.h"
#include "oled.h"
#include "max9814.h"

// Configuration
#define WIFI_SSID "Dialog 4G 208"
#define WIFI_PASS "Hasith2001"

#define PC_IP "192.168.8.198"  // <-- Change this to the Static IP of your Windows PC!
//#define PC_IP "192.168.8.155"
#define UDP_PORT 8080         // Must match the port in your C# App

// Function Prototypes
void General_Init(I2C_HandleTypeDef *hi2c);
void General_Run(void);

#endif /* GENERAL_H_ */
