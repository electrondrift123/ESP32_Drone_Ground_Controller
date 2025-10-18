#include "main_tx.h"
#include <Arduino.h>
#include "pin_config.h"
#include "RF24.h"

RF24 radio(NRF_CE_PIN, NRF_CSN_PIN);
uint8_t address[][6] = { "1Node", "2Node" };

bool main_tx_init(void){
    // Setup radio
  // SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  if (!radio.begin()) {
    Serial.println("nRF24 not detected - but SPI works!");
    Serial.println("Trying reset...");
    
    // Manual reset procedure
    digitalWrite(NRF_CE_PIN, LOW);
    digitalWrite(NRF_CSN_PIN, HIGH);
    delay(100);
    radio.begin();
  }
  // SPI.endTransaction();

  Serial.println("radio initialized!");

  // --- RADIO SETTINGS TO MATCH RX ---
  radio.setPALevel(RF24_PA_HIGH);
  radio.setDataRate(RF24_250KBPS);
  radio.setChannel(108);
  radio.setCRCLength(RF24_CRC_16);
  radio.enableDynamicPayloads();
  radio.enableAckPayload();
  // radio.setRetries(3, 5); // 3 retries, 5*250us delay
  radio.setRetries(10, 15); // 10 retries, 15*250us delay

  radio.openWritingPipe(address[0]); // Send to RX
  radio.stopListening(); // TX mode

  if (!radio.isChipConnected()) {
    Serial.println("Chip disconnected!");
    return false;
  } else {
    Serial.println("Chip still connected");
    return true;
  }
}
