#include "general.h"
#include <stdio.h> // Needed for sprintf

// --- Global Variables for STM32CubeMonitor ---
volatile float global_db_value = 0.0f;        // Monitor the microphone
volatile uint32_t global_loop_counter = 0;    // Monitor "CPU Activity"
volatile uint32_t global_i2c_error = 0;       // Monitor I2C health

// State tracking variable
uint8_t wifi_is_connected = 0;
uint8_t esp_is_ready = 0;
char current_ip[16] = "0.0.0.0";

// ---> ADD THESE THREE LINES HERE <---
// This tells general.c to go look in main.c for these variables
extern ADC_HandleTypeDef hadc1;
extern I2C_HandleTypeDef hi2c1;
extern UART_HandleTypeDef huart2;

// Import the ADC handle from main.c
extern ADC_HandleTypeDef hadc1;

void General_Init(I2C_HandleTypeDef *hi2c) {
	// Initialize the Microphone ADC Calibration
	MAX9814_Init(&hadc1);

    // 1. Initialize the Display
    OLED_Init(hi2c);
    OLED_Clear();

    // Project Title Boot Screen
    OLED_Print(0, 0, "ESP01S");
    HAL_Delay(500);
    OLED_Print(35, 0, "Booting");
    OLED_Update();

    // 2. Initialize ESP-01S
    OLED_Print(0, 16, "Init ESP-01S");
    OLED_Update();

    if (WIFI_Init()) {
    	OLED_ClearArea(0, 26, 128, 10);
        OLED_Print(0, 26, "ESP-01S: READY");
        esp_is_ready = 1;
        OLED_Update();
    } else {
    	OLED_ClearArea(0, 26, 128, 10);
        OLED_Print(0, 26, "ESP-01S: FAIL");
        esp_is_ready = 0;
        OLED_Update();
        return; // Halt further execution if no ESP is found
    }

    // 3. Connect to Wi-Fi
    OLED_Print(0, 36, "Connecting AP");
    OLED_Update();

    if (WIFI_Connect(WIFI_SSID, WIFI_PASS)) {
        OLED_ClearArea(0, 46, 128, 10);
        OLED_Print(0, 46, "Connected!");
        wifi_is_connected = 1;

        OLED_Update();

		// CRITICAL FIX: Wait 500ms here so the router can assign the IP!
		HAL_Delay(500);
        // Fetch the IP address once connected
		WIFI_GetIP(current_ip);

    } else {
        OLED_ClearArea(0, 46, 128, 10);
        OLED_Print(0, 46, "Timeout");
        wifi_is_connected = 0;
    }

    OLED_Update();
    HAL_Delay(500);

    // Clear screen for the main run loop
    //OLED_ClearArea(0, 0, 128, 64);
    OLED_Update();
}

void General_Run(void) {

	// ----------------------------------------------------------
	// CUBEMONITOR TASK: Monitor CPU "breathing"
	// ----------------------------------------------------------
	global_loop_counter++;

    // Static variable for non-blocking UI updates
    static uint32_t last_ui_update = 0;
    static uint32_t last_wifi_send = 0;

    // Static variable so the OLED can share the latest reading from the Wi-Fi task
    static float db_history[5] = {0.0f};
	static uint8_t db_count = 0;

    // Buffer for formatting the dB text
	char mic_buffer[15];
	char wifi_tx_buffer[64];

	// ==========================================================
	// TASK 1: Read Mic and Send via Wi-Fi (Every 200ms / 5Hz)
	// ==========================================================
	if (HAL_GetTick() - last_wifi_send >= 200) {
		last_wifi_send = HAL_GetTick();

		// 1. Get the latest Decibel reading
		global_db_value = MAX9814_Get_Decibels(&hadc1);

		// 2. Manage the 5-item list for the OLED
		if (db_count >= 5) {
			db_count = 0; // Reset back to 0 to "clear" the list after 5 readings
		}
		db_history[db_count] = global_db_value; // Store the new reading
		db_count++;                       // Move to the next line for next time

		// 3. Wi-Fi Transmission
		if (wifi_is_connected) {
			int len = snprintf(wifi_tx_buffer, sizeof(wifi_tx_buffer), "Audio: %.1f dB\r\n", global_db_value);
			HAL_UART_Transmit(&huart2, (uint8_t*)wifi_tx_buffer, len, 100);
		}
	}

    // Update the OLED every 1 second
	// keep this decoupled from the mic read so the UI doesn't flicker wildly
    if (HAL_GetTick() - last_ui_update >= 100) {
        last_ui_update = HAL_GetTick();

        // Update CubeMonitor with the I2C bus health status
		global_i2c_error = HAL_I2C_GetError(&hi2c1);

        OLED_ClearArea(0, 0, 128, 15); // Clear Status bar
        OLED_ClearArea(0, 16, 70, 48); // Clear details block

        OLED_Print(0, 0, "Status");

        // --- NEW: Print ESP Hardware Status ---
        if (esp_is_ready) {
            OLED_Print(0, 16, "ESP-01S: OK");
        } else {
            OLED_Print(0, 16, "ESP-01S: ERROR");
        }

        // --- Print Wi-Fi Connection Status ---
        if (wifi_is_connected) {
             OLED_Print(0, 26, "Wi-Fi: ONLINE ");
             // Here you could send AT+CIPSEND to push sensor data

             OLED_Print(50, 0, current_ip);
        } else {
             OLED_Print(0, 26, "Wi-Fi: OFFLINE");
             // Add reconnect logic here if needed
        }

        // --- Print the latest Mic Decibel Value ---
        snprintf(mic_buffer, sizeof(mic_buffer), "Audio: %.1f dB", global_db_value);
		OLED_Print(0, 56, mic_buffer);

		for (uint8_t i = 0; i < db_count; i++) {
			snprintf(mic_buffer, sizeof(mic_buffer), "%.1f dB", db_history[i]);
			OLED_Print(75, 16 + (i * 10), mic_buffer);
		}

        OLED_Update();
    }
}
