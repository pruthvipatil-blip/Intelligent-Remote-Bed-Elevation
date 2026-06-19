# Intelligent Remote Bed Elevation Control System (MediGlide-IoT)

An IoT-based smart medical assistant system designed to enable remote, safe, and smooth adjustment of a hospital bed's elevation angle (0–90°) using an ESP32 microcontroller, an automated servo motor, and an Adafruit IO cloud dashboard.

## 🚀 Features
* **Cloud-Controlled Incline:** Smooth adjustment via an online dashboard slider or quick-action command presets.
* **Safety Interpolation Logic:** Eliminates sudden jerky movements; the servo safely glides at a controlled maximum speed (1.5° per step) to protect patient comfort.
* **Clinical Preset Modes:** One-click modes for Sleeping (10°), Breathing Support (45°), and Emergency Alert (90°).
* **Hardware Alert System:** Local visual feedback via an I2C OLED display and an active status LED, alongside an audible buzzer that sounds continuously during Emergency Mode.

## 🛠️ Hardware Architecture (Simulated in Wokwi)
* **ESP32 DevKit V1** (Core Microcontroller with Wi-Fi)
* **Standard Servo Motor** (Bed elevation mechanism connected to GPIO 18)
* **SSD1306 OLED Display (128x64 I2C)** (Real-time local data monitoring on GPIO 21/22)
* **Active Buzzer** (Critical/Emergency indicator on GPIO 25)
* **LED** (System shifting activity status light on GPIO 26)

## ☁️ Cloud & Dashboard Configuration
The system connects via MQTT to **Adafruit IO**, utilizing a single feed: `bed-angle`. 
The control dashboard includes:
* **Slider Block:** Manual adjustment between 0° and 90°.
* **Instant Button Blocks:** Quick presets for Sleeping (10°), Breathing Support (45°), and Emergency (90°).
* **Gauge Block:** Visual confirmation of the target angle.

## 📁 Repository Structure
* `sketch.ino` - The complete ESP32 Arduino control firmware.
* `diagram.json` - The Wokwi virtual hardware circuit design layout.
* `README.md` - Project documentation and setup guide.

## 🌐 Live Simulation
You can run and test the live hardware simulation directly in your browser here: 
👉 [Launch Wokwi Simulation](https://wokwi.com/projects/466424690992855041)
