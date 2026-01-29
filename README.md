# IoT-MedConnect: A Time-Controlled Smart Medicine Dispenser

An IoT-integrated medical adherence system designed to prevent medication misuse and ensure timely intake using physical access control and redundant emergency alerting.

## 📌 Project Overview
**IoT-MedConnect** is a hardware-software integrated solution developed for the **Microprocessors and Microcontrollers Laboratory (CSE 4326)** at United International University. The system physically locks medication and only grants access at scheduled times. It uses a dual-alert system: a real-time web dashboard for monitoring and a GSM-based voice call alert for emergencies.

## 🚀 Key Features
* **Physical Access Control:** Uses a Solenoid lock to prevent early or repeated medication intake.
* **3-Tier Organization:** Automatically rotates between Morning, Afternoon, and Night dosages using a Servo motor.
* **Intake Verification:** IR sensors confirm if the pill has actually been removed from the slot.
* **GSM Emergency Calling:** Utilizes the **SIM800L module** to place a direct voice call to the caregiver if a dose is missed.
* **IoT Dashboard:** Real-time data synchronization via **ESP32** for remote monitoring.

## 🛠️ Hardware Components
* **Microcontrollers:** Arduino UNO (System Logic), ESP32 (IoT Connectivity)
* **GSM Module:** SIM800L (Cellular Voice Alerts)
* **Sensors:** DS3231 RTC (Precision Timing), IR Sensor (Pill Detection)
* **Actuators:** SG90 Servo Motor (Rotation), 12V Solenoid Lock (Security)
* **Display:** 0.96" OLED I2C



## 💻 Software Environment
* **Language:** C++ (Arduino Sketch)
* **Platform:** Arduino IDE
* **Libraries Used:** * `RTClib.h` (For DS3231)
  * `Servo.h` (For motor control)
  * `SoftwareSerial.h` (For SIM800L AT Commands)
  * `WiFi.h` & `HTTPClient.h` (For ESP32 Dashboard)

## 📁 Repository Structure
* `/Software`: Arduino (.ino) source codes for UNO and ESP32.
* `/Hardware`: Circuit schematics and Fritzing diagrams.
* `/Documentation`: IEEE Format Project Report and Presentation.

## 👥 Group 06 - Team Members
* **Arman Kabir** (ID: 0112230478) - *Team Lead*
* **Member 2 Name** (ID: XXXXXXXX)
* **Member 3 Name** (ID: XXXXXXXX)
* **Member 4 Name** (ID: XXXXXXXX)
* **Member 5 Name** (ID: XXXXXXXX)

**Course Teacher:** Md. Shafqat Talukder Rakin  
**Institution:** United International University (UIU)

## 📺 Demonstration
[Link to YouTube Video]
