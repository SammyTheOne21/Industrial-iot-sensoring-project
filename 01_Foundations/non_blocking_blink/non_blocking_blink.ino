/*
 * Non-Blocking LED Blink using millis()
 * 
 * SAVE TO: 01_Foundations/non_blocking_blink/non_blocking_blink.ino
 * 
 * Purpose: Demonstrates industrial-grade non-blocking code pattern
 * This is ESSENTIAL for real-time embedded systems where multiple
 * tasks must run simultaneously without blocking each other.
 * 
 * Hardware: ESP32 (any board)
 * LED: Built-in LED on GPIO 2
 * 
 * Key Concepts:
 * - millis() returns time since program started
 * - Non-blocking: loop() runs continuously without delay()
 * - Multiple tasks can run "simultaneously"
 * 
 * Author: [Your Name]
 * Date: January 2026
 * Project: Tynatech IoT Internship Preparation
 */

// Global variables
unsigned long previousMillis = 0;  // Last LED update time
const long interval = 1000;        // Blink interval (1 second)
int ledState = LOW;                // Current LED state

void setup() {
  Serial.begin(115200);
  pinMode(2, OUTPUT);
  
  Serial.println("========================================");
  Serial.println("  Non-Blocking Blink Demo Started");
  Serial.println("========================================");
  Serial.println("LED will toggle every 1 second");
  Serial.println("Loop continues running without blocking");
  Serial.println("========================================\n");
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Check if interval has passed (NON-BLOCKING!)
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    
    // Toggle LED
    ledState = !ledState;
    digitalWrite(2, ledState);
    
    // Print status
    Serial.print("Time: ");
    Serial.print(currentMillis);
    Serial.print(" ms | LED State: ");
    Serial.println(ledState ? "ON" : "OFF");
  }
  
  // CRITICAL: Code here runs continuously!
  // In real systems, you would:
  // - Check sensor readings
  // - Process incoming data
  // - Respond to events
  // - Handle communication
  
  // Example: Non-blocking serial check
  if (Serial.available() > 0) {
    char received = Serial.read();
    Serial.print("Received: ");
    Serial.println(received);
  }
}

/*
 * WHY NON-BLOCKING IS BETTER:
 * 
 * BAD (blocking):
 * void loop() {
 *   digitalWrite(2, HIGH);
 *   delay(1000);  // BLOCKS everything!
 *   digitalWrite(2, LOW);
 *   delay(1000);  // BLOCKS again!
 * }
 * 
 * GOOD (non-blocking):
 * - Uses millis() to check time
 * - loop() keeps running
 * - System remains responsive
 * 
 * INDUSTRIAL IMPORTANCE:
 * - Safety systems need immediate response
 * - Multiple sensors monitored constantly
 * - Communication handled without delays
 * - Blocking = potential failure
 */