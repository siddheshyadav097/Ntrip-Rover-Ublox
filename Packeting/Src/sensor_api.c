/**
 *  @file          :  sensor_api.c
 *  @author        :  Aakash/Ratna
 *  @date          :  15/06/2017
 *  @brief         :  Includes definitions of APIs for serial init, ADC config and acquiring, decoding & providing the sensor data.
 *  @filerevision  :  1.0
 *  
 */

 
 /*----includes-----*/
#include <math.h>
#include "sensor_api.h"
#include "lib_api.h"
#include "qtimespent.h" 
#include "debug_log.h"
#include "bat_api.h"

//format for getting data from serial port is $SEN,14.27,9.27,*48

//#define PRESET_VALUE 0xFFFF
//#define POLYNOMIAL  0x8408
   
 /*----variables-----*/
float sensorVoltage[NUM_OF_ANALOG_INPUTS];
uint32_t sensorSampleStartTick = 0; 
uint16_t totalSensorCount[NUM_OF_ANALOG_INPUTS];
uint32_t sensorAdcVal[NUM_OF_ANALOG_INPUTS];
uint16_t sensorAdcAvgResult[NUM_OF_ANALOG_INPUTS];
uint8_t sensorSampleCounter = 0;
EmergencyStatus_e emgrState1 = EMERGENCY_OFF;
//EmergencyStatus_e emgrState2 = EMERGENCY_OFF;
AnalogData_t analogData;
SupplyInfo_st* supplyVoltage;
uint8_t sensorCalib = 0;

uint8_t sensorScanCompleted = RESET;
 /*----private functions-----*/
void ConvertToSensorVoltages(void)
{
    static uint8_t j = 0;
    
    for(j = 0; j < NUM_OF_ANALOG_INPUTS; j++)
    {
        sensorVoltage[j] = ComputeVolatge(sensorAdcAvgResult[j]);
//      sensorVoltage[j] = (sensorAdcAvgResult[j]/ADC1_12BIT_RESOLUTION)*ADC1_VREF_VOLTAGE;
//      sensorVoltage[j] = (sensorVoltage[j]*SUPPLY_VOLTAGE_RATIO);
    }
}

 /*----public functions-----*/
 
void InitAnalogSensors(void)
{
  memset(&analogData,0,sizeof(AnalogData_t));
  sensorSampleStartTick = GetStartTime();
  emgrState1 = EMERGENCY_OFF;
//emgrState2 = EMERGENCY_OFF;
  InitAnalogADCPort();
}



void AnalogDataHandler(void)
{
  static uint8_t i = 0;
  
  // average and sample Analog adc values
    if(TimeSpent(sensorSampleStartTick,SENSOR_SAMPLE_INTERVAL_MS))      //100 ms
    {
        TriggerSensorADC();  // start the sensor adc
        
        sensorSampleStartTick = GetStartTime();
        
        if(sensorScanCompleted == SET)   //this flag is set in the callback of the adc 
        {
            ReadSensorValues(sensorAdcVal);
            
            for(i = 0; i < NUM_OF_ANALOG_INPUTS; i++)
            {
              totalSensorCount[i] += sensorAdcVal[i];
            }
            sensorSampleCounter++; //avg data sampling counter
            sensorCalib = 0;   //make it 0 after taking the desired averages make it 1
            
            if(sensorSampleCounter >= NUM_SENSOR_SAMPLES_AVG)
            {
                for(i = 0; i < NUM_OF_ANALOG_INPUTS; i++)
                {
                  sensorAdcAvgResult[i] = totalSensorCount[i]/sensorSampleCounter;
                }
                sensorSampleCounter = 0;
                
                memset(totalSensorCount,0,sizeof(totalSensorCount));
                memset(&analogData,0,sizeof(AnalogData_t));
                
                ConvertToSensorVoltages();
                EmergencyStateHandler();
                
                sensorScanCompleted  = RESET;
                
                sensorCalib = 1;
            }
        }
//        ReadAnalogChannels();
    }
}

void EmergencyStateHandler(void)
{
      float emrvoltage = 0.0;
      float emrThresholdVtg = 0.0;
      
      supplyVoltage = GetSupplyInfo();
      
      emrThresholdVtg = (66.0/989)* supplyVoltage->mainIPVolt;
     
      emrvoltage =  sensorVoltage[2];
      if(emrvoltage <= EMERGENCY_LOWER_THRESHOLD)
      {
         emgrState1 = EMERGENCY_TAMPERED;
      }
      else if(emrvoltage <= emrThresholdVtg)
      {
        emgrState1 = EMERGENCY_ON;
      }
      else if(emrvoltage >= emrThresholdVtg)
      {
         emgrState1 = EMERGENCY_OFF;
      }
}

AnalogData_t* GetAnalogSensorData(void)
{
  analogData.analogData1 = sensorVoltage[0];
  analogData.analogData2 = sensorVoltage[1];
  analogData.emergencyState1  = sensorVoltage[2];
//analogData.emergencyState2  = sensorVoltage[3];
  analogData.emergencyStatus1 = emgrState1;
//analogData.emergencyStatus2 = emgrState2;
  return(&analogData);
}


