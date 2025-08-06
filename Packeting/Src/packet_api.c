/**
 *  @file          :  packet_api.c
 *  @author        :  prajakta
 *  @date          :  
 *  @brief         :  Includes initialization of parameters and modules required for packeting.
 *  				  Processing of tasks required to gather data from various sub-modules is carried out in Packet_Task
 *  				  and at a defined packet interval Make_Packet is issued to create a packet using the collected data.
 *  				  On creation of a packet a callback is provided to the main application with the packet created.
 *  @filerevision  :  1.0
 *  
 */

 
 /*----includes-----*/
#include "debug_log.h"
#include "packet_api.h"
#include "mem_config_api.h"
#include "mem_packet_api.h"
#include "led_api.h"
#include "gsm_common.h"
#include "gsm_utility.h"
#include "gsm_idle_task.h"
#include "gsm_init_cmd.h"
#include "ais_app.h"
//#include "serial_fuel_api.h"
#include "serial_rfid_api.h"
#include "crc16.h"
#include "digio_api.h"
#include "buzzer_app.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

 /*----variables-----*/
uint8_t  packetBuffer[PACKET_BUFSIZE]= {0}; 
uint8_t  PacketServer_buff[PACKET_BUFSIZE]= {0};
uint8_t  PacketConn_buff[100]= {0};
uint16_t tx_packet_length = 0,packet_length=0;
uint16_t checkSum = 0;
uint32_t makeLivePackStartTime = 0;
uint32_t liveDataInterval = 0;
uint32_t historyDataInterval = 0;
uint32_t healthDataInterval = 0;
uint32_t makeHealthPacketTime = 0;
uint8_t  flagIgnitionOn=0;
uint32_t oneSecTaskTick =0;
uint32_t rfidEpcGetTick =0;
int64_t  Timediff;
uint32_t frameNumber = 0,packetNo =0;

GpsData_t*       gps_data = {0};
SupplyInfo_st*   supplyInfoStruct = NULL;
DigitalInput_t*  digitalInputStruct= NULL;
//DigitalOutput_t* digitalOutputStruct= NULL;
AnalogData_t*    sensorDataStruct= NULL;
cellIdStruct_st* cellInfoStruct= NULL;
gsmNeighcellIdStruct_st* NeighCellInfoStruct= NULL;
//Dht11SensorData_st*   RhSensorStruct = {0};

intervals_st* intervalsValue= NULL;
static aisDataConfig_st* aisDeviceData= NULL;
//FuelPacketData_st* fuelAvgDataPtr= NULL;

DataPacket_t    AISPacketStruct = {0};
DataPacket_t    AISMemPacketStruct = {0};
DataPacket_t    AISHistoryDataStruct = {0};

uint16_t packetReadyCntr = 0;
uint32_t packetReadyTime = 0;
uint8_t LivePacketReadyFlag=0;

packetSendState_et packetSendState = PACKET_STATE_IDLE;                         //packet send state 
packetSendType_et  packetSendType = PACKET_LIVE;                                 //type of packet unit sending
pvtSendHandler_et  sendState;    
                                                //instance for pvt statemachine state
ntrippacketSendState_et ntrippacketSendState = NTRIP_PACKET_STATE_IDLE;
ntripSendHandler_et  ntripsendState;

/*Rfid data handling flags*/
uint8_t epcGetTagCnt        = 0;
uint8_t flagEpcData         = 0;
uint8_t epcTotalUnsentCnt   = 0;
uint8_t epcRemainingTagCnt  = 0;

epcDataGetType_et epcDataState = CHECK_FOR_NUM_OF_TAGS;

AISAlert_st alertTypeStruct = {0};
static AISDataStruct_st aisPacketData = {0};
static AisLacCellIdStruct aisLocationData = {0};

RfEpcData_st rfEpcData = {0};
RfEpcData_st packetEpcBuff = {0};

uint8_t flagRtcTimeUpdate = 0;

static uint8_t imeiPacketBuff[25]= {0};
static uint8_t ccidPacketBuff[25]= {0};
static uint8_t OperatorBuff[30]  = {0};

extern uint8_t battCalibrate;
extern uint8_t sensorCalib;
TimePara_t time_config={0};

PacketAlertType_et packetAlertType = NORMAL_ALERT;



////////
uint8_t ntripheaderflag=0;
uint8_t gpggasendflag=0;
uint32_t ntripPackStartTime=0;
uint16_t Ntrip_tx_packet_length = 0;
uint8_t  NtripPacketServer_buff[1500]= {0};
////////



 static AISAlert_st AISAlertTypeEntry[MAX_PACKET_TYPE]={
  
  {"EA",  EMERGENCY_ALERT,            0},
  {"IN",  IGNITION_ON_ALERT,          0},
  {"IF",  IGNITION_OFF_ALERT,         0},
  {"BL",  INT_BAT_LOW_ALRET,          0},
  {"BN",  INT_BAT_LOW_REMOVED_ALERT,  0},
  {"BD",  VEH_BATT_DISCONNECTED_ALERT,0},
  {"BR",  VEH_BATT_CONNECTED_ALERT,   0},
  {"NR" , NORMAL_ALERT,               0},
  {"HP",  HEALTH_DATA_ALERT,          0},
  {"TA",  TAMPER_ALERT,               0},
  {"HB",  HARSH_BREAKING_ALRET,       0},
  {"HA",  HARSH_ACC_ALERT,            0},
  {"RT",  RASH_TURNING_ALRET,         0}
};

//to store packet type
uint8_t arrayDataType[3] = {0};
 /*----private functions-----*/

void AppVersionPrint(void)
{
  LOG_DBG(CH_GSM,"VTS GOLF Project - V%d.%d.%d",MAJOR_SW_VER,MINOR_SW_VER,MICRO_SW_VER);
}

void packetVarInit(void)
{
  memset(&AISPacketStruct,0,sizeof(DataPacket_t)); 
  AisClearRfidBuffers();           //clear the current rfid buffers
}

void PacketHandlerStatesInit(void)
{
//   GsmSocketClose(0);
   snprintf((char*)imeiPacketBuff,sizeof(imeiPacketBuff),"%s","0");
   snprintf((char*)ccidPacketBuff,sizeof(ccidPacketBuff),"%s","0");
   snprintf((char*)OperatorBuff,sizeof(OperatorBuff),"%s","0");
   GetAISDefaultPAcketData();     //Get AIS default Data like vendor Id ,fw version,veh reg number
}
void PacketHandler(void)
{
   // update all the inputs get gsm , gps paramters in AISPacketStruct structure
   PacketTask();  
}

PacketAlertType_et GetAlertType(void)
{
	uint8_t i;
    packetAlertType = NORMAL_ALERT;
	for(i = 0; i < NUM_ELEMS(AISAlertTypeEntry); i++)
	{
		if((AISAlertTypeEntry[i].flagIsAlertGenerated) == 1)
		{
			packetAlertType = AISAlertTypeEntry[i].alertCode;
			break;
    }
	}
	return packetAlertType;
}
//uint8_t* GetPacketTypeString(uint8_t alertVal,uint8_t* dataArray)

void GetPacketTypeString(uint8_t alertVal,uint8_t* dataArray)
{
   uint8_t i=0;
   for(i = 0; i < NUM_ELEMS(AISAlertTypeEntry); i++)
	{
		if((AISAlertTypeEntry[i].alertCode) == alertVal)
		{
			break;
        }
	}
   if(i > NUM_ELEMS(AISAlertTypeEntry))    //to check if any junk value is received at the alertVal
   {
           strncpy((char*)dataArray,(const char*)AISAlertTypeEntry[7].alertType,2);
//        return ((uint8_t*)AISAlertTypeEntry[7].alertType); 
   }
   strncpy((char*)dataArray,(const char*)AISAlertTypeEntry[i].alertType,2);
//	return ((uint8_t*)AISAlertTypeEntry[i].alertType); 
}

