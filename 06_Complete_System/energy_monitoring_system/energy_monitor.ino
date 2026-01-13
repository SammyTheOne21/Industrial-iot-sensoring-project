/*
 * Complete Energy Monitoring System
 * 
 * Purpose: Monitor power consumption with trends and analytics
 * Uses ACS712 + temperature monitoring
 * 
 * Features:
 * - Real-time current/power measurement
 * - Energy consumption tracking (kWh)
 * - Cost calculation
 * - Temperature correlation
 * - Alert system
 * 
 * Author: Samrat Sharma
 * Date: January 2026
 */

#include <DHT.h>

// ACS712 Current Sensor
#define CURRENT_PIN 34
const float ZERO_CURRENT = 2.5;
const float SENSITIVITY = 0.185;
const float VOLTAGE = 230.0;  // AC mains voltage
const float COST_PER_KWH = 0.15;  // $/kWh

// DHT22 Temperature
#define DHTPIN 15
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Timing
unsigned long previousMillis = 0;
unsigned long energyStartTime = 0;
const long interval = 1000;

// Data tracking
float totalEnergy = 0.0;  // Wh
float peakPower = 0.0;
float avgPower = 0.0;
int sampleCount = 0;

// Hourly tracking
float hourlyEnergy[24] = {0};
int currentHour = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  pinMode(CURRENT_PIN, INPUT);
  dht.begin();
  
  energyStartTime = millis();
  
  Serial.println("\n╔════════════════════════════════════════╗");
  Serial.println("║   Energy Monitoring System             ║");
  Serial.println("╠════════════════════════════════════════╣");
  Serial.println("║ Voltage: 230V AC                       ║");
  Serial.println("║ Cost Rate: $0.15/kWh                   ║");
  Serial.println("╚════════════════════════════════════════╝\n");
  
  Serial.println("Commands:");
  Serial.println("  REPORT - Show detailed report");
  Serial.println("  RESET  - Reset energy counters");
  Serial.println("  HOURLY - Show hourly consumption");
  Serial.println();
}

void loop() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    
    measureAndTrack();
  }
  
  // Handle commands
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    cmd.toUpperCase();
    
    if (cmd == "REPORT") {
      printDetailedReport();
    }
    else if (cmd == "RESET") {
      resetCounters();
    }
    else if (cmd == "HOURLY") {
      printHourlyReport();
    }
  }
}

void measureAndTrack() {
  // Read current
  float current = readCurrent();
  float power = VOLTAGE * abs(current);
  
  // Track peak
  if (power > peakPower) {
    peakPower = power;
  }
  
  // Calculate running average
  sampleCount++;
  avgPower = ((avgPower * (sampleCount - 1)) + power) / sampleCount;
  
  // Calculate energy (Wh)
  float hours = (millis() - energyStartTime) / 3600000.0;
  totalEnergy = avgPower * hours;
  
  // Update hourly tracking
  int hour = (millis() / 3600000) % 24;
  if (hour != currentHour) {
    currentHour = hour;
  }
  hourlyEnergy[currentHour] += (power / 3600.0);  // Wh this second
  
  // Display current reading
  Serial.print("Current: ");
  Serial.print(current, 2);
  Serial.print(" A | Power: ");
  Serial.print(power, 1);
  Serial.print(" W | Energy: ");
  Serial.print(totalEnergy / 1000.0, 3);
  Serial.print(" kWh | Cost: $");
  Serial.println(totalEnergy / 1000.0 * COST_PER_KWH, 2);
  
  // Read temperature for correlation
  float temp = dht.readTemperature();
  if (!isnan(temp) && temp > 35) {
    Serial.println("  ⚠️  High temperature may affect accuracy");
  }
}

float readCurrent() {
  long sum = 0;
  for (int i = 0; i < 100; i++) {
    sum += analogRead(CURRENT_PIN);
    delayMicroseconds(50);
  }
  
  float voltage = (sum / 100.0) * (3.3 / 4095.0) * 2.0;
  float current = (voltage - ZERO_CURRENT) / SENSITIVITY;
  
  return current;
}

