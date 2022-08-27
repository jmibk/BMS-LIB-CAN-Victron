#include "can_victron.h"

//constructor - does nothing
CanVictron::CanVictron(void) {
    }

bool CanVictron::init(gpio_num_t portTX=GPIO_NUM_5, gpio_num_t portRX=GPIO_NUM_35) {
    //Initialize configuration structures using macro initializers
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(portTX, portRX, TWAI_MODE_NORMAL);
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    //Install TWAI driver
    if (twai_driver_install(&g_config, &t_config, &f_config) != ESP_OK) 
        return false;

    //Start TWAI driver
    if (twai_start() != ESP_OK) 
        return false;

    return true;
    }

bool CanVictron::init() {
    return init(GPIO_NUM_5, GPIO_NUM_35);
    }
    
bool CanVictron::_send_canbus_message(uint32_t identifier, uint8_t *buffer, uint8_t length) {
    twai_message_t message;
    message.identifier = identifier;
    message.flags = TWAI_MSG_FLAG_NONE;
    message.data_length_code = length;
    memcpy(&message.data, buffer, length);

    // Queue message for transmission
    if (twai_transmit(&message, pdMS_TO_TICKS(250)) != ESP_OK) 
        return false;

    return true;
    }

void CanVictron::_message_351(void) {
    struct data351 {  
        uint16_t chargevoltagelimit;  // CVL   
        int16_t maxchargecurrent;     // CCL
        int16_t maxdischargecurrent;  // DCL
        uint16_t dischargevoltage;  // Not currently used by Victron
    };
    data351 data;

    data.chargevoltagelimit = _bms_data.chargevoltagelimit * 10;
    data.maxchargecurrent = _bms_data.maxchargecurrent * 10;
    data.maxdischargecurrent = _bms_data.maxdischargecurrent * 10;
    data.dischargevoltage = _bms_data.dischargevoltage * 10;

    _send_canbus_message(0x351, (uint8_t *)&data, sizeof(data351));
    }

// Transmit the device name via two CAN Messages
void CanVictron::_message_370_371(void) {
    _send_canbus_message(0x370, (uint8_t *)&_bms_data.devicename, 8);
    _send_canbus_message(0x371, (uint8_t *)&_bms_data.devicename[8], 8);
    }

void CanVictron::_message_35e() {
    _send_canbus_message(0x35e, (uint8_t *)&_bms_data.devicename, 6);
    }