void UpdateAISPacketAlertFlag(PacketAlertType_et UpdateAlertType)
{
   AISAlertTypeEntry[UpdateAlertType].flagIsAlertGenerated = 1;
}

/**
 *  @brief This function gets default data to be sent in the packet
 *  @param [in] void
 *  @return void
 */
void GetAISDefaultPAcketData(void)
{
   memset(&aisPacketData, 0, sizeof(AISDataStruct_st));
   strncpy((char*)aisPacketData.uiHeader,AIS_HEADER,strlen(AIS_HEADER));
   strncpy((char*)aisPacketData.uiVendorID,AIS_VENDORID,strlen(AIS_VENDORID));
   snprintf((char*)aisPacketData.uiSWRev,sizeof(aisPacketData.uiSWRev),"%d.%d.%d",MAJOR_SW_VER,MINOR_SW_VER,MICRO_SW_VER);
}

/**
 *  @brief This function gets run, stop, health and emergency intervals stored from the memory.
 *  @param [in] void
 *  @return void
 */
void GetPacketIntervalVal(void)
{
  intervalsValue = GetIntervals();             //get intervals
  liveDataInterval  =  intervalsValue->runModeInterval_sec;                 //if ignition is on load the run mode interval to send the live packet
  historyDataInterval = (intervalsValue->runModeInterval_sec)/2;
  healthDataInterval = (intervalsValue->healthPacketInterval_sec) * MS_CONV_FACTOR;
}
 
/**
 *  @brief This function gets the vendor id stored in the memory.
 *  @param [in] void
 *  @return void
 */
void GetUnitIdForPacketing(void)
{
  aisDeviceData   = GetAisDeviceData();          //get vendor id
  if(atoi((const char*)aisDeviceData->unitId) == 0)                 //this means nothing is stored in the mem ,load 7000 on the ram
  {
    memset(aisDeviceData, 0, sizeof(aisDataConfig_st));
    strncpy((char *)aisDeviceData->unitId,AIS_UNIT_ID,4); 
  }
}

/**
 *  @brief This function copies time & date data from both the pointers into the local c type structures. 
 *  Convert that time structure into seconds using "mktime" standard time function of c. Then it compares
 *  both the seconds & depending on the greater value it takes the time difference by using "difftime" 
 *  standard time function of c & returns the value to time difference.
 *  @param [in] ptr1 pointer for 1st time structure
 *  @param [in] ptr2 pointer for 2nd time structure
 *  @return Returns the value for time difference between 2 time structure
 */
static int64_t Compare_time(TimePara_t *ptr1, TimePara_t *ptr2)
{
	static int64_t time_diff;
	struct tm Time1, Time2;
	time_t sec_time1, sec_time2;
	
	memset(&Time1,0,sizeof(struct tm));
	memset(&Time2,0,sizeof(struct tm));
	
	Time1.tm_hour = ptr1->hour;
	Time1.tm_min = ptr1->minute;
	Time1.tm_sec = ptr1->second;
	Time1.tm_mday = ptr1->day;
	Time1.tm_mon = ptr1->month - 1;
	
	//////////////////////new added 26/08/22
	Time1.tm_year = ptr1->year + 2000;
	
	Time1.tm_year = Time1.tm_year - 1900;
	
	Time2.tm_hour = ptr2->hour;
	Time2.tm_min = ptr2->minute;
	Time2.tm_sec = ptr2->second;
	Time2.tm_mday = ptr2->day;
	Time2.tm_mon = ptr2->month - 1;
	
	////////////////////new added on 26/08/22
	Time2.tm_year = ptr2->year  + 2000;
	Time2.tm_year = Time2.tm_year - 1900;
	
	sec_time1 = mktime(&Time1);  //gps/gsm time
	sec_time2 = mktime(&Time2);  //rtc time
	
	if(sec_time2 > sec_time1)
	{
		time_diff = (int64_t)difftime(sec_time2,sec_time1);
	}
	else if(sec_time2 < sec_time1 )
	{
		time_diff = (int64_t)difftime(sec_time1,sec_time2);
	}
    else
    {
      time_diff = 0;
    }
	
	return time_diff;
}

 /**
 *  @brief This function gets the gsm network time, & if GPS data string is valid then get the GPS time
 *  To send the latest time in live packet we will compare the gsm network time with the valid gps time. 
 *  If that time difference Timediff > MAX_ALLOWED_TIME_DIFF(defined in packet_api.h file) then set rtc time
 *  with gps time structure or else the rtc time will be same as gsm network time.
 *  @param [in] time_struct pointer for current time
 *  @return void
 */
uint8_t PacketTimeStampInvalid  =0;

static void Get_Current_Time(TimePara_t *time_struct)
{
    TimePara_t gpstime_struct = {0};
  	TimePara_t gsmtime_struct = {0};
        
	if(Get_RTC_time(time_struct) == False)
	{
	   PacketTimeStampInvalid = 1;
	}	
	if(Is_Gps_Valid())  
	{
		Get_Gps_time((GpsTimePara_t *)&gpstime_struct);   //get gps time
		
		Timediff = Compare_time(&gpstime_struct, time_struct);  //compare gps time with the rtc time
		
			if(Timediff > MAX_ALLOWED_TIME_DIFF) //if rtc time and gps time difference is > than 2 min
			{
                //compare gps time with gsm time
                if((GsmIsNetworkRegistered()))// || (flagRtcTimeUpdate == 1))
                {
                    if( (flagRtcTimeUpdate == 2)) //valid time updated from gsm (flagRtcTimeUpdate == 1) ||
                    {
                        if(Get_Gsm_time((TimePara_t *)&gsmtime_struct))
                        {
                          Timediff = Compare_time(&gpstime_struct, &gsmtime_struct);  //compare gps time with gsm time
                          
                          if(Timediff <= MAX_ALLOWED_TIME_DIFF) //if time diff betn gsm and gps is <= 2 min
                          {
                            Set_RTC_time((TimePara_t *)&gpstime_struct);  //update gps time to rtc
				                    memcpy(time_struct, &gpstime_struct, sizeof(TimePara_t));
																												PacketTimeStampInvalid = 0;

                          }
                          else   //this means gps time is not valid,compare rtc time with gsm time
                          {
                            Timediff = Compare_time(&gsmtime_struct, time_struct);
                            
                             if(Timediff > MAX_ALLOWED_TIME_DIFF)
                             {
                               Set_RTC_time((TimePara_t *)&gsmtime_struct);
                               memcpy(time_struct, &gsmtime_struct, sizeof(TimePara_t));
															 														PacketTimeStampInvalid = 0;

                             }
                          }
                        }
                    }
                     flagRtcTimeUpdate = 0;
                }
			}
	}
	else
	{
       if((GsmIsNetworkRegistered())) //|| (flagRtcTimeUpdate == 1))  //it should  when +QNITZ: URC is received
       {
           if((flagRtcTimeUpdate == 2))  //(flagRtcTimeUpdate == 1) || 
           {
                if(Get_Gsm_time((TimePara_t *)&gsmtime_struct))
                {
                  Timediff = Compare_time(&gsmtime_struct, time_struct);
                  
                      if(Timediff > MAX_ALLOWED_TIME_DIFF)
                      {
                              Set_RTC_time((TimePara_t *)&gsmtime_struct);
                              memcpy(time_struct, &gsmtime_struct, sizeof(TimePara_t));
																										PacketTimeStampInvalid = 0;

                      }
                }
           }
            flagRtcTimeUpdate = 0;
	    }  
     }       
}
/**
 *  @brief GetSizeOfPacket() - This function returns the number of bytes to be allocated for AIS Server data
 *  in memory. this function is called at the start in InitMemory()
 *  @return uint32_t type size of AISPacketStruct
 */
