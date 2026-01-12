/*
 * I2C Scanner - Find All I2C Devices
 * 
 * Purpose: Scan I2C bus and find connected devices
 * Essential for debugging I2C sensors
 * 
 * Hardware: ESP32
 * I2C Pins: SDA = GPIO 21, SCL = GPIO 22 (default)
 * 
 * Author: [Your Name]
 * Date: January 2026
 */

#include <Wire.h>

void setup() {
  Serial.begin(115200);
  Wire.begin();  // Initialize I2C with default pins
  
  delay(1000);
  
  Serial.println("\n╔════════════════════════════════════════╗");
  Serial.println("║         I2C Device Scanner             ║");
  Serial.println("╠════════════════════════════════════════╣");
  Serial.println("║ SDA: GPIO 21                           ║");
  Serial.println("║ SCL: GPIO 22                           ║");
  Serial.println("║ Scanning addresses 0x01 to 0x7F        ║");
  Serial.println("╚════════════════════════════════════════╝\n");
  
  delay(1000);
}

void loop() {
  scanI2C();
  delay(5000);  // Scan every 5 seconds
}

void scanI2C() {
  byte error, address;
  int deviceCount = 0;
  
  Serial.println("Scanning I2C bus...\n");
  Serial.println("Addr\tStatus\t\tDevice (Common)");
  Serial.println("────────────────────────────────────────");
  
  for(address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    
    if (error == 0) {
      // Device found
      Serial.print("0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      Serial.print("\t✓ Found\t\t");
      printDeviceName(address);
      Serial.println();
      deviceCount++;
    }
    else if (error == 4) {
      Serial.print("0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      Serial.println("\t✗ Error\t\tUnknown error");
    }
  }
  
  Serial.println("────────────────────────────────────────");
  
  if (deviceCount == 0) {
    Serial.println("⚠ No I2C devices found!");
    Serial.println("\nTroubleshooting:");
    Serial.println("1. Check wiring (SDA to GPIO 21, SCL to GPIO 22)");
    Serial.println("2. Verify device power supply");
    Serial.println("3. Check pull-up resistors (4.7kΩ typical)");
    Serial.println("4. Ensure device is I2C-compatible");
  }
  else {
    Serial.print("\n✓ Found ");
    Serial.print(deviceCount);
    Serial.println(" device(s)\n");
  }
}

void printDeviceName(byte address) {
  // Common I2C device addresses
  switch(address) {
    case 0x20:
    case 0x21:
    case 0x22:
    case 0x23:
    case 0x24:
    case 0x25:
    case 0x26:
    case 0x27:
      Serial.print("PCF8574 I/O Expander");
      break;
    case 0x3C:
    case 0x3D:
      Serial.print("OLED Display (SSD1306)");
      break;
    case 0x48:
    case 0x49:
    case 0x4A:
    case 0x4B:
      Serial.print("ADS1115 ADC or TMP102 Temp");
      break;
    case 0x50:
    case 0x51:
    case 0x52:
    case 0x53:
    case 0x54:
    case 0x55:
    case 0x56:
    case 0x57:
      Serial.print("EEPROM (AT24C)");
      break;
    case 0x68:
      Serial.print("DS1307 RTC or MPU6050 IMU");
      break;
    case 0x76:
    case 0x77:
      Serial.print("BMP280/BME280 Pressure");
      break;
    default:
      Serial.print("Unknown device");
  }
}

/*
 * I2C PROTOCOL BASICS:
 * 
 * WIRING:
 * - SDA (Data): GPIO 21 on ESP32
 * - SCL (Clock): GPIO 22 on ESP32
 * - VCC: 3.3V or 5V (check device)
 * - GND: Ground
 * - Pull-up resistors: 4.7kΩ on SDA and SCL
 * 
 * ADDRESSING:
 * - 7-bit address (0x01 to 0x7F)
 * - Some devices have configurable addresses
 * - Multiple devices on same bus (different addresses)
 * 
 * COMMON DEVICES:
 * 0x3C - OLED Display (your Heltec has this!)
 * 0x76/0x77 - BMP280 Pressure (your sensor!)
 * 0x68 - RTC or IMU
 * 
 * COMMUNICATION:
 * 1. START condition
 * 2. Send address + R/W bit
 * 3. Wait for ACK
 * 4. Send/receive data
 * 5. STOP condition
 * 
 * TROUBLESHOOTING:
 * 
 * No devices found?
 * - Check power supply
 * - Verify correct pins
 * - Add pull-up resistors
 * - Check cable length (< 1m)
 * 
 * Device found but not working?
 * - Wrong address variant (try 0x76 vs 0x77)
 * - Voltage mismatch (3.3V vs 5V)
 * - Incorrect library
 * 
 * USAGE WITH YOUR SENSORS:
 * 
 * Day 5: Use this to find your BMP280
 * Expected address: 0x76 or 0x77
 * 
 * If BMP280 not found:
 * - Check SDO pin (determines address)
 * - SDO to GND = 0x76
 * - SDO to VCC = 0x77
 */