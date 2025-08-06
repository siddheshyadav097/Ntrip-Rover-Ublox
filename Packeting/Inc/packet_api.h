/**
 *  \file packet_api.h
 *  \brief All modules dependencies required for packet creation are included.
 *  	   User data types are defined for packet configuration parameters and cell info data.
 *  	   Also public function declarations for packet init and task are included.
 */

#ifndef __PACKET_API_H__
#define __PACKET_API_H__


 /*----includes-----*/
#include "lib_port.h"
#include "gps_api.h"
#include "gsm_statemachine.h"
#include "bat_api.h"
#include "rtc_port.h"
#include "sensor_api.h"
#include "qtimespent.h"
#include "sys_para.h"

 /*----constants-----*/
#define MAX_PACKET_TYPE                13

#define PACKET_BUFSIZE                 1500
#define PACKET_SEND_MAX_TIMEOUT        60000
  
#define MAX_ALLOWED_TIME_DIFF		   10//changed from 120 seconds to 10 seconds prajakta 141122//120
#define DEFAULT_ONE_SEC_TASK_INTERVAL  1000
#define RFID_EPC_GET_DATA_INTERVAL     5000
  
//#define DEFAULT_PACKET_MSG_TYPE		25
#define DEFAULT_AIS_HEADER_LEN           3
#define AIS_HEADER                      "EPB"
#define AIS_VENDORID                    "QDNET"
   
#define MAJOR_SW_VER                  1
#define MINOR_SW_VER                  0
#define MICRO_SW_VER                  0 
   
#define MS_CONV_FACTOR               1000
   

   
typedef struct
{
   uint8_t   uiHeader[10];
   uint8_t   uiVendorID[10];
   uint8_t	 uiSWRev[20];
}AISDataStruct_st;


typedef struct
{
   char      packetlac[10];
   char      packetcid[10];
   char      packetlac1[10];
   char      packetcid1[10];
   char      packetlac2[10];
   char      packetcid2[10];
   char      packetlac3[10];
   char      packetcid3[10];
//   char      packetlac4[10];
//   char      packetcid4[10];
}AisLacCellIdStruct;

typedef enum
{
   EMERGENCY_ALERT=0,
   IGNITION_ON_ALERT,
   IGNITION_OFF_ALERT,
   INT_BAT_LOW_ALRET,
   INT_BAT_LOW_REMOVED_ALERT,
   VEH_BATT_DISCONNECTED_ALERT,
   VEH_BATT_CONNECTED_ALERT,
   NORMAL_ALERT,
   HEALTH_DATA_ALERT,
   TAMPER_ALERT,
   HARSH_BREAKING_ALRET,
   HARSH_ACC_ALERT,
   RASH_TURNING_ALRET,
   TOTAL_NUM_OF_ALERT
}PacketAlertType_et;

typedef enum
{
   LIVE_PACKET = 0,
   HISTORY_PACKET,
}PacketStatus_et;

typedef enum
{
    PACKET_STATE_IDLE,
    PACKET_SEND_DATA,
    PACKET_SENDING_IN_PROGRESS,
    PACKET_DATA_SEND_FAILURE,
    PACKET_SOCK_CLOSE_WAIT,
}packetSendState_et;

typedef enum
{
    NTRIP_PACKET_STATE_IDLE,
    NTRIP_PACKET_SEND_DATA,
    NTRIP_PACKET_SENDING_IN_PROGRESS,
    NTRIP_PACKET_DATA_SEND_FAILURE,
    NTRIP_PACKET_SOCK_CLOSE_WAIT,
}ntrippacketSendState_et;


typedef enum
{
    PACKET_LIVE,
    PACKET_HISTORY,
    PACKET_CONNECT
}packetSendType_et;

typedef enum
{
  CHECK_FOR_NUM_OF_TAGS,
  GET_THE_EPC_ID
}epcDataGetType_et;


//Response and their matching return values
typedef struct
{
    char const *alertType;
	PacketAlertType_et  alertCode;
    uint8_t  flagIsAlertGenerated;
}AISAlert_st;