uint32_t GetSizeOfPacket(void)        //function for memory
{
   return (sizeof(DataPacket_t));
}

/**
 *  @brief aisGetPacketLen() - This function returns the packet length , starts calculating from $ and ends up at *
 *   Input -  Pointer to the Live or history packet data.
 *  @return uint16_t type length of the packet
 */
uint16_t  aisGetPacketLen(uint8_t *ucSourceAdr)
{
	uint16_t ucTemp1 = 0, getlen =0;
	packet_length = 0;
    
    ucTemp1 = ucSourceAdr[packet_length];
    
    if( ucTemp1 == '$')
    {
      getlen = 1;
    }
    else
    {
      return 0;
    }
    if(getlen)
    {
        for( ; ucTemp1 != '*'  ; packet_length++)
        {
            ucTemp1 = ucSourceAdr[packet_length];
            if(packet_length >= 1500) //512
            {
              return 0;
            }
        }
    }
	return(packet_length);
}

/**
 *  @brief GetHexCellIDDataForPacket() - this function converts the decimal LAC's and cell ids into hexadecimal number
 *  for packeting, as they are stored as decimal into the memory.
 *  @return - void
 */
void GetHexCellIDDataForPacket(DataPacket_t* GetCellIdPacketData)               //converting hex cell data to decimal
{  
    memset(&aisLocationData,0,sizeof(AisLacCellIdStruct));
  
    if(GetCellIdPacketData->lac != 0)
    {
          decimal_to_hexadecimal(GetCellIdPacketData->lac,aisLocationData.packetlac);
    }
    else
    {
	   strncpy((char*)aisLocationData.packetlac,"0",1);
    }
  
    if(GetCellIdPacketData->cid != 0)
    {
          decimal_to_hexadecimal(GetCellIdPacketData->cid,aisLocationData.packetcid);
    }
    else
    {
	  strncpy((char*)aisLocationData.packetcid,"0",1);
    }
    
    if(GetCellIdPacketData->lac1 != 0)
    {
          decimal_to_hexadecimal(GetCellIdPacketData->lac1,aisLocationData.packetlac1);
    }
    else
    {
	  strncpy((char*)aisLocationData.packetlac1,"0",1);
    }
    
    if(GetCellIdPacketData->cid1 != 0)
    {
          decimal_to_hexadecimal(GetCellIdPacketData->cid1,aisLocationData.packetcid1);
    }
    else
    {
	   strncpy((char*)aisLocationData.packetcid1,"0",1);
    }
    
     if(GetCellIdPacketData->lac2 != 0)
    {
          decimal_to_hexadecimal(GetCellIdPacketData->lac2,aisLocationData.packetlac2);
    }
    else
    {
	   strncpy((char*)aisLocationData.packetlac2,"0",1);
    }
    
    if(GetCellIdPacketData->cid2 != 0)
    {
          decimal_to_hexadecimal(GetCellIdPacketData->cid2,aisLocationData.packetcid2);
    }
    else
    {
	  strncpy((char*)aisLocationData.packetcid2,"0",1);
    }
    
    if(GetCellIdPacketData->lac3 != 0)
    {
          decimal_to_hexadecimal(GetCellIdPacketData->lac3,aisLocationData.packetlac3);
    }
    else
    {
      strncpy((char*)aisLocationData.packetlac3,"0",1);
    }
    
    if(GetCellIdPacketData->cid3 != 0)
    {
           decimal_to_hexadecimal(GetCellIdPacketData->cid3,aisLocationData.packetcid3);
    }
    else
    {
      strncpy((char*)aisLocationData.packetcid3,"0",1);
    }
    
//    if(GetCellIdPacketData->lac4 != 0)
//    {
//        lac4 = decimal_to_hexadecimal(GetCellIdPacketData->lac4);
//        memcpy((char*)aisLocationData.packetlac4,(char*)lac4,sizeof(aisLocationData.packetlac4));
////      LOG_DBG(CH_PACKET,"Hex lac4 = %s", aisPacketData.packetlac4);
//    }
//    else
//    {
//      memset(aisLocationData.packetlac4,0,sizeof(aisLocationData.packetlac4));
//      strncpy((char*)aisLocationData.packetlac4,"0",1);
//    }
//    
//    if(GetCellIdPacketData->cid4 != 0)
//    {
//        cid4 = decimal_to_hexadecimal(GetCellIdPacketData->cid4);
//        memcpy((char*)aisLocationData.packetcid4,(char*)cid4,sizeof(aisLocationData.packetcid4));
////      LOG_DBG(CH_PACKET,"Hex cid4 = %s", aisPacketData.packetcid4);
//    }
//    else
//    {
//      memset(aisLocationData.packetcid4,0,sizeof(aisLocationData.packetcid4));
//      strncpy((char*)aisLocationData.packetcid4,"0",1);
//    }
}

void AisClearRfidBuffers(void)
{
    memset(AISPacketStruct.epcId1,0,sizeof(AISPacketStruct.epcId1));
    memset(AISPacketStruct.epcId2,0,sizeof(AISPacketStruct.epcId2));
    memset(AISPacketStruct.epcId3,0,sizeof(AISPacketStruct.epcId3));
    memset(AISPacketStruct.epcId4,0,sizeof(AISPacketStruct.epcId4));
    memset(AISPacketStruct.epcId5,0,sizeof(AISPacketStruct.epcId5));
    
    snprintf((char*)AISPacketStruct.epcId1,sizeof(AISPacketStruct.epcId1),"%c",'0');
    snprintf((char*)AISPacketStruct.epcId2,sizeof(AISPacketStruct.epcId2),"%c",'0');
    snprintf((char*)AISPacketStruct.epcId3,sizeof(AISPacketStruct.epcId3),"%c",'0');
    snprintf((char*)AISPacketStruct.epcId4,sizeof(AISPacketStruct.epcId4),"%c",'0');
snprintf((char*)AISPacketStruct.epcId5,sizeof(AISPacketStruct.epcId5),"%c",'0');
}

void GetRFIDData(void)
{
    uint8_t getEpcData = 0;
    uint8_t epcValidData = 0;
    uint8_t epcFillPtr =0;
    
   for(getEpcData =0; epcFillPtr < epcGetTagCnt; getEpcData++)
   {
      memset((void*)&packetEpcBuff,0,sizeof(RfEpcData_st));
      epcValidData = RfidGetEPCData(getEpcData,&packetEpcBuff); //copy the data in the form of the hex ascii in the packetEpcBuff
      if(epcValidData)
      {
        epcValidData = 0;
        memset((void*)&rfEpcData,0,sizeof(RfEpcData_st));
        RfidBytesToHex(packetEpcBuff.RfepcId,packetEpcBuff.RfepcIdLen,(char*)rfEpcData.RfepcId);  //convert the data from the hexAscii to hex in the rfEpcData Struct
        switch(epcFillPtr)
        {
           case 0:
           snprintf((char*)AISPacketStruct.epcId1,sizeof(AISPacketStruct.epcId1),"%s",(char*)rfEpcData.RfepcId);  //for unsigned hex character
           break;
             
           case 1:
           snprintf((char*)AISPacketStruct.epcId2,sizeof(AISPacketStruct.epcId2),"%s",(char*)rfEpcData.RfepcId);
           break;
           
           case 2:
           snprintf((char*)AISPacketStruct.epcId3,sizeof(AISPacketStruct.epcId3),"%s",(char*)rfEpcData.RfepcId);
           break;
            
           case 3:
           snprintf((char*)AISPacketStruct.epcId4,sizeof(AISPacketStruct.epcId4),"%s",(char*)rfEpcData.RfepcId);
            break;
            
           case 4:
           snprintf((char*)AISPacketStruct.epcId5,sizeof(AISPacketStruct.epcId5),"%s",(char*)rfEpcData.RfepcId);
           break;

          default:
            break;
        }
        epcFillPtr++;
      }
   }
}
void RfidGetPacketEpcData(void)
{
  AisClearRfidBuffers();           //clear the current rfid buffers
  GetRFIDData();
}

