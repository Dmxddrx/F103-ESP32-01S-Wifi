#include "wifi.h"
#include "main.h" // ADDED: Needed for the ESP_RST_Pin definition
#include "oled.h"

char buffer[512];

void WIFI_SendCommand(char* command) {
    HAL_UART_Transmit(&huart2, (uint8_t*)command, strlen(command), 1000);
}

int8_t WIFI_WaitForResponse(char* expected_response, uint32_t timeout) {
    uint32_t startTime = HAL_GetTick();
    memset(buffer, 0, sizeof(buffer));
    uint16_t index = 0;

    while (HAL_GetTick() - startTime < timeout) {
        uint8_t data;
        if (HAL_UART_Receive(&huart2, &data, 1, 10) == HAL_OK) {
            // SAFETY FIX: Prevent buffer overflow if expected response never arrives
            if (index < 511) {
                buffer[index++] = data;
            }
            if (strstr(buffer, expected_response)) {
                return 1; // Success
            }
        }
    }
    return 0; // Timeout
}

int8_t WIFI_Init(void) {
    // --- ADVANCED HARDWARE BOOT & RESET SEQUENCE ---

    // 1. Force the ESP into Normal Boot Mode by pulling GPIO0 HIGH
    HAL_GPIO_WritePin(GPIOA, ESP_IO0_Pin, GPIO_PIN_SET);

    // 2. Pull RST LOW to hard-reset the ESP
    HAL_GPIO_WritePin(ESP_RST_GPIO_Port, ESP_RST_Pin, GPIO_PIN_RESET);
    HAL_Delay(100);

    // 3. Pull RST HIGH to let it boot (while GPIO0 is still held HIGH)
    HAL_GPIO_WritePin(ESP_RST_GPIO_Port, ESP_RST_Pin, GPIO_PIN_SET);

    // 4. Give the ESP-01S 1 second to fully boot up
    HAL_Delay(800);

    // 5. Release GPIO0 (Set it back to an input)
    // This prevents the STM32 from interfering if the ESP tries to use the pin later.
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = ESP_IO0_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // 6. Flush the UART data register to clear out the boot garbage
    // (The ESP spits out a bunch of random text at 74880 baud when it first turns on)
    __HAL_UART_FLUSH_DRREGISTER(&huart2);
    // ------------------------------------------------

    // Now proceed with normal AT Commands
    // Inside wifi.c - WIFI_Init
    WIFI_SendCommand("AT\r\n");
    if (!WIFI_WaitForResponse("OK", 500)) {
        // Optional: Print the first 10 characters of whatever the ESP actually sent
        OLED_Print(0, 50, buffer);
        OLED_Update();
        return 0;
    }

    WIFI_SendCommand("AT+CWMODE=1\r\n");
    if (!WIFI_WaitForResponse("OK", 500)) return 0;

    return 1; // Init successful
}

int8_t WIFI_Connect(char* ssid, char* password) {
    char conn_cmd[128];
    sprintf(conn_cmd, "AT+CWJAP=\"%s\",\"%s\"\r\n", ssid, password);

    WIFI_SendCommand(conn_cmd);

    // Give it 15 seconds to connect to the router
    if (WIFI_WaitForResponse("WIFI GOT IP", 15000)) {
        return 1;
    }
    return 0;
}

int8_t WIFI_GetIP(char* ip_out) {
    WIFI_SendCommand("AT+CIFSR\r\n");

    // The response looks like: +CIFSR:STAIP,"192.168.1.15"
    if (WIFI_WaitForResponse("OK", 1000)) {
        char* start = strstr(buffer, "STAIP,\"");
        if (start) {
            start += 7; // Move past STAIP,"
            char* end = strchr(start, '\"');
            if (end) {
                size_t len = end - start;
                strncpy(ip_out, start, len);
                ip_out[len] = '\0'; // Null terminate
                return 1;
            }
        }
    }
    return 0;
}
