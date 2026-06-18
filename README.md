# smart_stethoscope

Smart-Stethoscope-IoT: An economical IoT healthcare device using NodeMCU (ESP8266) & an HW484 microphone. It records 20-second heartbeat acoustic samples and streams them to a ThingSpeak server via Wi-Fi. This telemedicine project bridges the gap in rural healthcare by allowing doctors to remotely retrieve, amplify, and analyze cardiac data.

# Smart Stethoscope

## Hardware Requirements

* NodeMCU (ESP8266) board

* HW-484 microphone module

* Stethoscope chest piece

* Breadboard and jumper wires

* LED bulb

* Micro-USB cable

## Steps to Run

1. **Assemble the Hardware:**

   * Fix the HW-484 microphone onto the stethoscope chest piece.

   * Connect the microphone's analog output to pin `A0` on the NodeMCU.

   * Wire up the LED to the NodeMCU (the code uses `LED_BUILTIN` by default).

2. **Prepare the IDE:**

   * Open `heartbeat_read.cpp` in the Arduino IDE.

   * Ensure you have the ESP8266 boards installed in your Boards Manager.

   * Install the `ThingSpeak` library (by MathWorks) from the Library Manager.

3. **Configure Credentials:**

   * Set up a free channel on ThingSpeak with 4 fields (BPM, Beats, Average reading, Peak reading).

   * Update the code with your specific Wi-Fi and ThingSpeak details:

     ```cpp

     const char* ssid = "YOUR_WIFI_SSID";

     const char* password = "YOUR_WIFI_PASSWORD";

     unsigned long channelID = 1234567;

     const char* writeKey = "YOUR_WRITE_API_KEY";

     ```

4. **Flash & Run:**

   * Upload the code to the NodeMCU.

   * Open the Serial Monitor and set the baud rate to `115200`.

   * Wait for the "WiFi Connected Successfully!" message.

   * When the LED turns off and the monitor says "Recording 20 sec", hold the stethoscope firmly to the chest.

   * After 20 seconds, the results will print to the serial monitor and automatically push to your ThingSpeak dashboard.