/*
 * DHT22 Temperature & Humidity Sensor
 * 
 * Purpose: Read DHT22 digital sensor
 * One of your main sensors!
 * 
 * Hardware:
 * - ESP32
 * - DHT22 sensor (arrives Jan 13)
 * 
 * Connections:
 * DHT22 → ESP32
 * VCC → 3.3V
 * DATA → GPIO 15
 * GND → GND
 * 
 * Note: 10kΩ pull-up resistor between DATA and VCC
 * (Usually built into DHT22 modules)
 * 
 * Author: Samrat Sharma
 * Date: January 2026
 */

#include <DHT.h>

// DHT22 configuration
#define DHTPIN 15
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

// Timing
unsigned long previousMillis = 0;
const long interval = 2000;  // Read every 2 seconds (DHT22 limit)

// Statistics
float minTemp = 999, maxTemp = -999;
float minHum = 999, maxHum = -999;
int readingCount = 0;
int errorCount = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n╔════════════════════════════════════════╗");
  Serial.println("║    DHT22 Temperature & Humidity        ║");
  Serial.println("╠════════════════════════════════════════╣");
  Serial.println("║ Sensor: DHT22 (AM2302)                 ║");
  Serial.println("║ Pin: GPIO 15                           ║");
  Serial.println("╚════════════════════════════════════════╝\n");
  
  // Initialize DHT22
  dht.begin();
  
  Serial.println("DHT22 Specifications:");
  Serial.println("  Temperature: -40 to 80°C (±0.5°C)");
  Serial.println("  Humidity: 0-100% RH (±2%)");
  Serial.println("  Sample rate: Max 0.5 Hz (every 2 sec)");
  Serial.println();
  
  Serial.println("Time\t\tTemp(°C)\tHumidity(%)\tStatus");
  Serial.println("──────────────────────────────────────────────────────");
}

void loop() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    
    readAndDisplayDHT22();
  }
}

void readAndDisplayDHT22() {
  readingCount++;
  
  // Read sensor
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  
  // Check if reading failed
  if (isnan(humidity) || isnan(temperature)) {
    Serial.print(millis() / 1000);
    Serial.println("\t\tError\t\tError\t\t❌ Read failed");
    errorCount++;
    return;
  }
  
  // Update statistics
  if (temperature < minTemp) minTemp = temperature;
  if (temperature > maxTemp) maxTemp = temperature;
  if (humidity < minHum) minHum = humidity;
  if (humidity > maxHum) maxHum = humidity;
  
  // Display reading
  Serial.print(millis() / 1000);
  Serial.print("\t\t");
  Serial.print(temperature, 1);
  Serial.print("\t\t");
  Serial.print(humidity, 1);
  Serial.print("\t\t");
  
  // Status indicators
  if (temperature > 30) {
    Serial.println("🔥 Hot");
  } else if (temperature < 15) {
    Serial.println("❄️ Cold");
  } else if (humidity > 70) {
    Serial.println("💧 Humid");
  } else if (humidity < 30) {
    Serial.println("🏜️ Dry");
  } else {
    Serial.println("✓ Normal");
  }
  
  // Periodic summary
  if (readingCount % 30 == 0) {
    printSummary();
  }
}

void printSummary() {
  Serial.println("\n┌─────────────────────────────────────┐");
  Serial.println("│         SENSOR STATISTICS           │");
  Serial.println("├─────────────────────────────────────┤");
  Serial.print("│ Readings: ");
  Serial.print(readingCount);
  Serial.print(" (");
  Serial.print(errorCount);
  Serial.println(" errors)         │");
  Serial.print("│ Temp Range: ");
  Serial.print(minTemp, 1);
  Serial.print(" to ");
  Serial.print(maxTemp, 1);
  Serial.println(" °C     │");
  Serial.print("│ Humidity Range: ");
  Serial.print(minHum, 1);
  Serial.print(" to ");
  Serial.print(maxHum, 1);
  Serial.println(" %  │");
  Serial.println("└─────────────────────────────────────┘\n");
}

