/*
 * BMP280 Barometric Pressure Sensor
 * 
 * Purpose: Read atmospheric pressure and temperature
 * I2C digital sensor (very accurate!)
 * 
 * Hardware:
 * - ESP32
 * - BMP280 sensor (arrives Jan 14)
 * 
 * Connections (I2C):
 * BMP280 → ESP32
 * VCC → 3.3V
 * GND → GND
 * SDA → GPIO 21
 * SCL → GPIO 22
 * 
 * I2C Address: 0x76 or 0x77
 * (Run I2C scanner first to find it)
 * 
 * Author: Samrat Sharma
 * Date: January 2026
 */

#include <Wire.h>
#include <Adafruit_BMP280.h>

Adafruit_BMP280 bmp;  // I2C interface

// Timing
unsigned long previousMillis = 0;
const long interval = 1000;

// Statistics
float minPressure = 9999, maxPressure = 0;
float minTemp = 999, maxTemp = -999;
int readingCount = 0;

// Altitude calculation
const float SEA_LEVEL_PRESSURE = 1013.25;  // hPa

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n╔════════════════════════════════════════╗");
  Serial.println("║      BMP280 Pressure Sensor            ║");
  Serial.println("╠════════════════════════════════════════╣");
  Serial.println("║ Protocol: I2C                          ║");
  Serial.println("║ Address: 0x76 or 0x77                  ║");
  Serial.println("╚════════════════════════════════════════╝\n");
  
  // Initialize I2C
  Wire.begin();
  
  Serial.print("Initializing BMP280... ");
  
  // Try address 0x76 first
  if (!bmp.begin(0x76)) {
    Serial.println("Failed at 0x76, trying 0x77...");
    
    // Try address 0x77
    if (!bmp.begin(0x77)) {
      Serial.println("❌ BMP280 not found!");
      Serial.println("\nTroubleshooting:");
      Serial.println("1. Run I2C scanner to find address");
      Serial.println("2. Check wiring (SDA=21, SCL=22)");
      Serial.println("3. Verify 3.3V power");
      Serial.println("4. Check pull-up resistors (4.7kΩ)");
      while (1) delay(10);
    }
  }
  
  Serial.println("✓ Success!");
  
  // Configure sensor
  bmp.setSampling(
    Adafruit_BMP280::MODE_NORMAL,     // Continuous measurement
    Adafruit_BMP280::SAMPLING_X2,     // Temp oversampling x2
    Adafruit_BMP280::SAMPLING_X16,    // Pressure oversampling x16
    Adafruit_BMP280::FILTER_X16,      // IIR filter x16
    Adafruit_BMP280::STANDBY_MS_500   // 0.5s standby
  );
  
  Serial.println("\nBMP280 Configuration:");
  Serial.println("  Mode: Normal (continuous)");
  Serial.println("  Pressure: 16x oversampling");
  Serial.println("  Filter: 16x IIR");
  Serial.println();
  
  Serial.println("Time\t\tTemp(°C)\tPressure(hPa)\tAltitude(m)");
  Serial.println("───────────────────────────────────────────────────────");
}

void loop() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    
    readAndDisplay();
  }
}

void readAndDisplay() {
  readingCount++;
  
  // Read sensor
  float temperature = bmp.readTemperature();
  float pressure = bmp.readPressure() / 100.0;  // Convert Pa to hPa
  float altitude = bmp.readAltitude(SEA_LEVEL_PRESSURE);
  
  // Update statistics
  if (pressure < minPressure) minPressure = pressure;
  if (pressure > maxPressure) maxPressure = pressure;
  if (temperature < minTemp) minTemp = temperature;
  if (temperature > maxTemp) maxTemp = temperature;
  
  // Display
  Serial.print(millis() / 1000);
  Serial.print("\t\t");
  Serial.print(temperature, 2);
  Serial.print("\t\t");
  Serial.print(pressure, 2);
  Serial.print("\t\t");
  Serial.print(altitude, 1);
  
  // Weather indication
  if (pressure < 1000) {
    Serial.println("\t☁️ Low (storm?)");
  } else if (pressure > 1020) {
    Serial.println("\t☀️ High (fair)");
  } else {
    Serial.println("\t✓ Normal");
  }
  
  // Periodic summary
  if (readingCount % 60 == 0) {
    printSummary();
  }
}