//This Function continuously monitors the tags in the epc struct and if tag count is > 5 it immediately sends the tag data on the server
void RfidEpcDataMonitorHandler(void)
{
   epcTotalUnsentCnt = RfidGetAvailableTagsCnt();
   if(epcTotalUnsentCnt > 5)
   {
      epcGetTagCnt = 5; //first get the 5 tags's epc id
      //as this epcRemainingTagCnt is not used in the code now this is just for printing the remaining tags
      //as the tags are continuously updating in the interrupt for this reason we are constantly reading the tag count
      epcRemainingTagCnt = epcTotalUnsentCnt - 5;   //get the remaining num of tag count  if tag Count > 5
      //LOG_DBG(CH_RFID,"Num Of tags in Queue: %d ,Remaining Tags : %d", epcTotalUnsentCnt, epcRemainingTagCnt);
      RfidGetPacketEpcData();
      flagEpcData  = 1; 
      epcTotalUnsentCnt = 0;
      epcGetTagCnt  = 0;
   }
}

//this function is used to check if their are any remaining tags in the struct
void RfidGetRemainingTagCnt(void)
{
   epcTotalUnsentCnt = RfidGetAvailableTagsCnt();
   
   if(epcTotalUnsentCnt > 0 &&  epcTotalUnsentCnt <= 5)//If num of available tags is > 0 and <=  5
   {
      epcGetTagCnt       = epcTotalUnsentCnt;
      //LOG_DBG(CH_RFID,"Num Of tags Pending to be sent in live packet: %d ", epcTotalUnsentCnt);
      RfidGetPacketEpcData();
      epcTotalUnsentCnt = 0;
      epcGetTagCnt  = 0;
   }
}

void PacketGetImeiCcid(void)
{    
    memset(imeiPacketBuff,0,sizeof(imeiPacketBuff));                            //Get IMEI number
    GetIMEINumber(imeiPacketBuff);
    
    memset(ccidPacketBuff,0,sizeof(ccidPacketBuff));                            //Get CCID number
    GetCCIDNumber(ccidPacketBuff);
}

void PacketGetOperatorName(void)
{
     memset(OperatorBuff,0,sizeof(OperatorBuff));
     GetOperatorNameInit(OperatorBuff);
}
  
/**
 *  @brief AISMakePacket() - This function forms the packet from the live or history packet structre passed to it.
 *  This function forms the live 'NR', all the types of alert packets as well as history packets.
 *  All the default packet data like vendor id, veh reg no, imei number, sw ver are not stored into the memory.
 *  Frame number is only incremented for a live packet formed.
      GetPacketData->input2State,\
      GetPacketData->input3State,\
      GetPacketData->input4State,\
      GetPacketData->digitalOutput1,\
      GetPacketData->digitalOutput2,\
GetPacketData->analogData1,\
 *  @return - void
 */

static void AISMakePacket(DataPacket_t* GetPacketData)                          //Common function for creating live/history packet
{
    char packetStatus = 'L';
        
    GetHexCellIDDataForPacket(GetPacketData);                                   //Convert cell id from decimal to hex
    
    if(GetPacketData->packetStatus == 0){packetStatus='L';}else{packetStatus='H';}
    
    memset(arrayDataType,0,sizeof(arrayDataType));
    GetPacketTypeString(GetPacketData->packetType,arrayDataType);                //getPAcketType String from the enum value
    
    packetNo = GetPacketData->frameNumber;  //Get the current packet number whether history or the live packet number to check with the ack
    
    memset(packetBuffer, 0, sizeof(packetBuffer));
   snprintf((char *)packetBuffer,sizeof(packetBuffer),"$%s,%s,%s,%s,%c,%s,%s,%d,%02d%02d%04d,%02d%02d%02d,%0.9lf,%c,%0.9lf,%c,%0.1f,%0.2f,%d,%0.2f,%0.2f,%0.2f,%s,%u,%u,%0.1f,%0.1f,%u,%u,%u,%u,%s,%s,%u,%u,%s,%s,%d,%u,%u,%s,%s,%d,%u,%u,%s,%s,%d,%X,%u,%d,%d,%d,%u,%s,%s,%s,%s,%s,%0.3f,%s,%06u",\
      
      aisPacketData.uiHeader,\
      aisPacketData.uiVendorID,\
      aisPacketData.uiSWRev,\
      arrayDataType,\
      packetStatus,\
      imeiPacketBuff,\
      aisDeviceData->unitId,\
      GetPacketData->gpsFix,\
      GetPacketData->day,\
      GetPacketData->month,\
      GetPacketData->year,\
      GetPacketData->hour,\
      GetPacketData->minute,\
      GetPacketData->second,\
      GetPacketData->latitude,\
      GetPacketData->latDir,\
	    GetPacketData->longitude,\
	    GetPacketData->longDir,\
      GetPacketData->speed,\
      GetPacketData->heading,\
      GetPacketData->noOfSat,\
      GetPacketData->altitude,\
      GetPacketData->pDop,\
      GetPacketData->hDop,\
      OperatorBuff,\
      GetPacketData->ignitionState,\
      GetPacketData->mainPwrStatus,\
      GetPacketData->mainIPVolt,\
      GetPacketData->intBatVoltage,\
      GetPacketData->gsmModemState,\
      GetPacketData->rssi,\
      GetPacketData->mcc,\
      GetPacketData->mnc,\
      aisLocationData.packetlac,\
      aisLocationData.packetcid,\
      GetPacketData->mcc1,\
      GetPacketData->mnc1,\
      aisLocationData.packetlac1,\
      aisLocationData.packetcid1,\
      GetPacketData->dbm1,\
      GetPacketData->mcc2,\
      GetPacketData->mnc2,\
      aisLocationData.packetlac2,\
      aisLocationData.packetcid2,\
      GetPacketData->dbm2,\
      GetPacketData->mcc3,\
      GetPacketData->mnc3,\
      aisLocationData.packetlac3,\
      aisLocationData.packetcid3,\
      GetPacketData->dbm3,\
      GetPacketData->rfidTagStatus,\
      GetPacketData->rfidSmState,\
      GetPacketData->RhSensitivity,\
      GetPacketData->Temperature,\
      GetPacketData->dbm4,\
      GetPacketData->input1State,\
      GetPacketData->epcId1,\
      GetPacketData->epcId2,\
      GetPacketData->epcId3,\
      GetPacketData->epcId4,\
      GetPacketData->epcId5,\
      GetPacketData->analogData1,\
      ccidPacketBuff,\
      GetPacketData->frameNumber\
     ); 
    
    if(packetStatus == 'L' )              //for a live NORMAL Packet
    {
      packetReadyCntr = 0;                                                      
      LivePacketReadyFlag = 1;
    }
}

void GpsTask(void)
{
//    if(Is_Gps_Valid())
//    {
//      if(Distance_Task() == 1)
//      {
//        Update_Distance();
//      }
//    }
}


/*----public functions-----*/
 
/**
 *  @brief This function initializes the battery configuration & gps configuration, 
 *  RTC Initialization,Digital IO Initialization,Analog sensor data initialization,Serial ports initalization
 *  GPS, GSM, POWER/ BATTERY Status LED Initialization.
 *  It gets the tick for one second task interval, health packet time interval and  live data packet sending interval.
 *  @param [in]  void
 *  @return void
 */
