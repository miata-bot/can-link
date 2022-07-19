#include <Arduino.h>
#include <RFM69.h>
#include <RFM69registers.h>
#include "HexDump.h"

#if defined(ARDUINO_SAMD_ZERO) && defined(SERIAL_PORT_USBVIRTUAL)
// Required for Serial on Zero based boards
#define Serial SERIAL_PORT_USBVIRTUAL
#endif

#if defined(ARDUINO_SAMD_FEATHER_M0)
  #define RFM69_CS 8
  #define RFM69_INT 3
  #define RFM69_RST 4
  #define LED 13
#endif

#define NODEID        2
#define NETWORKID     100
#define GATEWAYID     1
#define FREQUENCY     RF69_915MHZ
// #define ENCRYPTKEY "sampleEncryptKey"

// Radio object
RFM69 radio(RFM69_CS, RFM69_INT);

void radioConfigure() {
  radio.initialize(FREQUENCY,NODEID,NETWORKID);
  radio.setHighPower(); //must include this only for RFM69HW/HCW!
  // radio.encrypt(ENCRYPTKEY);
}

void radioReset()
{
  // manual reset
  digitalWrite(RFM69_RST, HIGH);
  delay(10);
  digitalWrite(RFM69_RST, LOW);
  delay(10);
}

void radioBlink(byte PIN, int DELAY_MS)
{
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN,HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN,LOW);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB
  }
  pinMode(RFM69_RST, OUTPUT);
  digitalWrite(RFM69_RST, LOW);
  Serial.println("Feather RFM69 RX Test!\r\n");

  radioReset();
  radioConfigure();
  pinMode(LED, OUTPUT);
}

uint8_t payload[4] = {0, 0, 0, 0}; 

void loop() {
  // put your main code here, to run repeatedly:

  while(Serial.available()) {
    uint8_t inByte = Serial.read();
    if(inByte == 49) {
      Serial.println("set_color(red)");
      // payload[0] = 4;
      // payload[1] = 255;
      // payload[2] = 0;
      // payload[3] = 0;
      char payload[] = {0x20, 0x0, 0xff, 0x00, 0x00, 0x00};
      radio.send(0, &payload, 6, false);
    }

    if(inByte == 50) {
      char payload[] = {0x11};
      radio.send(0, &payload, 1, false);
    }

    if(inByte == 51) {
      uint16_t new_leader_id = 1;
      char payload[] = {0x10, 0, 0};
      payload[2] = new_leader_id & 0xFF;
      payload[1] = (new_leader_id >> 8);
      radio.send(0, &payload, 3, false);
    } 

    if(inByte == 52) {
      uint16_t leader_id = 2;
      char payload[] = {0x12, 0, 0};
      payload[2] = leader_id & 0xFF;
      payload[1] = (leader_id >> 8);
      radio.send(0, payload, 3, false);
    }
  }
  if(radio.receiveDone()) {
    Serial.print('[');Serial.print(radio.SENDERID);Serial.print("] ");
    // Serial.print((char*)radio.DATA);
    HexDump(Serial, radio.DATA, radio.DATALEN);
    Serial.print("   [RX_RSSI:");Serial.print(radio.RSSI);Serial.print("]");

    if (radio.ACKRequested())
    {
      radio.sendACK();
      Serial.println(" - ACK sent");
    } else {
      Serial.println();
    }

    // leader request
    if(radio.DATA[0] == 0x11) {
      uint16_t leader_id = 2;
      char payload[] = {0x12, 0, 0};
      payload[2] = leader_id & 0xFF;
      payload[1] = (leader_id >> 8);
      radio.send(0, payload, 3, false);
    }
  }
  radio.receiveDone();
  Serial.flush(); 
  delay(10);
}