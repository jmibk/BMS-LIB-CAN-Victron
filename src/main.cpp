#include <Arduino.h>
#include "can_victron.h"

CanVictron can = CanVictron();

void setup() {
  //init serial connection
  Serial.begin(115200);
  Serial.println("Demo - CAN for Victron BMS");
 
  //init can
  if (can.init(GPIO_NUM_5, GPIO_NUM_35))  //TX, RX
    Serial.println("CAN initialised");
  else
    Serial.println("CAN failed!");
  }

void loop() {  
  //set some values
  can.set_chargevoltagelimit(14.9);

  //send can messages
  can.send_messages();

  //some delay
  delay(1000);
  }