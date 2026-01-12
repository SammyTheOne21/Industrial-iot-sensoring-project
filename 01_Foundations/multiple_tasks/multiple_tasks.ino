/*
 * Multiple Non-Blocking Tasks
 * 
 * SAVE TO: 01_Foundations/multiple_tasks/multiple_tasks.ino
 * 
 * Purpose: Run multiple independent tasks simultaneously
 * Simulates industrial monitoring system with:
 * - LED heartbeat indicator
 * - Status reporting
 * - Sensor monitoring
 * 
 * Hardware: ESP32 with LED on GPIO 2
 * 
 * Key Learning:
 * - Each task has own timer
 * - Tasks run at different intervals
 * - No task blocks another
 * - System stays responsive
 * 
 * Author: [Your Name]
 * Date: January 2026
 */

// ========== TIMERS FOR EACH TASK ==========
unsigned long prevLED = 0;
unsigned long prevSerial = 0;
unsigned long prevSensor = 0;

// ========== INTERVALS ==========
const long ledInterval = 500;       // Blink every 0.5s
const long serialInterval = 2000;   // Print every 2s
const long sensorInterval = 100;    // Check every 0.1s

// ========== STATE VARIABLES ==========
int counter = 0;
int sensorAlertCount = 0;

void setup() {
  Serial.begin(115200);
  pinMode(2, OUTPUT);
  delay(1000);
  
  Serial.println("\n========================================");
  Serial.println("   MULTI-TASK MONITORING SYSTEM");
  Serial.println("========================================");
  Serial.println("Task 1: LED Heartbeat   (500ms)");
  Serial.println("Task 2: Status Report   (2000ms)");
  Serial.println("Task 3: Sensor Monitor  (100ms)");
  Serial.println("========================================\n");
}

void loop() {
  unsigned long now = millis();
  
  // ========== TASK 1: LED HEARTBEAT ==========
  if (now - prevLED >= ledInterval) {
    prevLED = now;
    digitalWrite(2, !digitalRead(2));  // Toggle
  }
  
  // ========== TASK 2: STATUS REPORT ==========
  if (now - prevSerial >= serialInterval) {
    prevSerial = now;
    
    Serial.println("┌─────────────────────────────────────┐");
    Serial.print("│ System Running... Counter: ");
    Serial.print(counter);
    Serial.println("        │");
    Serial.print("│ Uptime: ");
    Serial.print(now / 1000);
    Serial.println(" seconds              │");
    Serial.print("│ Sensor Alerts: ");
    Serial.print(sensorAlertCount);
    Serial.println("                  │");
    Serial.println("└─────────────────────────────────────┘\n");
    
    counter++;
  }
  
  // ========== TASK 3: SENSOR MONITORING ==========
  if (now - prevSensor >= sensorInterval) {
    prevSensor = now;
    
    int sensorValue = analogRead(34);
    
    if (sensorValue > 3000) {
      Serial.print("⚠️  ALERT! Threshold exceeded: ");
      Serial.println(sensorValue);
      sensorAlertCount++;
    }
  }
  
  // ========== COMMAND HANDLER ==========
  if (Serial.available() > 0) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    
    if (cmd == "STATUS") {
      Serial.println("[INFO] All systems operational");
    } 
    else if (cmd == "RESET") {
      counter = 0;
      sensorAlertCount = 0;
      Serial.println("[INFO] Counters reset");
    }
    else if (cmd == "HELP") {
      Serial.println("Commands: STATUS, RESET, HELP");
    }
  }
}

/*
 * KEY CONCEPTS:
 * 
 * 1. INDEPENDENT TIMERS
 *    Each task tracks own time
 * 
 * 2. NON-BLOCKING CHECKS
 *    if (now - previous >= interval)
 *    This is INSTANT - doesn't wait
 * 
 * 3. STATE PRESERVATION
 *    Variables keep values between loops
 * 
 * 4. SCALABILITY
 *    Easy to add more tasks
 * 
 * REAL-WORLD ANALOGY:
 * Factory supervisor who:
 * - Checks Machine A every 5 min
 * - Checks Machine B every 10 min
 * - Checks Machine C every 1 min
 * They WALK AROUND (non-blocking)
 * Not WAIT at each machine (blocking)
 * 
 * INDUSTRIAL APPLICATIONS:
 * 
 * MANUFACTURING:
 * - Monitor temp every 1s
 * - Check pressure every 0.5s
 * - Log data every 10s
 * 
 * ENERGY MONITORING:
 * - Measure current every 100ms
 * - Calculate power every 1s
 * - Send LoRaWAN every 5min
 * 
 * PREDICTIVE MAINTENANCE:
 * - Sample vibration at 1kHz
 * - Analyze FFT every 1s
 * - Send alerts immediately
 */