/*
Usage:
#include "can_victron.h"

CanVictron can = CanVictron();

void setup() {
  can.init(GPIO_NUM_5, GPIO_NUM_35);       ////TX, RX, returns true or false
  }

void loop() {  
  can.set_chargevoltagelimit(14.9); 
  can.send_messages();      //send can messages
  }
*/

#ifndef CAN_VICTRON_H
#define CAN_VICTRON_H
#include <Arduino.h>
#include <driver/twai.h>

#define PRODUCT_NAME "EnergieBuam CAN"      //max 15 caracters

class CanVictron {
    private:
        bool _send_canbus_message(uint32_t identifier, uint8_t *buffer, uint8_t length);
        void _message_351(void);
        void _message_370_371(void);
        void _message_35e(void);
        void _message_35a(void);
        void _message_372(void);
        void _message_35f(void);
        void _message_355(void);
        void _message_356(void);
        void _message_373(void);
        void _message_374_375_376_377(void);

        struct bms_data_struct {  
        //0x371, 0x372, 0x35e
        char      devicename[16] = PRODUCT_NAME;   //

        //0x351
        float     chargevoltagelimit = 61.2;            //[V] 0x351, 0-1
        float     maxchargecurrent = 110.4;             //[A] 0x351, 2-3
        float     maxdischargecurrent = 144.8;          //[A] 0x351, 4-5
        float     dischargevoltage = 32.4;              //[V] 0x351, 6-7

        //0x35a
        bool      alarmGeneral = false;                 //0x35a, 0, 0-1
        bool      alarmHighVoltage = false;             //0x35a, 0, 2-3
        bool      alarmLowVoltage = false;              //0x35a, 0, 4-5
        bool      alarmHighTemperature = false;         //0x35a, 0, 6-7
        bool      alarmLowTemeprature = false;          //0x35a, 1, 0-1
        bool      alarmHighTemperatureCharge = false;   //0x35a, 1, 2-3
        bool      alarmLowTemperatureCharge = false;    //0x35a, 1, 4-5
        bool      alarmHighCurrentDischarge = false;    //0x35a, 1, 6-7
        bool      alarmHighCurrentCharge = false;       //0x35a, 2, 0-1
        bool      alarmContactor = false;               //0x35a, 2, 2-3
        bool      alarmShortCircuit = false;            //0x35a, 2, 4-5
        bool      alarmBMSInternal = false;             //0x35a, 2, 6-7
        bool      alarmCellImbalance = false;           //0x35a, 3, 0-1
        bool      warningGeneral = false;               //0x35a, 4, 0-1
        bool      warningighVoltage = false;            //0x35a, 4, 2-3
        bool      warningLowVoltage = false;            //0x35a, 4, 4-5
        bool      warningHighTemperature = false;       //0x35a, 4, 6-7
        bool      warningLowTemeprature = false;        //0x35a, 5, 0-1
        bool      warningHighTemperatureCharge = false; //0x35a, 5, 2-3
        bool      warningLowTemperatureCharge = false;  //0x35a, 5, 4-5
        bool      warningHighCurrentDischarge = false;  //0x35a, 5, 6-7
        bool      warningHighCurrentCharge = false;     //0x35a, 6, 0-1
        bool      warningContactor = false;             //0x35a, 6, 2-3
        bool      warningShortCircuit = false;          //0x35a, 6, 4-5
        bool      warningBMSInternal = false;           //0x35a, 6, 6-7
        bool      warningCellImbalance = false;         //0x35a, 7, 0-1
        bool      systenStatusOnline = true;            //0x35a, 7, 2-3

        //0x372
        uint16_t  numberofmodulesok = 16;               //0x372 0-1 Total number of cells
        uint16_t  numberofmodulesblockingcharge = 0;    //0x372 2-3 
        uint16_t  numberofmodulesblockingdischarge = 0; //0x372 4-5
        uint16_t  numberofmodulesoffline = 0;           //0x372 6-7