void InitPacketConfig(void)
{
    Init_LED_Config();
	 // BuzzerInit();
    //InitSerialPorts();
   // RfidInit();
    packetVarInit();
    InitBatteryConfig();
    InitGPSConfig();
    gps_data = ClearGpsPacketData();
    InitRTC();
		RelayEnableInit();
    BuzzerEnableInit();
    //InitDigitalIOConfig();
    //DWT_Delay_Init();
    //RH_SENSOR_GPIO_INIT();
    //InitAnalogSensors();
    oneSecTaskTick           = GetStartTime();
    rfidEpcGetTick           = GetStartTime();
    makeHealthPacketTime     = GetStartTime();
    makeLivePackStartTime    = GetStartTime();
}

void AISGetLivePacketStructCopy(void)
{
   memset(&AISMemPacketStruct,0x00,sizeof(DataPacket_t));
   memcpy( (void*)&AISMemPacketStruct , (void*)&AISPacketStruct , sizeof(AISMemPacketStruct));   //copy the live data buff in mem packet buff to store in mem
   AISMemPacketStruct.packetStatus = HISTORY_PACKET;      
}

void AISGetConnectPacket(void)
{
//    uint8_t* imeiDataPtr = NULL;
////    uint8_t* ccidDataPtr = NULL;
//    uint8_t imepPacketBuff[20];
//    uint8_t ccidPacketBuff[30];
//    tx_packet_length = 0;
//    
//    GetAISDefaultPAcketData();                                                  //Get AIS default Data like vendor Id ,fw version,veh reg number
//    
//    memset(imepPacketBuff,0,sizeof(imepPacketBuff));                            //Get IMEI number
//    memset(ccidPacketBuff,0,sizeof(ccidPacketBuff));                            //Get CCID number
//    
//    imeiDataPtr = GetIMEINumber();
//    strcpy((char*)imepPacketBuff,(char*)imeiDataPtr);
//    
//    ccidDataPtr     = GetCCIDNumber();
//    strcpy((char*)ccidPacketBuff,(char*)ccidDataPtr);
//    
//    memset(packetBuffer, 0, sizeof(packetBuffer));
//    snprintf((char *)packetBuffer,sizeof(packetBuffer),"$%s,CP,%s,%s,V%s,AIS140,0.0,0.0*",\
//      aisDeviceData->aisvehRegNo,\
//      imepPacketBuff,\
//      ccidPacketBuff,\
//      aisPacketData.uiSWRev\
//      );
//    
//    tx_packet_length = aisGetPacketLen((unsigned char*)packetBuffer);
}

void GetFuelData(void)
{ 
   // fuelAvgDataPtr = GetAvgFuelData();
//    AISPacketStruct.analogData1 = fuelAvgDataPtr->fuelLevel;
//    AISPacketStruct.analogData2 =  fuelAvgDataPtr->rawFuelRelativeLevel;//fuelAvgDataPtr->fuelRelativeLevelAvg; //currently sending raw value not averaged value
}

void getGsmGpsDataForPacketing(void)
{
         
        gps_data = GetGpsDataForPacketing();
        if(gps_data->validity == 'A')                                           //If gps data is valid then only copy the values else keep it 0
        {
				
//				    if(gps_data->gpsFix == 4)
//						{
//						  Set_gpsLEDState(GPS_FIX_VALID);						  
//						}
//						else if(gps_data->gpsFix == 5)
//						{
//						  Set_gpsLEDState(GPS_FLOAT_VALID);						
//						}
//						else
//						{
//              Set_gpsLEDState(GPS_VALID);
//						}
				
            //Set_gpsLEDState(GPS_VALID);
            AISPacketStruct.gpsFix    = gps_data->gpsFix;
            AISPacketStruct.validity  = gps_data->validity;
            AISPacketStruct.latitude  = gps_data->latitude;
            AISPacketStruct.latDir    = gps_data->latDir;                            
            AISPacketStruct.longitude = gps_data->longitude;                         
            AISPacketStruct.longDir   = gps_data->longDir;                         
            AISPacketStruct.speed     = gps_data->speed;                               
            AISPacketStruct.heading   = gps_data->heading;                             
            AISPacketStruct.altitude  = gps_data->altitude;                           
            AISPacketStruct.noOfSat   = gps_data->noOfSat;                          
            AISPacketStruct.pDop      = gps_data->pDop;                               
            AISPacketStruct.hDop      = gps_data->hDop;   
        }
        else if(gps_data->validity == 'V')
        {
            //Set_gpsLEDState(GPS_INVALID);
            AISPacketStruct.gpsFix    = gps_data->gpsFix;
            AISPacketStruct.validity  = gps_data->validity;
            AISPacketStruct.latitude  = gps_data->latitude;
            AISPacketStruct.latDir    = gps_data->latDir;                            
            AISPacketStruct.longitude = gps_data->longitude;                         
            AISPacketStruct.longDir   = gps_data->longDir;                         
            AISPacketStruct.speed     = gps_data->speed;                               
            AISPacketStruct.heading   = gps_data->heading;                             
            AISPacketStruct.altitude  = gps_data->altitude;                           
            AISPacketStruct.noOfSat   = gps_data->noOfSat;                          
            AISPacketStruct.pDop      = gps_data->pDop;                               
            AISPacketStruct.hDop      = gps_data->hDop;   
        }
     
       if(PacketTimeStampInvalid == 1)
				{
        AISPacketStruct.day    = 1;
        AISPacketStruct.month  = 1;
        AISPacketStruct.year   = (2000 + time_config.year);//(2000 + time_config.year);
        AISPacketStruct.hour   = time_config.hour;
        AISPacketStruct.minute = time_config.minute;
        AISPacketStruct.second = time_config.second;
				}
				else
				{
				AISPacketStruct.day    = time_config.day;
        AISPacketStruct.month  = time_config.month;
        AISPacketStruct.year   = (2000 + time_config.year);//(2000 + time_config.year);
        AISPacketStruct.hour   = time_config.hour;
        AISPacketStruct.minute = time_config.minute;
        AISPacketStruct.second = time_config.second;
        }
        
        //Get Cell tower id data	
        cellInfoStruct           = GetServingNeighCellInfo();
        AISPacketStruct.rssi     = cellInfoStruct->rssi;
        AISPacketStruct.mcc      = cellInfoStruct->mcc;    
        AISPacketStruct.mnc      = cellInfoStruct->mnc;
        AISPacketStruct.lac      = cellInfoStruct->lac;
        AISPacketStruct.cid      = cellInfoStruct->cellid;
    
        NeighCellInfoStruct      = GetNeighbouringCellInfo();
        AISPacketStruct.mcc1     = NeighCellInfoStruct->nc1Mcc;    
        AISPacketStruct.mnc1     = NeighCellInfoStruct->nc1Mnc;
        AISPacketStruct.lac1     = NeighCellInfoStruct->nc1Lac;
        AISPacketStruct.cid1     = NeighCellInfoStruct->nc1Cellid;
        AISPacketStruct.dbm1     = NeighCellInfoStruct->nc1dbm;
        
        AISPacketStruct.mcc2     = NeighCellInfoStruct->nc2Mcc;    
        AISPacketStruct.mnc2     = NeighCellInfoStruct->nc2Mnc;
        AISPacketStruct.lac2     = NeighCellInfoStruct->nc2Lac;
        AISPacketStruct.cid2     = NeighCellInfoStruct->nc2Cellid;
        AISPacketStruct.dbm2     = NeighCellInfoStruct->nc2dbm;
        
        AISPacketStruct.mcc3     = NeighCellInfoStruct->nc3Mcc;    
        AISPacketStruct.mnc3     = NeighCellInfoStruct->nc3Mnc;
        AISPacketStruct.lac3     = NeighCellInfoStruct->nc3Lac;
        AISPacketStruct.cid3     = NeighCellInfoStruct->nc3Cellid;
        AISPacketStruct.dbm3     = NeighCellInfoStruct->nc3dbm;
        
//      AISPacketStruct.mcc4     = NeighCellInfoStruct->nc4Mcc;    
//      AISPacketStruct.mnc4     = NeighCellInfoStruct->nc4Mnc;
//      AISPacketStruct.lac4     = NeighCellInfoStruct->nc4Lac;
//      AISPacketStruct.cid4     = NeighCellInfoStruct->nc4Cellid;
        AISPacketStruct.dbm4     = NeighCellInfoStruct->nc4dbm;
}

