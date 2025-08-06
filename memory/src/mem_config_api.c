/****************************************************** CONFIG PARAMETERS ********************************************************/
#include <string.h>
#include "mem_config_api.h"
#include "memorydriver.h"
#include "qmath.h"
#include "debug_log.h"




static gsmSimslot_st gsmSimslot;
static intervals_st intervals;
static thresholds_st thresholds;
static emergencyParams_st emergencyParams;
static serverConfig_st server0Config;
static serverConfig_st server1Config;
static gprsConfig_st gprsConfig_sim0;
static gprsConfig_st gprsConfig_sim1;
static aisDataConfig_st aisUnitData;
//static aisDataConfig_st aisNullUnitData = {0};
static rfReaderPwrTh_st rfPwrThresholds;



static void SaveMultipleCopies(uint8_t *data, uint16_t length, uint32_t blockNumberStart);
static uint8_t ReadMultipleCopies(uint8_t *data, uint16_t length, uint32_t blockNumberStart);

static void ReadIntervalsFromFlash(void);	
//static void ReadGsmActiveSimSlot(void);
static void ReadThresholdsFromFlash(void);											
static void ReadEmergencyParamsFromFlash(void);											
static void ReadServer0ConfigFromFlash(void);											
static void ReadServer1ConfigFromFlash(void);											
static void ReadGprsSim0ConfigFromFlash(void);											
static void ReadGprsSim1ConfigFromFlash(void);											
static void ReadAisUnitDataFromFlash(void);
static void ReadRfidPwrThresholdFromFlash(void);

//static void LoadDefaultGsmSimSlot(void);
static void LoadDefaultRfidPwrThreshold(void);
static void LoadDefaultIntervals(void);
static void LoadDefaultThresholds(void);
static void LoadDefaultEmergencyParams(void);
static void LoadDefaultServer0Config(void);
static void LoadDefaultServer1Config(void);
 void LoadDefaultGprsSim0Config(void);
static void LoadDefaultGprsSim1Config(void);
//static void LoadDefaultUnitData(void);

//static void UpdateGsmSimSlotToFlash(void);
static void UpdateRfidReaderPwrThToFlash(void);
static void UpdateIntervalsToFlash(void);
static void UpdateThresholdsToFlash(void);
static void UpdateEmergencyParamsToFlash(void);
static void UpdateServer0ConfigToFlash(void);
static void UpdateServer1ConfigToFlash(void);
static void UpdateGprsSim0ConfigToFlash(void);
static void UpdateGprsSim1ConfigToFlash(void);
static void UpdateAisDefaultUnitData(void);

//extern uint8_t GetVehNumFromIMEI;


/**************************************************************** GET FUNCTIONS*********************************************************/
uint8_t GetGsmActiveSimValue(void)
{
   return gsmSimslot.gsmSimSlotVal;
}
intervals_st* GetIntervals(void)
{
    return (&intervals);
}
thresholds_st* GetThresholds(void)
{
    return(&thresholds);
}
emergencyParams_st* GetEmergencyParams(void)
{
    return(&emergencyParams);
}
serverConfig_st*  GetServer0Config(void)
{
    return(&server0Config);
}
serverConfig_st* GetServer1Config(void)
{
   return(&server1Config);
}
gprsConfig_st* GetGprs0SimConfig(void)
{
   return(&gprsConfig_sim0);
}
gprsConfig_st* GetGprs1SimConfig(void)
{
 return(&gprsConfig_sim1);
}
aisDataConfig_st* GetAisDeviceData(void)
{
//   if(aisUnitData.checksum != 0)
//   {
      return(&aisUnitData);
//   }
//   else
//   {
//     memset(&aisNullUnitData,0,sizeof(aisDataConfig_st));
//     return (&aisNullUnitData);
//   }
}

rfReaderPwrTh_st* GetRfidPwrThValues(void)
{
  return(&rfPwrThresholds);
}

/***************************************************************** SET FUNCTIONS ******************************************************/
//void SetGsmSimSlot0(void)
//{
//   gsmSimslot.gsmSimSlotVal =0;
//   UpdateGsmSimSlotToFlash();
//}
//
//void SetGsmSimSlot1(void)
//{
//   gsmSimslot.gsmSimSlotVal = 1;
//   UpdateGsmSimSlotToFlash();
//}
//void SetGsmSimSlot(uint8_t val)
//{
//    gsmSimslot.gsmSimSlotVal =val;
//    UpdateGsmSimSlotToFlash();
//}

