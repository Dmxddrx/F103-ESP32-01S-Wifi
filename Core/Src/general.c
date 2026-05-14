#include "general.h"

// State tracking variable
uint8_t wifi_is_connected = 0;
uint8_t esp_is_ready = 0;
char current_ip[16] = "0.0.0.0";

void General_Init(I2C_HandleTypeDef *hi2c) {
    // 1. Initialize the Display
    OLED_Init(hi2c);
    OLED_Clear();

    // Project Title Boot Screen
    OLED_Print(0, 0, "ESP01S");
    HAL_Delay(100);
    OLED_Print(35, 0, "Booting");
    OLED_Update();

    // 2. Initialize ESP-01S
    //OLED_ClearArea(0, 0, 128, 64); // Clear the "System Booting" text
    OLED_Print(0, 16, "Init ESP-01S");
    OLED_Update();

    if (WIFI_Init()) {
        OLED_Print(0, 25, "ESP-01S: READY");
        esp_is_ready = 1;
        OLED_Update();
    } else {
        OLED_Print(0, 25, "ESP-01S: FAIL");
        esp_is_ready = 0;
        OLED_Update();
        return; // Halt further execution if no ESP is found
    }

    // 3. Connect to Wi-Fi
    //OLED_ClearArea(10, 45, 110, 10);
    OLED_Print(0, 35, "Connecting AP");
    OLED_Update();

    if (WIFI_Connect(WIFI_SSID, WIFI_PASS)) {
        OLED_ClearArea(10, 45, 110, 10);
        OLED_Print(0, 45, "Connected!");
        wifi_is_connected = 1;

        OLED_Update();

		// CRITICAL FIX: Wait 500ms here so the router can assign the IP!
		HAL_Delay(500);
        // Fetch the IP address once connected
		WIFI_GetIP(current_ip);

    } else {
        OLED_ClearArea(10, 45, 110, 10);
        OLED_Print(0, 45, "Timeout");
        wifi_is_connected = 0;
    }

    OLED_Update();
    HAL_Delay(500);

    // Clear screen for the main run loop
    //OLED_ClearArea(0, 0, 128, 64);
    OLED_Update();
}

void General_Run(void) {
    // Static variable for non-blocking UI updates
    static uint32_t last_ui_update = 0;

    // Update the OLED every 1 second
    if (HAL_GetTick() - last_ui_update >= 1000) {
        last_ui_update = HAL_GetTick();

        OLED_ClearArea(0, 0, 128, 64); // Clear the whole screen for a fresh frame

        OLED_Print(0, 0, "Status");

        // --- NEW: Print ESP Hardware Status ---
        if (esp_is_ready) {
            OLED_Print(0, 25, "ESP-01S: OK");
        } else {
            OLED_Print(0, 25, "ESP-01S: ERROR");
        }

        // --- Print Wi-Fi Connection Status ---
        if (wifi_is_connected) {
             OLED_Print(0, 35, "Wi-Fi: ONLINE ");
             // Here you could send AT+CIPSEND to push sensor data

             OLED_Print(50, 0, current_ip);
        } else {
             OLED_Print(0, 35, "Wi-Fi: OFFLINE");
             // Add reconnect logic here if needed
        }

        OLED_Update();
    }
}
