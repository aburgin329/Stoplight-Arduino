# ESP8266 Stoplight Controller

This project uses an ESP8266 module to simulate a stoplight control system. It creates a WiFi access point and hosts a web interface for both manual and automatic control of a stoplight (red, yellow, green LEDs).

## Overview

The code implements:
- A WiFi Access Point with SSID `ESP8266_Stoplight` and password `12345678`.
- A web server running on port 80 that provides a user-friendly interface to control the stoplight.
- Two modes of operation:
  - **Manual Mode:** Select a light (red, yellow, or green) directly.
  - **Automatic Mode:** Cycle through the lights with customizable intervals.
- API endpoints to query the current status, control the lights, and update the cycle intervals.

## Features

- **Access Point Mode:** The ESP8266 hosts its own WiFi network.
- **Web Interface:** A responsive page to control the stoplight.
- **Manual & Automatic Control:** Toggle between manually setting a light and auto cycling.
- **Customizable Cycle Intervals:** Set custom intervals for red, yellow, and green lights.
- **Hardware Status:** Displays the number of connected stations and system uptime.

## Hardware Requirements

- **ESP8266 Board:** Examples include NodeMCU, Wemos D1 Mini, etc.
- **LEDs:** Three LEDs (red, yellow, and green) with appropriate resistors.
- **Breadboard and Jumper Wires:** For prototyping.
- **Power Supply:** Suitable for the ESP8266 board.

## Software Requirements

- **Arduino IDE or PlatformIO:** For compiling and uploading the code.
- **ESP8266 Libraries:**
  - [ESP8266WiFi](https://github.com/esp8266/Arduino)
  - [ESP8266WebServer](https://github.com/esp8266/Arduino)

## Setup Instructions

1. **Wiring:**
   - Connect the red, yellow, and green LEDs to the GPIO pins defined in the code:
     - Red LED → Pin `D1`
     - Yellow LED → Pin `D2`
     - Green LED → Pin `D3`
   - Ensure each LED is connected with the appropriate resistor.

2. **Configure the Arduino IDE:**
   - Install the ESP8266 board package via the Board Manager.
   - Select the correct board (e.g., NodeMCU 1.0 or Wemos D1 Mini).

3. **Upload the Code:**
   - Copy the provided code into a new sketch.
   - Verify and upload the code to your ESP8266 board.

4. **Connect to the Access Point:**
   - After the upload, the ESP8266 creates a WiFi network with the SSID `ESP8266_Stoplight`.
   - Connect to this network using the password `12345678`.

5. **Access the Web Interface:**
   - Open a web browser and go to `http://192.168.4.1/` to access the control interface.
   - Use the provided buttons to turn the system on/off, switch between manual and automatic modes, or change the cycle intervals.

## Web API Endpoints

- **`/`**  
  Serves the main HTML page with the user interface.

- **`/get_status`**  
  Returns a JSON object with the current status (power state, mode, current light, connected stations, uptime, and auto intervals).

- **`/turn_on`** and **`/turn_off`**  
  Turn the system on or off.

- **`/manual_cycle?light=<color>`**  
  Sets the current light manually. Valid values for `<color>` are `red`, `yellow`, and `green`.

- **`/start_auto`** and **`/stop_auto`**  
  Start or stop automatic cycling of the stoplight.

- **`/set_cycle_intervals?red=<value>&yellow=<value>&green=<value>`**  
  Update the cycle intervals for the lights (values are in seconds, with a minimum of 1 second).

## Customization

- **Pin Assignments:**  
  Modify `RED_PIN`, `YELLOW_PIN`, and `GREEN_PIN` if you are using different GPIO pins.

- **Cycle Intervals:**  
  Adjust the default interval values (`autoIntervalRed`, `autoIntervalYellow`, `autoIntervalGreen`) in the code if needed.

- **Web Interface:**  
  The HTML/CSS/JavaScript can be customized within the `handleRoot` function to match your design requirements.

## Troubleshooting

- **Access Point Issues:**  
  Ensure that your ESP8266 board is properly powered and the correct board settings are selected in the Arduino IDE.

- **LED Control:**  
  Verify the wiring and that the correct GPIO pins are being used for each LED.

- **Web Interface Not Loading:**  
  Make sure your device is connected to the `ESP8266_Stoplight` WiFi network, and then navigate to `http://192.168.4.1/`.

## License

This project is provided "as is" without any warranty. Feel free to modify and distribute the code for personal or educational use.
