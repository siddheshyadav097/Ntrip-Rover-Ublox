/**
 *  \file sensor_api.h
 *  \brief Includes typedefs for analog input data and public function declarations for serial init, ADC config and 
 *  	   acquiring, decoding and providing the sensor data.
 */

#ifndef __SENSOR_API_H__
#define __SENSOR_API_H__


 /*----includes-----*/
#include "sensor_port.h"

/*----macros-------*/
#define NUM_OF_ANALOG_INPUTS           3  //4
#define SENSOR_SAMPLE_INTERVAL_MS      100//2
#define NUM_SENSOR_SAMPLES_AVG         10//3

//#define ADC1_12BIT_RESOLUTION          4096
//#define ADC1_VREF_VOLTAGE              3.3
//#define SUPPLY_VOLTAGE_RATIO           4      //24v vehiclebatt/3v adc supply voltage

#define EMERGENCY_UPPER_THRESHOLD      10
#define EMERGENCY_LOWER_THRESHOLD      0.1

 /*----typedefs-----*/
typedef enum
{
	EMERGENCY_OFF,
	EMERGENCY_ON,
    EMERGENCY_TAMPERED
	
}EmergencyStatus_e;

typedef struct
{
      float analogData1;
      float analogData2;
      float emergencyState1;
//    float emergencyState2;
      EmergencyStatus_e emergencyStatus1;
//    uint8_t emergencyStatus2;
	
}AnalogData_t;


 /*----public function declarations-----*/
void InitAnalogSensors(void);
void AnalogDataHandler(void);
void EmergencyStateHandler(void);
AnalogData_t* GetAnalogSensorData(void);

//BOOL Get_Sensor_Data(AnalogData_t *analogDataPtr);
//BOOL Get_RFID_Data(RfidData_t *rfidDataPtr);

#endif