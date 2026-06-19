#include <WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <ESP32Servo.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ============================================================================
// CONFIGURATION & CREDENTIALS
// ============================================================================

// WiFi Configuration (Wokwi Default Virtual Network)
#define WLAN_SSID       "Wokwi-GUEST"
#define WLAN_PASS       ""

// Adafruit IO Configuration (Updated with your credentials)
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define IO_USERNAME     "raj12"
#define IO_KEY          "aio_aoWg38lvT63BnR7e1ahm5WBTH67U" 

// Hardware Pin Layout
#define SERVO_PIN       18
#define BUZZER_PIN      25
#define LED_PIN         26
#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT   64

// ============================================================================
// OBJECTS & GLOBALS
// ============================================================================

Servo bedServo;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Movement & Safety Logic Variables
float currentAngle = 10.0; // Start at Sleeping Mode (10 degrees)
float targetAngle = 10.0;
const float MAX_SPEED = 1.5; // Smooth transition scale (Max change per loop frame)
unsigned long lastUpdate = 0;

// Networking Instances
WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, IO_USERNAME, IO_KEY);
Adafruit_MQTT_Subscribe bedAngleFeed = Adafruit_MQTT_Subscribe(&mqtt, IO_USERNAME "/feeds/bed-angle");

// Function Prototypes
void MQTT_connect();
void updateOLED(String mode, int angle, String status);

// ============================================================================
// SYSTEM SETUP
// ============================================================================

void setup() {
  Serial.begin(115200);
  
  // Set Pin Modes
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_PIN, LOW);

  // Initialize SSD1306 I2C OLED Display (Address 0x3C)
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("OLED initialization failed!"));
    for(;;); // Lock here if hardware misconfiguration occurs
  }
  
  // Boot Screen Initial Splash
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 15);
  display.println("  SMART BED CONTROL");
  display.println("  Initializing Wifi...");
  display.display();

  // Initialize Servo Motor
  bedServo.attach(SERVO_PIN, 500, 2400); 
  bedServo.write((int)currentAngle);

  // Establish Wi-Fi Connection
  Serial.print("Connecting to Virtual Wi-Fi");
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi Network Connected!");

  // Subscribe to Adafruit IO Feed
  mqtt.subscribe(&bedAngleFeed);
  
  // Update screen status to show system is live
  updateOLED("Sleeping", (int)currentAngle, "Stable");
}

// ============================================================================
// MAIN EXECUTIVE LOOP
// ============================================================================

void loop() {
  // Maintain active MQTT cloud connection
  MQTT_connect();

  // 1. Process cloud updates from Adafruit IO
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5))) { // Non-blocking check (5ms timeout)
    if (subscription == &bedAngleFeed) {
      float receivedValue = String((char *)bedAngleFeed.lastread).toFloat();
      
      // Absolute Safety Constraints Protection (Bound data between 0 and 90)
      if(receivedValue < 0) receivedValue = 0;
      if(receivedValue > 90) receivedValue = 90;

      targetAngle = receivedValue;
      Serial.print("Cloud Stream Target Angle: ");
      Serial.println(targetAngle);
    }
  }

  // 2. Non-Blocking Smooth Linear Interpolation Execution (Runs every 30ms)
  if (millis() - lastUpdate > 30) {
    lastUpdate = millis();

    if (currentAngle != targetAngle) {
      // Actively transitioning state indicators
      digitalWrite(LED_PIN, HIGH); 

      float angleDifference = targetAngle - currentAngle;
      
      // Restrict quick acceleration transitions to safeguard user comfort
      if (angleDifference > MAX_SPEED) {
        currentAngle += MAX_SPEED;
      } else if (angleDifference < -MAX_SPEED) {
        currentAngle -= MAX_SPEED;
      } else {
        currentAngle = targetAngle; // Snaps to targeted spot if difference is fractional
      }

      bedServo.write((int)currentAngle);

      // Determine Clinical Position Categorization Label
      String currentMode = "Custom Layout";
      if ((int)targetAngle == 10) currentMode = "Sleeping Mode";
      else if ((int)targetAngle == 45) currentMode = "Breathing Supp";
      else if ((int)targetAngle == 90) currentMode = "EMERGENCY ALERT";

      // Emergency Mode Buzzer Routine
      if((int)targetAngle == 90) {
        digitalWrite(BUZZER_PIN, HIGH); // Alarm sounds constantly when 90° emergency is flagged
      } else {
        digitalWrite(BUZZER_PIN, LOW);
      }

      updateOLED(currentMode, (int)currentAngle, "Adjusting...");
      
    } else {
      // System achieves target angle configuration equilibrium
      digitalWrite(LED_PIN, LOW); 
      
      String finalMode = "Custom Layout";
      if ((int)currentAngle == 10) finalMode = "Sleeping Mode";
      else if ((int)currentAngle == 45) finalMode = "Breathing Supp";
      else if ((int)currentAngle == 90) finalMode = "EMERGENCY ALERT";

      // Safety drop catch for the buzzer
      if ((int)currentAngle != 90) {
         digitalWrite(BUZZER_PIN, LOW);
      }
      
      updateOLED(finalMode, (int)currentAngle, "Stable");
    }
  }
}

// ============================================================================
// SUBSYSTEM UTILITY FUNCTIONS
// ============================================================================

// local OLED Render Subroutine
void updateOLED(String mode, int angle, String status) {
  display.clearDisplay();
  
  // UI Panel Static Header Frame
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("SMART HOSPITAL BED");
  display.drawFastHLine(0, 10, 128, SSD1306_WHITE);
  
  // Display Current Operating Setup
  display.setCursor(0, 18);
  display.print("Config: ");
  display.print(mode);
  
  // Main Angle Display Output
  display.setCursor(0, 32);
  display.setTextSize(2);
  display.print(angle);
  display.write(247); // Generates standard degree (°) ASCII sign symbol
  display.print(" ELEV");
  
  // System Safety Loop Activity Metric
  display.setTextSize(1);
  display.setCursor(0, 55);
  display.print("Status: ");
  display.print(status);
  
  display.display();
}

// Active Broker Connectivity Watchdog Routine
void MQTT_connect() {
  if (mqtt.connected()) return;

  Serial.print("Connecting to Adafruit IO... ");
  uint8_t retries = 3;
  
  while (mqtt.connect() != 0) { 
       Serial.println("Connection Failed. Retrying Broker Link...");
       mqtt.disconnect();
       delay(5000); 
       retries--;
       if (retries == 0) {
         Serial.println("System Core Fault: Could not bind cloud assets. Resetting system.");
         while (1); // Triggers Wokwi crash watch halt
       }
  }
  Serial.println("Adafruit IO Connected Successfully!");
}
