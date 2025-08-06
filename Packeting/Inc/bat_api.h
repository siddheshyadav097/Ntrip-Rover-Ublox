/**
 *  \file bat_api.h
 *  \brief Header file for application to monitor battery status.
 *  	   Includes the type definition for all Battery states possible 
 *  	   and function declarations for APIs provided to the user application.
 */

#ifndef __BAT_API_H
#define __BAT_API_H


 /*----includes-----*/
#include "bat_port.h"

/*----constants-----*/
//#define MIN_BATTERY_ADC_VAL		        2870
//#define MAX_BATTERY_ADC_VAL		        3395



#define ADC_SAMPLE_INTERVAL_MS          50//100
#define NUM_BAT_ADC_SAMPLES_AVG         10

#define BATTERY_MIN_OPERATING_VOLTAGE	3.2//3600
#define BATTERY_MAX_OPERATING_VOLTAGE	4.2//4200
   
#define BATTERY_LOW_THRESHOLD            3.2


#define VEHICLE_BATTERY_LOW_THRESHOLD       8  //3
#define VEHICLE_BATTERY_NORMAL_VOLTAGE      10  //6

   
#define BATTERY_PERCENT_LOW	             15
#define BATTERY_PERCENT_NORMAL           40


#define VDD_APPLI                      (3.3)   /* Value of analog voltage supply Vdda (unit: mV) */
#define RANGE_12BITS                   (4095)   /* Max value for a full range of 12 bits (4096 values) */

#define ComputeVolatge(ADC_DATA)                                          \
  ( (ADC_DATA) * VDD_APPLI / RANGE_12BITS)


 /*----typedefs-----*/
typedef enum
{
    BATTERY_NORMAL,
    BATTERY_LOW,
    BATTERY_CHARGING,
    BATTERY_FULL
}batteryLevelState_et;

typedef enum
{
    VEHICLE_BATTERY_DISCONNECTED,
    VEHICLE_BATTERY_CONNECTED
      
}vehicleBatteryState_et;


typedef enum
{
    RFID_POWER_DISCONNECTED,
    RFID_POWER_CONNECTED
      
}rfidReaderPwrState_et;

typedef  struct
{
	float mainIPVolt;
	float intBatVoltage;
	    float analogIp1;
    float analogIp2;
    float vrefIntVoltage;
	vehicleBatteryState_et mainPwrStatus;
    rfidReaderPwrState_et ReaderPwrStatus;
    batteryLevelState_et intBatStatus;
    uint8_t intBatLevel;
}SupplyInfo_st;

 /*----public function declarations-----*/
void InitBatteryConfig(void);
uint8_t GetBatteryLevel(void);
void GetSupplyVoltages(void);
void BatteryHandler(void);
batteryLevelState_et BatteryStateHandler(uint8_t batteryLevel);
rfidReaderPwrState_et RfidReaderVoltageHandler(void);
vehicleBatteryState_et VehicleBatStateHandler(void);
//BOOL GetSupplyInfo(SupplyInfo_st* supplyInfoPtr);
SupplyInfo_st* GetSupplyInfo(void);
uint8_t MonitorUnitVoltage(void);
#endif
