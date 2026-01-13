/*
 * RS485 Control Logic Test
 * 
 * Purpose: Test RS485 module control (DE/RE pins)
 * Prepare for Modbus RTU communication
 * 
 * Hardware:
 * - ESP32
 * - RS485 to TTL module (arrives Jan 12)
 * 
 * Connections:
 * ESP32 → RS485 Module
 * GPIO 17 (TX2) → DI (Data In)
 * GPIO 16 (RX2) → RO (Receiver Out)
 * GPIO 4        → DE & RE (Driver/Receiver Enable)
 * 3.3V → VCC
 * GND → GND
 * 
 * Author: Samrat Sharma
 * Date: January 2026
 */

// Pin definitions
#define RX2 16
#define TX2 17
#define DE_RE 4  // Driver Enable / Receiver Enable

// Timing
unsigned long previousMillis = 0;
const long interval = 2000;

// Statistics
int messagesSent = 0;
int messagesReceived = 0;

void setup() {
  // USB Serial for debugging
  Serial.begin(115200);
  
  // UART2 for RS485
  Serial2.begin(9600, SERIAL_8N1, RX2, TX2);
  
  // Configure DE/RE pin
  pinMode(DE_RE, OUTPUT);
  setReceiveMode();  // Start in receive mode
  
  delay(1000);
  
  Serial.println("\n╔════════════════════════════════════════╗");
  Serial.println("║        RS485 Control Test              ║");
  Serial.println("╠════════════════════════════════════════╣");
  Serial.println("║ Baud: 9600, 8N1                       ║");
  Serial.println("║ TX: GPIO 17 → RS485 DI                ║");
  Serial.println("║ RX: GPIO 16 ← RS485 RO                ║");
  Serial.println("║ DE/RE: GPIO 4                          ║");
  Serial.println("╚════════════════════════════════════════╝\n");
  
  Serial.println("RS485 Half-Duplex Communication");
  Serial.println("Sending test message every 2 seconds\n");
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Send message periodically
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    
    sendRS485Message("SENSOR:TEMP:25.5:HUMIDITY:60.2");
  }
  
  // Check for received data
  if (Serial2.available()) {
    String received = receiveRS485Message();
    
    Serial.print("📨 Received: ");
    Serial.println(received);
    messagesReceived++;
  }
  
  // Handle serial commands
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    handleCommand(cmd);
  }
}

void setTransmitMode() {
  digitalWrite(DE_RE, HIGH);  // Enable transmitter
  delayMicroseconds(10);      // Small delay for mode switching
}

void setReceiveMode() {
  digitalWrite(DE_RE, LOW);   // Enable receiver
  delayMicroseconds(10);
}

void sendRS485Message(String message) {
  // Switch to transmit mode
  setTransmitMode();
  
  // Send message
  Serial2.println(message);
  Serial2.flush();  // Wait for transmission to complete
  
  messagesSent++;
  
  // Switch back to receive mode
  setReceiveMode();
  
  // Debug output
  Serial.print("📤 Sent #");
  Serial.print(messagesSent);
  Serial.print(": ");
  Serial.println(message);
}

String receiveRS485Message() {
  // Already in receive mode (default state)
  String message = Serial2.readStringUntil('\n');
  message.trim();
  return message;
}

void handleCommand(String cmd) {
  cmd.toUpperCase();
  
  if (cmd == "STATUS") {
    printStatus();
  }
  else if (cmd == "TEST") {
    runCommTest();
  }
  else if (cmd.startsWith("SEND ")) {
    String msg = cmd.substring(5);
    sendRS485Message(msg);
  }
  else {
    Serial.println("Commands: STATUS, TEST, SEND <message>");
  }
}

void printStatus() {
  Serial.println("\n═══ RS485 Statistics ═══");
  Serial.print("Messages Sent: ");
  Serial.println(messagesSent);
  Serial.print("Messages Received: ");
  Serial.println(messagesReceived);
  Serial.print("Uptime: ");
  Serial.print(millis() / 1000);
  Serial.println(" seconds");
  Serial.println();
}

void runCommTest() {
  Serial.println("\n═══ RS485 Communication Test ═══");
  Serial.println("Sending 5 test messages...\n");
  
  for (int i = 0; i < 5; i++) {
    String testMsg = "TEST_MESSAGE_" + String(i);
    sendRS485Message(testMsg);
    delay(500);
  }
  
  Serial.println("\nTest complete!");
  Serial.println("Check if other device received messages\n");
}

/*
 * RS485 FUNDAMENTALS:
 * 
 * HALF-DUPLEX COMMUNICATION:
 * - Can send OR receive, not both simultaneously
 * - Must control DE/RE pin to switch modes
 * - DE (Driver Enable): HIGH = Transmit
 * - RE (Receiver Enable): LOW = Receive
 * 
 * CRITICAL TIMING:
 * 1. Set DE/RE HIGH (transmit mode)
 * 2. Small delay (10µs)
 * 3. Send data
 * 4. Wait for transmission complete (flush)
 * 5. Small delay
 * 6. Set DE/RE LOW (receive mode)
 * 
 * DIFFERENTIAL SIGNALING:
 * RS485 uses two wires: A and B
 * Signal = voltage difference between A and B
 * 
 * Mark (1): A > B (typically +2V to +6V difference)
 * Space (0): A < B (typically -2V to -6V difference)
 * 
 * NOISE IMMUNITY:
 * Noise affects both wires equally
 * Difference remains constant
 * Much more reliable than single-ended
 * 
 * BUS TOPOLOGY:
 * 
 * Device 1    Device 2    Device 3
 *    |            |            |
 *    ├────────────┼────────────┤  A line
 *    ├────────────┼────────────┤  B line
 *    └────────────┴────────────┘  GND
 * 
 * TERMINATION:
 * 120Ω resistor between A and B
 * At BOTH ends of bus
 * Prevents signal reflections
 * 
 * SPECIFICATIONS:
 * - Max distance: 1200 meters
 * - Max speed: 10 Mbps
 * - Max nodes: 32 (with standard drivers)
 * - Voltage range: -7V to +12V
 * 
 * COMMON ISSUES:
 * 
 * 1. Data corruption:
 *    → Check termination resistors
 *    → Verify twisted pair cable
 *    → Check ground connections
 * 
 * 2. No communication:
 *    → Swap A and B lines
 *    → Check DE/RE control
 *    → Verify power supply
 * 
 * 3. Intermittent errors:
 *    → Add delay after mode switch
 *    → Check cable quality
 *    → Reduce baud rate
 * 
 * MODBUS OVER RS485 (Day 6):
 * 
 * Modbus RTU uses RS485 physical layer:
 * - Master controls bus (polling)
 * - Slaves respond when addressed
 * - Half-duplex: one talks at a time
 * 
 * Your application (Day 6-7):
 * - ESP32 = Modbus Master
 * - Sensors = Modbus Slaves
 * - RS485 bus connects all devices
 * 
 * TESTING WITHOUT SECOND DEVICE:
 * 
 * Can't test fully without another RS485 device
 * But you can verify:
 * - Code compiles
 * - DE/RE switching works
 * - UART2 transmits data
 * 
 * On Day 6, you'll connect actual Modbus devices!
 */