void SetIntervals(intervals_st* data)
{
    memcpy(&intervals, data, sizeof(intervals_st));
    UpdateIntervalsToFlash();
}
void SetThresholds(thresholds_st* data)
{
    memcpy(&thresholds, data, sizeof(thresholds_st));
    UpdateThresholdsToFlash();
}
void SetEmergencyParams(emergencyParams_st* data)
{
    memcpy(&emergencyParams, data, sizeof(emergencyParams_st));
    UpdateEmergencyParamsToFlash();
}
void SetServer0Config(serverConfig_st* data)
{
    memcpy(&server0Config, data, sizeof(serverConfig_st));
    UpdateServer0ConfigToFlash();
}
void SetServer1Config(serverConfig_st* data)
{
    memcpy(&server1Config, data, sizeof(serverConfig_st));
    UpdateServer1ConfigToFlash();
}
void SetGprs0SimConfig(gprsConfig_st* data)
{
    memcpy(&gprsConfig_sim0, data, sizeof(gprsConfig_st));
    UpdateGprsSim0ConfigToFlash();
}
void SetGprs1SimConfig(gprsConfig_st* data)
{
    memcpy(&gprsConfig_sim1, data, sizeof(gprsConfig_st));
    UpdateGprsSim1ConfigToFlash();
}

void SetVehRegNum(aisDataConfig_st* data)
{
     memcpy(&aisUnitData, data, sizeof(aisDataConfig_st));
     UpdateAisDefaultUnitData();
}

void SetRfidPowerThresholds(rfReaderPwrTh_st* data)
{
    memcpy(&rfPwrThresholds, data, sizeof(rfReaderPwrTh_st));
    UpdateRfidReaderPwrThToFlash();
}

/*******************************************************************************************************************************************/
void ReadAllConfigParamsFromFlash()
{ 
        ReadIntervalsFromFlash();      
        ReadThresholdsFromFlash();
        ReadEmergencyParamsFromFlash();
        ReadServer0ConfigFromFlash();
        ReadServer1ConfigFromFlash();
        ReadGprsSim0ConfigFromFlash();
        ReadGprsSim1ConfigFromFlash();	
        ReadAisUnitDataFromFlash();
        ReadRfidPwrThresholdFromFlash();
}

void ReadIntervalsFromFlash(void)											
{     
    if(ReadMultipleCopies((uint8_t *)&intervals, sizeof(intervals_st), INTERVALS_BLOCK_NUMBER))
    {
        //do nothing
         __nop();
    }
    else
    {
        // check sum invalid so load default values into structure and store it into memory 
        // this case will happen at first time and memory write failure
         LOG_DBG(CH_PACKET,"MEMORY INTERVALS RESET");
        LoadDefaultIntervals();
        UpdateIntervalsToFlash();
    }	
}
void ReadThresholdsFromFlash(void)											
{     
    if(ReadMultipleCopies((uint8_t *)&thresholds, sizeof(thresholds_st), THRESHOLDS_BLOCK_NUMBER))
    {
        //do nothing
			__nop();
    }
    else
    {
        // check sum invalid so load default values into structure and store it into memory 
        // this case will happen at first time and memory write failure
      LOG_DBG(CH_PACKET,"MEMORY THRESHOLDS RESET");
        LoadDefaultThresholds();
        UpdateThresholdsToFlash();
    }	
}
void ReadEmergencyParamsFromFlash(void)											
{   
    if(ReadMultipleCopies((uint8_t *)&emergencyParams, sizeof(emergencyParams_st), EMERGENCY_PARAMS_BLOCK_NUMBER))
    {
        //do nothing
				__nop();
    }
    else
    {
        // check sum invalid so load default values into structure and store it into memory 
        // this case will happen at first time and memory write failure
        LOG_DBG(CH_PACKET,"MEMORY EMER PARAM RESET");
        LoadDefaultEmergencyParams();
        UpdateEmergencyParamsToFlash();
    }		
}
void ReadServer0ConfigFromFlash(void)											
{     
    if(ReadMultipleCopies((uint8_t *)&server0Config, sizeof(serverConfig_st), SERVER0_CONFIG_BLOCK_NUMBER))
    {
        //do nothing
				__nop();
    }
    else
    {
        // check sum invalid so load default values into structure and store it into memory 
        // this case will happen at first time and memory write failure
        LOG_DBG(CH_PACKET,"MEMORY SERVER0 CONFIG RESET");
        LoadDefaultServer0Config();
        UpdateServer0ConfigToFlash();
    }			
}
void ReadServer1ConfigFromFlash(void)											
{     
    if(ReadMultipleCopies((uint8_t *)&server1Config, sizeof(serverConfig_st), SERVER1_CONFIG_BLOCK_NUMBER))
    {
        //do nothing
					__nop();
    }
    else
    {
        // check sum invalid so load default values into structure and store it into memory 
        // this case will happen at first time and memory write failure
        LOG_DBG(CH_PACKET,"MEMORY SERVER1 RESET");
        LoadDefaultServer1Config();
        UpdateServer1ConfigToFlash();
    }			
}
void ReadGprsSim0ConfigFromFlash(void)											
{     
    if(ReadMultipleCopies((uint8_t *)&gprsConfig_sim0, sizeof(gprsConfig_st), GPRS_SIM1_CONFIG_BLOCK_NUMBER))
    {
        //do nothing
        __nop();
    }
    else
    {
        // check sum invalid so load default values into structure and store it into memory 
        // this case will happen at first time and memory write failure
        LOG_DBG(CH_PACKET,"MEMORY GPRS SIM0 RESET");
        LoadDefaultGprsSim0Config();
        UpdateGprsSim0ConfigToFlash();
    }	
}
void ReadGprsSim1ConfigFromFlash(void)											
{     
    if(ReadMultipleCopies((uint8_t *)&gprsConfig_sim1, sizeof(gprsConfig_st), GPRS_SIM2_CONFIG_BLOCK_NUMBER))
    {
        //do nothing
				__nop();
    }
    else
    {
        // check sum invalid so load default values into structure and store it into memory 
        // this case will happen at first time and memory write failure
        LOG_DBG(CH_PACKET,"MEMORY GPRS SIM1 RESET");
        LoadDefaultGprsSim1Config();
        UpdateGprsSim1ConfigToFlash();
    }	
}

