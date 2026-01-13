/*
 * Modbus RTU Slave Simulator
 * 
 * Purpose: ESP32 acts as Modbus slave device
 * Simulates sensor with Modbus registers
 * 
 * Upload this to SECOND ESP32 for testing
 * 
 * Registers:
 * 100: Temperature (0.1°C resolution)
 * 101: Humidity (0.1% resolution)
 * 102: Pressure (0.1 hPa resolution)
 * 
 * Author: Samrat Sharma
 * Date: January 2026
 */

#include <ModbusRTU.h>

#define RX2 16
#define TX2 17
#define DE_RE 4

ModbusRTU mb;

// Simulated sensor data (will update periodically)
uint16_t holdingRegs[10] = {
  250,   // Reg 100: 25.0°C
  605,   // Reg 101: 60.5%
  10132, // Reg 102: 1013.2 hPa
  0, 0, 0, 0, 0, 0, 0
};

unsigned long previousMillis = 0;

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RX2, TX2);
  
  pinMode(DE_RE, OUTPUT);
  digitalWrite(DE_RE, LOW);
  
  // Initialize Modbus slave
  mb.begin(&Serial2, DE_RE);
  mb.slave(1);  // Slave address 1
  
  // Add holding registers (starting at address 100)
  mb.addHreg(100, 0, 10);
  
  Serial.println("\n╔════════════════════════════════════════╗");
  Serial.println("║    Modbus RTU Slave Simulator         ║");
  Serial.println("╠════════════════════════════════════════╣");
  Serial.println("║ Slave Address: 1                      ║");
  Serial.println("║ Registers: 100-109                    ║");
  Serial.println("╚════════════════════════════════════════╝\n");
  
  Serial.println("Waiting for Modbus master queries...\n");
}

void loop() {
  // Handle Modbus requests
  mb.task();
  
  // Update simulated sensor data periodically
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= 5000) {
    previousMillis = currentMillis;
    updateSensorData();
  }
  
  yield();
}

void updateSensorData() {
  // Simulate changing sensor values
  holdingRegs[0] = 250 + random(-20, 20);  // Temp: 23-27°C
  holdingRegs[1] = 605 + random(-50, 50);  // Humidity: 55-65%
  holdingRegs[2] = 10132 + random(-50, 50); // Pressure: 1008-1018 hPa
  
  // Write to Modbus registers
  for (int i = 0; i < 3; i++) {
    mb.Hreg(100 + i, holdingRegs[i]);
  }
  
  Serial.print("Updated registers: ");
  Serial.print("Temp=");
  Serial.print(holdingRegs[0] / 10.0);
  Serial.print("°C, Hum=");
  Serial.print(holdingRegs[1] / 10.0);
  Serial.print("%, Press=");
  Serial.print(holdingRegs[2] / 10.0);
  Serial.println("hPa");
}

/*
 * MODBUS SLAVE SIMULATOR USAGE:
 * 
 * SETUP:
 * 1. Upload modbus_master.ino to ESP32 #1
 * 2. Upload this code to ESP32 #2
 * 3. Connect via RS485:
 *    ESP32 #1 A ──── ESP32 #2 A
 *    ESP32 #1 B ──── ESP32 #2 B
 *    Common GND
 * 4. Power both
 * 5. Master will read slave registers
 * 
 * TESTING WITHOUT SECOND ESP32:
 * 
 * Use software like:
 * - Modbus Poll (Windows)
 * - QModMaster (Cross-platform)
 * - pymodbus (Python)
 * 
 * REAL MODBUS DEVICES:
 * 
 * When you have actual Modbus sensors:
 * - Energy meters
 * - Temperature controllers
 * - PLCs
 * - Variable frequency drives
 * 
 * Replace slave simulator with real device
 * Master code remains same!
 */