        //0x35f
        uint16_t  batterymodel = 123;                    //0x35f, 0-1: Not used
        uint8_t   firmwareversionpre = 1;                //0x35f, 3: 1.xx Prefix
        uint8_t   firmwareversionpost = 0;               //0x35f, 2: xx.00 Suffix
        uint16_t  onlinecapacityinAh = 280;              //[Ah] 0x35f, 5-6

        //0x355
        uint16_t  stateofchargevalue = 99;                //[%] 0x355, 0-1
        uint16_t  stateofhealthvalue = 100;               //[%] 0x355, 2-3
        uint16_t  highresolutionsoc = 125;                //[%] 0x355, 4-5

        //0x356
        float     batteryvoltage = 66.8;                  //[V] 0x356, 0-1
        float     batterycurrent = 16.9;                  //[A] 0x356, 2-3
        float     batterytemperature = 27.9;              //[degC] 0x356, 4-5

        //0x373
        float     mincellvoltage = 3.06;                  //[V] 0x373, 0-1
        float     maxcellvoltage = 3.644;                 //[V] 0x373, 2-3
        uint16_t  lowestcelltemperature = 23.8;           //[degC] 0x373, 4-5
        uint16_t  highestcelltemperature = 26.9;          //[degC] 0x373, 6-7

        //0x374, 0x375, 0x376, 0x377
        uint16_t  lowestcellvoltagenumber = 4;            //0x374
        uint16_t  highestcellvoltagenumber = 9;           //0x375
        uint16_t  lowestcelltemperaturenumber = 1;        //0x376
        uint16_t  highestcelltemperaturenumber = 14;      //0x377
        };
        bms_data_struct _bms_data;
		
		unsigned long	_last_message_sent_millis;
		uint16_t		_time_between_messages_ms = 0;

    public:
        CanVictron(void);
        bool init(gpio_num_t portTX, gpio_num_t portRX);
        bool init(void);
		
		void set_time_between_messages(float value)					{_bms_data._time_between_messages_ms = value * 1000;}
		
        bool send_messages(void);

        //void set_devicename(uint8_t *buffer)                      {_bms_data.devicename = buffer;}
        void set_chargevoltagelimit(float value)                    {_bms_data.chargevoltagelimit = value;}
        void set_maxchargecurrent(float value)                      {_bms_data.maxchargecurrent = value;}
        void set_maxdischargecurrent(float value)                   {_bms_data.maxdischargecurrent = value;}
        void set_dischargevoltage(float value)                      {_bms_data.dischargevoltage = value;}