void printSummary() {
  Serial.println("\n┌─────────────────────────────────────┐");
  Serial.println("│       PRESSURE STATISTICS           │");
  Serial.println("├─────────────────────────────────────┤");
  Serial.print("│ Readings: ");
  Serial.print(readingCount);
  Serial.println("                        │");
  Serial.print("│ Pressure: ");
  Serial.print(minPressure, 1);
  Serial.print(" - ");
  Serial.print(maxPressure, 1);
  Serial.println(" hPa  │");
  Serial.print("│ Temp: ");
  Serial.print(minTemp, 1);
  Serial.print(" - ");
  Serial.print(maxTemp, 1);
  Serial.println(" °C         │");
  
  // Pressure trend
  float currentPressure = bmp.readPressure() / 100.0;
  float trend = currentPressure - ((minPressure + maxPressure) / 2);
  Serial.print("│ Trend: ");
  if (trend > 0.5) {
    Serial.println("Rising ↗               │");
  } else if (trend < -0.5) {
    Serial.println("Falling ↘              │");
  } else {
    Serial.println("Stable →               │");
  }
  
  Serial.println("└─────────────────────────────────────┘\n");
}

/*
 * BMP280 SENSOR DETAILS:
 * 
 * TECHNOLOGY:
 * - Piezo-resistive pressure sensor
 * - Measures absolute atmospheric pressure
 * - Temperature sensor included
 * - 24-bit ADC (high precision!)
 * 
 * SPECIFICATIONS:
 * Pressure:
 * - Range: 300-1100 hPa
 * - Accuracy: ±1 hPa (±8m altitude)
 * - Resolution: 0.01 hPa (0.08m)
 * 
 * Temperature:
 * - Range: -40 to +85°C
 * - Accuracy: ±1°C
 * - Resolution: 0.01°C
 * 
 * I2C COMMUNICATION:
 * - Address: 0x76 (SDO to GND)
 * - Address: 0x77 (SDO to VCC)
 * - Speed: Up to 3.4 MHz
 * - 3.3V logic compatible
 * 
 * PRESSURE UNITS:
 * 1 hPa = 1 millibar = 100 Pascals
 * Standard atmosphere = 1013.25 hPa
 * 
 * ALTITUDE CALCULATION:
 * 
 * Altitude (m) = 44330 * (1 - (P/P0)^(1/5.255))
 * Where:
 * - P = measured pressure
 * - P0 = sea level pressure (1013.25 hPa)
 * 
 * For accurate altitude:
 * - Get local sea level pressure
 * - From weather station or airport
 * - Update SEA_LEVEL_PRESSURE constant
 * 
 * PRESSURE TRENDS:
 * 
 * Rising (>2 hPa/3hr):
 * → Weather improving
 * → High pressure system
 * 
 * Falling (>2 hPa/3hr):
 * → Weather deteriorating
 * → Low pressure/storm
 * 
 * Stable (±1 hPa/3hr):
 * → Weather steady
 * 
 * INDUSTRIAL APPLICATIONS:
 * 
 * ALTITUDE MONITORING:
 * - Drone/UAV systems
 * - Building floor detection
 * - Elevator position
 * 
 * WEATHER STATION:
 * - Local weather prediction
 * - Agricultural monitoring
 * - Outdoor activities safety
 * 
 * HVAC SYSTEMS:
 * - Building pressure monitoring
 * - Ventilation control
 * - Cleanroom pressure differential
 * 
 * LEAK DETECTION:
 * - Sealed enclosure monitoring
 * - Detect pressure drops
 * - Early warning system
 * 
 * CALIBRATION:
 * 
 * Temperature compensation:
 * - BMP280 handles this internally
 * - No user calibration needed
 * 
 * Pressure calibration:
 * - Compare to reference barometer
 * - Apply offset if needed:
 *   correctedP = pressure + offset;
 * 
 * COMBINING SENSORS:
 * 
 * BMP280 + DHT22:
 * - Absolute vs relative humidity
 * - Calculate dew point
 * - Better weather prediction
 * 
 * Formula for dew point:
 * Td = T - ((100 - RH) / 5)
 * Where T = temperature, RH = rel. humidity
 * 
 * TYNATECH APPLICATION:
 * 
 * Multi-sensor node:
 * - BMP280: Pressure & temp
 * - DHT22: Humidity & temp
 * - ACS712: Power consumption
 * 
 * Send via LoRaWAN:
 * - All data in one packet
 * - Efficient remote monitoring
 * - Weather + energy tracking
 * 
 * TROUBLESHOOTING:
 * 
 * Sensor not found:
 * → Run I2C scanner
 * → Check SDO pin (determines address)
 * → Verify power and ground
 * → Check I2C pull-ups
 * 
 * Incorrect readings:
 * → Allow warm-up (30 seconds)
 * → Check altitude setting
 * → Verify sensor not damaged
 * 
 * Noisy readings:
 * → Enable IIR filter (done in code)
 * → Increase oversampling
 * → Shield from air currents
 */
