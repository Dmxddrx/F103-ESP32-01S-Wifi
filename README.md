# F103-ESP32-01S-Wifi 📡

A robust, C-based driver and status dashboard for bridging an **STM32F103** microcontroller with an **ESP-01S (ESP8266)** Wi-Fi module. 

This repository provides a clean implementation for managing ESP-01S AT commands via UART, featuring an automated hardware reset sequence, reliable network connection handling, and a non-blocking I2C OLED status dashboard to monitor real-time connectivity.

## Current Specs (v1.0)

* **Hardware Reset & Boot:** Includes a dedicated GPIO sequence to hard-reset the ESP-01S and force normal boot mode, eliminating phantom AT command failures.
* **Smart AT Parsing:** Uses custom, timeout-driven UART buffer parsing (`WIFI_WaitForResponse`) to prevent CPU blocking while waiting for network responses (like DHCP IP assignments).
* **Network Capabilities:** Easily initialize the ESP module, connect to Access Points (WPA/WPA2), and extract the local station IP address.
* **Live OLED Dashboard:** A 1Hz non-blocking UI update loop that displays hardware readiness, Wi-Fi status (ONLINE/OFFLINE), and the current IP address.

## System Architecture

* **Communication:** The STM32 interfaces with the ESP-01S over UART (default 115200 baud) using standard AT firmware.
* **UI Integration:** The `General_Run` loop uses `HAL_GetTick()` to refresh an I2C OLED display asynchronously, ensuring that background sensor data collection or other STM32 tasks are never delayed by screen rendering. 

## Usage

Include the general and Wi-Fi headers in your project. Call the initialization sequence before your main loop, and place the run function inside the `while(1)` loop.

```c
#include "general.h"
#include "wifi.h"

int main(void) {
    // STM32 HAL Initialization & System Clock Config...
    
    // Pass your configured I2C handle to initialize the OLED & Network
    General_Init(&hi2c1);

    while (1) {
        // Runs the non-blocking OLED status dashboard (updates every 1000ms)
        General_Run();
        
        // Add your sensor polling or application logic here
    }
}