void ReadAisUnitDataFromFlash(void)
{
   if(ReadMultipleCopies((uint8_t *)&aisUnitData, sizeof(aisDataConfig_st), AIS_VENDOR_ID_VEH_REG_BLOCK_NUMBER))
    {
      //do nothing
				__nop();
    }
   else
   {
        // check sum invalid so load default values into structure and store it into memory 
        // this case will happen at first time and memory write failure
       //if values are junk or 0xff
            memset(&aisUnitData,0,sizeof(aisDataConfig_st));
           LOG_DBG(CH_PACKET,"MEMORY UNIT ID NOT VALID");
//         LoadDefaultUnitData();
//         UpdateAisDefaultUnitData();
//         GetVehNumFromIMEI = 1;
   }
}

void ReadRfidPwrThresholdFromFlash(void)
{
  if(ReadMultipleCopies((uint8_t *)&rfPwrThresholds,sizeof(rfReaderPwrTh_st),RFID_POWER_THRESHOLD_BLOCK_NUMBER))
  {
       //do nothing
			__nop();
  }
  else
  {
        // check sum invalid so load default values into structure and store it into memory 
        // this case will happen at first time and memory write failure
       LOG_DBG(CH_PACKET,"MEMORY RF TH RESET");
        LoadDefaultRfidPwrThreshold();
        UpdateRfidReaderPwrThToFlash();
  }

}
void LoadDefaultRfidPwrThreshold(void)
{
  //thresholds for 12 v
  rfPwrThresholds.rfRdrLowerTh_12V = RFID_12V_VEH_BATT_LOWER_THRESHOLD;
  rfPwrThresholds.rfRdrUpperTh_12V = RFID_12V_VEH_BATT_UPPER_THRESHOLD;
  
  //thresholds for 24 v
  rfPwrThresholds.rfRdrLowerTh_24V = RFID_24V_VEH_BATT_LOWER_THRESHOLD;
  rfPwrThresholds.rfRdrUpperTh_24V = RFID_24V_VEH_BATT_UPPER_THRESHOLD;
  
  rfPwrThresholds.ReaderOnToOffTime  = READER_ON_TO_OFF_CHECK_TIME; 
  rfPwrThresholds.ReaderOffToOnTime  = READER_OFF_TO_ON_CHECK_TIME;
}

