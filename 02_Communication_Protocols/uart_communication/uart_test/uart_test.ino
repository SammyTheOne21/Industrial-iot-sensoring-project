/*
 * UART Communication Test
 * 
 * Purpose: Test UART2 communication for RS485
 * Understanding serial communication before RS485
 * 
 * Hardware: ESP32
 * UART2 Pins: RX = GPIO 16, TX = GPIO 17
 * 
 * Author: [Your Name]
 * Date: January 2026
 */

// UART2 pin definitions
#define RX2 16
#define TX2 17

// Statistics
unsigned long messagesSent = 0;
unsigned long messagesReceived = 0;

void setup() {
  // USB Serial for debugging
  Serial.begin(115200);
  
  // UART2 for communication
  Serial2.begin(9600, SERIAL_8N1, RX2, TX2);
  
  delay(1000);
  
  Serial.println("\n╔════════════════════════════════════════╗");
  Serial.println("║      UART2 Communication Test          ║");
  Serial.println("╠════════════════════════════════════════╣");
  Serial.println("║ UART2 Config: 9600 baud, 8N1          ║");
  Serial.println("║ RX2: GPIO 16                           ║");
  Serial.println("║ TX2: GPIO 17                           ║");
  Serial.println("╚════════════════════════════════════════╝\n");
  
  Serial.println("Commands:");
  Serial.println("  SEND   - Send test message");
  Serial.println("  STATUS - Show statistics");
  Serial.println("  LOOP   - Start loopback test");
  Serial.println();
}

void loop() {
  // Check USB Serial for commands
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    command.toUpperCase();
    
    if (command == "SEND") {
      sendTestMessage();
    }
    else if (command == "STATUS") {
      printStatus();
    }
    else if (command == "LOOP") {
      loopbackTest();
    }
    else {
      Serial.println("Unknown command");
    }
  }
  
  // Check UART2 for received data
  if (Serial2.available()) {
    String received = Serial2.readStringUntil('\n');
    messagesReceived++;
    
    Serial.print("📨 Received: ");
    Serial.println(received);
  }
}

void sendTestMessage() {
  unsigned long timestamp = millis();
  
  // Format: MSG#123:1234567890
  String message = "MSG#" + String(messagesSent) + ":" + String(timestamp);
  
  Serial2.println(message);
  messagesSent++;
  
  Serial.print("📤 Sent: ");
  Serial.println(message);
}

void printStatus() {
  Serial.println("\n═══ UART2 Statistics ═══");
  Serial.print("Messages Sent: ");
  Serial.println(messagesSent);
  Serial.print("Messages Received: ");
  Serial.println(messagesReceived);
  Serial.print("Uptime: ");
  Serial.print(millis() / 1000);
  Serial.println(" seconds");
  Serial.println();
}

void loopbackTest() {
  Serial.println("\n═══ Loopback Test ═══");
  Serial.println("For loopback, connect TX2 (GPIO 17) to RX2 (GPIO 16)");
  Serial.println("Sending 10 messages...\n");
  
  int successCount = 0;
  
  for (int i = 0; i < 10; i++) {
    String testMsg = "TEST" + String(i);
    
    // Clear any pending data
    while(Serial2.available()) Serial2.read();
    
    // Send message
    Serial2.println(testMsg);
    Serial.print("Sent: " + testMsg + " → ");
    
    // Wait for response (max 1 second)
    unsigned long startWait = millis();
    while (!Serial2.available() && (millis() - startWait < 1000));
    
    if (Serial2.available()) {
      String received = Serial2.readStringUntil('\n');
      received.trim();
      
      if (received == testMsg) {
        Serial.println("✓ Match!");
        successCount++;
      }
      else {
        Serial.print("✗ Got: ");
        Serial.println(received);
      }
    }
    else {
      Serial.println("✗ Timeout");
    }
    
    delay(100);
  }
  
  Serial.println("\n────────────────────");
  Serial.print("Success Rate: ");
  Serial.print(successCount);
  Serial.println("/10");
  Serial.println();
}

/*
 * UART BASICS:
 * 
 * UART = Universal Asynchronous Receiver-Transmitter
 * 
 * CONFIGURATION (8N1):
 * - 8 data bits
 * - No parity bit
 * - 1 stop bit
 * 
 * BAUD RATE:
 * Speed of communication (bits per second)
 * Common: 9600, 19200, 38400, 57600, 115200
 * Both devices must use SAME baud rate!
 * 
 * WIRING:
 * Device A TX → Device B RX
 * Device A RX ← Device B TX
 * Common GND
 * 
 * ESP32 UART PORTS:
 * UART0: USB (Serial) - for programming
 * UART1: Usually internal (flash)
 * UART2: GPIO 16/17 - available for use
 * 
 * LOOPBACK TEST:
 * Connect TX2 to RX2 directly
 * Data sent comes right back
 * Tests UART functionality
 * 
 * FRAME STRUCTURE:
 * 
 * [START][D0 D1 D2 D3 D4 D5 D6 D7][STOP]
 *   1 bit    8 data bits           1 bit
 * 
 * START bit: Always 0 (LOW)
 * DATA bits: LSB first
 * STOP bit: Always 1 (HIGH)
 * 
 * RS485 CONNECTION (Day 6):
 * 
 * This UART2 will connect to RS485 module:
 * ESP32 TX2 → RS485 DI (Data In)
 * ESP32 RX2 → RS485 RO (Read Out)
 * 
 * Then RS485 A/B lines go to Modbus bus
 * 
 * TROUBLESHOOTING:
 * 
 * No data received?
 * - Check baud rate matches
 * - Verify TX/RX not swapped
 * - Check common ground
 * - Test with loopback first
 * 
 * Garbled data?
 * - Baud rate mismatch
 * - Wrong configuration (8N1 vs others)
 * - Cable too long
 * - Electrical interference
 */