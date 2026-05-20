#ifndef WCMCU75_H_
#define WCMCU75_H_

#include "stm32f1xx_hal.h"

/* * I2C ADDRESS CONFIGURATION
 * The base address of the LM75 is 0x48 (7-bit).
 * Because STM32 HAL uses 8-bit shifted addresses, we shift it by 1: (0x48 << 1) = 0x90
 * If A0, A1, and A2 are all connected to GND, this address is correct.
 */
#define WCMCU75_ADDRESS 0x90

// LM75 Internal Register Addresses
#define LM75_REG_TEMP   0x00
#define LM75_REG_CONF   0x01
#define LM75_REG_THYST  0x02
#define LM75_REG_TOS    0x03

// Function Prototypes
int8_t WCMCU75_Init(I2C_HandleTypeDef *hi2c);
float WCMCU75_ReadTemp(void);

#endif /* WCMCU75_H_ */