        void set_alarmGeneral(bool value)                           {_bms_data.alarmGeneral = value;}
        void set_alarmHighVoltage(bool value)                       {_bms_data.alarmHighVoltage = value;}
        void set_alarmLowVoltage(bool value)                        {_bms_data.alarmLowVoltage = value;}
        void set_alarmHighTemperature(bool value)                   {_bms_data.alarmHighTemperature = value;}
        void set_alarmLowTemeprature(bool value)                    {_bms_data.alarmLowTemeprature = value;}
        void set_alarmHighTemperatureCharge(bool value)             {_bms_data.alarmHighTemperatureCharge = value;}
        void set_alarmLowTemperatureCharge(bool value)              {_bms_data.alarmLowTemperatureCharge = value;}
        void set_alarmHighCurrentDischarge(bool value)              {_bms_data.alarmHighCurrentDischarge = value;}
        void set_alarmHighCurrentCharge(bool value)                 {_bms_data.alarmHighCurrentCharge = value;}
        void set_alarmContactor(bool value)                         {_bms_data.alarmContactor = value;}
        void set_alarmShortCircuit(bool value)                      {_bms_data.alarmShortCircuit = value;}
        void set_alarmBMSInternal(bool value)                       {_bms_data.alarmBMSInternal = value;}
        void set_alarmCellImbalance(bool value)                     {_bms_data.alarmCellImbalance = value;}
        void set_warningGeneral(bool value)                         {_bms_data.warningGeneral = value;}
        void set_warningighVoltage(bool value)                      {_bms_data.warningighVoltage = value;}
        void set_warningLowVoltage(bool value)                      {_bms_data.warningLowVoltage = value;}
        void set_warningHighTemperature(bool value)                 {_bms_data.warningHighTemperature = value;}
        void set_warningLowTemeprature(bool value)                  {_bms_data.warningLowTemeprature = value;}
        void set_warningHighTemperatureCharge(bool value)           {_bms_data.warningHighTemperatureCharge = value;}
        void set_warningLowTemperatureCharge(bool value)            {_bms_data.warningLowTemperatureCharge = value;}
        void set_warningHighCurrentDischarge(bool value)            {_bms_data.warningHighCurrentDischarge = value;}
        void set_warningHighCurrentCharge(bool value)               {_bms_data.warningHighCurrentCharge = value;}
        void set_warningContactor(bool value)                       {_bms_data.warningContactor = value;}
        void set_warningShortCircuit(bool value)                    {_bms_data.warningShortCircuit = value;}
        void set_warningBMSInternal(bool value)                     {_bms_data.alarmGeneral = value;}
        void set_warningCellImbalance(bool value)                   {_bms_data.warningCellImbalance = value;}
        void set_systenStatusOnline(bool value)                     {_bms_data.systenStatusOnline = value;}

        void set_numberofmodulesok(uint16_t value)                  {_bms_data.numberofmodulesok = value;}
        void set_numberofmodulesblockingcharge(uint16_t value)      {_bms_data.numberofmodulesblockingcharge = value;}
        void set_numberofmodulesblockingdischarge(uint16_t value)   {_bms_data.numberofmodulesblockingdischarge = value;}
        void set_numberofmodulesoffline(uint16_t value)             {_bms_data.numberofmodulesoffline = value;}

        void set_batteryModel(uint16_t value)                       {_bms_data.batterymodel = value;}
        void set_firmwareversionPre(uint8_t value)                  {_bms_data.firmwareversionpre = value;}
        void set_firmwareversionPost(uint8_t value)                 {_bms_data.firmwareversionpost = value;}
        void set_onlinecapacityinAh(uint16_t value)                 {_bms_data.onlinecapacityinAh = value;}

        void set_stateofchargevalue(uint16_t value)                 {_bms_data.stateofchargevalue = value;}
        void set_stateofhealthvalue(uint16_t value)                 {_bms_data.stateofhealthvalue = value;}
        void set_highresolutionsoc(uint16_t value)                  {_bms_data.highresolutionsoc = value;}

        void set_batteryvoltage(float value)                        {_bms_data.batteryvoltage = value;}
        void set_batterycurrent(float value)                        {_bms_data.batterycurrent = value;}
        void set_batterytemperature(float value)                    {_bms_data.batterytemperature = value;}

        void set_mincellvoltage(float value)                        {_bms_data.mincellvoltage = value;}
        void set_maxcellvoltage(float value)                        {_bms_data.maxcellvoltage = value;}
        void set_lowestcelltemperature(uint16_t value)              {_bms_data.lowestcelltemperature = value;}
        void set_highestcelltemperature(uint16_t value)             {_bms_data.highestcelltemperature = value;}

        void set_lowestcellvoltagenumber(uint16_t value)            {_bms_data.lowestcellvoltagenumber = value;}
        void set_highestcellvoltagenumber(uint16_t value)           {_bms_data.highestcellvoltagenumber = value;}
        void set_lowestcelltemperaturenumber(uint16_t value)        {_bms_data.lowestcelltemperaturenumber = value;}
        void set_highestcelltemperaturenumber(uint16_t value)       {_bms_data.highestcelltemperaturenumber = value;}

    };

#endif