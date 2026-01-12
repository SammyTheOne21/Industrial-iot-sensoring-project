/*
 * ESP32 Analog Reading and ADC Basics
 * 
 * SAVE TO: 01_Foundations/analog_reading/analog_reading.ino
 * 
 * Purpose: Understanding Analog-to-Digital Conversion on ESP32
 * Foundation for reading ALL analog sensors
 * 
 * Hardware: ESP32 (Heltec WiFi LoRa 32 V3)
 * Analog Pin: GPIO 34 (ADC1_CH6)
 * 
 * ESP32 ADC Specifications:
 * - 12-bit resolution (0-4095 range)
 * - Reference voltage: 3.3V
 * - ADC1 pins: 32, 33, 34, 35, 36, 39
 * - ADC2: Avoid (conflicts with WiFi)
 * 
 * Author: [Your Name]
 * Date: January 2026
 */

// ========== CONFIGURATION ==========
const int ANALOG_PIN = 34;           // GPIO 34 (ADC1_CH6)
const int SAMPLE_INTERVAL = 500;     // Read every 500ms
const int SAMPLES_TO_AVERAGE = 10;   // Average 10 readings

// ========== TIMER ==========
unsigned long previousMillis = 0;

void setup() {
  Serial.begin(115200);
  pinMode(ANALOG_PIN, INPUT);
  
  delay(1000);
  
  Serial.println("\n========================================");
  Serial.println("    ESP32 ADC (Analog Reading) Demo");
  Serial.println("========================================");
  Serial.println("ADC Resolution: 12-bit (0-4095)");
  Serial.println("Reference Voltage: 3.3V");
  Serial.println("Pin: GPIO 34 (ADC1_CH6)");
  Serial.println("========================================\n");
  
  Serial.println("Time(ms)\tRaw ADC\tVoltage(V)\tPercent(%)");
  Serial.println("─────────────────────────────────────────────────");
}

void loop() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= SAMPLE_INTERVAL) {
    previousMillis = currentMillis;
    
    // ========== AVERAGED READING (Reduces Noise) ==========
    int sum = 0;
    for (int i = 0; i < SAMPLES_TO_AVERAGE; i++) {
      sum += analogRead(ANALOG_PIN);
      delayMicroseconds(100);  // Small delay between samples
    }
    int avgRawValue = sum / SAMPLES_TO_AVERAGE;
    
    // ========== CONVERT TO VOLTAGE ==========
    // Formula: Voltage = (ADC / ADC_MAX) * Reference_Voltage
    float voltage = avgRawValue * (3.3 / 4095.0);
    
    // ========== CONVERT TO PERCENTAGE ==========
    float percentage = (avgRawValue / 4095.0) * 100.0;
    
    // ========== DISPLAY RESULTS ==========
    Serial.print(currentMillis);
    Serial.print("\t\t");
    Serial.print(avgRawValue);
    Serial.print("\t");
    Serial.print(voltage, 3);  // 3 decimal places
    Serial.print("\t\t");
    Serial.print(percentage, 1);
    Serial.println("%");
    
    // ========== VISUAL BAR GRAPH ==========
    printBarGraph(percentage);
  }
}

// ========== HELPER: VISUAL BAR GRAPH ==========
void printBarGraph(float percentage) {
  Serial.print("[");
  int bars = (int)(percentage / 5);  // 20 bars for 100%
  for (int i = 0; i < 20; i++) {
    if (i < bars) {
      Serial.print("█");
    } else {
      Serial.print("░");
    }
  }
  Serial.println("]");
}

/*
 * ========== UNDERSTANDING ESP32 ADC ==========
 * 
 * ADC = Analog-to-Digital Converter
 * Converts continuous voltage to discrete values
 * 
 * RESOLUTION: 12-bit = 2^12 = 4096 values (0-4095)
 * 
 * VOLTAGE CONVERSION:
 * If ADC = 2048 (half of 4095)
 * Voltage = 2048 / 4095 * 3.3V = 1.65V
 * 
 * EXAMPLE READINGS:
 * Raw ADC | Voltage | Meaning
 * --------|---------|----------
 * 0       | 0.00V   | Ground (0%)
 * 1024    | 0.82V   | 25%
 * 2048    | 1.65V   | 50%
 * 3072    | 2.47V   | 75%
 * 4095    | 3.30V   | Maximum (100%)
 * 
 * ========== COMMON ISSUES & SOLUTIONS ==========
 * 
 * ISSUE 1: Noisy readings (values jump)
 * SOLUTION: Take multiple samples and average
 * 
 * ISSUE 2: Max voltage not 3.3V
 * SOLUTION: ESP32 ADC is non-linear at extremes
 *           Usable range: ~0.15V to 3.1V
 * 
 * ISSUE 3: Readings affected by WiFi
 * SOLUTION: Use ADC1 pins (32-39), not ADC2
 * 
 * ISSUE 4: Low precision (±100mV error)
 * SOLUTION: Use calibration or external ADC
 * 
 * ========== SENSOR APPLICATIONS ==========
 * 
 * TEMPERATURE (LM35):
 * - Output: 10mV/°C
 * - Code: temperature = voltage * 100;
 * 
 * CURRENT (ACS712):
 * - Zero: 2.5V, Sensitivity: 185mV/A
 * - Code: current = (voltage - 2.5) / 0.185;
 * 
 * VOLTAGE DIVIDER:
 * - Measure voltages > 3.3V
 * - Code: actualV = voltage * scaleFactor;
 * 
 * POTENTIOMETER:
 * - Variable resistance → variable voltage
 * - Code: value = map(raw, 0, 4095, 0, 100);
 * 
 * ========== INDUSTRIAL USAGE ==========
 * 
 * QUALITY CONTROL:
 * Read sensor → Compare threshold → Accept/Reject
 * 
 * PROCESS MONITORING:
 * Continuous ADC → Dashboard → Alert if out of range
 * 
 * CALIBRATION:
 * Known input → Measure ADC → Calculate correction
 * 
 * DATA LOGGING:
 * Periodic reads → Store with timestamp → Analyze
 */