/*----typedefs-----*/
typedef struct
{
//   uint8_t startChar;           //$   
//   uint8_t header[3];           //EPB
//   uint8_t vendorId[8];         //AIS01422
//   uint8_t fwVersion;          //01
    PacketAlertType_et packetType;          //NR  -->
    PacketStatus_et packetStatus;         //L 
//    uint8_t imeiBuff[15];       
//    uint8_t vehicleRegNo[16];   //DLIPC9821
    uint8_t gpsFix;               //1
    uint16_t day;
    uint16_t month;
    uint32_t year;
    uint16_t hour;
    uint16_t minute;
    uint16_t second;
    uint8_t validity;                          //A
    //float latitude;                            //19.114105
    double latitude;  
	  uint8_t latDir;                            //N
    //float longitude;                           //72.880392
    double longitude; 
		uint8_t longDir;                           //E
    float speed;                               //40    
    float heading;                             //221
    float altitude;                            //32.6
    uint8_t noOfSat;                           //4
    float pDop;                                //2.5
    float hDop;                                //1.3
//    uint8_t nwOperator[30];     
    uint8_t ignitionState;                      //1
    vehicleBatteryState_et mainPwrStatus;        //1
    float mainIPVolt;                           //12.5
    float intBatVoltage;                       //4.2
    gsmState_et gsmModemState;
//    uint8_t emergencyStatus1;                  //0
//  uint8_t emergencyStatus2;                  //0
    uint8_t rssi;
    uint16_t mcc;    
    uint16_t mnc;
    uint16_t lac;
    uint32_t cid;
    uint16_t mcc1;    
    uint16_t mnc1;
    uint16_t lac1;
    uint32_t cid1;
    int8_t dbm1;
    uint16_t mcc2;    
    uint16_t mnc2;
    uint16_t lac2;
    uint32_t cid2;
    int8_t dbm2;
    uint16_t mcc3;    
    uint16_t mnc3;
    uint16_t lac3;
    uint32_t cid3;
    int8_t dbm3;
//    uint16_t mcc4;    
//    uint16_t mnc4;
    uint16_t rfidTagStatus;    
    uint16_t rfidSmState;
    uint8_t RhSensitivity;
    uint8_t Temperature;
//    uint16_t lac4;
//    uint32_t cid4;
    int8_t dbm4;
    uint8_t input1State;        //0
    uint8_t epcId1[30];
    uint8_t epcId2[30];
    uint8_t epcId3[30];
    uint8_t epcId4[30];
    uint8_t epcId5[30];
//    uint8_t input2State;        //0
//    uint8_t input3State;        //0
//    uint8_t input4State;        //0
//    uint8_t digitalOutput1;     //0
//    uint8_t digitalOutput2;     //0
//    uint32_t analogData1;          //29.8
//     uint32_t analogData2;          //35.8
    float analogData1;          //29.8   //currently sending ccid num instead of this
    //float analogData2;       //35.8
    uint32_t frameNumber;       //816326
    uint16_t checksum;          //checksum for memeory                                  
//    uint8_t endChar;            //* 
}DataPacket_t;

typedef struct
{
    uint16_t mcc;    
    uint16_t mnc;
    uint16_t lac;
    uint32_t cid;
    int8_t dbm;
}CellidPara_t;

 /*----public function declarations-----*/
void AppVersionPrint(void);
void PacketHandler(void);
void PacketHandlerStatesInit(void);
PacketAlertType_et GetAlertType(void);
void GetPacketIntervalVal(void);
void GetUnitIdForPacketing(void);
void UpdateAISPacketAlertFlag(PacketAlertType_et UpdateAlertType);
void GetAISDefaultPAcketData(void);
void InitPacketConfig(void);//(* packet_callback_ptr)(uint8_t*, uint16_t), void(* logError_ptr)(uint8_t*));
//void Packet_Config_Params(PacketConfig_t *pack_struct);
//void AISStorePacketToMemory(uint8_t* memDataPtr);
void PacketGetImeiCcid(void);
void PacketGetOperatorName(void);
uint32_t GetSizeOfPacket(void);
void AisClearRfidBuffers(void);
void PacketTask(void);//(CellidPara_t *cellid_para);
uint8_t  packetStateIsIdle(void);
uint32_t GetPacketStructSize(void);
//void PrepareHealthPacket(void);
char * conv_itoa(signed long num);
packetSendState_et GetHttpPacketSendState(void);
void packetSetState(packetSendState_et state);
uint8_t Distance_Task(void);
void Update_Distance(void);

///////ntrip
void NtripPacketTask(void);
uint8_t NtrippacketStateIsReady(void);
void NtripPacketHandler(void);
void NtripPacketProcess(void);
void ntrippacketSetState(ntrippacketSendState_et state);
void setntripheaderflag(void);
uint8_t getntripheaderflag(void);
void NtripPacket(void);
void NtripPacketSend(void);
uint16_t  ntripGetPacketLen(uint8_t *ucSourceAdr);
void resetntripheaderflag(void);
#endif