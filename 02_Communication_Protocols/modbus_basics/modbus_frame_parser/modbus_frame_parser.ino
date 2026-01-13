/*
 * Modbus RTU Frame Parser and Simulator
 * 
 * Purpose: Understand Modbus RTU frame structure
 * Parse and generate Modbus messages
 * 
 * Author: Samrat Sharma
 * Date: January 2026
 */

// Modbus Function Codes
#define MODBUS_READ_COILS           0x01
#define MODBUS_READ_DISCRETE        0x02
#define MODBUS_READ_HOLDING_REG     0x03
#define MODBUS_READ_INPUT_REG       0x04
#define MODBUS_WRITE_SINGLE_COIL    0x05
#define MODBUS_WRITE_SINGLE_REG     0x06
#define MODBUS_WRITE_MULTIPLE_REGS  0x10

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n╔════════════════════════════════════════╗");
  Serial.println("║   Modbus RTU Frame Parser             ║");
  Serial.println("╚════════════════════════════════════════╝\n");
  
  // Demonstrate different Modbus frames
  demonstrateReadHoldingRegisters();
  delay(2000);
  
  demonstrateWriteSingleRegister();
  delay(2000);
  
  demonstrateResponse();
}

void loop() {
  // Practice: Try parsing frames from Serial Monitor
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    parseModbusFrame(input);
  }
}

void demonstrateReadHoldingRegisters() {
  Serial.println("═══ READ HOLDING REGISTERS (0x03) ═══");
  
  // Example: Read 2 registers starting at address 100 from device 1
  byte frame[] = {0x01, 0x03, 0x00, 0x64, 0x00, 0x02};
  
  Serial.println("\nRequest Frame:");
  Serial.print("Slave Address: 0x");
  Serial.println(frame[0], HEX);
  Serial.print("Function Code: 0x");
  Serial.println(frame[1], HEX);
  Serial.print("Start Address: ");
  Serial.println((frame[2] << 8) | frame[3]);
  Serial.print("Quantity: ");
  Serial.println((frame[4] << 8) | frame[5]);
  
  Serial.println("\nWhat this does:");
  Serial.println("→ Asks device 1 to send values from registers 100-101");
  Serial.println();
}

void demonstrateWriteSingleRegister() {
  Serial.println("═══ WRITE SINGLE REGISTER (0x06) ═══");
  
  // Example: Write value 1234 to register 100 on device 1
  uint16_t value = 1234;
  byte frame[] = {
    0x01,                    // Slave address
    0x06,                    // Function code
    0x00, 0x64,             // Register address (100)
    (byte)(value >> 8),     // Value high byte
    (byte)(value & 0xFF)    // Value low byte
  };
  
  Serial.println("\nRequest Frame:");
  Serial.print("Slave Address: 0x");
  Serial.println(frame[0], HEX);
  Serial.print("Function Code: 0x");
  Serial.println(frame[1], HEX);
  Serial.print("Register: ");
  Serial.println((frame[2] << 8) | frame[3]);
  Serial.print("Value: ");
  Serial.println((frame[4] << 8) | frame[5]);
  
  Serial.println("\nWhat this does:");
  Serial.println("→ Writes 1234 to register 100 on device 1");
  Serial.println();
}

void demonstrateResponse() {
  Serial.println("═══ TYPICAL RESPONSE ═══");
  
  // Response to read 2 registers: values are 200 and 300
  byte response[] = {
    0x01,        // Slave address
    0x03,        // Function code
    0x04,        // Byte count (4 bytes = 2 registers)
    0x00, 0xC8,  // Register 1: 200
    0x01, 0x2C   // Register 2: 300
  };
  
  Serial.println("\nResponse Frame:");
  Serial.print("Slave Address: 0x");
  Serial.println(response[0], HEX);
  Serial.print("Function Code: 0x");
  Serial.println(response[1], HEX);
  Serial.print("Byte Count: ");
  Serial.println(response[2]);
  
  uint16_t reg1 = (response[3] << 8) | response[4];
  uint16_t reg2 = (response[5] << 8) | response[6];
  
  Serial.print("Register 1 Value: ");
  Serial.println(reg1);
  Serial.print("Register 2 Value: ");
  Serial.println(reg2);
  Serial.println();
}

void parseModbusFrame(String hexString) {
  Serial.println("\n═══ PARSING YOUR FRAME ═══");
  Serial.print("Input: ");
  Serial.println(hexString);
  
  // Remove spaces and convert to uppercase
  hexString.replace(" ", "");
  hexString.toUpperCase();
  
  if (hexString.length() < 4) {
    Serial.println("Error: Frame too short");
    return;
  }
  
  // Parse slave address
  byte slaveAddr = hexToByte(hexString.substring(0, 2));
  Serial.print("Slave Address: ");
  Serial.println(slaveAddr);
  
  // Parse function code
  byte funcCode = hexToByte(hexString.substring(2, 4));
  Serial.print("Function Code: 0x");
  Serial.print(funcCode, HEX);
  Serial.print(" (");
  printFunctionName(funcCode);
  Serial.println(")");
}

byte hexToByte(String hex) {
  return (byte)strtol(hex.c_str(), NULL, 16);
}

void printFunctionName(byte code) {
  switch(code) {
    case 0x01: Serial.print("Read Coils"); break;
    case 0x02: Serial.print("Read Discrete Inputs"); break;
    case 0x03: Serial.print("Read Holding Registers"); break;
    case 0x04: Serial.print("Read Input Registers"); break;
    case 0x05: Serial.print("Write Single Coil"); break;
    case 0x06: Serial.print("Write Single Register"); break;
    case 0x10: Serial.print("Write Multiple Registers"); break;
    default: Serial.print("Unknown");
  }
}

/*
 * MODBUS FRAME STRUCTURE:
 * 
 * [Slave Addr][Function][Data...][CRC-16]
 *    1 byte     1 byte    N bytes  2 bytes
 * 
 * COMMON FUNCTION CODES:
 * 0x03 - Read Holding Registers (most common)
 * 0x04 - Read Input Registers
 * 0x06 - Write Single Register
 * 0x10 - Write Multiple Registers
 * 
 * REGISTER TYPES:
 * Holding Registers (4xxxx) - Read/Write
 * Input Registers (3xxxx) - Read Only
 * 
 * PRACTICE EXERCISES:
 * 
 * 1. Send: 01 03 00 64 00 01
 *    What does this request?
 * 
 * 2. Send: 02 06 00 32 00 FA
 *    What does this do?
 * 
 * 3. Design a frame to read 5 registers
 *    starting at address 200 from device 3
 */