// Send alarm details to Victron over CANBUS
void CanVictron::_message_35a(void) {
    struct data35a {
        uint8_t byte0;
        uint8_t byte1;
        uint8_t byte2;
        uint8_t byte3;
        uint8_t byte4;
        uint8_t byte5;
        uint8_t byte6;
        uint8_t byte7;
    };
    data35a data;

    memset(&data, 0, sizeof(data35a));

    // B00 = Alarm not supported
    // B10 = Alarm/warning active
    // B01 = Alarm/warning inactive (status = OK)

    // These constants are actually bit swapped compared to the notes above
    // as the Victron kit uses little-endian ordering when transmitting on CANBUS
    const uint8_t BIT01_ALARM = B00000001;
    const uint8_t BIT23_ALARM = B00000100;
    const uint8_t BIT45_ALARM = B00010000;
    const uint8_t BIT67_ALARM = B01000000;

    const uint8_t BIT01_OK = B00000010;
    const uint8_t BIT23_OK = B00001000;
    const uint8_t BIT45_OK = B00100000;
    const uint8_t BIT67_OK = B10000000;

    // BYTE 0
    data.byte0 |= _bms_data.alarmGeneral?BIT01_ALARM:BIT01_OK;                 //(bit 0+1) General alarm (not implemented)
    data.byte0 |= _bms_data.alarmHighVoltage?BIT23_ALARM:BIT23_OK;             //(bit 2+3) Battery high voltage alarm
    data.byte0 |= _bms_data.alarmLowVoltage?BIT45_ALARM:BIT45_OK;              //(bit 4+5) Battery low voltage alarm
    data.byte0 |= _bms_data.alarmHighTemperature?BIT67_ALARM:BIT67_OK;         //(bit 6+7) Battery high temperature alarm
        
    // BYTE 1
    data.byte1 |= _bms_data.alarmLowTemeprature?BIT01_ALARM:BIT01_OK;          // 1 (bit 0+1) Battery low temperature alarm
    data.byte1 |= _bms_data.alarmHighTemperatureCharge?BIT23_ALARM:BIT23_OK;   // 1 (bit 2+3) Battery high temperature charge alarm
    data.byte1 |= _bms_data.alarmLowTemperatureCharge?BIT45_ALARM:BIT45_OK;    // 1 (bit 4+5) Battery low temperature charge alarm
    data.byte1 |= _bms_data.alarmHighCurrentDischarge?BIT67_ALARM:BIT67_OK;    // 1 (bit 6+7) Battery high current alarm

    // BYTE 2
    data.byte2 |= _bms_data.alarmHighCurrentCharge?BIT01_ALARM:BIT01_OK;       // 2 (bit 0+1) Battery high charge current alarm
    data.byte2 |= _bms_data.alarmContactor?BIT23_ALARM:BIT23_OK;               // 2 (bit 2+3) Contactor Alarm (not implemented)
    data.byte2 |= _bms_data.alarmShortCircuit?BIT45_ALARM:BIT45_OK;            // 2 (bit 4+5) Short circuit Alarm (not implemented)
    data.byte2 |= _bms_data.alarmBMSInternal?BIT67_ALARM:BIT67_OK;             // 2 (bit 6+7) BMS internal alarm

    // BYTE 3
    data.byte3 |= _bms_data.alarmCellImbalance?BIT01_ALARM:BIT01_OK;           // 3 (bit 0+1) Cell imbalance alarm
    // 3 (bit 2+3) Reserved
    // 3 (bit 4+5) Reserved
    // 3 (bit 6+7) Reserved

    // BYTE 4
    data.byte4 |= _bms_data.warningGeneral?BIT01_ALARM:BIT01_OK;               // 4 (bit 0+1) General warning (not implemented)
    data.byte4 |= _bms_data.warningighVoltage?BIT23_ALARM:BIT23_OK;            // 4 (bit 2+3) Battery low voltage warning
    data.byte4 |= _bms_data.warningLowVoltage?BIT45_ALARM:BIT45_OK;            // 4 (bit 4+5) Battery high voltage warning
    data.byte4 |= _bms_data.warningHighTemperature?BIT67_ALARM:BIT67_OK;       // 4 (bit 6+7) Battery high temperature warning

    // BYTE 5
    data.byte5 |= _bms_data.warningLowTemeprature?BIT01_ALARM:BIT01_OK;        // 5 (bit 0+1) Battery low temperature warning
    data.byte5 |= _bms_data.warningHighTemperatureCharge?BIT23_ALARM:BIT23_OK; // 5 (bit 2+3) Battery high temperature charge warning
    data.byte5 |= _bms_data.warningLowTemperatureCharge?BIT45_ALARM:BIT45_OK;  // 5 (bit 4+5) Battery low temperature charge warning
    data.byte5 |= _bms_data.warningHighCurrentDischarge?BIT67_ALARM:BIT67_OK;           // 5 (bit 6+7) Battery high current warning

    // BYTE 6
    data.byte6 |= _bms_data.warningHighCurrentCharge?BIT01_ALARM:BIT01_OK;     // 6 (bit 0+1) Battery high charge current warning
    data.byte6 |= _bms_data.warningContactor?BIT23_ALARM:BIT23_OK;             // 6 (bit 2+3) Contactor warning (not implemented)
    data.byte6 |= _bms_data.warningShortCircuit?BIT45_ALARM:BIT45_OK;          // 6 (bit 4+5) Short circuit warning (not implemented)
    data.byte6 |= _bms_data.warningBMSInternal?BIT67_ALARM:BIT67_OK;           // 6 (bit 6+7) BMS internal warning

    // BYTE 7
    data.byte7 |= _bms_data.warningCellImbalance?BIT01_ALARM:BIT01_OK;         // 7 (bit 0+1) Cell imbalance warning
    data.byte7 |= _bms_data.systenStatusOnline?BIT23_ALARM:BIT23_OK;           // 7 (bit 2+3) System status (online/offline) [1]
    // 7 (rest) Reserved
                    
    _send_canbus_message(0x35a, (uint8_t *)&data, sizeof(data35a));
    }

void CanVictron::_message_372(void) {
    struct data372 {
        uint16_t numberofmodulesok;
        uint16_t numberofmodulesblockingcharge;
        uint16_t numberofmodulesblockingdischarge;
        uint16_t numberofmodulesoffline;
    };
    data372 data;

    data.numberofmodulesok = _bms_data.numberofmodulesok;                  //TotalNumberOfCells
    data.numberofmodulesblockingcharge = _bms_data.numberofmodulesblockingcharge;
    data.numberofmodulesblockingdischarge = _bms_data.numberofmodulesblockingdischarge;
    data.numberofmodulesoffline = _bms_data.numberofmodulesoffline;

    _send_canbus_message(0x372, (uint8_t *)&data, sizeof(data372));
    }

