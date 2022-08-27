#include <Arduino.h>
#include "can_victron.h"

CanVictron can = CanVictron();

void setup() {
  //init serial connection
  Serial.begin(115200);
  Serial.println("Demo - CAN for Victron BMS");
 
  //init can
  if (can.init(GPIO_NUM_5, GPIO_NUM_35)) { //TX, RX
    can.set_time_between_messages(2.0);
    Serial.println("CAN initialised");
    }
  else
    Serial.println("CAN failed!");
  }

void loop() {  
  Serial.println("TICK");

  //set some values
  can.set_chargevoltagelimit(14.9);
  can.set_stateofchargevalue((millis()/1000)%100);

  //send can messages
  if (can.send_messages())
    Serial.println("CAN MESSAGE SENT");

  //some delay
  delay(100);
  }