void AISGetPacketChecksum(void)
{
    checkSum = 0;
    tx_packet_length = 0;
   
    memset(PacketServer_buff,0,sizeof(PacketServer_buff));           //copy the live packet from packetBuffer to PacketServer_Buff
   
    checkSum = Crc16Get(packetBuffer,strlen((char*)packetBuffer));   //checksum for the packet
    
    snprintf((char*)PacketServer_buff,sizeof(PacketServer_buff),"%s,%X*",packetBuffer,checkSum); 
    tx_packet_length = aisGetPacketLen((unsigned char*)PacketServer_buff);  //get the length of the packet to be sent
    
}

packetSendState_et GetHttpPacketSendState(void)
{
   return packetSendState;
}


void packetSetState(packetSendState_et state)
{
   if(packetSendState == state)
   {
     return;
   }
   packetSendState = state;
   
   switch(packetSendState)
   { 
       case PACKET_STATE_IDLE:
            LOG_DBG(CH_PACKET,"State - PACKET_STATE_IDLE");
         break;
         
      case PACKET_SEND_DATA:
            LOG_DBG(CH_PACKET,"State - PACKET_SEND_DATA");
        break;
        
      case PACKET_SENDING_IN_PROGRESS:
           LOG_DBG(CH_PACKET,"State - PACKET_SENDING_IN_PROGRESS");
        break;
        
      case PACKET_DATA_SEND_FAILURE:
           LOG_DBG(CH_PACKET,"State - PACKET_DATA_SEND_FAILURE");
           break;
           
      case PACKET_SOCK_CLOSE_WAIT:
           LOG_DBG(CH_PACKET,"State - PACKET_SOCK_CLOSE_WAIT");
        break;
   }
}

uint8_t  packetStateIsIdle(void)
{
   if((packetSendState == PACKET_STATE_IDLE)) 
   {
      return True;
   }
   return False;
}

uint8_t packetStateIsReady(void)
{
   //If packet state is idle and gprs is activated then only open the socket else store it directly into the memory
   if((packetSendState == PACKET_STATE_IDLE) && GsmGprsIsActive() && (GsmSocketIsOpened(0) == 1)) 
   {
      return True;
   }
   return False;
}


void SendSinglePacketProcess(void)
{
       uint8_t flagPvtBusy = 0;
       
      // Packet state machine - check whether live packet is generated,check whether a packet sending in progress
      switch(packetSendState)
      {
          case PACKET_STATE_IDLE:
                
            break;
            
          case PACKET_SEND_DATA:
                flagPvtBusy = PvtSendPacket(PacketServer_buff, tx_packet_length,packetNo);
                if(flagPvtBusy)  //If pvt is in wait for packet
                {
                   flagPvtBusy = 0;
                   packetSetState(PACKET_SENDING_IN_PROGRESS);
                }
                else
                {
                   packetSetState(PACKET_DATA_SEND_FAILURE); 
                }
          break;
            
          case PACKET_SENDING_IN_PROGRESS:
                sendState = GetPvtSendState();
                if(sendState == PVT_ACK_SUCCESS)
                {
                    if(packetSendType == PACKET_HISTORY)
                    {
                      // delete the packet as it is sent succesfully
                        DeleteSentPacket();
                    }
                    packetSetState(PACKET_STATE_IDLE);//PACKET_SOCK_CLOSE_WAIT);
									  PvtSendSetState(PVT_WAIT_FOR_PACKET);
										
                }
                else if((sendState == PVT_SOCK_OPEN_FAIL)||(sendState ==  PVT_SOCK_WRITE_FAIL) || \
                  (sendState == PVT_GET_ACK_FAIL) || (sendState == PVT_GPRS_DEACT_STATE))
                {
                    // if the current packet that was sent is live then store it in memory
                    // other wise no need to store as it is already in memory
                    if(packetSendType == PACKET_LIVE)
                    {
                        LOG_INFO(CH_PACKET,"LIVE PACKET SENDING FAILED... SAVE PACKET TO MEMORY,PT = %d, PN = %d",AISMemPacketStruct.packetType,AISMemPacketStruct.frameNumber);
                        WritePacketToMem((uint8_t*)&AISMemPacketStruct);        //here we have to store the back up copy of struct
                    }
                    //packetSetState(PACKET_SOCK_CLOSE_WAIT);
										 packetSendState = PACKET_STATE_IDLE;
                    if(GetPvtSendState() != PVT_WAIT_FOR_SOCK_OPEN)
                    {
                        PvtSendSetState(PVT_WAIT_FOR_PACKET);
                    }
                }
            break;
            
           case PACKET_DATA_SEND_FAILURE:
              if(packetSendType == PACKET_LIVE)
              {
                LOG_INFO(CH_PACKET,"PVT Statemachine Busy... SAVE PACKET TO MEMORY,PT = %d, PN = %d",AISMemPacketStruct.packetType,AISMemPacketStruct.frameNumber);
                WritePacketToMem((uint8_t*)&AISMemPacketStruct);        //here we have to store the back up copy of struct
              }
              packetSetState(PACKET_SOCK_CLOSE_WAIT);
             break;
             
         case PACKET_SOCK_CLOSE_WAIT:
         
           break;
           
      }
}

void SaveLivePacketStructToMem(void)
{ 
    LOG_INFO(CH_GSM,"SAVE LIVE PACKET TO MEMORY");
    AISPacketStruct.packetStatus = HISTORY_PACKET;
    WritePacketToMem((uint8_t*)&AISPacketStruct);          //dont take the copy of the struct ,store live struct directly to memory
    LivePacketReadyFlag = 0;
    AisClearRfidBuffers();           //clear the current rfid buffers 
}

void checkWhetherToPacketSendOrSave(void)
{
       if(AISPacketStruct.packetStatus == LIVE_PACKET)
      {
           frameNumber++;                                                           //if live packet then increment the frame number
           LOG_INFO(CH_PACKET,"PACKET NUMBER = %d",frameNumber);
           if(frameNumber > 999999){ frameNumber=0;}
           AISPacketStruct.frameNumber  = frameNumber;                               //and get the copy of the frame no in the structure
           
           packetReadyCntr = 0;
           
           if((AISAlertTypeEntry[AISPacketStruct.packetType].flagIsAlertGenerated ==1) )//if packet is a alert ,live packet then clear the flag alert generated
           {
              AISAlertTypeEntry[AISPacketStruct.packetType].flagIsAlertGenerated = 0;  //clear the generated alert
           }
      }
      if(packetStateIsReady() == 0)
      {
        SaveLivePacketStructToMem();   //if gprs is not active or packet state is not idle store packet directly to the memory
      }
      else
      {
          AISGetLivePacketStructCopy();    //Get the copy of this alert data struct to store it if sending fails or if failed to open the socket
          AISMakePacket(&AISPacketStruct);
          AISGetPacketChecksum();          //form the final packet apppending checksum and *
          AisClearRfidBuffers();           //clear the current rfid buffers
          LOG_INFO(CH_PACKET,"AIS PACKET : %s",PacketServer_buff);
          packetSendType = PACKET_LIVE;
          packetSetState(PACKET_SEND_DATA);
      }
}