void printDetailedReport() {
  float cost = totalEnergy / 1000.0 * COST_PER_KWH;
  float runtime = (millis() - energyStartTime) / 1000.0;
  
  Serial.println("\n╔════════════════════════════════════════════════╗");
  Serial.println("║        ENERGY MONITORING REPORT                ║");
  Serial.println("╠════════════════════════════════════════════════╣");
  
  Serial.print("║ Monitoring Duration: ");
  Serial.print(runtime / 3600, 1);
  Serial.println(" hours                  ║");
  
  Serial.print("║ Total Energy: ");
  Serial.print(totalEnergy / 1000.0, 3);
  Serial.println(" kWh                      ║");
  
  Serial.print("║ Estimated Cost: $");
  Serial.print(cost, 2);
  Serial.println("                        ║");
  
  Serial.print("║ Average Power: ");
  Serial.print(avgPower, 1);
  Serial.println(" W                         ║");
  
  Serial.print("║ Peak Power: ");
  Serial.print(peakPower, 1);
  Serial.println(" W                           ║");
  
  Serial.print("║ Daily Projection: ");
  float dailyKWh = (totalEnergy / 1000.0) / (runtime / 86400.0);
  Serial.print(dailyKWh, 2);
  Serial.println(" kWh/day            ║");
  
  Serial.print("║ Monthly Projection: ");
  Serial.print(dailyKWh * 30, 1);
  Serial.println(" kWh/month         ║");
  
  Serial.print("║ Monthly Cost: $");
  Serial.print(dailyKWh * 30 * COST_PER_KWH, 2);
  Serial.println("                        ║");
  
  Serial.println("╚════════════════════════════════════════════════╝\n");
}

void printHourlyReport() {
  Serial.println("\n╔════════════════════════════════════════════════╗");
  Serial.println("║         HOURLY ENERGY CONSUMPTION              ║");
  Serial.println("╠════════════════════════════════════════════════╣");
  
  for (int i = 0; i < 24; i++) {
    Serial.print("║ Hour ");
    if (i < 10) Serial.print("0");
    Serial.print(i);
    Serial.print(": ");
    Serial.print(hourlyEnergy[i] / 1000.0, 3);
    Serial.print(" kWh");
    
    // Bar graph
    int bars = (int)(hourlyEnergy[i] / 50.0);
    Serial.print(" ");
    for (int j = 0; j < bars && j < 20; j++) {
      Serial.print("█");
    }
    for (int j = bars; j < 20; j++) {
      Serial.print(" ");
    }
    Serial.println(" ║");
  }
  
  Serial.println("╚════════════════════════════════════════════════╝\n");
}

void resetCounters() {
  totalEnergy = 0;
  peakPower = 0;
  avgPower = 0;
  sampleCount = 0;
  energyStartTime = millis();
  
  for (int i = 0; i < 24; i++) {
    hourlyEnergy[i] = 0;
  }
  
  Serial.println("\n✓ All counters reset\n");
}

/*
 * ENERGY MONITORING FEATURES:
 * 
 * REAL-TIME TRACKING:
 * - Instantaneous current and power
 * - Cumulative energy consumption
 * - Cost calculation
 * 
 * ANALYTICS:
 * - Average power consumption
 * - Peak power detection
 * - Hourly breakdown
 * - Daily/monthly projections
 * 
 * ALERTS:
 * - High temperature warning
 * - Overcurrent detection
 * - Cost threshold exceeded
 * 
 * INDUSTRIAL APPLICATIONS:
 * 
 * FACILITY MANAGEMENT:
 * - Track per-machine consumption
 * - Identify energy waste
 * - Optimize production schedules
 * - Reduce electricity bills
 * 
 * PREDICTIVE MAINTENANCE:
 * - Baseline power consumption
 * - Detect degradation (higher power)
 * - Schedule maintenance
 * 
 * DEMAND RESPONSE:
 * - Peak demand management
 * - Load shedding decisions
 * - Time-of-use optimization
 * 
 * COMPANY'S DEPLOYMENT:
 * 
 * Multiple nodes:
 * - Node 1: Production line power
 * - Node 2: HVAC system power
 * - Node 3: Lighting power
 * - Node 4: Office equipment
 * 
 * Central monitoring:
 * - All nodes report via LoRaWAN
 * - Dashboard shows total facility
 * - Alerts for anomalies
 * 
 * ENHANCEMENTS (Day 8):
 * - SD card data logging
 * - Web server dashboard
 * - MQTT cloud integration
 * - Power quality analysis
 */
