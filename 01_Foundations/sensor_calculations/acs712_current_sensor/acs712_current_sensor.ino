/*
 * ACS712 Current Sensor - Calculation and Simulation
 * 
 * SAVE TO: 01_Foundations/sensor_calculations/acs712_current_sensor.ino
 * 
 * Purpose: Learn current calculation from ACS712 Hall-Effect sensor
 * THIS IS YOUR ACTUAL SENSOR - use this code on Day 5!
 * 
 * Hardware: 
 * - ESP32 (Heltec WiFi LoRa 32 V3)
 * - ACS712-5A Current Sensor (arrives Jan 13)
 * - Analog pin: GPIO 34
 * 
 * ACS712-5A Specifications:
 * - Range: -5A to +5A (DC or AC)
 * - Zero current output: 2.5V (Vcc/2)
 * - Sensitivity: 185 mV/A
 * 
 * Output Voltage Formula:
 * Vout = 2.5V + (Current × Sensitivity)
 * 
 * Examples:
 * 0A   → 2.500V
 * +1A  → 2.685V (2.5 + 0.185)
 * -1A  → 2.315V (2.5 - 0.185)
 * +5A  → 3.425V (2.5 + 0.925)
 * 
 * Author: Samrat Sharma
 * Date: January 2026
 */

// ========== CONFIGURATION ==========
const int SENSOR_PIN = 34;
const float VREF = 3.3;              // ESP32 ADC reference
const int ADC_MAX = 4095;            // 12-bit maximum
const float ZERO_CURRENT_VOLTAGE = 2.5;  // Output at 0A
const float SENSITIVITY = 0.185;     // 185 mV/A for ACS712-5A

// Averaging for noise reduction
const int NUM_SAMPLES = 50;
const int SAMPLE_DELAY_US = 100;

// Timing
unsigned long previousMillis = 0;
const long interval = 500;

// Statistics tracking
float peakCurrent = 0.0;
float totalEnergy = 0.0;
unsigned long energyStartTime = 0;

void setup() {
  Serial.begin(115200);
  pinMode(SENSOR_PIN, INPUT);
  
  delay(1000);
  energyStartTime = millis();
  
  Serial.println("\n╔════════════════════════════════════════╗");
  Serial.println("║   ACS712 Current Sensor Monitor       ║");
  Serial.println("╠════════════════════════════════════════╣");
  Serial.println("║ Sensor: ACS712-5A Hall Effect         ║");
  Serial.println("║ Range: -5A to +5A                     ║");
  Serial.println("║ Zero Current: 2.5V                    ║");
  Serial.println("║ Sensitivity: 185mV/A                  ║");
  Serial.println("╚════════════════════════════════════════╝\n");
  
  Serial.println("Time(s)\tVoltage(V)\tCurrent(A)\tPower(W)\tStatus");
  Serial.println("───────────────────────────────────────────────────────────");
}

void loop() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    
    // ========== STEP 1: READ AND AVERAGE ADC ==========
    long sum = 0;
    for (int i = 0; i < NUM_SAMPLES; i++) {
      sum += analogRead(SENSOR_PIN);
      delayMicroseconds(SAMPLE_DELAY_US);
    }
    int avgADC = sum / NUM_SAMPLES;
    
    // ========== STEP 2: CONVERT ADC TO VOLTAGE ==========
    float voltage = (avgADC / (float)ADC_MAX) * VREF;
    
    // ========== STEP 3: CALCULATE CURRENT ==========
    // Formula: Current = (Vout - VzeroPoint) / Sensitivity
    float current = (voltage - ZERO_CURRENT_VOLTAGE) / SENSITIVITY;
    
    // ========== STEP 4: CALCULATE POWER ==========
    // Assuming 230V AC mains
    float assumedVoltage = 230.0;
    float power = assumedVoltage * abs(current);
    
    // ========== STEP 5: TRACK STATISTICS ==========
    if (abs(current) > abs(peakCurrent)) {
      peakCurrent = current;
    }
    
    // Energy = Power × Time
    float elapsedSeconds = (currentMillis - energyStartTime) / 1000.0;
    totalEnergy = power * elapsedSeconds;
    
    // ========== STEP 6: DISPLAY RESULTS ==========
    float timeSeconds = currentMillis / 1000.0;
    
    Serial.print(timeSeconds, 1);
    Serial.print("\t");
    Serial.print(voltage, 3);
    Serial.print("\t\t");
    Serial.print(current, 3);
    Serial.print("\t\t");
    Serial.print(power, 2);
    Serial.print("\t\t");
    
    // Status indication
    if (abs(current) < 0.1) {
      Serial.println("✓ Normal");
    } else if (abs(current) < 4.0) {
      Serial.println("⚠ Active");
    } else {
      Serial.println("⚠️ HIGH!");
    }
    
    // ========== ALERTS ==========
    if (abs(current) > 4.5) {
      Serial.println("🚨 ALERT: Current near maximum rating!");
    }
    
    // Periodic summary every 10 seconds
    if ((int)timeSeconds % 10 == 0 && timeSeconds > 1) {
      printSummary(timeSeconds);
    }
  }
}

