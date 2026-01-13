/*
 * ACS712 Current Sensor - Hardware Integration
 * 
 * Purpose: Measure AC/DC current using ACS712
 * This is your energy monitoring sensor!
 * 
 * Hardware:
 * - ESP32
 * - ACS712-5A Current Sensor (arrives Jan 13)
 * 
 * Connections:
 * ACS712 → ESP32
 * VCC → 5V
 * GND → GND
 * OUT → Voltage divider → GPIO 34
 * 
 * IMPORTANT: Voltage Divider needed!
 * ACS712 output: 0-5V
 * ESP32 input: Max 3.3V
 * Use 2:1 divider (e.g., 10kΩ + 10kΩ)
 * 
 * Author: Samrat Sharma
 * Date: January 2026
 */

// Configuration
const int SENSOR_PIN = 34;
const float VREF = 3.3;
const int ADC_MAX = 4095;

// ACS712-5A specifications
const float ZERO_CURRENT_VOLTAGE = 2.5;  // At 0A
const float SENSITIVITY = 0.185;         // 185 mV/A

// Voltage divider ratio (if used)
const float DIVIDER_RATIO = 2.0;  // 2:1 divider

// Sampling
const int NUM_SAMPLES = 100;
const int SAMPLE_DELAY_US = 50;

// Timing
unsigned long previousMillis = 0;
const long interval = 1000;

// Calibration
float zeroCurrentReading = 2.5;  // Will be calibrated
bool calibrated = false;

// Statistics
float peakCurrent = 0.0;
float totalEnergy = 0.0;  // Watt-hours
unsigned long energyStartTime = 0;

void setup() {
  Serial.begin(115200);
  pinMode(SENSOR_PIN, INPUT);
  
  delay(1000);
  energyStartTime = millis();
  
  Serial.println("\n╔════════════════════════════════════════╗");
  Serial.println("║   ACS712 Current Sensor (Hardware)     ║");
  Serial.println("╠════════════════════════════════════════╣");
  Serial.println("║ Sensor: ACS712-5A Hall Effect         ║");
  Serial.println("║ Range: -5A to +5A                     ║");
  Serial.println("║ Pin: GPIO 34 (with voltage divider)   ║");
  Serial.println("╚════════════════════════════════════════╝\n");
  
  Serial.println("⚠ SAFETY WARNING:");
  Serial.println("  - Use voltage divider for ACS712 output");
  Serial.println("  - ACS712 outputs 0-5V, ESP32 max 3.3V");
  Serial.println("  - Disconnect load before calibration\n");
  
  Serial.println("Commands:");
  Serial.println("  CAL - Calibrate zero point (no load!)");
  Serial.println("  START - Start monitoring");
  Serial.println();
}

void loop() {
  // Handle commands
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    cmd.toUpperCase();
    
    if (cmd == "CAL") {
      calibrateZeroPoint();
    }
    else if (cmd == "START") {
      if (!calibrated) {
        Serial.println("⚠ Please calibrate first (CAL command)");
      }
      else {
        Serial.println("Starting continuous monitoring...\n");
        Serial.println("Time(s)\tVoltage(V)\tCurrent(A)\tPower(W)\tEnergy(Wh)");
        Serial.println("─────────────────────────────────────────────────────────────");
      }
    }
  }
  
  // Continuous monitoring (if calibrated)
  if (calibrated) {
    unsigned long currentMillis = millis();
    
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      measureAndDisplay();
    }
  }
}

void calibrateZeroPoint() {
  Serial.println("\n═══ ZERO POINT CALIBRATION ═══");
  Serial.println("Disconnect all loads from ACS712");
  Serial.println("Press ENTER when ready...");
  
  while (!Serial.available());
  Serial.readStringUntil('\n');
  
  Serial.println("Calibrating (30 samples)...");
  
  float sum = 0;
  for (int i = 0; i < 30; i++) {
    float voltage = readAverageVoltage();
    sum += voltage;
    Serial.print(".");
    delay(100);
  }
  
  zeroCurrentReading = sum / 30.0;
  calibrated = true;
  
  Serial.println("\n✓ Calibration complete!");
  Serial.print("Zero point: ");
  Serial.print(zeroCurrentReading, 3);
  Serial.println(" V");
  Serial.println("You can now connect loads and start monitoring\n");
}

float readAverageVoltage() {
  long sum = 0;
  for (int i = 0; i < NUM_SAMPLES; i++) {
    sum += analogRead(SENSOR_PIN);
    delayMicroseconds(SAMPLE_DELAY_US);
  }
  
  int avgADC = sum / NUM_SAMPLES;
  float voltage = (avgADC / (float)ADC_MAX) * VREF;
  
  // Compensate for voltage divider
  voltage = voltage * DIVIDER_RATIO;
  
  return voltage;
}

