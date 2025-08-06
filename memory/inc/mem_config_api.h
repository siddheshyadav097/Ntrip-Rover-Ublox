/**
 *  @file          :  mem_port.c
 *  @author        :  Vishnu
 *  @date          :
 *  @brief         :  Brief description
 *  @filerevision  :  2.0
 *  
 */

#ifndef __MEM_CONFIG_API_H
#define __MEM_CONFIG_API_H

#include <stdint.h>
   
 //max locations can be used for config is till block num 127  
#define INTERVALS_BLOCK_NUMBER				            0                       //3 blocks assigned for each parameter
#define THRESHOLDS_BLOCK_NUMBER				            3
#define EMERGENCY_PARAMS_BLOCK_NUMBER			        6	
#define SERVER0_CONFIG_BLOCK_NUMBER			            9
#define SERVER1_CONFIG_BLOCK_NUMBER			            12
#define GPRS_SIM1_CONFIG_BLOCK_NUMBER			        15	
#define GPRS_SIM2_CONFIG_BLOCK_NUMBER			        18	
#define GSM_SIM_SLOT_VALUE_BLOCK_NUMBER                 21
#define AIS_VENDOR_ID_VEH_REG_BLOCK_NUMBER              24
#define RFID_POWER_THRESHOLD_BLOCK_NUMBER               27

#define MEM_CLEAR_FLAG                                  0                      //For testing 
   
#define DEFAULT_GSM_SIM_SLOT                            0

#define DEFAULT_RUN_MODE_INTERVAL_SEC			          3//6  //3//20	
#define DEFAULT_STOP_MODE_INTERVAL_SEC			        20
#define DEFAULT_HEALTH_PACKET_INTERVAL_SEC		        900		
#define DEFAULT_EMERGENCY_PACKET_INTERVAL_SEC	        60

#define DEFAULT_SPEED_THRESHOLD					        100
#define DEFAULT_HARSH_BRAKING_THRESHOLD			        100
#define DEFAULT_ACCELERATION_THRESHOLD			        100
#define DEFAULT_RASH_TURN_THRESHOLD				        100

#define DEFAULT_EMERGENCY_MOBILE_NO				        "+919876543210"
#define DEFAULT_EMERGENCY_TIMEOUT_SEC		            300


//caddy unit url - "golf.qdnet.com/gpsdata" 
//buggy data url - "golf.qdnet.com/buggydata"

#define DEFAULT_SERVER0_ADDRESS					       "rtk.qdnet.com/rtk_endpoint"//"golf.qdnet.com/buggydata"//"rtk.qdnet.com/rtk_endpoint"//"golf.qdnet.com/buggydata"//"rtk.qdnet.com/rtk_endpoint" //"golf.qdnet.com/buggydata"//"52.52.107.1"//"golf.qdnet.com/combined_endpoint"//"golf.qdnet.com/buggydata" //"golf.qdnet.com/combined_endpoint"//gpsdata"// "golf.qdnet.com/gpsdata"//"golf.qdnet.com/buggydata"//"golf.qdnet.com/gpsdata"//"golf.qdnet.com/buggydata"//"golf.qdnet.com/gpsdata"//"golf.qdnet.com/buggydata"//"golf.qdnet.com/gpsdata"    //"pkt.cleanupmumbai.com/pktDump/p2.pl" //"v2.cleanupmumbai.com/pktDump/v2.pl" //Http Ip
#define DEFAULT_SERVER0_PORT					         80//2101//  80                                   //Http Port
   
#define DEFAULT_SERVER1_ADDRESS					       "164.52.216.21"//"52.52.107.1"//"164.52.216.21"//"52.52.107.1"//"103.206.29.4"//"52.52.107.1"//"golf.qdnet.com/combined_endpoint"//"ais140.qdnet.com"
#define DEFAULT_SERVER1_PORT					         2101//2105//2101//80  //1910
   
#define DEFAULT_SERVER_REQ_TYPE                         "POST"

#define DEFAULT_GPRS_SIM0_APN					         "airtelgprs.com"//"qdnet.vodafone.in"
#define DEFAULT_GPRS_SIM0_USERNAME				        ""
#define DEFAULT_GPRS_SIM0_PASS					        ""
#define DEFAULT_GPRS_SIM1_APN					        "internet"
#define DEFAULT_GPRS_SIM1_USERNAME				        ""
#define DEFAULT_GPRS_SIM1_PASS					        ""
   