/**
 *  @brief This function collects data from different modules at every 1 second. It sets the battery state by
 *  calling BatteryMonitorHandler, Gets the current battery level from bat_api file, Gets GPS data from gps_api file,
 *  Gets current time from rtc_port file, Gets cell info data from main file. Depending on the value of 
 *  pack_config.runmode_interval, Make_Packet function will be called and in that, packet_buffer will be filled
 *  @param [in] cellid_para pointer for serving cell tower data received from main app
 *  @return void
 */
static vehicleBatteryState_et prevPwrStatus = VEHICLE_BATTERY_CONNECTED;
//static inputState_e prevIgnitionStatus = INPUT_OFF;
//static  batteryLevelState_et prevIntBatteryStatus = BATTERY_NORMAL;

void PacketTask(void)
{
    supplyInfoStruct = GetSupplyInfo();   //Get Supply Info data
    
    AISPacketStruct.mainPwrStatus = supplyInfoStruct->mainPwrStatus;
    AISPacketStruct.mainIPVolt    = supplyInfoStruct->mainIPVolt;
    AISPacketStruct.intBatVoltage  = supplyInfoStruct->intBatVoltage;
    AISPacketStruct.analogData1    = supplyInfoStruct->analogIp1;
	  //AISPacketStruct.analogData2    = supplyInfoStruct->analogIp2;
    
   //Get Digital Input State and ignition status
    digitalInputStruct             = GetDigitalInputStatus();
    AISPacketStruct.ignitionState  = digitalInputStruct->ignitionState;
    AISPacketStruct.input1State    = GetSerialPwrKeyState();   //serial power enable pin status
//  AISPacketStruct.input1State    = digitalInputStruct->input1State;    
    
    AISPacketStruct.gsmModemState  = GsmGetModemState();  //get the current gsm modem state
    
    AISPacketStruct.rfidTagStatus  = getRfidReadTagStatus();  //get commabd response for the reader
    
   AISPacketStruct.rfidSmState    = GetRfidReaderSendState();   //rfid statemachine state
    
    //RhSensorStruct                 = GetRhSensorData();//get the dht11 sensor data
    
    //AISPacketStruct.RhSensitivity  = RhSensorStruct->RhSensitivity; //get the relative sensitivity
    //AISPacketStruct.Temperature    = RhSensorStruct->Temperature;  //get the temperature
    
    if(supplyInfoStruct->mainPwrStatus == VEHICLE_BATTERY_DISCONNECTED && prevPwrStatus == VEHICLE_BATTERY_CONNECTED  && battCalibrate == 1)
    {
      UpdateAISPacketAlertFlag(VEH_BATT_DISCONNECTED_ALERT);
      prevPwrStatus = AISPacketStruct.mainPwrStatus;                            //update the main power status
    }
    else if(supplyInfoStruct->mainPwrStatus == VEHICLE_BATTERY_CONNECTED  && prevPwrStatus == VEHICLE_BATTERY_DISCONNECTED && battCalibrate == 1)
    {
      UpdateAISPacketAlertFlag(VEH_BATT_CONNECTED_ALERT);
      prevPwrStatus = AISPacketStruct.mainPwrStatus;                            //update the main power status
    }
    
    if(Is_Gps_Valid())
    {
		 //Set_gpsLEDState(GPS_VALID);
		        if(gps_data->gpsFix == 4)
						{
						  Set_gpsLEDState(GPS_FIX_VALID);						  
						}
						else if(gps_data->gpsFix == 5)
						{
						  Set_gpsLEDState(GPS_FLOAT_VALID);						
						}
						else
						{
              Set_gpsLEDState(GPS_VALID);
						}
			 
		}
    
    else{Set_gpsLEDState(GPS_INVALID);}
              
    //GPS Data/Time,GSM Data Cell ID task are called every 1 second
    if(TimeSpent(oneSecTaskTick,DEFAULT_ONE_SEC_TASK_INTERVAL))   
    {
        memset(&time_config,0,sizeof(TimePara_t));
        Get_Current_Time(&time_config);                                         //get and update rtc time and date
        
//        GpsTask();       //gps task to calculate the distance every 1 second
 
        packetReadyTime = liveDataInterval;                                     //history packet send counter depending on run and stop mode interval
        packetReadyCntr++;
        oneSecTaskTick = GetStartTime();
    }
    if(TimeSpent(rfidEpcGetTick,RFID_EPC_GET_DATA_INTERVAL))                   //scan for tags every 5 secs 
    {
      RfidEpcDataMonitorHandler();                                              //check for epc Id's in the epc id buff
      rfidEpcGetTick = GetStartTime();
    } 
    
      AISPacketStruct.packetType  = GetAlertType();                             //get the type of alert code from alert flag,if any alert is been generated then immediatley make the packet and send to pvt statemachine
    
//      if(AISPacketStruct.packetType != NORMAL_ALERT)                            //If any of the alert is generated rather tha NR and HP send them immediately
//      {
//          AISPacketStruct.packetStatus = LIVE_PACKET;
//          getGsmGpsDataForPacketing();                                          //get gsm operator,cell id,gps coordinates for packeting
//          GetFuelData();  
//          checkWhetherToPacketSendOrSave();                                     //if packet state is idle then create socket or else directlr store the packet to the memory  
//      }
//      else if(flagEpcData)
//      {
//           LOG_DBG(CH_RFID,"SENDING 5 TAGS ON SERVER...");
//           flagEpcData  = 0;
//           AISPacketStruct.packetStatus = LIVE_PACKET;
//           getGsmGpsDataForPacketing();                                         //get gsm operator,cell id,gps coordinates for packeting
//           GetFuelData();  
//           checkWhetherToPacketSendOrSave();                                     //if packet state is idle then create socket or else directlr store the packet to the memory
//      }
      //This condition is executed whenever makepack_starttime >= runmode_interval, on true condition,it fills the packetBuffer
	  //else
			if(TimeSpent(makeLivePackStartTime,((liveDataInterval) * MS_CONV_FACTOR)))     //if epc data is pending in the buffer
	  {
           RfidGetRemainingTagCnt();   //If remaining tags are their in the struct then send it in this packet
           
           AISPacketStruct.packetStatus = LIVE_PACKET;
           getGsmGpsDataForPacketing();                                         //get gsm operator,cell id,gps coordinates for packeting
           GetFuelData();
           checkWhetherToPacketSendOrSave();                                     //if packet state is idle then create socket or else directlr store the packet to the memory
		         makeLivePackStartTime = GetStartTime();
	  }
//      else if(TimeSpent(makeHealthPacketTime,healthDataInterval)) //Is Time to send health packet
//      {
//           AISPacketStruct.packetType  = HEALTH_DATA_ALERT;
//           AISPacketStruct.packetStatus = LIVE_PACKET;
//           getGsmGpsDataForPacketing();                                         //get gsm operator,cell id,gps coordinates for packeting
//           GetFuelData(); 
//           checkWhetherToPacketSendOrSave();                                     //if packet state is idle then create socket or else directlr store the packet to the memory
//           makeHealthPacketTime = GetStartTime();
//      }
      else if((GetNumPacketsInMem() > 0) && (packetStateIsReady()))   //((packetReadyTime - packetReadyCntr) >= historyDataInterval )&&check if unit has history packets stored in memory
      {
            //LOG_DBG(CH_GSM,"READ PACKET FROM MEMORY");              //read struct ,form the packet and then send it on server
            packetSendType = PACKET_HISTORY;
            memset(&AISHistoryDataStruct,0,sizeof(DataPacket_t));
            ReadPacketFromMem((uint8_t*)&AISHistoryDataStruct);     //get the history data from the memory
            AISMakePacket(&AISHistoryDataStruct);                   //form the packet from the history data
            AISGetPacketChecksum();                                 //Get the checksum appended final packet in the packetServer_buff
           
            LOG_INFO(CH_PACKET,"AIS HISTORY PACKET : %s",PacketServer_buff);
            packetSetState(PACKET_SEND_DATA);  //change the packet state from idle to create socket
      }
      
      SendSinglePacketProcess();     //This function handles the statemachine to create the socket and send the packet
}