//void LoadDefaultGsmSimSlot(void)
//{
//   gsmSimslot.gsmSimSlotVal = DEFAULT_GSM_SIM_SLOT;
//}
void LoadDefaultIntervals(void)
{	
    intervals.runModeInterval_sec          = DEFAULT_RUN_MODE_INTERVAL_SEC;
    intervals.stopModeInterval_sec         = DEFAULT_STOP_MODE_INTERVAL_SEC;
    intervals.healthPacketInterval_sec     = DEFAULT_HEALTH_PACKET_INTERVAL_SEC;
    intervals.emergencyPacketInterval_sec  = DEFAULT_EMERGENCY_PACKET_INTERVAL_SEC;
}
void LoadDefaultThresholds(void)
{	
    thresholds.speedThreshold        	   = DEFAULT_SPEED_THRESHOLD;
    thresholds.harshBrakingThreshold   	   = DEFAULT_HARSH_BRAKING_THRESHOLD;
    thresholds.accelerationThreshold       = DEFAULT_ACCELERATION_THRESHOLD;
    thresholds.rashTurnThreshold      	   = DEFAULT_RASH_TURN_THRESHOLD;
}
void LoadDefaultEmergencyParams(void)
{	
    memset(emergencyParams.mobileNumber,0,sizeof(emergencyParams.mobileNumber));
    strcpy((char *)emergencyParams.mobileNumber, DEFAULT_EMERGENCY_MOBILE_NO);
	
    emergencyParams.timeout_sec  = DEFAULT_EMERGENCY_TIMEOUT_SEC;
}
void LoadDefaultServer0Config(void)
{	
    memset(server0Config.serverAddr,0,sizeof(server0Config.serverAddr));
    memset(server0Config.serverReqType,0,sizeof(server0Config.serverReqType));
    strcpy((char *)server0Config.serverAddr, DEFAULT_SERVER0_ADDRESS);
    strcpy((char*)server0Config.serverReqType,DEFAULT_SERVER_REQ_TYPE);
	
    server0Config.port = DEFAULT_SERVER0_PORT;
}
void LoadDefaultServer1Config(void)
{	
    memset(server1Config.serverAddr,0,sizeof(server1Config.serverAddr));
    strcpy((char *)server1Config.serverAddr, DEFAULT_SERVER1_ADDRESS);
    strcpy((char*)server1Config.serverReqType,DEFAULT_SERVER_REQ_TYPE);
	
    server1Config.port  = DEFAULT_SERVER1_PORT;
}
void LoadDefaultGprsSim0Config(void)
{	
    memset(gprsConfig_sim0.apn,0,sizeof(gprsConfig_sim0.apn));
    memset(gprsConfig_sim0.username,0,sizeof(gprsConfig_sim0.username));
    memset(gprsConfig_sim0.pass,0,sizeof(gprsConfig_sim0.pass));
    
    strcpy((char *)gprsConfig_sim0.apn, DEFAULT_GPRS_SIM0_APN);
    strcpy((char *)gprsConfig_sim0.username, DEFAULT_GPRS_SIM0_USERNAME);
    strcpy((char *)gprsConfig_sim0.pass, DEFAULT_GPRS_SIM0_PASS);
}
void LoadDefaultGprsSim1Config(void)
{	
    memset(gprsConfig_sim1.apn,0,sizeof(gprsConfig_sim1.apn));
    memset(gprsConfig_sim1.username,0,sizeof(gprsConfig_sim1.username));
    memset(gprsConfig_sim1.pass,0,sizeof(gprsConfig_sim1.pass));

    strcpy((char *)gprsConfig_sim1.apn, DEFAULT_GPRS_SIM1_APN);
    strcpy((char *)gprsConfig_sim1.username, DEFAULT_GPRS_SIM1_USERNAME);
    strcpy((char *)gprsConfig_sim1.pass, DEFAULT_GPRS_SIM1_PASS);
}
//static void LoadDefaultUnitData(void)
//{
//   memset(aisUnitData.unitId,0,sizeof(aisUnitData.unitId));
//   strcpy((char *)aisUnitData.unitId, AIS_UNIT_ID);
//}


static void UpdateRfidReaderPwrThToFlash(void)
{
   //update 12 v configurations
   rfPwrThresholds.checksum = GetCrc16((uint8_t *)&rfPwrThresholds,sizeof(rfReaderPwrTh_st) -2); 
   SaveMultipleCopies((uint8_t *)&rfPwrThresholds,sizeof(rfReaderPwrTh_st),RFID_POWER_THRESHOLD_BLOCK_NUMBER);
}


//static void UpdateGsmSimSlotToFlash(void)
//{
//  gsmSimslot.checksum = GetCrc16((uint8_t *)&gsmSimslot,sizeof(gsmSimslot_st) -2); 
//  SaveMultipleCopies((uint8_t *)&gsmSimslot,sizeof(gsmSimslot_st),GSM_SIM_SLOT_VALUE_BLOCK_NUMBER);
//}

