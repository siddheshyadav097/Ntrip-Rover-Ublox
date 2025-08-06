/**
 *  \file sensor_port.h
 *  \brief Includes serial port and analog sensor initialization and acquire sensor data.
 */

#ifndef __SENSOR_PORT_H__
#define __SENSOR_PORT_H__


 /*----includes-----*/
#include "lib_port.h"
//#include "mcu_port.h"

 /*----macros-----*/
#define ANALOG_SENSOR_PORT      ADC1

#define ANALOG_INPUT_CH1       ADC_CHANNEL_6
#define ANALOG_INPUT_CH2       ADC_CHANNEL_7
#define ANALOG_EMGR_CH1        ADC_CHANNEL_8
//#define ANALOG_EMGR_CH2        ADC_CHANNEL_9

 /*----typedefs-----*/

 /*----public function declarations-----*/

void InitAnalogADCPort(void);
void ReadAnalogChannels(void);
void TriggerSensorADC(void);
void ReadSensorValues(uint32_t* sensorAdcValPtr);
//void SensorDMA1Init(void);
//void GetSerialSensorData(uint8_t *serial_sensordata, int32_t datalength);
//void GetSerialRFIDSensorData(uint8_t *rfid_sensordata, int32_t rfiddatalength);
//void SerialControlConfig(uint8_t disableFlag);
//void WriteSerialData(unsigned char *buf,int len);

#endif