/*
 * Multi-Sensor IoT Dashboard
 * 
 * Purpose: Read all sensors and display on serial dashboard
 * Integration of: DHT22, ACS712, BMP280, HC-SR04, SW-420
 * 
 * Hardware: ESP32 + All sensors (Day 7)
 * 
 * Author: Samrat Sharma
 * Date: January 2026
 */

#include <DHT.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>

// DHT22
#define DHTPIN 15
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// ACS712
#define CURRENT_PIN 34
const float ZERO_CURRENT = 2.5;
const float SENSITIVITY = 0.185;

// BMP280
Adafruit_BMP280 bmp;

// HC-SR04
#define TRIG_PIN 5
#define ECHO_PIN 18

// SW-420
#define VIBRATION_PIN 13

// Timing
unsigned long previousMillis = 0;
const long interval = 2000;

// Data structure
struct SensorReadings {
  float temperature_dht;
  float humidity;
  float temperature_bmp;
  float pressure;
  float current;
  float power;
  float distance;
  bool vibration;
  bool valid;
} readings;

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  // Initialize sensors
  Serial.println("\n╔════════════════════════════════════════╗");
  Serial.println("║   Multi-Sensor IoT Dashboard           ║");
  Serial.println("╚════════════════════════════════════════╝\n");
  
  Serial.println("Initializing sensors...");
  
  // DHT22
  dht.begin();
  Serial.println("✓ DHT22 initialized");
  
  // BMP280
  Wire.begin();
  if (bmp.begin(0x76) || bmp.begin(0x77)) {
    Serial.println("✓ BMP280 initialized");
  } else {
    Serial.println("✗ BMP280 not found");
  }
  
  // HC-SR04
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  Serial.println("✓ HC-SR04 initialized");
  
  // SW-420
  pinMode(VIBRATION_PIN, INPUT);
  Serial.println("✓ SW-420 initialized");
  
  // ACS712
  pinMode(CURRENT_PIN, INPUT);
  Serial.println("✓ ACS712 initialized");
  
  Serial.println("\nStarting continuous monitoring...\n");
  delay(1000);
}

void loop() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    
    readAllSensors();
    displayDashboard();
  }
}

void readAllSensors() {
  // DHT22
  readings.temperature_dht = dht.readTemperature();
  readings.humidity = dht.readHumidity();
  
  // BMP280
  readings.temperature_bmp = bmp.readTemperature();
  readings.pressure = bmp.readPressure() / 100.0;
  
  // ACS712
  int sum = 0;
  for (int i = 0; i < 50; i++) {
    sum += analogRead(CURRENT_PIN);
    delayMicroseconds(100);
  }
  float voltage = (sum / 50.0) * (3.3 / 4095.0) * 2.0;  // With divider
  readings.current = (voltage - ZERO_CURRENT) / SENSITIVITY;
  readings.power = abs(readings.current) * 230.0;
  
  // HC-SR04
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  readings.distance = (duration / 2.0) * 0.0343;
  
  // SW-420
  readings.vibration = (digitalRead(VIBRATION_PIN) == LOW);
  
  // Validation
  readings.valid = !isnan(readings.temperature_dht) && 
                   !isnan(readings.humidity);
}