static void UpdateIntervalsToFlash(void)
{
    intervals.checksum = GetCrc16((uint8_t *)&intervals,sizeof(intervals_st) - 2); 
    SaveMultipleCopies((uint8_t *)&intervals,sizeof(intervals_st),INTERVALS_BLOCK_NUMBER);
}
static void UpdateThresholdsToFlash(void)
{
    thresholds.checksum = GetCrc16((uint8_t *)&thresholds,sizeof(thresholds_st) - 2);
    SaveMultipleCopies((uint8_t *)&thresholds,sizeof(thresholds_st),THRESHOLDS_BLOCK_NUMBER);
}
static void UpdateEmergencyParamsToFlash(void)
{
    emergencyParams.checksum = GetCrc16((uint8_t *)&emergencyParams,sizeof(emergencyParams_st) - 2);
    SaveMultipleCopies((uint8_t *)&emergencyParams,sizeof(emergencyParams_st),EMERGENCY_PARAMS_BLOCK_NUMBER);
}
static void UpdateServer0ConfigToFlash(void)
{
    server0Config.checksum = GetCrc16((uint8_t *)&server0Config,sizeof(serverConfig_st) - 2);
    SaveMultipleCopies((uint8_t *)&server0Config,sizeof(serverConfig_st),SERVER0_CONFIG_BLOCK_NUMBER);
}
static void UpdateServer1ConfigToFlash(void)
{
    server1Config.checksum = GetCrc16((uint8_t *)&server1Config,sizeof(serverConfig_st) - 2);
    SaveMultipleCopies((uint8_t *)&server1Config,sizeof(serverConfig_st),SERVER1_CONFIG_BLOCK_NUMBER);
}
static void UpdateGprsSim0ConfigToFlash(void)
{
    gprsConfig_sim0.checksum = GetCrc16((uint8_t *)&gprsConfig_sim0,sizeof(gprsConfig_st) - 2);
    SaveMultipleCopies((uint8_t *)&gprsConfig_sim0,sizeof(gprsConfig_st),GPRS_SIM1_CONFIG_BLOCK_NUMBER);
}
static void UpdateGprsSim1ConfigToFlash(void)
{ 
    gprsConfig_sim1.checksum = GetCrc16((uint8_t *)&gprsConfig_sim1,sizeof(gprsConfig_st) - 2);
    SaveMultipleCopies((uint8_t *)&gprsConfig_sim1,sizeof(gprsConfig_st),GPRS_SIM2_CONFIG_BLOCK_NUMBER);
}

static void UpdateAisDefaultUnitData(void)
{
   aisUnitData.checksum =  GetCrc16((uint8_t *)&aisUnitData,sizeof(aisDataConfig_st) - 2);
   SaveMultipleCopies((uint8_t *)&aisUnitData,sizeof(aisDataConfig_st),AIS_VENDOR_ID_VEH_REG_BLOCK_NUMBER);
}

//saves 3 copies of the structure in 3 consecutive blocks starting from blockNumberStart 
void SaveMultipleCopies(uint8_t *data, uint16_t length, uint32_t blockNumberStart)
{
    uint8_t i=0;	
    uint32_t address;

    if(length < MEM_4KB_SIZE)
    {
        for(i=0; i<3; i++)
        {
            address = ((blockNumberStart + i ) * MEM_4KB_SIZE);		//save data in 3 consecutive blocks
            
            EraseBlock4KB(blockNumberStart + i);
            WriteBlock(data, address, length); 
        }
    }
    else
    {
        //should never get here (config data size should be maintained less than MEM_4KB_SIZE)
    }
}

/**
 *  reads 3 consecutive blocks for the config data
 *  exits as soon as the first block with a valid checksum is found
 */
uint8_t ReadMultipleCopies(uint8_t *data, uint16_t length, uint32_t blockNumberStart)
{
    uint32_t address;
    uint8_t i;
    
    for(i=0; i<3; i++)
    {
        address = ((blockNumberStart + i ) * MEM_4KB_SIZE);		//read data from 3 consecutive blocks
        ReadBlock(data, address, length);
        if(GetCrc16(data, length) == 0)		//0 indicates valid crc
        {
            return 1;		//indicates successful read
        }
    }
    //never gets here when CRC is valid (if CRC is invalid load Default Parameters)
    return 0;
}

