# Protocol Reference Guide

Complete reference for all communication protocols used in this project.

---

## Table of Contents

1. [Modbus RTU](#modbus-rtu)
2. [LoRaWAN](#lorawan)
3. [I2C](#i2c)
4. [UART](#uart)
5. [RS485](#rs485)

---

## Modbus RTU

### Overview

Modbus RTU is a serial communication protocol for industrial devices.
- **RTU** = Remote Terminal Unit
- **Master-Slave** architecture
- **Serial communication** over RS485 or RS232
- **Industry standard** since 1979

### Frame Structure
```
[Address][Function][Data][CRC-16]
 1 byte    1 byte   N bytes 2 bytes
```

**Example Request:**
```
01 03 00 64 00 02 C5 CC
│  │  │  │  │  │  └─┴─ CRC-16 checksum
│  │  │  │  └──┴───── Quantity: 2 registers
│  │  └──┴────────── Start address: 100 (0x0064)
│  └──────────────── Function: 0x03 (Read Holding)
└─────────────────── Slave address: 1
```

**Example Response:**
```
01 03 04 00 C8 01 2C XX XX
│  │  │  │  │  │  │  └─┴─ CRC-16
│  │  │  │  │  └──┴───── Register 2: 300 (0x012C)
│  │  │  └──┴────────── Register 1: 200 (0x00C8)
│  │  └──────────────── Byte count: 4
│  └─────────────────── Function: 0x03
└────────────────────── Slave address: 1
```

### Function Codes

| Code | Name | Description |
|------|------|-------------|
| 0x01 | Read Coils | Read 1-2000 coils (binary outputs) |
| 0x02 | Read Discrete Inputs | Read 1-2000 inputs (binary) |
| 0x03 | Read Holding Registers | Read 1-125 registers (16-bit, R/W) |
| 0x04 | Read Input Registers | Read 1-125 registers (16-bit, RO) |
| 0x05 | Write Single Coil | Write 1 coil |
| 0x06 | Write Single Register | Write 1 register |
| 0x0F | Write Multiple Coils | Write multiple coils |
| 0x10 | Write Multiple Registers | Write multiple registers |

### Register Types

**Coils (0x):** Binary outputs (relays, switches)
- Read/Write
- 1 bit each
- Address: 00001-09999

**Discrete Inputs (1x):** Binary inputs (sensors, switches)
- Read-only
- 1 bit each
- Address: 10001-19999

**Input Registers (3x):** Analog inputs (sensor values)
- Read-only
- 16-bit (2 bytes)
- Address: 30001-39999

**Holding Registers (4x):** Configuration and data
- Read/Write
- 16-bit (2 bytes)
- Address: 40001-49999

### Data Encoding

**16-bit Integer:**
```
Value: 1234 decimal
High byte: 0x04 (4)
Low byte: 0xD2 (210)
Wire format: 04 D2
```

**32-bit Float (IEEE 754):**
```
Value: 25.5
As 32-bit float: 0x41CC0000
Split into 2 registers:
  Register 1: 0x41CC (high word)
  Register 2: 0x0000 (low word)
```

**Signed vs Unsigned:**
- Unsigned: 0 to 65535
- Signed: -32768 to 32767
- Use two's complement for negative

### Common Baud Rates

- 9600 bps (most common)
- 19200 bps
- 38400 bps
- 57600 bps
- 115200 bps

**Configuration:** 8-N-1 (8 data bits, No parity, 1 stop bit)

### Error Codes

| Code | Name | Meaning |
|------|------|---------|
| 0x01 | Illegal Function | Function code not supported |
| 0x02 | Illegal Data Address | Register address doesn't exist |
| 0x03 | Illegal Data Value | Value out of range |
| 0x04 | Slave Device Failure | Unrecoverable error |
| 0x05 | Acknowledge | Long operation in progress |
| 0x06 | Slave Device Busy | Busy, retry later |

### CRC Calculation
```cpp
uint16_t ModbusCRC(uint8_t *buf, int len) {
  uint16_t crc = 0xFFFF;
  
  for (int pos = 0; pos < len; pos++) {
    crc ^= (uint16_t)buf[pos];
    
    for (int i = 8; i != 0; i--) {
      if ((crc & 0x0001) != 0) {
        crc >>= 1;
        crc ^= 0xA001;
      } else {
        crc >>= 1;
      }
    }
  }
  
  return crc;
}
```

---

## LoRaWAN

### Overview

LoRaWAN is a Low Power Wide Area Network protocol.
- **Long range:** 2-15 km
- **Low power:** Years on battery
- **Low data rate:** 0.3-50 kbps
- **Star topology:** Devices → Gateway → Server

### Network Architecture
```
End Devices ──(LoRa)──> Gateway ──(IP)──> Network Server ──> Application Server
```

### Device Classes

**Class A (Lowest Power):**
- Uplink when device wants
- Two downlink windows after each uplink
- **Your sensors use this!**
- Battery life: years

**Class B (Scheduled):**
- Scheduled downlink windows
- Beacon synchronized
- Battery life: months

**Class C (Always Listening):**
- Continuous reception
- Immediate downlink
- Must be powered
- Use for actuators

### Activation Methods

**OTAA (Over-The-Air Activation) - RECOMMENDED:**
```
Device ──Join Request──> Network
       <──Join Accept──
       [Session keys generated]
```

Keys needed:
- **DevEUI:** 8 bytes, unique device ID
- **AppEUI:** 8 bytes, application ID
- **AppKey:** 16 bytes, secret key

**ABP (Activation By Personalization):**
```
Pre-configured session keys
No join procedure
Less secure
```

### Data Rates (EU868)

| DR | SF | Bandwidth | Bitrate | Max Payload |
|----|----|-----------| --------|-------------|
| 0 | SF12 | 125 kHz | 250 bps | 51 bytes |
| 1 | SF11 | 125 kHz | 440 bps | 51 bytes |
| 2 | SF10 | 125 kHz | 980 bps | 51 bytes |
| 3 | SF9 | 125 kHz | 1760 bps | 115 bytes |
| 4 | SF8 | 125 kHz | 3125 bps | 222 bytes |
| 5 | SF7 | 125 kHz | 5470 bps | 222 bytes |

**ADR (Adaptive Data Rate):**
- Network automatically adjusts DR
- Optimizes for range and airtime
- Enable in most cases

### Duty Cycle Limits (EU868)

- **1% duty cycle** on 868 MHz band
- **Max 36 seconds per hour**
- Fair Access Policy on TTN: 30s uplink/day

**Example:**
- Message takes 1 second
- Can send 36 messages per hour
- Or 1 message every 100 seconds

### Frame Structure
```
[MHDR][DevAddr][FCtrl][FCnt][FOpts][FPort][FRMPayload][MIC]
1 byte 4 bytes  1 byte 2 bytes 0-15   1 byte  N bytes    4 bytes
```

**Uplink Example:**
40 [DevAddr] 00 0001 00 01 [Encrypted Payload] [MIC]
│   └────────┘ │  │    │  │  └─────────────────┘ └───┘
│   Device Addr│  │    │  │  Your sensor data      Message
│   (4 bytes)  │  │    │  │                        Integrity
│              │  │    │  └─ FPort (1)             Code
│              │  │    └──── Frame Options
│              │  └───────── Frame Counter
│              └──────────── Frame Control
└─────────────────────────── Message Type (Unconfirmed Up)
### Payload Encryption

- **AES-128** encryption
- **AppSKey:** Encrypts application payload
- **NwkSKey:** Calculates MIC
- Keys derived from AppKey during join

### Frequency Channels (EU868)

- 868.1 MHz
- 868.3 MHz
- 868.5 MHz
- 867.1 MHz
- 867.3 MHz
- 867.5 MHz
- 867.7 MHz
- 867.9 MHz

### Best Practices

1. **Send data every 10+ minutes** (not seconds)
2. **Keep payloads small** (<50 bytes typical)
3. **Use binary encoding** (not JSON)
4. **Enable ADR** for optimal performance
5. **Respect duty cycle** limits
6. **Use confirmed messages sparingly** (ACK uses downlink)
7. **Implement exponential backoff** on join failures
8. **Monitor frame counter** (security)

### Payload Optimization

**Bad (JSON - 45 bytes):**
```json
{"temp":25.5,"hum":60.2,"pres":1013}
```

**Good (Binary - 6 bytes):**
```
[0x00][0xFF][0x02][0x5A][0x27][0x95]
 Temp  Temp  Hum   Hum   Press Press
 High  Low   High  Low   High  Low
```

**Encoding:**
```cpp
// Temperature 25.5°C → 255 (0.1°C resolution)
uint16_t temp = temperature * 10;
payload[0] = temp >> 8;
payload[1] = temp & 0xFF;

// Humidity 60.2% → 602
uint16_t hum = humidity * 10;
payload[2] = hum >> 8;
payload[3] = hum & 0xFF;

// Pressure 1013hPa → 10130
uint16_t pres = pressure * 10;
payload[4] = pres >> 8;
payload[5] = pres & 0xFF;
```

---

## I2C

### Overview

I2C (Inter-Integrated Circuit) is a 2-wire serial protocol.
- **2 wires:** SDA (data), SCL (clock)
- **Multi-master, multi-slave**
- **7-bit or 10-bit addressing**
- **Synchronous** communication

### Physical Layer
```
MCU              Sensor 1         Sensor 2
│                │                │
├─── SDA ────────┼────────────────┤
│    (4.7kΩ)     │                │
│      │         │                │
├─── SCL ────────┼────────────────┤
│    (4.7kΩ)     │                │
│      │         │                │
└─── VCC        VCC              VCC
```

**Pull-up resistors:** 4.7kΩ (typical) on both SDA and SCL

### Communication Sequence
```
START → ADDRESS+R/W → ACK → DATA → ACK → ... → STOP

START: SDA falls while SCL high
STOP:  SDA rises while SCL high
ACK:   SDA pulled low by receiver
```

### Timing Diagram
```
SCL  ──┐  ┌──┐  ┌──┐  ┌──┐  ┌──┐  ┌──┐  ┌──┐  ┌──┐  ┌──┐  ┌─
       └──┘  └──┘  └──┘  └──┘  └──┘  └──┘  └──┘  └──┘  └──┘

SDA  ──────┐     ┌─────┐  ┌──┐  ┌──┐  ┌─────────────┐
START      └─────┘  1  └──┘0 └──┘0 └──┘ ACK         └── STOP
           A6 A5 A4 A3 A2 A1 A0 R/W
           [7-bit Address]   [Read/Write]
```

### Speed Modes

| Mode | Speed | Use Case |
|------|-------|----------|
| Standard | 100 kHz | Basic sensors |
| Fast | 400 kHz | Most common |
| Fast Plus | 1 MHz | Advanced |
| High Speed | 3.4 MHz | Specialized |

### Common I2C Addresses

| Device | Address(es) | Notes |
|--------|-------------|-------|
| OLED SSD1306 | 0x3C, 0x3D | Display |
| BMP280 | 0x76, 0x77 | Pressure sensor |
| ADS1115 | 0x48-0x4B | ADC |
| PCF8574 | 0x20-0x27 | I/O expander |
| DS1307 | 0x68 | RTC |
| MPU6050 | 0x68, 0x69 | IMU |

### Arduino Wire Library
```cpp
#include <Wire.h>

// Master initialization
Wire.begin();  // Default: SDA=21, SCL=22 on ESP32

// Write to slave
Wire.beginTransmission(0x76);  // Address
Wire.write(0xF4);              // Register
Wire.write(0x27);              // Value
Wire.endTransmission();

// Read from slave
Wire.beginTransmission(0x76);
Wire.write(0xF7);              // Register to read
Wire.endTransmission(false);   // Repeated start
Wire.requestFrom(0x76, 3);     // Read 3 bytes

if (Wire.available() >= 3) {
  uint8_t msb = Wire.read();
  uint8_t lsb = Wire.read();
  uint8_t xlsb = Wire.read();
}
```

### Troubleshooting

**No ACK (device not responding):**
- Check address (use I2C scanner)
- Verify power supply
- Check pull-up resistors
- Verify SDA/SCL connections

**Data corruption:**
- Add capacitors (100nF) near sensor
- Reduce speed (Wire.setClock(100000))
- Shorten wires (<1m recommended)
- Check for interference

---

## UART

### Overview

UART (Universal Asynchronous Receiver-Transmitter)
- **2 wires:** TX (transmit), RX (receive)
- **No clock** - asynchronous
- **Point-to-point** communication
- **Full-duplex** (send and receive simultaneously)

### Frame Structure
```
[START][D0][D1][D2][D3][D4][D5][D6][D7][PARITY][STOP]
  1 bit        8 data bits (LSB first)   0-1 bit 1-2 bits

Typical: 8N1 (8 data, No parity, 1 stop)
```

**Example byte 0x53 ('S'):**
```
Idle  Start D0 D1 D2 D3 D4 D5 D6 D7 Stop  Idle
HIGH  LOW   1  1  0  0  1  0  1  0  HIGH  HIGH
      └───────────────────────────┘
           Bits sent LSB first
           0x53 = 01010011 binary
           Sent as: 11001010
```

### Common Baud Rates

- 9600 bps (default for many devices)
- 19200 bps
- 38400 bps
- 57600 bps
- **115200 bps** (common for ESP32)
- 230400 bps
- 460800 bps
- 921600 bps

**Both devices must use same baud rate!**

### Wiring
```
Device A          Device B
  TX    ────────→   RX
  RX    ←────────   TX
  GND   ──────────  GND
```

**Critical:** Cross TX/RX connections!

### ESP32 UART Ports

| UART | Default Pins | Use |
|------|--------------|-----|
| UART0 | TX=1, RX=3 | USB (programming) |
| UART1 | TX=10, RX=9 | Internal (flash) |
| UART2 | TX=17, RX=16 | **Available for use** |

### Arduino Serial Library
```cpp
// UART0 (USB)
Serial.begin(115200);
Serial.println("Hello");

// UART2 (custom pins)
Serial2.begin(9600, SERIAL_8N1, 16, 17);
//            baud   config      RX  TX

Serial2.write(0x01);  // Send byte
if (Serial2.available()) {
  char c = Serial2.read();
}
```

### Configuration Options

**Data bits:** 5, 6, 7, 8
**Parity:** None, Even, Odd
**Stop bits:** 1, 2

**Examples:**
- SERIAL_8N1: 8 data, no parity, 1 stop (default)
- SERIAL_7E1: 7 data, even parity, 1 stop
- SERIAL_8E2: 8 data, even parity, 2 stop

---

## RS485

### Overview

RS485 is a differential signaling standard for UART.
- **2 wires:** A (non-inverting), B (inverting)
- **Differential:** Signal = voltage between A and B
- **Long distance:** Up to 1200 meters
- **Multi-drop:** Up to 32 devices on one bus
- **Noise immune:** Common mode rejection

### Differential Signaling
```
Binary '1' (MARK):
  A voltage > B voltage (typically +2V to +6V difference)
  
Binary '0' (SPACE):
  A voltage < B voltage (typically -2V to -6V difference)

Example:
  Sending '1': A = +5V, B = 0V → Difference = +5V
  Sending '0': A = 0V, B = +5V → Difference = -5V
```

**Noise Immunity:**
```
Without noise:
  A = +5V, B = 0V → Diff = +5V (Logic 1)

With +2V common-mode noise on both:
  A = +7V, B = +2V → Diff = +5V (Still Logic 1!)
  
Noise cancelled because it affects both wires equally!
```

### Bus Topology
```
Device 1       Device 2       Device 3       Device 4
120Ω                                         120Ω
 ├─┬─────────────┬─────────────┬─────────────┬─┤
 A │             │             │             │ A
 B │             │             │             │ B
 └─┴─────────────┴─────────────┴─────────────┴─┘
   Termination                             Termination
```

**Termination Resistors:**
- 120Ω between A and B
- At BOTH ends of bus
- Prevents signal reflections
- Critical for reliable communication

### Wiring
```
ESP32          RS485 Module        RS485 Bus
GPIO 17 TX ──→ DI (Data In)        
GPIO 16 RX ←── RO (Receiver Out)
GPIO 4     ──→ DE (Driver Enable)  
GPIO 4     ──→ RE (Receiver Enable) (tied together)
                                 A ─┬─ To other devices
                                 B ─┘
```

**Cable Recommendations:**
- Use **twisted pair** cable
- CAT5/CAT6 cable works well
- Shield for EMI environments
- Keep cable impedance ~120Ω

### Half-Duplex Control

RS485 is half-duplex - can transmit OR receive, not both.

**Control DE/RE pins:**
```cpp
void setTransmitMode() {
  digitalWrite(DE_RE, HIGH);  // Enable driver
  delayMicroseconds(10);      // Wait for switch
}

void setReceiveMode() {
  digitalWrite(DE_RE, LOW);   // Enable receiver
}

// Transmit sequence:
setTransmitMode();
Serial2.write(data, length);
Serial2.flush();              // Wait for TX complete
delayMicroseconds(10);
setReceiveMode();
```

### Specifications

| Parameter | Value |
|-----------|-------|
| Max Distance | 1200 m (4000 ft) |
| Max Speed | 10 Mbps |
| Max Devices | 32 (standard), 256 (with repeaters) |
| Voltage Range | -7V to +12V (common mode) |
| Differential | ±200mV minimum (receiver) |
| Cable | Twisted pair, 120Ω impedance |

### Troubleshooting

**No communication:**
- Check A/B not swapped
- Verify termination resistors
- Check DE/RE control timing
- Test with slower baud rate

**Data corruption:**
- Add/check termination
- Use shielded twisted pair
- Ensure common ground
- Reduce baud rate
- Check for loose connections

**Intermittent errors:**
- Check cable quality
- Verify termination at ends only
- Add small delay after mode switch
- Check for EMI sources

---

## Protocol Comparison

| Protocol | Wires | Speed | Distance | Devices | Use Case |
|----------|-------|-------|----------|---------|----------|
| **Modbus RTU** | 2 (via RS485) | 9600-115200 bps | 1200m | 247 | Industrial control |
| **LoRaWAN** | Wireless | 0.3-50 kbps | 2-15 km | Unlimited | Long-range IoT |
| **I2C** | 2 | 100kHz-3.4MHz | <1m | 127 | On-board sensors |
| **UART** | 2 | 9600-921600 bps | <15m | 2 | Point-to-point |
| **RS485** | 2 | Up to 10 Mbps | 1200m | 32 | Industrial buses |

---

## Quick Reference Tables

### Modbus Function Codes Quick Reference

| Hex | Dec | Name | Typical Use |
|-----|-----|------|-------------|
| 0x01 | 1 | Read Coils | Relay status |
| 0x03 | 3 | Read Holding | Sensor data |
| 0x04 | 4 | Read Input | Analog inputs |
| 0x06 | 6 | Write Single | Setpoint |
| 0x10 | 16 | Write Multiple | Configuration |

### LoRaWAN Spreading Factors

| SF | Range | Speed | Use When |
|----|-------|-------|----------|
| 7 | Short | Fast | Close to gateway |
| 9 | Medium | Medium | Normal operation |
| 12 | Long | Slow | Maximum range needed |

### I2C Speed Selection

| Application | Speed | Why |
|-------------|-------|-----|
| Simple sensor | 100kHz | Safe, compatible |
| Most sensors | 400kHz | Good balance |
| Fast ADC | 1MHz+ | High sample rate |

---

## Additional Resources

- **Modbus Spec:** modbus.org
- **LoRaWAN Spec:** lora-alliance.org
- **I2C Spec:** i2c-bus.org
- **RS485 Standard:** TIA-485-A

---

*Last updated: January 2026*