// ========== SUMMARY REPORT ==========
void printSummary(float runtime) {
  Serial.println("\n┌─────────────────────────────────────┐");
  Serial.println("│        MONITORING SUMMARY           │");
  Serial.println("├─────────────────────────────────────┤");
  Serial.print("│ Runtime: ");
  Serial.print(runtime, 0);
  Serial.println(" seconds              │");
  Serial.print("│ Peak Current: ");
  Serial.print(abs(peakCurrent), 3);
  Serial.println(" A            │");
  Serial.print("│ Total Energy: ");
  Serial.print(totalEnergy / 1000.0, 2);
  Serial.println(" kWs          │");
  Serial.println("└─────────────────────────────────────┘\n");
}

/*
 * ========== CALCULATION BREAKDOWN ==========
 * 
 * EXAMPLE: Measuring 2A current
 * 
 * Step 1: ACS712 outputs voltage
 *   Vout = 2.5V + (2A × 0.185V/A) = 2.87V
 * 
 * Step 2: ESP32 reads via ADC
 *   ADC = (2.87V / 3.3V) × 4095 = 3559
 * 
 * Step 3: Convert ADC back to voltage
 *   V = (3559 / 4095) × 3.3V = 2.87V
 * 
 * Step 4: Calculate current
 *   I = (2.87V - 2.5V) / 0.185 = 2.0A ✓
 * 
 * ========== CALIBRATION ==========
 * 
 * Zero point (2.5V) may vary due to:
 * - Component tolerance
 * - Temperature effects
 * - Supply voltage variations
 * 
 * TO CALIBRATE:
 * 1. Disconnect all loads (0A)
 * 2. Read sensor for 30 seconds
 * 3. Average the readings
 * 4. Update ZERO_CURRENT_VOLTAGE constant
 * 
 * ========== NOISE REDUCTION ==========
 * 
 * Current sensors are noisy! Solutions:
 * 
 * 1. AVERAGING: 50 samples
 * 2. CAPACITOR: 100nF on output → GND
 * 3. SHIELDED WIRES: Twisted pair
 * 4. STABLE POWER: Clean 5V supply
 * 
 * ========== INDUSTRIAL APPLICATIONS ==========
 * 
 * MOTOR MONITORING:
 * - Detect overload conditions
 * - Predict bearing failure (current spikes)
 * - Verify motor startup
 * 
 * ENERGY MANAGEMENT:
 * - Track consumption per machine
 * - Identify inefficient equipment
 * - Calculate operating costs
 * 
 * PREDICTIVE MAINTENANCE:
 * - Baseline normal current
 * - Alert on 20% deviation
 * - Schedule maintenance before failure
 * 
 * SAFETY SYSTEMS:
 * - Overcurrent protection
 * - Ground fault detection
 * - Arc fault detection
 * 
 * ========== COMPANY'S RELEVANCE ==========
 * 
 * Energy monitoring is CORE Industrial focus!
 * 
 * Use cases:
 * - Industrial machine power consumption
 * - HVAC system optimization
 * - Solar panel monitoring
 * - Battery management systems
 * - Microgrid load balancing
 * 
 * ACS712 + ESP32 + LoRaWAN = 
 * Complete wireless energy monitoring!
 * 
 * ========== NEXT STEPS (Day 5) ==========
 * 
 * When hardware arrives:
 * 1. Connect ACS712 to ESP32
 * 2. Calibrate zero point
 * 3. Test with known loads
 * 4. Integrate with Modbus (Day 6)
 * 5. Send data via LoRaWAN (Day 7)
 */