//#define VEH_BATT_VTG_12V          1
//#define VEH_BATT_VTG_24V          1
   
//#ifdef VEH_BATT_VTG_12V
   
#define RFID_12V_VEH_BATT_LOWER_THRESHOLD        12.5
#define RFID_12V_VEH_BATT_UPPER_THRESHOLD        12.6 
   
//#elif   VEH_BATT_VTG_24V
   
#define RFID_24V_VEH_BATT_LOWER_THRESHOLD        25.5
#define RFID_24V_VEH_BATT_UPPER_THRESHOLD        25.6
   
   
#define READER_OFF_TO_ON_CHECK_TIME       10000   //scan for 10 sec to switch ON the reader
#define READER_ON_TO_OFF_CHECK_TIME       120000  //scan for 120sec to switch OFF the reader
   
//#endif
   
   
#define AIS_UNIT_ID                      "9002"//"7001"// "7000"


typedef  struct
{                                                
    uint8_t gsmSimSlotVal;
    uint16_t checksum;  	
}gsmSimslot_st;


typedef  struct
{                                                
    uint16_t runModeInterval_sec;    	
    uint16_t stopModeInterval_sec;      
    uint16_t healthPacketInterval_sec;      
    uint16_t emergencyPacketInterval_sec; 	
    uint16_t checksum;  	
}intervals_st;


typedef  struct
{                                         
    uint16_t speedThreshold;       
    uint16_t harshBrakingThreshold;    	
    uint16_t accelerationThreshold;    
    uint16_t rashTurnThreshold; 
    uint16_t checksum;  
}thresholds_st;

#define MOB_NO_BUFF_SIZE		20

typedef  struct
{                                         
    uint8_t mobileNumber[MOB_NO_BUFF_SIZE];       
    uint16_t timeout_sec;
    uint16_t checksum;  
}emergencyParams_st;


#define SERVER_ADDR_BUFF_SIZE	    100
#define SERVER_REQ_TYPE_BUFF_SIZE	10

typedef  struct
{                                         
    uint8_t serverAddr[SERVER_ADDR_BUFF_SIZE];     
    uint16_t port;
    uint8_t  serverReqType[SERVER_REQ_TYPE_BUFF_SIZE];
    uint16_t checksum;   	
}serverConfig_st;


#define APN_BUFF_SIZE		50
#define USER_BUFF_SIZE		20
#define PASS_BUFF_SIZE		20

typedef  struct
{            
    uint8_t apn[APN_BUFF_SIZE];     
    uint8_t username[USER_BUFF_SIZE];
    uint8_t pass[PASS_BUFF_SIZE];
    uint16_t checksum;  	
}gprsConfig_st;



typedef  struct
{
   float rfRdrLowerTh_12V;
   float rfRdrUpperTh_12V;
   float rfRdrLowerTh_24V;
   float rfRdrUpperTh_24V;
   uint32_t ReaderOnToOffTime;
   uint32_t ReaderOffToOnTime;
   uint16_t checksum;
}rfReaderPwrTh_st;



typedef  struct
{
   uint8_t unitId[30];
   uint16_t checksum;
}aisDataConfig_st;

uint8_t GetGsmActiveSimValue(void);
intervals_st* GetIntervals(void);
thresholds_st* GetThresholds(void);
emergencyParams_st* GetEmergencyParams(void);
serverConfig_st*  GetServer0Config(void);
serverConfig_st* GetServer1Config(void);
gprsConfig_st* GetGprs0SimConfig(void);
gprsConfig_st* GetGprs1SimConfig(void);
aisDataConfig_st* GetAisDeviceData(void);
rfReaderPwrTh_st* GetRfidPwrThValues(void);

//void SetGsmSimSlot0(void);
//void SetGsmSimSlot1(void);
//void SetGsmSimSlot(uint8_t val);
void SetIntervals(intervals_st* data);
void SetThresholds(thresholds_st* data);
void SetEmergencyParams(emergencyParams_st* data);
void SetServer0Config(serverConfig_st* data);
void SetServer1Config(serverConfig_st* data);
void SetGprs0SimConfig(gprsConfig_st* data);
void SetGprs1SimConfig(gprsConfig_st* data);
void SetVehRegNum(aisDataConfig_st* data);
void SetRfidPowerThresholds(rfReaderPwrTh_st* data);
void ReadAllConfigParamsFromFlash();
//void ReadGsmActiveSimSlot(void);
 void LoadDefaultGprsSim0Config(void);

	


#endif