void displayDashboard() {
  // Clear screen (optional)
  Serial.println("\n\n\n");
  
  // Header
  Serial.println("╔══════════════════════════════════════════════════╗");
  Serial.println("║          IoT SENSOR DASHBOARD                    ║");
  Serial.println("╠══════════════════════════════════════════════════╣");
  
  // Timestamp
  Serial.print("║ Time: ");
  Serial.print(millis() / 1000);
  Serial.println(" seconds                                ║");
  
  Serial.println("╠══════════════════════════════════════════════════╣");
  
  // Environmental sensors
  Serial.println("║ 🌡️  ENVIRONMENTAL SENSORS                        ║");
  Serial.println("╟──────────────────────────────────────────────────╢");
  
  Serial.print("║   Temperature (DHT22): ");
  if (!isnan(readings.temperature_dht)) {
    Serial.print(readings.temperature_dht, 1);
    Serial.println(" °C               ║");
  } else {
    Serial.println("N/A                    ║");
  }
  
  Serial.print("║   Humidity: ");
  if (!isnan(readings.humidity)) {
    Serial.print(readings.humidity, 1);
    Serial.println(" %                            ║");
  } else {
    Serial.println("N/A                                ║");
  }
  
  Serial.print("║   Temperature (BMP280): ");
  Serial.print(readings.temperature_bmp, 1);
  Serial.println(" °C               ║");
  
  Serial.print("║   Pressure: ");
  Serial.print(readings.pressure, 1);
  Serial.println(" hPa                       ║");
  
  // Energy monitoring
  Serial.println("╠══════════════════════════════════════════════════╣");
  Serial.println("║ ⚡ ENERGY MONITORING                              ║");
  Serial.println("╟──────────────────────────────────────────────────╢");
  
  Serial.print("║   Current: ");
  Serial.print(readings.current, 2);
  Serial.println(" A                             ║");
  
  Serial.print("║   Power: ");
  Serial.print(readings.power, 1);
  Serial.println(" W                              ║");
  
  // Distance measurement
  Serial.println("╠══════════════════════════════════════════════════╣");
  Serial.println("║ 📏 DISTANCE MEASUREMENT                          ║");
  Serial.println("╟──────────────────────────────────────────────────╢");
  
  Serial.print("║   Distance: ");
  if (readings.distance > 2 && readings.distance < 400) {
    Serial.print(readings.distance, 1);
    Serial.println(" cm                          ║");
  } else {
    Serial.println("Out of range                       ║");
  }
  
  // Vibration detection
  Serial.println("╠══════════════════════════════════════════════════╣");
  Serial.println("║ 📳 VIBRATION MONITORING                          ║");
  Serial.println("╟──────────────────────────────────────────────────╢");
  
  Serial.print("║   Status: ");
  if (readings.vibration) {
    Serial.println("⚠️  VIBRATION DETECTED!                ║");
  } else {
    Serial.println("✓ Normal                              ║");
  }
  
  // Alerts
  Serial.println("╠══════════════════════════════════════════════════╣");
  Serial.println("║ 🚨 ALERTS                                        ║");
  Serial.println("╟──────────────────────────────────────────────────╢");
  
  bool hasAlert = false;
  
  if (readings.temperature_dht > 30) {
    Serial.println("║   ⚠️  High temperature detected                  ║");
    hasAlert = true;
  }
  
  if (readings.humidity > 70) {
    Serial.println("║   ⚠️  High humidity detected                     ║");
    hasAlert = true;
  }
  
  if (abs(readings.current) > 4.0) {
    Serial.println("║   ⚠️  High current detected                      ║");
    hasAlert = true;
  }
  
  if (readings.vibration) {
    Serial.println("║   ⚠️  Abnormal vibration detected                ║");
    hasAlert = true;
  }
  
  if (!hasAlert) {
    Serial.println("║   ✓ All systems normal                           ║");
  }
  
  Serial.println("╚══════════════════════════════════════════════════╝");
}

/*
 * MULTI-SENSOR INTEGRATION:
 * 
 * This dashboard combines:
 * 1. DHT22: Temperature & Humidity
 * 2. BMP280: Pressure & Temperature
 * 3. ACS712: Current & Power
 * 4. HC-SR04: Distance
 * 5. SW-420: Vibration
 * 
 * REAL-TIME MONITORING:
 * Updates every 2 seconds
 * All sensor data in one view
 * Immediate alerts
 * 
 * NEXT STEPS (Day 8):
 * - Add data logging to SD card
 * - Send via LoRaWAN
 * - Create web dashboard
 * - Add OLED display
 * 
 * COMPANY'S APPLICATION:
 * This is a complete industrial monitoring node!
 */
