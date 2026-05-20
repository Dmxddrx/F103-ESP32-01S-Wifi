#include "wcmcu75.h"

// Store the I2C handle so the read function can use it
static I2C_HandleTypeDef *lm75_i2c;

int8_t WCMCU75_Init(I2C_HandleTypeDef *hi2c) {
    lm75_i2c = hi2c;

    // Ping the sensor on the I2C bus to make sure it is connected and responding
    if (HAL_I2C_IsDeviceReady(lm75_i2c, WCMCU75_ADDRESS, 3, 100) == HAL_OK) {
        return 1; // Sensor found successfully
    }
    return 0; // Sensor not found (Check wiring or A0/A1/A2 address pins)
}

float WCMCU75_ReadTemp(void) {
    uint8_t buffer[2];
    int16_t raw_temp;
    float final_temp = 0.0f;

    // Read 2 bytes from the Temperature Register (0x00)
    if (HAL_I2C_Mem_Read(lm75_i2c, WCMCU75_ADDRESS, LM75_REG_TEMP, I2C_MEMADD_SIZE_8BIT, buffer, 2, 100) == HAL_OK) {

        // Combine the two bytes (MSB first)
        raw_temp = (buffer[0] << 8) | buffer[1];

        // The LM75A stores the 11-bit temperature left-justified.
        // We shift right by 5 to align the bits correctly.
        raw_temp = raw_temp >> 5;

        // Handle negative temperatures (Two's complement)
        if (raw_temp & 0x0400) { // If the 11th bit (sign bit) is 1
            raw_temp |= 0xF800;  // Pad the top 5 bits with 1s to keep it negative in standard 16-bit math
        }

        // Multiply by the sensor's resolution (0.125 °C per bit)
        final_temp = (float)raw_temp * 0.125f;
    }

    return final_temp;
}