void measureAndDisplay() {
  // Read voltage
  float voltage = readAverageVoltage();
  
  // Calculate current
  float current = (voltage - zeroCurrentReading) / SENSITIVITY;
  
  // Calculate power (assuming 230V AC)
  float assumedVoltage = 230.0;
  float power = assumedVoltage * abs(current);
  
  // Calculate energy (Wh)
  float hours = (millis() - energyStartTime) / 3600000.0;
  totalEnergy = power * hours;
  
  // Track peak
  if (abs(current) > abs(peakCurrent)) {
    peakCurrent = current;
  }
  
  // Display
  float timeSeconds = millis() / 1000.0;
  Serial.print(timeSeconds, 1);
  Serial.print("\t");
  Serial.print(voltage, 3);
  Serial.print("\t\t");
  Serial.print(current, 3);
  Serial.print("\t\t");
  Serial.print(power, 2);
  Serial.print("\t\t");
  Serial.print(totalEnergy, 3);
  
  // Alerts
  if (abs(current) > 4.5) {
    Serial.print("\t⚠️ HIGH!");
  }
  
  Serial.println();
  
  // Summary every 60 seconds
  if ((int)timeSeconds % 60 == 0 && timeSeconds > 1) {
    printSummary();
  }
}

void printSummary() {
  Serial.println("\n┌──────────────────────────────────────┐");
  Serial.println("│     ENERGY MONITORING SUMMARY        │");
  Serial.println("├──────────────────────────────────────┤");
  Serial.print("│ Peak Current: ");
  Serial.print(abs(peakCurrent), 3);
  Serial.println(" A              │");
  Serial.print("│ Total Energy: ");
  Serial.print(totalEnergy, 3);
  Serial.println(" Wh           │");
  Serial.print("│ Est. Cost: $");
  Serial.print(totalEnergy * 0.15, 2);  // $0.15/kWh
  Serial.println("              │");
  Serial.println("└──────────────────────────────────────┘\n");
}

/*
 * VOLTAGE DIVIDER CALCULATION:
 * 
 * ACS712 output range: 0-5V
 * ESP32 safe input: 0-3.3V
 * 
 * Need to reduce by factor: 5V / 3.3V = 1.52
 * Use 2:1 divider for safety margin
 * 
 * Circuit:
 * ACS712 OUT ──┬─── 10kΩ ───┬─── ESP32 GPIO 34
 *              │            │
 *              └─── 10kΩ ───┴─── GND
 * 
 * Output voltage = Input / 2
 * 
 * In code, multiply reading by 2 to get actual voltage
 * 
 * ALTERNATIVE (if no divider):
 * Use ACS712-20A version (100 mV/A sensitivity)
 * Output range: 0-3.5V (closer to ESP32 limit)
 * But less sensitive for small currents
 * 
 * AC CURRENT MEASUREMENT:
 * 
 * For AC, ACS712 outputs 2.5V ± swing
 * Example 1A AC:
 * - Peak positive: 2.5V + 0.185V = 2.685V
 * - Peak negative: 2.5V - 0.185V = 2.315V
 * 
 * To measure AC RMS:
 * 1. Sample at high rate (> 1kHz)
 * 2. Calculate RMS over one cycle
 * 3. RMS = sqrt(mean(samples²))
 * 
 * DC MEASUREMENT (simpler):
 * Just read and average as shown in code
 * 
 * LOAD TESTING:
 * 
 * Test with known loads:
 * - 60W bulb: ~0.26A at 230V
 * - 100W bulb: ~0.43A
 * - Small heater: 1-2A
 * - Verify readings match expectations
 * 
 * INDUSTRIAL APPLICATIONS:
 * 
 * PREDICTIVE MAINTENANCE:
 * - Baseline normal current
 * - Detect gradual increases
 * - Indicates bearing wear, misalignment
 * - Schedule maintenance before failure
 * 
 * ENERGY MANAGEMENT:
 * - Track per-machine consumption
 * - Identify inefficient equipment
 * - Optimize production schedules
 * - Reduce electricity costs
 * 
 * SAFETY MONITORING:
 * - Detect overcurrent
 * - Ground fault detection
 * - Arc fault warning
 * - Automatic shutdown
 * 
 * COMPANY'S INTEGRATION:
 * 
 * Day 6: Add to Modbus registers
 * Day 7: Send via LoRaWAN
 * Day 8: Create energy dashboard
 * 
 * Combine with:
 * - Voltage sensor (optional)
 * - Power factor meter
 * - Temperature (DHT22)
 * → Complete power monitoring system
 */
