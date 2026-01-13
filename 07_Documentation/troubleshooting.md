# Troubleshooting Guide

Common issues and solutions for the Industrial IoT project.

---

## Table of Contents

1. [ESP32 Issues](#esp32-issues)
2. [Sensor Problems](#sensor-problems)
3. [Communication Issues](#communication-issues)
4. [LoRa/LoRaWAN Issues](#loraloraw-issues)
5. [Software/Code Issues](#softwarecode-issues)
6. [Power Issues](#power-issues)

---

## ESP32 Issues

### Cannot Upload Code

**Symptoms:**
- "Failed to connect to ESP32"
- "Timed out waiting for packet header"
- Upload fails at 0% or random percentage

**Solutions:**

1. **Hold BOOT button during upload**
   - Press and hold BOOT button on ESP32
   - Click Upload in Arduino IDE
   - Release BOOT when "Connecting..." appears

2. **Check USB cable**
   - Use data cable (not charge-only)
   - Try different cable
   - Try different USB port

3. **Install/Update drivers**
   - CP210x USB to UART driver
   - Download from Silicon Labs website
   - Restart computer after installation

4. **Select correct settings**
   - Board: "Heltec WiFi LoRa 32(V3)"
   - Upload Speed: 921600 (or try 115200)
   - Port: Correct COM port selected

5. **Check for conflicts**
   - Close Serial Monitor before upload
   - Disconnect GPIO 0 if anything connected
   - Disconnect GPIO 1/3 (USB UART)

### ESP32 Keeps Rebooting

**Symptoms:**
- Continuous restart loop
- Boot messages repeat
- Code doesn't run

**Solutions:**

1. **Power supply insufficient**
   - Use quality USB cable
   - Try powered USB hub
   - Use external 5V 2A supply

2. **Code crash**
   - Check for infinite loops
   - Verify array bounds
   - Add delay() in loop()
   - Look for stack overflow

3. **Watchdog timer**
   - Add `yield()` in long loops
   - Use `delay()` periodically
   - Disable watchdog if needed (testing only)

4. **Hardware short circuit**
   - Check for wiring shorts
   - Remove all sensors
   - Test ESP32 alone
   - Add sensors one by one

### No Serial Output

**Symptoms:**
- Serial Monitor blank
- No response from ESP32

**Solutions:**

1. **Check baud rate**
   - Match Serial Monitor to code (115200)
   - Both NL & CR settings
   - Try different baud rates

2. **Verify USB connection**
   - Check cable
   - Check drivers
   - Try different port

3. **Code issue**
   - Ensure `Serial.begin(115200)` in setup()
   - Add `delay(1000)` after Serial.begin()
   - Check code uploaded successfully

### GPIO Not Working

**Symptoms:**
- Digital read always LOW or HIGH
- Analog read always 0 or 4095
- Output doesn't control device

**Solutions:**

1. **Check pin mode**
```cpp
   pinMode(pin, INPUT);  // or OUTPUT
```

2. **Verify pin number**
   - Use correct GPIO number (not physical pin)
   - GPIO 6-11 used for flash (don't use)
   - GPIO 34-39 are input-only

3. **Check pull-up/pull-down**
```cpp
   pinMode(pin, INPUT_PULLUP);  // If needed
```

4. **ADC-specific**
   - Use ADC1 pins (32-39) not ADC2
   - ADC2 conflicts with WiFi

---

## Sensor Problems

### DHT22 - No Reading / NaN

**Symptoms:**
- `readTemperature()` returns NaN
- `readHumidity()` returns NaN

**Solutions:**

1. **Check wiring**
   - VCC to 3.3V (NOT 5V for ESP32)
   - DATA to GPIO 15
   - GND to GND
   - 10kΩ pull-up between DATA and VCC

2. **Power supply**
   - Stable 3.3V required
   - Add 100nF capacitor near sensor

3. **Timing**
   - Wait 2 seconds between reads minimum
   - Add 1 second delay in setup()

4. **Code check**
```cpp
   #include <DHT.h>
   #define DHTPIN 15
   #define DHTTYPE DHT22
   DHT dht(DHTPIN, DHTTYPE);
   
   void setup() {
     dht.begin();
     delay(1000);  // Important!
   }
```

5. **Sensor fault**
   - Try different DHT22 module
   - Check for physical damage

### BMP280 - Not Found

**Symptoms:**
- `begin()` returns false
- I2C scanner doesn't find it

**Solutions:**

1. **Run I2C scanner first**
   - Upload i2c_scanner.ino
   - Note the address found
   - Usually 0x76 or 0x77

2. **Check address in code**
```cpp
   if (!bmp.begin(0x76)) {  // Try 0x76
     // Try 0x77 if failed
   }
```

3. **Verify wiring**
   - SDA to GPIO 21
   - SCL to GPIO 22
   - VCC to 3.3V
   - GND to GND

4. **Check pull-ups**
   - 4.7kΩ on SDA and SCL
   - Usually on module already

5. **I2C bus issue**
   - Only one device at a time initially
   - Check for shorts on SDA/SCL
   - Keep wires short (<1m)

### ACS712 - Incorrect Readings

**Symptoms:**
- Always reads 0A
- Readings way off
- Negative when should be positive

**Solutions:**

1. **Verify voltage divider**
   - ACS712 OUT → 10kΩ → GPIO 34
   - Junction → 10kΩ → GND
   - Reduces 5V to 2.5V

2. **Calibrate zero point**
   - Disconnect all loads
   - Read sensor voltage
   - Update ZERO_CURRENT_VOLTAGE constant
```cpp
   const float ZERO_CURRENT_VOLTAGE = 2.48;  // Measured value
```

3. **Check sensitivity**
   - ACS712-5A: 185 mV/A
   - ACS712-20A: 100 mV/A
   - ACS712-30A: 66 mV/A
   - Use correct value in code

4. **Averaging**
   - Take 50-100 samples
   - Average to reduce noise
```cpp
   for (int i = 0; i < 100; i++) {
     sum += analogRead(34);
   }
   avgValue = sum / 100;
```

5. **Current connections**
   - Ensure load between IP+ and IP-
   - Check polarity for DC
   - Good screw terminal connections

### HC-SR04 - No Echo / Timeout

**Symptoms:**
- Always returns 0
- pulseIn() times out
- Erratic readings

**Solutions:**

1. **Check power**
   - HC-SR04 needs 5V (not 3.3V)
   - Ensure stable power supply

2. **Verify wiring**
   - TRIG to GPIO 5
   - ECHO to GPIO 18 (5V tolerant)
   - Or use voltage divider on ECHO

3. **Target issues**
   - Object too close (<2cm)
   - Object too far (>400cm)
   - Surface too soft (absorbs sound)
   - Surface angled (reflects away)

4. **Code timing**
```cpp
   digitalWrite(TRIG, LOW);
   delayMicroseconds(2);
   digitalWrite(TRIG, HIGH);
   delayMicroseconds(10);  // Exactly 10µs
   digitalWrite(TRIG, LOW);
   
   long duration = pulseIn(ECHO, HIGH, 30000);  // 30ms timeout
```

5. **Environment**
   - Temperature affects speed of sound
   - Adjust if needed for precision

### SW-420 - Always Triggered / Never Triggers

**Symptoms:**
- Digital output stuck LOW or HIGH
- Too sensitive or not sensitive enough

**Solutions:**

1. **Adjust potentiometer**
   - Rotate to change sensitivity
   - LED on module shows detection
   - Test with gentle tapping

2. **Check wiring**
   - DO to GPIO 13
   - VCC to 3.3V or 5V
   - GND to GND

3. **Logic level**
   - LOW = vibration detected
   - HIGH = no vibration
   - Inverted logic!

4. **Mounting**
   - Secure to vibrating surface
   - Not too tight (might miss vibration)
   - Not too loose (false triggers)

---

## Communication Issues

### I2C - Device Not Responding

**Symptoms:**
- Sensor not found
- I2C scanner shows no devices
- Random freezes

**Solutions:**

1. **Check physical connections**
   - SDA to GPIO 21
   - SCL to GPIO 22
   - Common ground
   - Stable power

2. **Pull-up resistors**
   - 4.7kΩ on SDA and SCL to 3.3V
   - Usually on module, but verify

3. **Address conflict**
   - Only one device at each address
   - Change address if possible (jumpers)

4. **Bus capacitance**
   - Keep wires short (<1m)
   - Remove unnecessary capacitance

5. **Speed issues**
```cpp
   Wire.setClock(100000);  // Slow down to 100kHz
```

### UART - No Data Received

**Symptoms:**
- Serial2.available() always 0
- Garbled data
- Random characters

**Solutions:**

1. **Baud rate mismatch**
   - Both devices must match exactly
   - Try 9600, 19200, 38400, 115200

2. **Wiring**
   - TX of one to RX of other (crossed!)
   - Common ground essential
```
   ESP32 TX2 (17) → Other device RX
   ESP32 RX2 (16) ← Other device TX
   GND ←→ GND
```

3. **Configuration**
```cpp
   Serial2.begin(9600, SERIAL_8N1, 16, 17);
   //            baud  8bit,No parity,1 stop
   //                  RX pin, TX pin
```

4. **Voltage levels**
   - ESP32 uses 3.3V logic
   - If other device is 5V, use level shifter

### RS485 - Communication Failures

**Symptoms:**
- Timeout errors
- CRC errors
- Intermittent communication

**Solutions:**

1. **Check A/B wiring**
   - Try swapping A and B if not working
   - All devices A-to-A, B-to-B

2. **Termination resistors**
   - 120Ω between A and B
   - At BOTH ends of bus only
   - Not at middle devices

3. **DE/RE control**
```cpp
   digitalWrite(DE_RE, HIGH);  // Transmit
   delayMicroseconds(50);      // Wait!
   Serial2.write(data);
   Serial2.flush();            // Wait for TX done
   delayMicroseconds(50);
   digitalWrite(DE_RE, LOW);   // Receive
```

4. **Cable quality**
   - Use twisted pair
   - Shield if EMI present
   - Keep length reasonable

5. **Baud rate**
   - Start with 9600
   - Increase after working

### Modbus - Slave Not Responding

**Symptoms:**
- Timeout on every request
- CRC errors
- Wrong slave ID errors

**Solutions:**

1. **Verify slave address**
   - Check slave device configuration
   - Address 1-247 valid
   - Use 1 for testing

2. **Function code**
   - 0x03 most common (holding registers)
   - Check what slave supports

3. **Register address**
   - May need offset (subtract 1 or 40001)
   - Read slave documentation

4. **RS485 issues**
   - See RS485 troubleshooting above
   - Verify with oscilloscope if available

5. **Timing**
   - Add delays between requests
```cpp
   result = node.readHoldingRegisters(100, 2);
   delay(100);  // Give slave time to process
```

---

## LoRa/LoRaWAN Issues

### LoRa.begin() Fails

**Symptoms:**
- Returns false
- "LoRa initialization failed"

**Solutions:**

1. **Check frequency**
```cpp
   #define LORA_BAND 868E6  // Europe
   // #define LORA_BAND 915E6  // US
   // #define LORA_BAND 433E6  // Asia
```

2. **Verify pins (Heltec V3)**
```cpp
   #define LORA_SCK 9
   #define LORA_MISO 11
   #define LORA_MOSI 10
   #define LORA_SS 8
   #define LORA_RST 12
   #define LORA_DIO0 14
   
   SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);
   LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
```

3. **Library version**
   - Use "LoRa" by Sandeep Mistry
   - Update to latest version

4. **Hardware fault**
   - Antenna connected?
   - Try different ESP32 board

### LoRaWAN Join Fails

**Symptoms:**
- Join request sent, no accept
- "EV_JOIN_FAILED"
- Times out

**Solutions:**

1. **Check keys**
   - DevEUI correct and unique
   - AppEUI matches TTN application
   - AppKey is secret key (16 bytes)
   - All in MSB format for LMIC

2. **Gateway coverage**
   - Check TTN Mapper for gateways nearby
   - Try outdoor location
   - Use SF12 for maximum range

3. **Frequency plan**
   - EU868, US915, AS923, etc.
   - Must match your region
   - Check TTN console settings

4. **ADR settings**
   - Disable ADR for testing
```cpp
   LMIC_setAdrMode(0);
```

5. **Frame counter**
   - Reset on TTN if reflashing often
   - Device Settings → Reset frame counters

### No Packets Received at Gateway

**Symptoms:**
- Uplinks sent but not in TTN console
- Gateway shows no activity

**Solutions:**

1. **Gateway online?**
   - Check TTN console
   - Gateway status "Connected"

2. **Range issue**
   - Move closer to gateway
   - Use outdoor antenna
   - Increase TX power to 20 dBm

3. **Frequency mismatch**
   - Verify regional settings
   - Check gateway frequency plan

4. **Payload too large**
   - Check max payload for data rate
   - DR0: 51 bytes max
   - Reduce payload size

---

## Software/Code Issues

### Compilation Errors

**"Library not found"**
```
Solution: Install missing library via Library Manager
Tools → Manage Libraries → Search → Install
```

**"Board not found"**
```
Solution: Install ESP32 board support
File → Preferences → Additional Board Manager URLs
Add: https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
Tools → Board Manager → Search "esp32" → Install
```

**"Function not declared"**
```
Solution: Check #include statements
Verify function spelling
Check library documentation
```

### Runtime Crashes

**Watchdog Timer Reset**
```cpp
// Add yields in long loops
for (int i = 0; i < 10000; i++) {
  // do work
  yield();  // Let system tasks run
}
```

**Stack Overflow**
```cpp
// Reduce local array sizes
// Use heap allocation for large buffers
// Check recursion depth
```

**Out of Memory**
```
Check available heap:
Serial.println(ESP.getFreeHeap());

Reduce global arrays
Free unused memory
Use F() macro for strings
```

### Strange Behavior

**Random crashes**
- Check for uninitialized variables
- Verify array bounds
- Add bounds checking

**Inconsistent readings**
- Add debouncing
- Use averaging
- Add delays between reads

---

## Power Issues

### ESP32 Keeps Resetting

**Brown-out detector**
```
Solutions:
- Use 5V 2A minimum power supply
- Add 1000µF capacitor near ESP32
- Check USB cable quality
- Use powered USB hub
```

### Sensors Not Working

**Insufficient current**
```
Each sensor draws power:
- DHT22: ~2.5mA
- BMP280: ~3mA
- ACS712: ~10mA
- HC-SR04: ~15mA (during ping)
- Total: ~30-50mA

Ensure power supply can handle load
```

### Battery Life Poor

**High power consumption**
```cpp
// Use deep sleep
ESP.deepSleep(10 * 60 * 1000000);  // 10 min

// Disable WiFi if not needed
WiFi.mode(WIFI_OFF);

// Lower LoRa TX power if acceptable
LoRa.setTxPower(14);  // Instead of 20
```

---

## Diagnostic Tools

### Serial Monitor Commands
```cpp
// Add debug commands
if (Serial.available()) {
  String cmd = Serial.readString();
  
  if (cmd == "STATUS") {
    // Print all sensor states
  }
  if (cmd == "TEST") {
    // Run self-test
  }
  if (cmd == "RESET") {
    ESP.restart();
  }
}
```

### I2C Scanner
```cpp
// Tools → File → Examples → Wire → i2c_scanner
// Finds all I2C devices and their addresses
```

### Oscilloscope/Logic Analyzer

For advanced debugging:
- Verify SPI/I2C/UART signals
- Check timing
- Measure voltages
- Detect noise

---

## Getting Help

### Information to Provide

When asking for help, include:

1. **Hardware**
   - ESP32 board model
   - Sensor models
   - Wiring diagram/photo

2. **Software**
   - Arduino IDE version
   - ESP32 board version
   - Library versions
   - Complete code (use pastebin)

3. **Problem**
   - Exact error message
   - Serial Monitor output
   - What you've tried

4. **Environment**
   - Operating system
   - USB cable type
   - Power supply specs

### Resources

- **Arduino Forum:** forum.arduino.cc
- **ESP32 Forum:** esp32.com
- **Stack Overflow:** stackoverflow.com (tag: esp32)
- **GitHub Issues:** For specific libraries
- **TTN Forum:** thethingsnetwork.org/forum

---

## Prevention Tips

1. **Test incrementally**
   - One sensor at a time
   - Verify each before adding next

2. **Use version control**
   - Git for code changes
   - Tag working versions

3. **Document changes**
   - Comment code thoroughly
   - Keep wiring notes

4. **Quality components**
   - Use known-good sensors
   - Quality jumper wires
   - Proper power supply

5. **Proper handling**
   - ESD precautions
   - Secure connections
   - Organized wiring

---

*Last updated: January 2026*