/*
 * DHT22 SENSOR DETAILS:
 * 
 * TECHNOLOGY:
 * - Capacitive humidity sensor
 * - NTC thermistor for temperature
 * - 8-bit microcontroller inside
 * - Custom 1-wire protocol
 * 
 * ADVANTAGES:
 * + Low cost (~$5)
 * + Good accuracy
 * + Digital output (no ADC needed)
 * + Single wire communication
 * + Easy to use
 * 
 * LIMITATIONS:
 * - Slow: Max 0.5 Hz (2 second minimum)
 * - Not for rapid changes
 * - Sensitive to power quality
 * - Requires pull-up resistor
 * 
 * COMMUNICATION PROTOCOL:
 * 
 * 1. ESP32 pulls line LOW for 1-10ms (start signal)
 * 2. ESP32 releases line (pull-up pulls HIGH)
 * 3. DHT22 responds with 80µs LOW, then 80µs HIGH
 * 4. DHT22 sends 40 bits of data:
 *    - 16 bits humidity
 *    - 16 bits temperature
 *    - 8 bits checksum
 * 5. Each bit encoded as timing:
 *    - 0: 26-28µs HIGH
 *    - 1: 70µs HIGH
 * 
 * DATA FORMAT:
 * 
 * Example reading: 0x02 0x58 0x01 0x0F 0x6A
 * 
 * Humidity: 0x0258 = 600 → 60.0%
 * Temperature: 0x010F = 271 → 27.1°C
 * Checksum: 0x6A = (0x02+0x58+0x01+0x0F) & 0xFF
 * 
 * WIRING TIPS:
 * 
 * Power:
 * - Use 3.3V (not 5V) for ESP32
 * - Stable power supply critical
 * - Add 100nF capacitor near sensor
 * 
 * Pull-up resistor:
 * - 10kΩ between DATA and VCC
 * - Usually included on DHT22 modules
 * - Check if your module has it
 * 
 * Cable length:
 * - Keep under 20 meters
 * - Use shielded cable if longer
 * 
 * TROUBLESHOOTING:
 * 
 * "Read failed" errors:
 * → Check wiring
 * → Verify 3.3V power
 * → Add/check pull-up resistor
 * → Try different GPIO pin
 * → Check sensor not faulty
 * 
 * Incorrect readings:
 * → Calibrate (compare to reference)
 * → Check for condensation
 * → Allow warm-up time (30 seconds)
 * → Avoid heat sources nearby
 * 
 * Slow response:
 * → This is normal! DHT22 is slow
 * → 2 second minimum between reads
 * → Use different sensor for fast reads
 * 
 * INDUSTRIAL APPLICATIONS:
 * 
 * HVAC CONTROL:
 * - Monitor room conditions
 * - Control heating/cooling
 * - Maintain comfort levels
 * 
 * AGRICULTURE:
 * - Greenhouse monitoring
 * - Optimal growing conditions
 * - Irrigation control
 * 
 * DATA CENTER:
 * - Server room monitoring
 * - Prevent overheating
 * - Track humidity for electronics
 * 
 * FOOD STORAGE:
 * - Cold storage monitoring
 * - Compliance tracking
 * - Alert on out-of-range
 * 
 * COMPANY'S APPLICATION:
 * 
 * Combine with other sensors:
 * - DHT22: Temperature & humidity
 * - ACS712: Power consumption
 * - BMP280: Atmospheric pressure
 * 
 * Send via LoRaWAN:
 * - All sensor data in one packet
 * - Energy-efficient monitoring
 * - Remote facility monitoring
 * 
 * CALIBRATION:
 * 
 * Temperature offset (if needed):
 * float correctedTemp = temperature - 0.5;
 * 
 * Humidity offset:
 * float correctedHum = humidity + 2.0;
 * 
 * Compare with reference thermometer
 * and adjust offsets accordingly
 * 
 * NEXT STEPS:
 * 
 * Day 5: Connect actual DHT22
 * Day 6: Integrate with Modbus
 * Day 7: Send data via LoRaWAN
 */