void NtripPacketHandler(void)
{
   // update all the inputs get gsm , gps paramters in AISPacketStruct structure
   NtripPacketTask();  
}


void NtripPacketTask(void)
{
	 if(ntripheaderflag==0 && NtrippacketStateIsReady()==1)
	 {
	  if(TimeSpent(ntripPackStartTime,2000))
		{
		 ntrippacketSetState(NTRIP_PACKET_SEND_DATA);
		 ntripPackStartTime = GetStartTime();
		}
	 }
	 else if((TimeSpent(ntripPackStartTime,((5) * MS_CONV_FACTOR))) && ntripheaderflag == 1 && GsmSocketIsOpened(1)==1)//NtrippacketStateIsReady()==1)     //if epc data is pending in the buffer
	 {
		 NtripPacketSend();                                     //if packet state is idle then create socket or else directlr store the packet to the memory
	   gpggasendflag=1;
		 ntripPackStartTime = GetStartTime();
	 }
	NtripPacketProcess();
}

uint8_t getntripheaderflag(void)
{
  return ntripheaderflag;
}

void resetntripheaderflag(void)
{
  ntripheaderflag=0;
}

void setntripheaderflag(void)
{
  ntripheaderflag=1;
}

uint8_t getgpggasendflag(void)
{
  return gpggasendflag;
}

void resetgpggasendflag(void)
{
  gpggasendflag=0;
}

void setgpggasendflag(void)
{
  gpggasendflag=1;
}

uint8_t NtrippacketStateIsReady(void)
{
   //If packet state is idle and gprs is activated then only open the socket else store it directly into the memory
   if((ntrippacketSendState == NTRIP_PACKET_STATE_IDLE) && GsmGprsIsActive() && (GsmSocketIsOpened(1) == 1)) 
   {
      return True;
   }
   return False;
}

void NtripPacketProcess(void)
{
       uint8_t flagNtripBusy = 0;
       
      // Packet state machine - check whether live packet is generated,check whether a packet sending in progress
      switch(ntrippacketSendState)
      {
          case NTRIP_PACKET_STATE_IDLE:
                
            break;
            
          case NTRIP_PACKET_SEND_DATA:
                flagNtripBusy = NtripSendPacket(NtripPacketServer_buff, Ntrip_tx_packet_length);
                if(flagNtripBusy)  //If pvt is in wait for packet
                {
                   flagNtripBusy = 0;
                   ntrippacketSetState(NTRIP_PACKET_SENDING_IN_PROGRESS);
                }
                else
                {
                   ntrippacketSetState(NTRIP_PACKET_DATA_SEND_FAILURE); 
                }
          break;
            
          case NTRIP_PACKET_SENDING_IN_PROGRESS:
                ntripsendState = GetntripSendState();
					
					      ntrippacketSetState(NTRIP_PACKET_STATE_IDLE);//s
					
                if(ntripsendState == NTRIP_ACK_SUCCESS)
                {
                    
                    ntrippacketSetState(NTRIP_PACKET_STATE_IDLE);//PACKET_SOCK_CLOSE_WAIT);
									  NtripSendSetState(NTRIP_WAIT_FOR_PACKET);
										
                }
                else if((ntripsendState == NTRIP_SOCK_OPEN_FAIL)||(ntripsendState ==  NTRIP_SOCK_WRITE_FAIL) || \
                  (ntripsendState == NTRIP_GET_ACK_FAIL) || (ntripsendState == NTRIP_GPRS_DEACT_STATE))
                {
                    
										 ntrippacketSendState = NTRIP_PACKET_STATE_IDLE;
                    if(GetntripSendState() != NTRIP_WAIT_FOR_SOCK_OPEN)
                    {
                        NtripSendSetState(NTRIP_WAIT_FOR_PACKET);
                    }
                }
            break;
            
           case NTRIP_PACKET_DATA_SEND_FAILURE:
              
              ntrippacketSetState(NTRIP_PACKET_SOCK_CLOSE_WAIT);
             break;
             
         case NTRIP_PACKET_SOCK_CLOSE_WAIT:
         
           break;
      }
}

void ntrippacketSetState(ntrippacketSendState_et state)
{
   if(ntrippacketSendState == state)
   {
     return;
   }
   ntrippacketSendState = state;
   
   switch(ntrippacketSendState)
   { 
       case NTRIP_PACKET_STATE_IDLE:
            LOG_DBG(CH_PACKET,"State - NTRIP_PACKET_STATE_IDLE");
         break;
         
      case NTRIP_PACKET_SEND_DATA:
            LOG_DBG(CH_PACKET,"State - NTRIP_PACKET_SEND_DATA");
        break;
        
      case NTRIP_PACKET_SENDING_IN_PROGRESS:
           LOG_DBG(CH_PACKET,"State - NTRIP_PACKET_SENDING_IN_PROGRESS");
        break;
        
      case NTRIP_PACKET_DATA_SEND_FAILURE:
           LOG_DBG(CH_PACKET,"State - NTRIP_PACKET_DATA_SEND_FAILURE");
           break;
           
      case NTRIP_PACKET_SOCK_CLOSE_WAIT:
           LOG_DBG(CH_PACKET,"State - NTRIP_PACKET_SOCK_CLOSE_WAIT");
        break;
   }
}

void NtripPacketSend(void)
{
	NtripPacket();
	LOG_INFO(CH_PACKET,"Ntrip send String : %s",NtripPacketServer_buff);
	ntrippacketSetState(NTRIP_PACKET_SEND_DATA);     
}

void NtripPacket(void)
{
    //checkSum = 0;
    Ntrip_tx_packet_length = 0;
   
    memset(NtripPacketServer_buff,0,sizeof(NtripPacketServer_buff));           //copy the live packet from packetBuffer to PacketServer_Buff
    snprintf((char*)NtripPacketServer_buff,sizeof(NtripPacketServer_buff),"%s",gpggapacketBuffer);
    //snprintf((char*)NtripPacketServer_buff,sizeof(NtripPacketServer_buff),"$GPGGA,055049.000,1906.8446506,N,07252.8416473,E,1,12,1.8,17.527,M,-65.200,M,,0000*4B\r\n");		
    Ntrip_tx_packet_length = ntripGetPacketLen((unsigned char*)NtripPacketServer_buff);  //get the length of the packet to be sent
    
}

uint16_t  ntripGetPacketLen(uint8_t *ucSourceAdr)
{
	uint16_t ntripucTemp1 = 0, ntripgetlen =0;
	packet_length = 0;
    
    ntripucTemp1 = ucSourceAdr[packet_length];
    
    if( ntripucTemp1 == '$')
    {
      ntripgetlen = 1;
    }
    else
    {
      return 0;
    }
    if(ntripgetlen)
    {
        for( ; ntripucTemp1 != '*'  ; packet_length++)
        {
            ntripucTemp1 = ucSourceAdr[packet_length];
            if(packet_length >= 1500) //512
            {
              return 0;
            }
        }
    }
	//return(packet_length);
	return(packet_length+4);
}
