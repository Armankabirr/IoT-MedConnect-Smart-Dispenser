# IoT-MedConnect: Smart Medicine Dispenser

An IoT-based physical access control system designed to improve medication adherence for elderly and chronic patients. Built with Arduino UNO and ESP32.

## 🚀 Key Features
* **Physical Locking:** Solenoid valve prevents access to medication outside of scheduled hours.
* **Automated Scheduling:** 3-tier rotation (Morning/Afternoon/Night) using a Servo motor and DS3231 RTC.
* **Intake Verification:** IR sensors detect pill removal to confirm medication was actually taken.
* **Real-time Dashboard:** Updates a web interface via ESP32.
* **Automated Alert Calling:** Initiates a real-time call to caregivers if a dose is missed.

## 🛠️ Hardware Requirements
* **Microcontrollers:** Arduino UNO, ESP32
* **Sensors:** DS3231 RTC, IR Sensor Module
* **Actuators:** SG90 Servo Motor, 12V Solenoid Lock Valve
* **Display:** 0.96" OLED (I2C)

## 📂 Repository Structure
* `/Software`: Contains `.ino` files for Arduino and ESP32.
* `/Hardware`: Circuit diagrams and Schematics.
* `/Report`: Final IEEE format project report.

## 🔧 Setup
1. Clone the repository.
2. Install libraries: `RTClib`, `ESP32`, `Adafruit_SSD1306`.
3. Upload `Arduino_Logic.ino` to the UNO.
4. Upload `IoT_Cloud_Bridge.ino` to the ESP32 with your Wi-Fi credentials.

## 👥 Team - Group 6 (UIU)
* Arman Kabir (0112230478)
* Member 2 Name (ID)
* Member 3 Name (ID)
* Member 4 Name (ID)
* Member 5 Name (ID)
