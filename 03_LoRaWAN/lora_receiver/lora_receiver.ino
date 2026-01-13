/*
 * LoRa Receiver - Basic Reception
 * 
 * Purpose: Receive data using LoRa radio
 * Upload this to a SECOND ESP32 for testing
 * 
 * Hardware: Heltec WiFi LoRa 32 V3
 * 
 * Author: Samrat Sharma
 * Date: January 2026
 */

#include <LoRa.h>
#include <Wire.h>

// LoRa pins for Heltec ESP32 V3
#define LORA_SCK 9
#define LORA_MISO 11
#define LORA_MOSI 10
#define LORA_SS 8
#define LORA_RST 12
#define LORA_DIO0 14

// Must match sender frequency!
#define LORA_BAND 868E6

int packetsReceived = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n╔════════════════════════════════════════╗");
  Serial.println("║        LoRa Receiver Test              ║");
  Serial.println("╠════════════════════════════════════════╣");
  Serial.println("║ Waiting for LoRa packets...           ║");
  Serial.println("╚════════════════════════════════════════╝\n");
  
  // Initialize LoRa
  Serial.print("Initializing LoRa... ");
  
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  
  if (!LoRa.begin(LORA_BAND)) {
    Serial.println("❌ Failed!");
    while (1);
  }
  
  Serial.println("✓ Success!");
  
  // Configure to match sender
  LoRa.setSpreadingFactor(12);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(5);
  
  Serial.println("Listening for packets...\n");
}

void loop() {
  int packetSize = LoRa.parsePacket();
  
  if (packetSize) {
    packetsReceived++;
    
    // Read packet
    String received = "";
    while (LoRa.available()) {
      received += (char)LoRa.read();
    }
    
    // Get RSSI (signal strength)
    int rssi = LoRa.packetRssi();
    
    // Get SNR (signal to noise ratio)
    float snr = LoRa.packetSnr();
    
    // Display
    Serial.println("┌─────────────────────────────────────┐");
    Serial.print("│ Packet #");
    Serial.print(packetsReceived);
    Serial.println("                          │");
    Serial.println("├─────────────────────────────────────┤");
    Serial.print("│ Data: ");
    Serial.print(received);
    
    // Padding
    for(int i = received.length(); i < 28; i++) {
      Serial.print(" ");
    }
    Serial.println("│");
    
    Serial.print("│ RSSI: ");
    Serial.print(rssi);
    Serial.println(" dBm                     │");
    
    Serial.print("│ SNR: ");
    Serial.print(snr);
    Serial.println(" dB                       │");
    Serial.println("└─────────────────────────────────────┘\n");
  }
}

/*
 * RSSI (Received Signal Strength Indicator):
 * 
 * -30 dBm  = Excellent (very close)
 * -60 dBm  = Very good
 * -90 dBm  = Good
 * -120 dBm = Poor (limit of reception)
 * 
 * SNR (Signal to Noise Ratio):
 * 
 * +10 dB = Excellent signal
 * 0 dB   = Signal equals noise
 * -10 dB = Signal below noise (but LoRa still works!)
 * -20 dB = Limit of LoRa sensitivity
 * 
 * RANGE TESTING:
 * 
 * 1. Upload sender to ESP32 #1
 * 2. Upload receiver to ESP32 #2
 * 3. Start with devices 1 meter apart
 * 4. Verify reception
 * 5. Gradually increase distance
 * 6. Note RSSI and SNR values
 * 7. Find maximum range
 * 
 * EXPECTED RANGE:
 * 
 * Indoor (obstacles): 200-500 meters
 * Outdoor (line of sight): 2-5 km
 * Rural (elevated): 10-15 km
 * 
 * IMPROVING RANGE:
 * 
 * - Increase spreading factor (SF12)
 * - Increase TX power (20 dBm)
 * - Better antenna placement
 * - Reduce obstacles
 * - Use external antenna
 */
