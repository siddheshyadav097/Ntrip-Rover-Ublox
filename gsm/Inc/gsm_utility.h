#ifndef __GSM_UTILITY_H
#define __GSM_UTILITY_H

#include "gsm_port.h"
#include "gsm_common.h"
#include "rtc_port.h"


/**
 *  @file gsm_tracking_data.h
 *  @brief 
 */
#define GSM_eLenCountryCode		         4
#define GSM_eLenOperatorCode	         4
#define GSM_eLenLocationCode		     5
#define GSM_eLenCellID			         5
#define GSM_eLenGSMSignal		        10
#define GSM_eLenGSMOperator		        30
#define GSM_elenRSSIIndBm               10

typedef struct{
  
    uint8_t    GSMCountryCode[GSM_eLenCountryCode];
	uint8_t    GSMNetworkCode[GSM_eLenOperatorCode];
	uint8_t	   GSMLocationCodeHex[GSM_eLenLocationCode];
    uint32_t   GSMLacInDec;
	uint8_t    GSMCellIdHex[GSM_eLenCellID];
    uint32_t   GSMCellIdInDec;
    uint8_t    GSmRSSIIndBm[GSM_elenRSSIIndBm];
    uint8_t    GSMSignal[GSM_eLenGSMSignal];
    uint8_t    GSMCurrentOperatorName[GSM_eLenGSMOperator];
}GSMTrackingdata_st;

 
typedef  struct
{
    uint16_t mcc;
    uint16_t mnc;
	uint16_t lac;
	uint32_t cellid;
    uint8_t   rssi;
	int8_t    dbm;   //uint8_t
}cellIdStruct_st;


typedef  struct 
{ 
    int8_t nc1dbm;
    uint16_t nc1Mcc;
    uint16_t nc1Mnc;
    uint16_t nc1Lac;
    uint32_t nc1Cellid;
    
    int8_t nc2dbm;
    uint16_t nc2Mcc;
    uint16_t nc2Mnc;
    uint16_t nc2Lac;
    uint32_t nc2Cellid;
    
    int8_t nc3dbm;
    uint16_t nc3Mcc;
    uint16_t nc3Mnc;
    uint16_t nc3Lac;
    uint32_t nc3Cellid;
    
    int8_t nc4dbm;
    uint16_t nc4Mcc;
    uint16_t nc4Mnc;
    uint16_t nc4Lac;
    uint32_t nc4Cellid;
    
    int8_t nc5dbm;
    uint16_t nc5Mcc;
    uint16_t nc5Mnc;
    uint16_t nc5Lac;
    uint32_t nc5Cellid;
    
    int8_t nc6dbm;
    uint16_t nc6Mcc;
    uint16_t nc6Mnc;
    uint16_t nc6Lac;
    uint32_t nc6Cellid;   
}gsmNeighcellIdStruct_st;


//typedef volatile struct 
//{
//	uint16_t year;
//	uint16_t month;
//	uint16_t day;
//	uint16_t hour;
//	uint16_t minute;
//	uint16_t second;
//    int32_t timezone;  
//}GSMtimeStruct_st ;
void GsmInitClearCellDataBuff(void);

void decimal_to_hexadecimal(int tempDecimal,char* decToHexArray);
BOOL Get_Gsm_time(TimePara_t* gsmtime_struct);
void ProcessQENGStart(void);
uint8_t ProcessQENGData(uint8_t *QengLinedata ,uint8_t QengLinelen);
uint8_t ProcessQENGResultGet(void);
uint8_t GsmGetSMSIndex(uint8_t *ListSmsLinedata ,uint8_t ListSmsLinelen);
void ProcessCSQData(uint8_t *CSQLinedata ,uint8_t CSQLinelen);
cellIdStruct_st* GetServingNeighCellInfo(void);
gsmNeighcellIdStruct_st* GetNeighbouringCellInfo(void);

#endif