void CanVictron::_message_35f(void) {
    struct data35f {  
        uint16_t BatteryModel;        // Not used
        uint16_t Firmwareversion;
        uint16_t OnlinecapacityinAh;
    };
    data35f data;

    // Need to swap bytes for this to make sense.
    data.BatteryModel = _bms_data.batterymodel;
    data.Firmwareversion = ((uint16_t)_bms_data.firmwareversionpost << 8) | _bms_data.firmwareversionpre;  
    data.OnlinecapacityinAh = _bms_data.onlinecapacityinAh;

    _send_canbus_message(0x35f, (uint8_t *)&data, sizeof(data35f));
    }

// S.o.C value
void CanVictron::_message_355(void) {
    struct data355 {
        uint16_t stateofchargevalue;
        uint16_t stateofhealthvalue;
        uint16_t highresolutionsoc;
    };
    data355 data;
    
    data.stateofchargevalue = _bms_data.stateofchargevalue;  // 0 SOC value un16 1 %
    data.stateofhealthvalue = _bms_data.stateofhealthvalue;
    data.highresolutionsoc = _bms_data.highresolutionsoc;   // 2 SOH value un16 1 %

    _send_canbus_message(0x355, (uint8_t *)&data, sizeof(data355));
    }

// Battery voltage
void CanVictron::_message_356(void) {
  struct data356 {
    int16_t batteryvoltage;
    int16_t batterycurrent;
    int16_t batterytemperature;
  };
  data356 data;

  // Use highest pack voltage calculated by controller and modules
  data.batteryvoltage = _bms_data.batteryvoltage * 100; 
  data.batterycurrent = _bms_data.batterycurrent * 10;   
  data.batterytemperature = _bms_data.batterytemperature * (int16_t)10;
 
  _send_canbus_message(0x356, (uint8_t *)&data, sizeof(data356));
  }

 void CanVictron::_message_373(void) {
    struct data373 {
        uint16_t mincellvoltage;
        uint16_t maxcellvoltage;
        uint16_t lowestcelltemperature;
        uint16_t highestcelltemperature;
    };
    data373 data;

    data.mincellvoltage = _bms_data.mincellvoltage * 1000;
    data.maxcellvoltage = _bms_data.maxcellvoltage * 1000;
    data.lowestcelltemperature = 273 + _bms_data.lowestcelltemperature;
    data.highestcelltemperature = 273 + _bms_data.highestcelltemperature;

    _send_canbus_message(0x373, (uint8_t *)&data, sizeof(data373));
    }

void CanVictron::_message_374_375_376_377(void) {
    struct candata   {
        char text[8];
    };
    candata data;

    //0x374
    memset(data.text, 0, 8);   // Clear all 8 bytes
    snprintf(data.text, 8, "Cell %d", _bms_data.lowestcellvoltagenumber);
    _send_canbus_message(0x374, (uint8_t *)&data, sizeof(candata));

    //0x375
    memset(data.text, 0, 8);   // Clear all 8 bytes
    snprintf(data.text, 8, "Cell %d", _bms_data.highestcellvoltagenumber);
    _send_canbus_message(0x375, (uint8_t *)&data, sizeof(candata));

    //0x376
    memset(data.text, 0, 8);   // Clear all 8 bytes
    snprintf(data.text, 8, "Cell %d", _bms_data.lowestcelltemperaturenumber);
    _send_canbus_message(0x376, (uint8_t *)&data, sizeof(candata));

    //0x377
    memset(data.text, 0, 8);   // Clear all 8 bytes
    snprintf(data.text, 8, "Cell %d", _bms_data.highestcelltemperaturenumber);
    _send_canbus_message(0x377, (uint8_t *)&data, sizeof(candata));
    }

bool CanVictron::send_messages(void) {
	// watch out if time to send
	if ( millis() < (_last_message_sent_millis + _time_between_messages_ms) ) {
		return false;
		}
	_last_message_sent_millis = millis();

    // minimum CAN-IDs required for the core functionality are 0x351, 0x355, 0x356 and 0x35A.
    // 351 message must be sent at least every 3 seconds - or Victron will stop charge/discharge
    _message_351();

    // Delay a little whilst sending packets to give ESP32 some breathing room and not flood the CANBUS
    //delay(100);

    // Advertise the diyBMS name on CANBUS
    _message_370_371();
    _message_35e();
    _message_35a();
    _message_372();
    _message_35f();

    //delay(100);
    _message_355();
    _message_356();

    //delay(100);

    // Detail about individual cells
    _message_373();
    _message_374_375_376_377();

    //return
	return true;
    }