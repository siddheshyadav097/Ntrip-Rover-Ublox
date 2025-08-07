#include "ais_app.h"
#include "gsm_api.h"
#include "gsm_sim_api.h"
#include "gsm_gprs_api.h"
#include "gsm_socket_api.h"
#include "gsm_idle_task.h"
#include "mem_config_api.h"
#include "mem_packet_api.h"
#include "lib_port.h"
#include "crc24q.h"

pvtSendHandler_et   pvtSendState = PVT_WAIT_FOR_PACKET;



ntripSendHandler_et   ntripSendState = NTRIP_WAIT_FOR_PACKET;
static rtcm_state_t rtcm_state = RTCM_STATE_WAIT_HEADER;
static rtcm_frame_state_t rtcm_frame_state = RTCM_FRAME_START;
static uint8_t header_buffer[64];
static uint16_t header_index = 0;
static uint8_t rtcm_data_buffer[2048];  // Make sure this is large enough
static uint16_t rtcm_data_length = 0;
static uint16_t rtcm_data_index = 0;
static uint16_t rtcm_expected_length = 0;
static uint32_t rtcm_crc = 0;
static uint8_t rtcm_crc_bytes[4];
static uint8_t rtcm_crc_index = 0;
uint8_t ntripPacketBuff[4096]= {0};
uint32_t nontripresponsetick=0;
static HeaderParseState header_parse_state = HEADER_STATE_START;


uint8_t AISSock0ReadBuff[MAX_SIZE_OF_READ_BUFF]= {0};
uint8_t AISSock1ReadBuff[MAX_SIZE_OF_READ_BUFF]={0};
uint16_t AISSock0ReadLen = 2500;
uint16_t AISSock1ReadLen = 2500;

uint8_t aisSock0 = 0 , aisSock1 = 1;

//uint8_t* packetdata = NULL;
//uint16_t PacketLen =0;
uint8_t socId =0;
uint8_t ntripsocId=1;
char *strHead = NULL,*strTail = NULL, *head= NULL;
uint8_t *start, *end, *para1, *para2, *para3, *recpPhnoPtr = NULL;
char *startAddress , *tempAdd1 = NULL; 

socketResponse_et sockRespState = SOCK_RESP_CHECK_START;
uint8_t sockRespBuff[512]= {0};
uint16_t sockRespIndex = 0;
uint32_t sockRespTick  =0; 
uint8_t fotaResponse = 0;

char tmpbuf[50]= {0};
static char smsStrTmp[50]= {0};
char tmp_buff[250],sendAckSmsBuff[200]= {0};
uint16_t SendSMSLen = 0;
uint8_t tmp_var[100]= {0};
uint8_t tmpVarLen = 0;
uint8_t urlBuffer[256]= {0};
uint32_t tmp_val =0;
float thVal =0;
uint8_t simSlot =0;
uint32_t aisPacketNo =0;


volatile uint8_t smsMessage[512]= {0};
volatile uint8_t smsNum[20]= {0};

uint8_t host[100] , path[100]= {0};
uint8_t ntriphost[100] , ntrippath[100]= {0};
uint8_t  httpHeader[512]= {0};
uint8_t  ntriphttpHeader[512]= {0};
uint8_t httpErrorBuff[50]= {0};

uint8_t httpPacketBuff[PACKET_BUFSIZE]= {0};

uint16_t httpHeaderLen = 0 ,totPacketLen = 0;
uint16_t ntriphttpHeaderLen = 0 ,ntripPacketLen=0;
//uint8_t flagSendFotaResponse = 0;   //flag to send the fota response on the server

uint8_t *fotaPhnoPtr = NULL;

intervals_st*       intervalsPtr = {0};
thresholds_st*      thresholdsPtr= {0};
emergencyParams_st* emergencyParamsPtr= {0};

serverConfig_st* server0ConfigPtr = {0};
serverConfig_st* server1ConfigPtr = {0};
ntripcredConfig_st* ntripcredPtr = {0};
gprsConfig_st* gprsConfigSim0Ptr= {0};
gprsConfig_st* gprsConfigSim1Ptr= {0};

aisDataConfig_st* aisDeviceDataPtr= {0};

cellIdStruct_st* cellIdData= {0};

SupplyInfo_st*   dbgSupplyInfo= {0};

rfReaderPwrTh_st* RfReaderThPtr= {0};
uint8_t respStarCnt = 0;

BuzzerState_et BuzzerState = BUZZER_IDLE_STATE;

//rfidConfigStruct_st* RfConfigrations= {0};

gsmSockSecureType_et s0SecurityType = GSM_SOCK_UNSECURE;
gsmSockSecureType_et s1SecurityType = GSM_SOCK_UNSECURE;

socketAck_et sockAckVal = SOCK_ACK_CHECK_START;

//uint8_t memSimSlotVal = 0;
extern uint32_t locationToBeWritten;
extern uint32_t currentReadLocationNo;
extern uint32_t numPacketsInMem;
extern uint32_t maxPacketsInMem;
//extern uint8_t HealthDataBuff[100];

DebugCmd_st deviceCmd = {0};
uint8_t debugCmdBuff[512]= {0};
ringBuffer_st serialCmdRing= {0};

uint8_t gprsCmdBuff[250]; //gprs command received from sock 1
uint16_t gprsCmdLen = 0;
uint8_t gprsRespBuff[250]; //gprs response to be sent on sock 2
uint16_t gprsRespLen = 0;
uint8_t flagGPRSCmd = 0;

uint8_t buzzerState = 0;
uint8_t RelayState = 0;

extern uint8_t flagGsmGprsDeactivate ;

static uint8_t GsmProcessSmsConfigCmd(uint8_t* cmd_databuff, uint16_t cmd_datalen);

void (* gsmLedStatusCb)(gsmStatus_et status);

uint32_t buzzerPulseTick = 0;
uint8_t flagBuzzerState = 0;
uint8_t flagBuzzerOff =0;
uint32_t buzzerOnTick = 0;

uint8_t flagStartRelay = 0;
uint8_t relayCount = 0;
uint32_t RelayStartTick = 0;
uint8_t flagRelayOn= 0;


static const char base64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

const char *username = "rover3";//"QdNetRover02";
const char *password = "adpcmm@850";//"Quectel2024";
char output_b64[128];  
const char *mountpoint = "testmount";
uint8_t* ntripuser_ccid;













void PrepareDebugSms(void)
{
  uint8_t *imeiPtr = {0};
  uint8_t gprsinitialized =0;
  
  uint8_t httpSmState = 0;
  
  char latDir, longDir;
  //float latitude, longitude;
	double latitude, longitude;
  GpsData_t *gpsData = {0};
  
  uint8_t rfRdrPwrState = 0;
  
  httpSmState = GetPvtSendState();//GetHttpPacketSendState();
  
  SendSMSLen = 0;

  gpsData = GetGpsDataForPacketing();
  latDir = gpsData->latDir;
  latitude = gpsData->latitude;
  longDir   = gpsData->longDir;
  longitude = gpsData->longitude;
 
  //memset(imeiPtr,0,sizeof(imeiPtr));
  imeiPtr = getIMEI();
  gprsinitialized = GsmGprsIsActive();
  //cellIdData      = GetServingNeighCellInfo();
  //rfRdrPwrState   = GetOutput1State();
  dbgSupplyInfo   = GetSupplyInfo();
  
  GetUnitIdFrmMemForAppFile();
  
  if(Is_Gps_Valid() == 0)
  {
       //currently using only 
//     861359033663731,V1.0.0,60,60,900,60,V,26,qdnet.vodafone.in,internet,1,ais140.qdnet.com,1910,ais140.qdnet.com,1910
    SendSMSLen =snprintf((char*)sendAckSmsBuff,sizeof(sendAckSmsBuff),"%s,%s,V%d.%d.%d,%d,%d,%d,%d,V,%d,%s,%s,%s,%d,%s,%d,%d,%0.1f,%0.1f,%d,%d",\
            aisDeviceDataPtr->unitId,imeiPtr,MAJOR_SW_VER,MINOR_SW_VER,MICRO_SW_VER,\
            intervalsPtr->runModeInterval_sec,intervalsPtr->stopModeInterval_sec,intervalsPtr->healthPacketInterval_sec,httpSmState,\
            cellIdData->rssi,gprsConfigSim0Ptr->apn,gprsConfigSim0Ptr->username,gprsConfigSim0Ptr->pass,\
            gprsinitialized,server0ConfigPtr->serverAddr,server0ConfigPtr->port,rfRdrPwrState,dbgSupplyInfo->mainIPVolt,dbgSupplyInfo->intBatVoltage,\
            numPacketsInMem,maxPacketsInMem);
  }
  else
  {
  SendSMSLen = snprintf((char*)sendAckSmsBuff,sizeof(sendAckSmsBuff),"%s,%s,V%d.%d.%d,%d,%d,%d,%d,A,%0.1f,%f,%c,%f,%c,%d,%s,%s,%s,%d,%s,%d,%d,%0.1f,%0.1f,%d,%d",aisDeviceDataPtr->unitId,imeiPtr,MAJOR_SW_VER,MINOR_SW_VER,MICRO_SW_VER,\
            intervalsPtr->runModeInterval_sec,intervalsPtr->stopModeInterval_sec,intervalsPtr->healthPacketInterval_sec,httpSmState,\
            gpsData->speed,latitude,latDir,longitude,longDir,\
            cellIdData->rssi,gprsConfigSim0Ptr->apn,gprsConfigSim0Ptr->username,gprsConfigSim0Ptr->pass,\
            gprsinitialized,server0ConfigPtr->serverAddr,server0ConfigPtr->port,rfRdrPwrState,dbgSupplyInfo->mainIPVolt,dbgSupplyInfo->intBatVoltage,\
            numPacketsInMem,maxPacketsInMem);
  }
}

void GetIpPort1Details(void)
{
 SendSMSLen =  snprintf((char *)sendAckSmsBuff,sizeof(sendAckSmsBuff),"\nIPPORT_1:%s,%d",server0ConfigPtr->serverAddr,server0ConfigPtr->port);
}
void GetIpPort2Details(void)
{
 SendSMSLen =  snprintf((char *)sendAckSmsBuff,sizeof(sendAckSmsBuff),"\nIPPORT_2:%s,%d",server1ConfigPtr->serverAddr,server1ConfigPtr->port);
}

void GetTrackInterval(void)
{
  SendSMSLen = snprintf((char *)sendAckSmsBuff, sizeof(sendAckSmsBuff),"\nTRACKINTERVAL:%d,%d,%d,%d",intervalsPtr->runModeInterval_sec,intervalsPtr->stopModeInterval_sec,intervalsPtr->healthPacketInterval_sec,intervalsPtr->emergencyPacketInterval_sec);
}

void GetAlertDetails(void)
{
  SendSMSLen = snprintf((char *)sendAckSmsBuff, sizeof(sendAckSmsBuff),"\nALERTDETAILS:%d,%d,%d,%d",thresholdsPtr->speedThreshold,thresholdsPtr->harshBrakingThreshold,thresholdsPtr->accelerationThreshold,thresholdsPtr->rashTurnThreshold);
}

void GetEmerDetails(void)
{
  SendSMSLen = snprintf((char *)sendAckSmsBuff, sizeof(sendAckSmsBuff),"\nEMERDETAILS:%s,%d",emergencyParamsPtr->mobileNumber,emergencyParamsPtr->timeout_sec);
}

void GetSim1GprsConfig(void)
{
   SendSMSLen = snprintf((char *)sendAckSmsBuff, sizeof(sendAckSmsBuff),"\nAPNUIDPWD_0:\"%s\",\"%s\",\"%s\"",gprsConfigSim0Ptr->apn,gprsConfigSim0Ptr->username,gprsConfigSim0Ptr->pass);
}

void GetNtripConfig(void)
{
   SendSMSLen = snprintf((char *)sendAckSmsBuff, sizeof(sendAckSmsBuff),"\nNtrip_Details:\"%s\",\"%s\",\"%s\"",ntripcredPtr->mountpoint,ntripcredPtr->ntripusername,ntripcredPtr->ntrippass);
}

/**
 *  @brief Process_Config_Cmd processes the configuration commands received either in sms or from server acknowledgement
 *  response, before processing any command it will first check the password received in cmd_databuff with the pre-defined
 *  password. Once password is Matched, it will check for different configuration formats, if any format is matched then it 
 *  copies all the new parameters for that configuration format into their respective structures & buffers and also
 *  updates their respective Configuration memory files.

		Existing  response for the packet is in ($ERROR=&UN=7000&PN=12227*)  this format. Kindly add these changes in the existing response  to receive a command through GPRS  ($ERROR=&CMD=COMMAND&UN=7000&PN=12227*) .

		Example:

		CMD= **1234Buzzer_Relay_State=A,B##&UN=7000&PN=12227*

		 **1234Buzzer_Relay_State=A,B##

		 A= Buzzer On/OFF (1 : ON , 0: OFF)

		 B= Relay On/OFF   (1 : ON , 0: OFF)
		1. **1234Buzzer_Relay_State=1,0##


 *  @param [in] cmd_databuff buffer to hold gprs command response
 *  @param [in] cmd_datalen  data length of gprs command response
 *  @return void
 */
static uint8_t GsmProcessSmsConfigCmd(uint8_t* cmd_databuff, uint16_t cmd_datalen)
{
    static uint8_t CheckPasswordBuff[5] = {0};
    para1 = NULL;
    para2 = NULL;
    uint8_t imeiBuffPtr[25] = {0};
    uint8_t ccidDataPtr[25] = {0};
    startAddress = NULL;
    tempAdd1 = NULL;
    uint8_t i =0;
    uint8_t OpeartorDataPtr[30] = {0};
    uint8_t  sim1CCIDBuff[25],sim1APN[20],sim1OpsName[20]= {0};
	
	memset(tmp_buff, 0, sizeof(tmp_buff));
    memset(tmp_var,0,sizeof(tmp_var));
    memset(smsStrTmp,0,sizeof(smsStrTmp));
	if((start = (uint8_t*)strstr((char*) cmd_databuff,"**")) != NULL)
	{	
        end = (uint8_t*)strstr((char*) start,"##");
        if(end)
        {	
            strncpy((char*)tmp_buff,(const char*)start+2+4, end-start-2-4);     //tmp_buff has the sms command
            memset(CheckPasswordBuff, 0, sizeof(CheckPasswordBuff));
            strncpy((char*)CheckPasswordBuff,(const char*)start+2, 4);          //**1234SET_IPPORT_1="hawkeye.qdvts.com",1883,##
            
            if(strcmp((const char*)CONFIGPASSWORD,(const char*)CheckPasswordBuff) == 0)//password has to be 1234
            {
               memset(sendAckSmsBuff, 0, sizeof(sendAckSmsBuff));
                if((start = (uint8_t*)strstr((const char*) tmp_buff,"GET_IMEI")) != NULL)//**1234GET_IMEI##
               {
                   memset(imeiBuffPtr,0,sizeof(imeiBuffPtr));
                   GetIMEINumber(imeiBuffPtr);
                   SendSMSLen = snprintf((char *)sendAckSmsBuff,sizeof(sendAckSmsBuff),"\nIMEI:%s",imeiBuffPtr);
                  return 1;
               }
							 else if((start = (uint8_t*)strstr((const char*) tmp_buff,"Buzzer_Relay_State=")) != NULL)
							 {
							        //**1234Buzzer_Relay_State=A,B##
								      startAddress = strchr((const char*)tmp_buff,'=');
                       if(startAddress != NULL)
                       {
													startAddress++;
													tempAdd1 = startAddress;
												 
												  //A= Buzzer On/OFF (1 : ON , 0: OFF)
                          //B= Relay On/OFF   (1 : ON , 0: OFF)
										
													strTail = strchr((char*)tempAdd1,',');
													if(strTail != NULL)
													{
														memset(smsStrTmp,0,sizeof(smsStrTmp));
														strncpy((char*)smsStrTmp,tempAdd1,strTail - tempAdd1);
														tmp_val = atoi((char*)smsStrTmp);
														
														buzzerState = tmp_val;
														
														LOG_DBG(CH_GSM,"BUZZER_STATE: %d",tmp_val);
														
														writeBuzzerPinState(tmp_val);   //set or reset the buzzer as per the state received from the server
														strTail++;
														
														startAddress = strchr((char*)tempAdd1,'\0');
														memset(smsStrTmp,0,sizeof(smsStrTmp));
														strncpy((char*)smsStrTmp,strTail,startAddress - strTail);
														tmp_val = atoi((char*)smsStrTmp);
														
														RelayState = tmp_val;
														
														writeRelayPinState(tmp_val);
														
														LOG_DBG(CH_GSM,"RELAY_STATE: %d",tmp_val);
														
													}
                           
                 SendSMSLen =snprintf((char *)sendAckSmsBuff,sizeof(sendAckSmsBuff),"\nBUZZER_RELAY_STATE:%d,%d",buzzerState,RelayState);
                 return 1;
                       }    
							 }
               else if((start = (uint8_t*)strstr((const char*) tmp_buff,"GET_UNITID")) != NULL)//**1234GET_UNITID##
               {
                    GetUnitIdFrmMemForAppFile();
                    SendSMSLen =snprintf((char *)sendAckSmsBuff,sizeof(sendAckSmsBuff),"\nUNIT_ID:%s",aisDeviceDataPtr->unitId);
                     
                    return 1;
               }
//               else if((start = (uint8_t*)strstr((const char*) tmp_buff,"GET_READER_CONFIG")) != NULL)//**1234GET_READER_CONFIG##
//               {
//                    RfConfigrations = RfidGetReaderConfigrations();
//                    SendSMSLen = snprintf((char *)sendAckSmsBuff,sizeof(sendAckSmsBuff),"\nReader_Configurations:%X,%X,%X,%X,%X,%X",RfConfigrations->readersAdd,RfConfigrations->rfPower,RfConfigrations->rfScanTime ,\
//                     RfConfigrations->readerWorkMode , RfConfigrations->rfMaxFrequency , RfConfigrations->rfMinFrequency );
//                    
//                    return 1;
//               }
               else if((start = (uint8_t*)strstr((const char*) tmp_buff,"GET_IPPORT_1")) != NULL)//**1234GET_IPPORT_1##
               {
                  GetIpPort1Details();
                   return 1;
               }
							 else if((start = (uint8_t*)strstr((const char*) tmp_buff,"GET_IPPORT_2")) != NULL)//**1234GET_IPPORT_1##
               {
                  GetIpPort2Details();
                   return 1;
               }
               else if((start = (uint8_t*)strstr((const char*) tmp_buff,"GET_SIMDETAILS")) != NULL)//**1234GET_SIMDETAILS##
               {
               //"SIMDETAILS: 8991200010568247133F,Vodafone,www
               //If user wants the details of the current working sim then check in memory it is active on which sim slot
               //memSimSlotVal update that value in this variable
                 
                 memset(sim1CCIDBuff,0,sizeof(sim1CCIDBuff));
                 memset(sim1APN,0,sizeof(sim1APN));
                 memset(sim1OpsName,0,sizeof(sim1OpsName));
                 memset(ccidDataPtr,0,sizeof(ccidDataPtr));
                 GetCCIDNumber(ccidDataPtr);
                 GetOperatorNameInit(OpeartorDataPtr);
                 
                  strncpy((char*)sim1CCIDBuff ,(char*)ccidDataPtr,sizeof(sim1CCIDBuff));
                  strncpy((char*)sim1APN,      (char*)gprsConfigSim0Ptr->apn,sizeof(sim1APN));
                  strncpy((char*)sim1OpsName , (char*)OpeartorDataPtr,sizeof(sim1OpsName));
    
                 SendSMSLen = snprintf((char *)sendAckSmsBuff, sizeof(sendAckSmsBuff),"\nSIMDETAILS:%s,%s,%s",\
                 sim1CCIDBuff,\
                 sim1OpsName,\
                 sim1APN\
                 );
                 return 1;
               }
               else if((start = (uint8_t*)strstr((const char*) tmp_buff,"GET_APNUIDPWD_0")) != NULL)//**1234GET_APNUIDPWD_0##
               {
                  GetSim1GprsConfig();
                 return 1;
               }
               else if((start = (uint8_t*)strstr((const char*) tmp_buff,"GET_ALERTDETAILS")) != NULL)//**1234GET_ALERTDETAILS##
               {
                 GetAlertDetails();
                 return 1;
               }
               else if((start = (uint8_t*)strstr((const char*) tmp_buff,"GET_TRACKINTERVAL")) != NULL)//**1234GET_TRACKINTERVAL##
               {
                   GetTrackInterval();
                    return 1;
               }
               else if((start = (uint8_t*)strstr((const char*) tmp_buff,"GET_EMERDETAILS")) != NULL)//**1234GET_EMERDETAILS##
               {
                 GetEmerDetails();
                 return 1;
               }
               else if((start = (uint8_t*)strstr((const char*) tmp_buff,"GET_DEBUG")) != NULL)//**1234GET_DEBUG##
               {
                 PrepareDebugSms();
                 return 1;
               }  
               else if((start = (uint8_t*)strstr((const char*) tmp_buff,"GET_MEMDEBUG")) != NULL)//**1234GET_MEMDEBUG##
               {
                  SendSMSLen =snprintf((char *)sendAckSmsBuff, sizeof(sendAckSmsBuff),"\nMEMDEBUG:W-%d,R-%d,N-%d,T-%d",locationToBeWritten,currentReadLocationNo,numPacketsInMem,maxPacketsInMem);
                 return 1;
               } 
               else if((start = (uint8_t*)strstr((const char*) tmp_buff,"GET_READER_TH")) != NULL)//**1234GET_READER_TH##
               {
                 RfReaderThPtr = GetRfidPwrThValues();
                 SendSMSLen = snprintf((char *)sendAckSmsBuff, sizeof(sendAckSmsBuff),"\nRDR_TH:%0.1f,%0.1f,%0.1f,%0.1f,%d,%d",RfReaderThPtr->rfRdrLowerTh_12V,RfReaderThPtr->rfRdrUpperTh_12V,RfReaderThPtr->rfRdrLowerTh_24V,RfReaderThPtr->rfRdrUpperTh_24V,RfReaderThPtr->ReaderOnToOffTime,RfReaderThPtr->ReaderOffToOnTime);
                 
                 return 1;
               } 
//               else if((start = (uint8_t*)strstr((const char*) tmp_buff,"SET_READER_TH")) != NULL)//**1234SET_READER_TH=12.5,12.6,25.5,25.6,##
//               {
//                       RfReaderThPtr = GetRfidPwrThValues();
//                       startAddress = strchr((const char*)tmp_buff,'=');
//                       if(startAddress != NULL)
//                       {
//                           startAddress++;
//                           tempAdd1 = startAddress;
//                           for(i=0; i<6 ;i++)
//                           {
//                                strTail = strchr((char*)tempAdd1,',');
//                                if(strTail != NULL)
//                                {
//                                  memset(smsStrTmp,0,sizeof(smsStrTmp));
//                                  strncpy((char*)smsStrTmp,tempAdd1,strTail - tempAdd1);
//                                  thVal = atof((char*)smsStrTmp);
//                                  strTail++;
//                                  tempAdd1 = strTail;
//                                  if((thVal != 0.0) && (i == 0)){RfReaderThPtr->rfRdrLowerTh_12V          = thVal;}
//                                  else if((thVal != 0.0) && (i == 1)){RfReaderThPtr->rfRdrUpperTh_12V     = thVal;}
//                                  if((thVal != 0.0) && (i == 2)){RfReaderThPtr->rfRdrLowerTh_24V          = thVal;}
//                                  else if((thVal != 0.0) && (i == 3)){RfReaderThPtr->rfRdrUpperTh_24V     = thVal;}
//                                  else if((thVal != 0) && (i == 4)){RfReaderThPtr->ReaderOnToOffTime     = (uint32_t)thVal;}
//                                  else if((thVal != 0) && (i == 5)){RfReaderThPtr->ReaderOffToOnTime     = (uint32_t)thVal;}
//                                }
//                           }
//                    SetRfidPowerThresholds(RfReaderThPtr);  //update the Th values in the memory   
//                    RfidGetReaderPwrTh();                   //update the thresholds for rfid reader pwr vtg
//                    }
//                    return 0;
//               }
          else if((start = (uint8_t*)strstr((const char*) tmp_buff,"SET_IPPORT_1=")) != NULL) //**1234SET_IPPORT_1="hawkeye.qdvts.com",1883,##
				  {
//                 LOG_INFOS(CH_SMS,"Change IPPORT 1 value");
                   server0ConfigPtr = GetServer0Config();
            if((para1 = (uint8_t*)strstr((const char*)tmp_buff, ",")) != NULL)
					  {
						  memset(server0ConfigPtr,0,sizeof(serverConfig_st));      //check       
						  strncpy((char *)server0ConfigPtr->serverAddr,(const char*)start+14,para1-start-15);
						
						  if((para2 = (uint8_t*)strstr((const char*) para1+1, ",")) != NULL)
						  {	
								memset(tmp_var,0,sizeof(tmp_var)); 
								strncpy((char *)tmp_var,(const char*)para1+1, para2-para1-1);
								server0ConfigPtr->port = atoi((const char*)tmp_var);
								strcpy((char*)server0ConfigPtr->serverReqType,DEFAULT_SERVER_REQ_TYPE);
								SetServer0Config(server0ConfigPtr);                 //set new ip and port into the memory
								server0ConfigPtr = GetServer0Config();
								memset(host,0,sizeof(host));
								memset(path,0,sizeof(path));
								if(sscanf((const char*)server0ConfigPtr->serverAddr, "%99[^/]%99[^\n]",host,path) == 2)
							  {
								//LOG_DBG(CH_SOCK,"Http Host Name and Path Resolved");
							  }
							  GsmUpdateIpPort(aisSock0,host,path,server0ConfigPtr->port,server0ConfigPtr->serverReqType);
//                            LOG_INFO(CH_SMS,"Server 1 IP = %s,Server 1 Port = %d",server0ConfigPtr->serverAddr,server0ConfigPtr->port);
							  GetIpPort1Details();
							return 1;
						  }
					 }

				}
				
				  else if((start = (uint8_t*)strstr((const char*) tmp_buff,"SET_IPPORT_2=")) != NULL) //**1234SET_IPPORT_1="hawkeye.qdvts.com",1883,##
				  {
//                 LOG_INFOS(CH_SMS,"Change IPPORT 1 value");
                   server1ConfigPtr = GetServer1Config();
            if((para1 = (uint8_t*)strstr((const char*)tmp_buff, ",")) != NULL)
					  {
						  memset(server1ConfigPtr,0,sizeof(serverConfig_st));      //check       
						  strncpy((char *)server1ConfigPtr->serverAddr,(const char*)start+14,para1-start-15);
						
						  if((para2 = (uint8_t*)strstr((const char*) para1+1, ",")) != NULL)
						  {	
								memset(tmp_var,0,sizeof(tmp_var)); 
								strncpy((char *)tmp_var,(const char*)para1+1, para2-para1-1);
								server1ConfigPtr->port = atoi((const char*)tmp_var);
								strcpy((char*)server1ConfigPtr->serverReqType,DEFAULT_SERVER_REQ_TYPE);
								SetServer1Config(server1ConfigPtr);                 //set new ip and port into the memory
								server1ConfigPtr = GetServer1Config();
								memset(ntriphost,0,sizeof(ntriphost));
								memset(ntrippath,0,sizeof(ntrippath));
								if(sscanf((const char*)server1ConfigPtr->serverAddr, "%99[^/]%99[^\n]",ntriphost,ntrippath) == 2)
							  {
								//LOG_DBG(CH_SOCK,"Http Host Name and Path Resolved");
							  }
							  GsmUpdateIpPort(aisSock1,ntriphost,ntrippath,server1ConfigPtr->port,server1ConfigPtr->serverReqType);
//                            LOG_INFO(CH_SMS,"Server 1 IP = %s,Server 1 Port = %d",server0ConfigPtr->serverAddr,server0ConfigPtr->port);
							  GetIpPort2Details();
							return 1;
						  }
					 }

				}
				
				else if((start = (uint8_t*)strstr((const char*) tmp_buff,"GET_NTRIP_DETAILS")) != NULL)//**1234GET_DEBUG##
               {
                 GetNtripConfig();
                 return 1;
               }
				
				
				
				
                //set track interval 
                else if((start = (uint8_t*)strstr((const char*) tmp_buff,"SET_TRACKINTERVAL=")) != NULL)//**1234SET_TRACKINTERVAL=Normal,Sleep,health,Emergnecy,##
               {  
                       //**1234SET_TRACKINTERVAL=5,10,10,20,##
//                       LOG_INFOS(CH_SMS,"Change Track Intervals");
                       intervalsPtr = GetIntervals();
                       
                       startAddress = strchr((const char*)tmp_buff,'=');
                       if(startAddress != NULL)
                       {
                           startAddress++;
                           tempAdd1 = startAddress;
                           for(i=0; i<4 ;i++)
                           {
                                strTail = strchr((char*)tempAdd1,',');
                                if(strTail != NULL)
                                {
                                  memset(smsStrTmp,0,sizeof(smsStrTmp));
                                  strncpy((char*)smsStrTmp,tempAdd1,strTail - tempAdd1);
                                  tmp_val = atoi((char*)smsStrTmp);
                                  strTail++;
                                  tempAdd1 = strTail;
                                  if(tmp_val != 0 && i == 0){intervalsPtr->runModeInterval_sec           = tmp_val;}
                                  else if(tmp_val != 0 && i == 1){intervalsPtr->stopModeInterval_sec     = tmp_val;}
                                  else if(tmp_val != 0 && i == 2){intervalsPtr->healthPacketInterval_sec = tmp_val;}
                                  else if(tmp_val != 0 && i == 3){intervalsPtr->emergencyPacketInterval_sec  = tmp_val;}
                                }
                           }
                        SetIntervals(intervalsPtr);                             //set intervals in the memory
                        intervalsPtr = GetIntervals();                          //get updated intervals from the memory
                        GetPacketIntervalVal();
                        GetTrackInterval();
                        return 1;
                       }    
               }
               else if((start = (uint8_t*)strstr((const char*) tmp_buff, "SET_UNITID=")) != NULL)//**1234SET_UNITID=KHGJM5678##
               {  
//                       LOG_INFOS(CH_SMS,"Change Vehicle Number");
                       memset(tmp_var,0,sizeof(tmp_var));
												strcpy((char *)tmp_var,(const char*)start+11);
                       tmpVarLen = strlen((char*)tmp_var);
                       //To DO: Copy this tmp_var to vehBuff and update it in the memory
                       GetUnitIdFrmMemForAppFile();                                 //get ptr of mem data
                       memset(aisDeviceDataPtr, 0, sizeof(aisDataConfig_st));
                        strncpy((char *)aisDeviceDataPtr->unitId,(const char*)tmp_var,strlen((char*)tmp_var));  //update the values in ptr to struct
                       SetVehRegNum(aisDeviceDataPtr);                           //set new vendor id into the memory
                       GetUnitIdFrmMemForAppFile();                                 //get updated value from the memory
                       GetUnitIdForPacketing();                                     //update vendor id for packeting
                        SendSMSLen = snprintf((char *)sendAckSmsBuff, sizeof(sendAckSmsBuff),"\nUNIT_ID:%s",aisDeviceDataPtr->unitId);
                       
                       return 1;
               }
               else if((start = (uint8_t*)strstr((const char*) tmp_buff, "SET_APNUIDPWD_0=")) != NULL)//**1234SET_APNUIDPWD="www","","",##, new updated cmd for 2 diff sim apn uid pwd//**1234SET_APNUIDPWD=0,"www","","",##
			  { 	
//                  LOG_INFOS(CH_SMS,"Change APNUIDPWD_0 value");
                    gprsConfigSim0Ptr = GetGprs0SimConfig();
                    
					if((para1 = (uint8_t*)strstr((const char*) tmp_buff, ",")) != NULL)
					{
                        memset(gprsConfigSim0Ptr, 0, sizeof(gprsConfig_st));
						strncpy((char *)gprsConfigSim0Ptr->apn,(const char*)start+17,para1-start-18);  //12
                        
						if((para2 = (uint8_t*)strstr((const char*) para1+1, ",")) != NULL)
						{
							strncpy((char *)gprsConfigSim0Ptr->username,(const char*)para1+2,para2-para1-3);
							
							if((para1 = (uint8_t*)strstr((const char*) para2+1, ",")) != NULL)
							{
								strncpy((char *)gprsConfigSim0Ptr->pass,(const char*)para2+2,para1-para2-3);
							}
                            SetGprs0SimConfig(gprsConfigSim0Ptr);               //update this new param in memory
														DeactGprsWhenIdle();                                 //Deactivate gprs
                            gprsConfigSim0Ptr = GetGprs0SimConfig();  
                            GetGprsConfigsFromMem();
														LOG_INFO(CH_SMS,"APN0 = %s,UID0 = %s,PWD0 = %s", gprsConfigSim0Ptr->apn,gprsConfigSim0Ptr->username,gprsConfigSim0Ptr->pass);
                            GetSim1GprsConfig();
                            return 1;
						}
					}
			   }
				 
				 
				else if((start = (uint8_t*)strstr((const char*) tmp_buff, "SET_NTRIP_DETAILS=")) != NULL)//**1234SET_APNUIDPWD="www","","",##, new updated cmd for 2 diff sim apn uid pwd//**1234SET_APNUIDPWD=0,"www","","",##
			  { 	
//                  LOG_INFOS(CH_SMS,"Change APNUIDPWD_0 value");
                    ntripcredPtr = GetNtripCred();
                    
					if((para1 = (uint8_t*)strstr((const char*) tmp_buff, ",")) != NULL)
					{
                        memset(ntripcredPtr, 0, sizeof(ntripcredConfig_st));
						strncpy((char *)ntripcredPtr->mountpoint,(const char*)start+19,para1-start-20);  //12
                        
						if((para2 = (uint8_t*)strstr((const char*) para1+1, ",")) != NULL)
						{
							strncpy((char *)ntripcredPtr->ntripusername,(const char*)para1+2,para2-para1-3);
							
							if((para1 = (uint8_t*)strstr((const char*) para2+1, ",")) != NULL)
							{
								strncpy((char *)ntripcredPtr->ntrippass,(const char*)para2+2,para1-para2-3);
							}
                            SetNtripCred(ntripcredPtr);               //update this new param in memory
                            ntripcredPtr = GetNtripCred();  
														LOG_INFO(CH_SMS,"Mount_point = %s,Ntrip_Username = %s,Ntrip_Password = %s", ntripcredPtr->mountpoint,ntripcredPtr->ntripusername,ntripcredPtr->ntrippass);
                            GetNtripConfig();
														
                            return 1;
						}
					}
			   }
				 
				 
				 
				 
				 
				 
				 
				 
				 
				 
				 
				 
				 
               else if((start = (uint8_t*)strstr((const char*) tmp_buff, "UPGRADE=")) != NULL)//**1234UPGRADE=ftp://hawkeye.qdnet.com/home/gpstrack/fotafile.bin@gpstrack:adpcmm##
				//**1234UPGRADE=ftp://aq.qdvts.com/fota_220817_0450.bin@packetdump@aq.qdvts.com:pdp@1234##
				{
					memset(urlBuffer,0,sizeof(urlBuffer));
					strcpy((char *)urlBuffer, (const char*)start+8);
          fotaResponse = 1;     //response will be sent after the FOTA process is over
					//GsmCloseNtripSocket();
					return 0;
				}
               else if((start = (uint8_t*)strstr((const char*) tmp_buff,"ERASE_MEMORY")) != NULL)//**1234ERASE_MEMORY##
				{
					resetMemory();
                    LOG_INFOS(CH_GSM,"Memory Erased");
                    SendSMSLen = snprintf((char *)sendAckSmsBuff, sizeof(sendAckSmsBuff),"\nMEMDEBUG: W-%d,R-%d,N-%d,T-%d",locationToBeWritten,currentReadLocationNo,numPacketsInMem,maxPacketsInMem);
					return 1;
				}					
				else if((start = (uint8_t*)strstr((const char*) tmp_buff,"RESET_GSM")) != NULL)//**1234RESET_GSM##
				{
//                  LOG_INFOS(CH_SMS,"Power Reset GSM Module");
                    SetPowerResetModemFlag();
					return 0;
				}
               else if((start = (uint8_t*)strstr((const char*) tmp_buff,"RESET_BOARD")) != NULL)//**1234RESET_BOARD##
				{
//                  LOG_INFOS(CH_SMS,"Reset The Unit");
					NVIC_SystemReset(); 
					return 0;
				}
               else
               {
                  return 0; 
               }
           }
           else
           {
             return 0;
           }
       }
    }
	return 0;
}

void serialDataRcvCb(uint8_t receiveData)
{
    RingBufferFill(&serialCmdRing,receiveData);
}

uint8_t flagserialCmdLineStart = 0;
uint8_t flagserialCmdLineStop = 0;
uint8_t flagSerialCmdReceived = 0;

void serialCmdHandler(void)
{
   uint8_t rcvCmdByte = 0;
   
   while((RingBufferDrain(&serialCmdRing,&rcvCmdByte) == 1))
   {
     if(flagserialCmdLineStart == 0 || flagserialCmdLineStart == 1)
     {
          if(flagserialCmdLineStart == 0){deviceCmd.debugCmdLen = 0;};
           if(rcvCmdByte == '*')
          {
            deviceCmd.debugCmdBuff[deviceCmd.debugCmdLen] = rcvCmdByte;
            deviceCmd.debugCmdLen++;
            flagserialCmdLineStart++;
          }
     }
     else
     {
       if(rcvCmdByte == '#')
       {
         if(flagserialCmdLineStop ==  0 || flagserialCmdLineStop == 1)
         {
            deviceCmd.debugCmdBuff[deviceCmd.debugCmdLen] = rcvCmdByte;
            deviceCmd.debugCmdLen++;
            flagserialCmdLineStop++;
            if(flagserialCmdLineStop == 1)
            {
              deviceCmd.debugCmdBuff[deviceCmd.debugCmdLen] = rcvCmdByte;
              deviceCmd.debugCmdLen++;
              flagSerialCmdReceived = 1;
              flagserialCmdLineStart = 0;;
              flagserialCmdLineStop = 0;
            }
         }
       }
       else
       {
            deviceCmd.debugCmdBuff[deviceCmd.debugCmdLen] = rcvCmdByte;
            deviceCmd.debugCmdLen++;
            if (deviceCmd.debugCmdLen >= 512)
            {
                // should not come here this can happen only if the line buffer size is smaller
                // than the line received
                deviceCmd.debugCmdLen = deviceCmd.debugCmdLen - 1;
            }
       }
     }
     if(flagSerialCmdReceived)
     {
       flagSerialCmdReceived = 0;
       if(GsmProcessSmsConfigCmd((uint8_t*)deviceCmd.debugCmdBuff, deviceCmd.debugCmdLen) == 1 )
       {
         
         LOG_DBG(CH_GSM,"SERIAL RESP: %s",sendAckSmsBuff);
         memset(&deviceCmd,0,sizeof(DebugCmd_st));
       } 
       else
       {
         LOG_DBG(CH_GSM,"INVALID COMMAND");
         memset(&deviceCmd,0,sizeof(DebugCmd_st));
       }
     }
   }
}

/**
 *  @brief GsmNewSmsReceivedCallback() This function is used to get all the details of new received sms, to process new sms it calls Process_Config_Cmd
 *  function. If the result of that function is true then it resets the smsDeleteFlag to delete that sms & calls fuction
 *  which prepares the acknowledgement sms. Also if ENABLE_ACK_SMS = 1 then only it will send ack sms 
 *  @param [in] phNo         Received sms phone no
 *  @param [in] smsDataBuff  Message content of received sms
 *  @param [in] smsDataLen   Length of message content
 *  @param [in] smsIdx       Index no of sms
 *  @return void
 */
void SmsReceiveCb(uint8_t* phNum, uint8_t* smsData, uint16_t smsLen)
{
    memset((unsigned char*)smsMessage,0,sizeof(smsMessage));
    memcpy((uint8_t *)smsMessage,smsData,smsLen);
    memset((unsigned char*)smsNum,0,sizeof(smsNum));
    strncpy((char *)smsNum,(const char *)phNum,sizeof(smsNum));
	if(GsmProcessSmsConfigCmd((uint8_t*)smsMessage, smsLen) == 1)
	{
		if(ENABLE_ACK_SMS == 1)
		{
            LOG_DBG(CH_GSM,"SMS Response : %s",sendAckSmsBuff);
			recpPhnoPtr = phNum;
            GsmSendSMS();
            SendSMS(recpPhnoPtr,sendAckSmsBuff,strlen((const char*)sendAckSmsBuff));
		}
        else
        {
           GsmDeleteSMS();    //if sms command doesnt match then delete the sms
        }
	}
    else
    {
         if(fotaResponse == 1)
        {
           fotaPhnoPtr =  phNum;                         //only copy the phone number to send the response after the FOTA Process.
           FotaStartFromFtp(urlBuffer,fotaPhnoPtr);   //pass the url and sms resp num to the fota file
        }
        else
        {
           GsmDeleteSMS();  //if any other sms is received
        }
         
    }
}


/**
 *  @brief All the status of GSM, whether it is Initialized, Registered or Deregistered is passed to 
 *  GSMLedStatusCb function.It toggles Gsm LED States based on the different states of the gsm module.
 *  @param [out] status 1 value from gsmStatus_et enum
 *  @return void
 */
void GSMLedStatusCb(gsmStatus_et status)
{
	switch(status)
	{
	   case GSM_NOT_INITIALIZED:	
			 Set_gprsLEDState(GSM_NOT_REG);
			break;
			
	   case GSM_NOT_REGISTERED:	
			Set_gprsLEDState(GSM_NOT_REG);
			break;
			
	   case GSM_REGISTERED:		
			Set_gprsLEDState(GSM_REG);
			break;

	   case GPRS_REGISTERED:		
			break;

	   case GPRS_ACTIVATED:		
			Set_gprsLEDState(GPRS_ACT);
			break;
               
       case PACKET_SENT_SUCCESS:
            Set_gprsLEDState(PACKET_SEND_SUCCESS);
            break;
                
       case PACKET_SENT_FAILED:
            break;
                        
		default:
			break;	
	}
}

void GetAISSockets(void)
{
   memset(AISSock1ReadBuff,0,sizeof(AISSock1ReadBuff));
   //memset(sockRespBuff,0,sizeof(sockRespBuff));
   memset(ntriphost,0,sizeof(ntriphost));
   memset(ntrippath,0,sizeof(ntrippath));
	 
	 memset(AISSock0ReadBuff,0,sizeof(AISSock0ReadBuff));
   memset(sockRespBuff,0,sizeof(sockRespBuff));
   memset(host,0,sizeof(host));
   memset(path,0,sizeof(path));
   
    
				 
	  if(sscanf((const char*)server0ConfigPtr->serverAddr, "%99[^/]%99[^\n]",host,path) == 2)
    {
//      LOG_DBG(CH_SOCK,"Http Host Name and Path Resolved");
        aisSock0 = GsmSocketGet(host,path,server0ConfigPtr->port,server0ConfigPtr->serverReqType,AISSock0ReadBuff, AISSock0ReadLen,s0SecurityType);
    }
		     GsmSocketOpen(aisSock0);




    if(sscanf((const char*)server1ConfigPtr->serverAddr, "%99[^/]%99[^\n]",ntriphost,ntrippath) == 2)
    {
//      LOG_DBG(CH_SOCK,"Http Host Name and Path Resolved");
        aisSock1 = GsmSocketGet(ntriphost,ntrippath,server1ConfigPtr->port,server1ConfigPtr->serverReqType,AISSock1ReadBuff, AISSock1ReadLen,s0SecurityType);
    }
		     aisSock1 = GsmSocketGet(ntriphost,ntrippath,server1ConfigPtr->port,server1ConfigPtr->serverReqType,AISSock1ReadBuff, AISSock1ReadLen,s0SecurityType);
		     GsmSocketOpen(aisSock1);

}



void GetIntervalsConfigfrmMem(void)
{
   intervalsPtr = GetIntervals();
   thresholdsPtr = GetThresholds();
   emergencyParamsPtr = GetEmergencyParams();
}

void GetUnitIdFrmMemForAppFile(void)
{
  aisDeviceDataPtr = GetAisDeviceData();
  if(atoi((const char*)aisDeviceDataPtr->unitId) == 0)    //this means nothing is stored in the mem/junk read from mem ,load 7000 on the ram
  {
    memset(aisDeviceDataPtr, 0, sizeof(aisDataConfig_st));
    strncpy((char *)aisDeviceDataPtr->unitId,AIS_UNIT_ID,4); 
  }
}

void GetServerConfigurations(void)
{
   server0ConfigPtr = GetServer0Config();
	 server1ConfigPtr = GetServer1Config();
}

void GetGprsConfigurations(void)
{
   gprsConfigSim0Ptr = GetGprs0SimConfig();
}

void GsmLedHandlerInit(void (* status_callback)(gsmStatus_et status))
{
   gsmLedStatusCb = status_callback;
}

void GetAllMemoryConfigurations(void)
{
  GetIntervalsConfigfrmMem();
  GetServerConfigurations();
  GetGprsConfigurations();
  GetPacketIntervalVal();
  GetGprsConfigsFromMem();
	getGPRSmemconfig();
  GetUnitIdFrmMemForAppFile();
	GetUnitIdForPacketing();
} 

void AisDebugInit(void)
{
   DebugLogInit();
   RingBufferInit(&serialCmdRing,debugCmdBuff,sizeof(debugCmdBuff));
}

void AisAppInit(void)
{
  GetAllMemoryConfigurations();
  InitPacketConfig();
  GsmLedHandlerInit(GSMLedStatusCb);
  GsmInit();
  GsmSmsInit(SmsReceiveCb);
  GetAISSockets();
}


uint32_t checkSockOpenTick = 0;
uint32_t sockReadUrcTick = 0;
uint32_t sockClosedTick = 0;
void PvtSendSetState(pvtSendHandler_et state)
{
	if(pvtSendState == state)
	{
		return; 
	}
    pvtSendState = state;
	switch(pvtSendState)
	{
    case PVT_WAIT_FOR_PACKET:
        LOG_DBGS(CH_GSM, "State - PWFP");
      break;
      
    case PVT_WAIT_FOR_SOCK_OPEN:
      GsmClearReadSockUrc();  //Clear the socket data receive/socket closed urc before opening socket
      GsmClearSockUrcStatus(); 
      LOG_DBGS(CH_GSM, "State - PWFSO");
      break;
      
    case PVT_IS_SOCK_OPENED:
      LOG_DBGS(CH_GSM, "State - PISO");
      break;
      
    case PVT_SOCK_OPEN_FAIL:
      LOG_DBGS(CH_GSM, "State - PSOF");
      break;
      
    case PVT_PACKET_WRITE_START:
      LOG_DBGS(CH_GSM, "State - PPWS");
      break;
      
    case PVT_GET_WRITE_STATUS:
      LOG_DBGS(CH_GSM, "State - PGWS");
      break;
      
    case PVT_SOCK_WRITE_FAIL:
      LOG_DBGS(CH_GSM, "State - PSWF");
      break;
      
    case PVT_WAIT_FOR_ACK_URC:
      sockReadUrcTick = GetStartTime();   //start time to wait for the ack
      LOG_DBGS(CH_GSM, "State - PWFAU");
      break;
     
    case PVT_GET_SOCK_ACK:
      SocketRespSetState(SOCK_RESP_CHECK_START);
      LOG_DBGS(CH_GSM, "State - PGSA");
      break;
      
    case PVT_ACK_SUCCESS:
      LOG_DBGS(CH_GSM, "State - PAS");
      break;

    case PVT_GET_ACK_FAIL:
      LOG_DBGS(CH_GSM, "State - PGAF");
      break;
    
    case PVT_CLOSE_SOCKET:
      sockClosedTick = GetStartTime();
      LOG_DBGS(CH_GSM, "State - PCS");
      break;
      
    case PVT_GPRS_DEACT_STATE:
       LOG_DBGS(CH_GSM, "State - PGDS");
       break;
		
		case PVT_SOCK_OPEN_TIMEOUT:
			break;
        
	}
}
 

void PvtSendHandler(void)
{
     socketResponse_et sockACKState;
     
	switch(pvtSendState)
    {
    case PVT_WAIT_FOR_PACKET:
      break;
      
    case PVT_WAIT_FOR_SOCK_OPEN:
         if(GsmSocketIsOpened(0))
				 {
					    //GsmSocketOpen(0);  //if gprs is activated then open the socket
							PvtSendSetState(PVT_IS_SOCK_OPENED);
				 }
      break;
      
    case PVT_IS_SOCK_OPENED:
        if(GsmSocketIsOpened(0))
        {
            PvtSendSetState(PVT_PACKET_WRITE_START);
        }
        else if(GsmSockCloseFlagStatus()) //if failed to open the socket and socket is closed  
        {
              PvtSendSetState(PVT_SOCK_OPEN_FAIL);
        }
        else if(flagGsmGprsDeactivate)  //if tried to open the socket for 3 min then store the packet in the mem,
        { 
             PvtSendSetState(PVT_GPRS_DEACT_STATE);
        }  
      break;
      
     case PVT_SOCK_OPEN_FAIL:
          PvtSendSetState(PVT_CLOSE_SOCKET);
      break;
         
    case PVT_PACKET_WRITE_START:
         if(AisSendDataPacket(socId,(char *)httpPacketBuff,totPacketLen))//if socket started writing data return true
         {
               PvtSendSetState(PVT_GET_WRITE_STATUS);
         }
         else 
         {
              LOG_ERR(CH_PACKET,"SOCKET BUSY...SOCKET WRITE FAIL");         //This is a rarest case as we are checking socket connection and then only pvt state is changed to wait for packet
              PvtSendSetState(PVT_GPRS_DEACT_STATE);
         }    
      break;
      
    case PVT_GET_WRITE_STATUS:
      if(GsmSockGetWriteState(socId) == GSM_SOCK_WRITE_SUCCESS)
      {
         PvtSendSetState(PVT_WAIT_FOR_ACK_URC); 
      }
      else if(GsmSockGetWriteState(socId) == GSM_SOCK_WRITE_FAIL)
      {
       PvtSendSetState(PVT_SOCK_WRITE_FAIL);   //PVT_SOCK_WRITE_FAIL //in the both the cases mcu sends socket close command to the gsm
      } 
      break;
      
    case PVT_WAIT_FOR_ACK_URC:
      if(GsmIsReadSockUrcRcvd() || TimeSpent(sockReadUrcTick,SOCK_ACK_URC_TIMEOUT)) //If URC received or timeout occured to get it set the state to read
      {
        sockReadUrcTick = GetStartTime();
        GsmClearReadSockUrc();
        GsmReadHttpSocket(); //change the socket read state to read start
        PvtSendSetState(PVT_GET_SOCK_ACK); 
      }
      break;
       
    case PVT_GET_SOCK_ACK:
       sockACKState = SocketResponseHandler();
       if(sockACKState == SOCK_RESP_SUCCESS)
       {
         LOG_DBG(CH_PACKET,"SOCK ACK RECEIVED");
         PvtSendSetState(PVT_ACK_SUCCESS); 
         gsmLedStatusCb(PACKET_SENT_SUCCESS);
				 ResetAckReceivedTick();
       }
       else if(sockACKState == SOCK_RESP_FAIL)
       {
         LOG_ERR(CH_PACKET,"FAILED TO GET SOCK ACK");
         PvtSendSetState(PVT_GET_ACK_FAIL); 
       }
      break;
      
      case PVT_ACK_SUCCESS:
			
			 // PvtSendSetState(PVT_WAIT_FOR_PACKET);
			break;
			
      case PVT_GET_ACK_FAIL: //failed to receive the sock ack , ERROR received in read command, ack mismatch,no data received in read command
       //ideally gsm module sends the 0, CLOSED URC with the sock ack only if it is not received immediatley close the socket
         if(GsmGetSockUrcStatus())
         {
           GsmClearSockUrcStatus();
           GsmCloseHttpSocket();
           PvtSendSetState(PVT_CLOSE_SOCKET); 
         }
         else 
         {
           LOG_DBG(CH_GSM,"0, CLOSED URC not received..Close the socket")
           GsmCloseHttpSocket();
           PvtSendSetState(PVT_CLOSE_SOCKET); 
         }
      break;
  
    case PVT_CLOSE_SOCKET:
    case PVT_SOCK_WRITE_FAIL:
      if(GsmSockCloseFlagStatus() || TimeSpent(sockClosedTick,MAX_SOCK_CLOSED_TIMEOUT))  //this status is of the AT+QICLOSE gsm commnads status if ERROR/Timeout received from the modem then max timeout is 5seconds
     {
        GsmResetSockCloseFlag();
        PvtSendSetState(PVT_WAIT_FOR_PACKET);
        packetSetState(PACKET_STATE_IDLE);  //change the state directly to idle
     }//add timespent
      break;
      
    case PVT_GPRS_DEACT_STATE:
        PvtSendSetState(PVT_WAIT_FOR_PACKET);
        packetSetState(PACKET_STATE_IDLE);  //change the state directly to idle
      break;
		
		case PVT_SOCK_OPEN_TIMEOUT:
			break;
    }
    
    //check whether gsm connectivity is lost in between
	if(pvtSendState != PVT_WAIT_FOR_PACKET)
	{
		if((GsmGprsIsActive() == 0) || (GsmSocketIsOpened(0) == 0))
		{
			// set the state to PVT_GPRS_DEACT_STATE
			PvtSendSetState(PVT_GPRS_DEACT_STATE);  
		}
	}
}


//if ok response is received $ERROR=&UN=8001&PN=1*
//no error received then unit id and packet number are received in the response
//if error in received content length 
//$ERROR=CONTENT-LENGTH&UN=8001&PN=12227*
//if ok response is received $ERROR=&UN=8001&PN=1*
//no error received then unit id and packet number are received in the response
//if error in received content length 
//$ERROR=CONTENT-LENGTH&UN=8001&PN=12227*
//$ERROR=&UN=7000&PN=354*

//new response format to receive the gprs command from the server
//$ERROR=&CMD=COMMAND&UN=7000&PN=12227*
//$ERROR=&CMD=**1234GET_DEBUG##&UN=7087&PN=380*

socketAck_et ProcessSockResp(uint8_t* ProcessAckBuf)
{
    char* p1 = NULL;
    char* p2 = NULL;
    char* p3 = NULL;
    char* cmdStart = NULL;
    char* cmdEnd   = NULL;
    uint32_t ackNo =0;
    
    switch(sockAckVal)
    {
        case SOCK_ACK_CHECK_START:
					if(GSMStrPrefixMatch((char*)ProcessAckBuf, "$ERROR="))
            { 
               p1 = strchr((char*)ProcessAckBuf , '=');
               p1++;
               p2  = strchr((char*)p1 , '&');
               
               if((p2-p1) == 0) //this means error field is blank
               {
                  //check if any GPRS Command is been received from the server
                  if((p1 = (char*)strstr((const char*) ProcessAckBuf,"CMD=")) != NULL)
                  {
                    cmdStart = strchr((char*)p1 , '=');
                    cmdStart++;
                    cmdEnd = strchr((char*)cmdStart , '&');
                      
                    if((cmdEnd - cmdStart) != 0)
                    {
                      memset(gprsCmdBuff,0,sizeof(gprsCmdBuff));
                      //copy the gprs command received from the server
                      strncpy((char*)gprsCmdBuff,cmdStart,(cmdEnd - cmdStart));
                      gprsCmdLen = cmdEnd - cmdStart;
                      
                      if(GsmProcessSmsConfigCmd((uint8_t*)gprsCmdBuff, gprsCmdLen) == 1)
                      {
                         if(SendSMSLen > 0)
                         {
                           memset(gprsRespBuff,0,sizeof(gprsRespBuff));
                           strncpy((char*)gprsRespBuff,(char*)sendAckSmsBuff,SendSMSLen);
                           gprsRespLen = SendSMSLen;
                           flagGPRSCmd = 1; 
                         }
                      }
                      else
                      {                          
                          if(fotaResponse == 1)
                          {
                             //  FotaStartFromGPRSCmd(urlBuffer);   //pass the url and to the fota file
                          }
                      }
                    }
                  }
                  sockAckVal = SOCK_ACK_CHECK_PACKET_NUM;
               }
               else
               {
                  //Check whether any error is received if yes then directly change the state to SOCK_ACK_CHECK_FAIL
                  memset(httpErrorBuff,0,sizeof(httpErrorBuff));
                  strncpy((char*)httpErrorBuff,p1,p2-p1);
                  
                  if(strstr((const char*) httpErrorBuff,"METHOD_QUERY-STRING") != NULL)
                  {sockAckVal = SOCK_ACK_CHECK_FAIL;}
                  else if(strstr((const char*) httpErrorBuff,"CONTENT-LENGTH") != NULL)
                  {sockAckVal = SOCK_ACK_CHECK_FAIL;}
                  else if(strstr((const char*) httpErrorBuff,"QUERY-STRING-PATTERN") != NULL)
                  {sockAckVal = SOCK_ACK_CHECK_FAIL;}
                  else if(strstr((const char*) httpErrorBuff,"MEMORY-ALLOCATION") != NULL)
                  {sockAckVal = SOCK_ACK_CHECK_FAIL;}
                  else if(strstr((const char*) httpErrorBuff,"DATA_FILE-STORAGE") != NULL)
                  {sockAckVal = SOCK_ACK_CHECK_FAIL;}
                  else
                  {sockAckVal = SOCK_ACK_CHECK_FAIL;}
               }
            }
//            if(GSMStrPrefixMatch((char*)ProcessAckBuf, "$ERROR="))
//            { 
//               p1 = strchr((char*)ProcessAckBuf , '=');
//               p1++;
//               p2  = strchr((char*)p1 , '&');
//               
//               if((p2-p1) == 0) //this means error field is blank
//               {
//                  sockAckVal = SOCK_ACK_CHECK_PACKET_NUM;
//               }
//               else
//               {
//                  //Check whether any error is received if yes then directly change the state to SOCK_ACK_CHECK_FAIL
//                  memset(httpErrorBuff,0,sizeof(httpErrorBuff));
//                  strncpy((char*)httpErrorBuff,p1,p2-p1);
//                  
//                  if(strstr((const char*) httpErrorBuff,"METHOD_QUERY-STRING") != NULL)
//                  {sockAckVal = SOCK_ACK_CHECK_FAIL;}
//                  else if(strstr((const char*) httpErrorBuff,"CONTENT-LENGTH") != NULL)
//                  {sockAckVal = SOCK_ACK_CHECK_FAIL;}
//                  else if(strstr((const char*) httpErrorBuff,"QUERY-STRING-PATTERN") != NULL)
//                  {sockAckVal = SOCK_ACK_CHECK_FAIL;}
//                  else if(strstr((const char*) httpErrorBuff,"MEMORY-ALLOCATION") != NULL)
//                  {sockAckVal = SOCK_ACK_CHECK_FAIL;}
//                  else if(strstr((const char*) httpErrorBuff,"DATA_FILE-STORAGE") != NULL)
//                  {sockAckVal = SOCK_ACK_CHECK_FAIL;}
//                  else
//                  {sockAckVal = SOCK_ACK_CHECK_FAIL;}
//               }
//            }
          break;
          
        case SOCK_ACK_CHECK_PACKET_NUM:  //get the received packet number
             if((p3 = (char*)strstr((const char*) ProcessAckBuf,"PN=")) != NULL)
             { 
                p1 = strchr((char*)p3 , '=');
                p1++;
                p2  = strchr((char*)p1 , '*');
                memset(tmp_var,0,sizeof(tmp_var));
                strncpy((char*)tmp_var,p1,p2-p1);
                ackNo = atoi((const char*)tmp_var);
                if(aisPacketNo == ackNo) 
                {
                       LOG_INFO(CH_GSM,"APP PACKET NO = %d",aisPacketNo);
                       LOG_INFO(CH_GSM,"ACK PACKET NO = %d",ackNo);
                       sockAckVal =  SOCK_ACK_CHECK_SUCCESS;
                }
                else 
                {
                     if(GsmSocketGetAvlData(socId) > 0)
                     {
                       LOG_DBG(CH_GSM,"Socket Ring Buff len > 0 wait");
                       sockAckVal =  SOCK_ACK_CHECK_WAIT;
                     }
                     else
                     {
                       sockAckVal =  SOCK_ACK_CHECK_FAIL;
                     }
               }            
             }
          break;
          
        case SOCK_ACK_CHECK_WAIT:
          break;
          
        case SOCK_ACK_CHECK_SUCCESS:
          break;
          
        case SOCK_ACK_CHECK_FAIL:
          break;
          
         default:
           break;
    }   
   return sockAckVal;
}

void SocketRespSetState(socketResponse_et state)
{
     sockRespState = state;
    switch(sockRespState)
    {
        case SOCK_RESP_CHECK_START:
            sockRespTick = GetStartTime(); 
            sockAckVal = SOCK_ACK_CHECK_START;        //ack val for packet numbers received  from the server
             respStarCnt = 0;
            LOG_DBGS(CH_GSM, "State - SOCK_RESP_CHECK_START");
        break;

        case SOCK_RESP_CHECK_END:
          LOG_DBGS(CH_GSM, "State - SOCK_RESP_CHECK_END");
        break;
        
        case SOCK_RESP_CHECK_DATA:
          LOG_DBGS(CH_GSM, "State - SOCK_RESP_CHECK_DATA");
          break;
          
        case SOCK_RESP_CHECK_CMD:
          LOG_DBGS(CH_GSM,"State - SOCK_RESP_CHECK_CMD");
        
          break;
          
        case SOCK_RESP_SUCCESS:
          LOG_DBGS(CH_GSM, "State - SOCK_RESP_SUCCESS");
        break;
        
        case SOCK_RESP_FAIL:
          LOG_DBGS(CH_GSM, "State - SOCK_RESP_FAIL");
        break;
    }
} 

socketResponse_et   SocketResponseHandler(void)
{
	
	uint8_t c = 0;
    socketAck_et ackType;
    
    switch(sockRespState)
    {
        case SOCK_RESP_CHECK_START:
            // read for one byte
            if(GsmSocketRead(socId,(uint8_t *)&c,1) == 1)
            {
                if(c == SOCK_RESP_START_BYTE)
                {
                    //copy the '$'
                    memset(sockRespBuff,0,sizeof(sockRespBuff));
                    sockRespIndex = 0;
                    sockRespBuff[sockRespIndex] = c;
                    sockRespIndex++;
                    SocketRespSetState(SOCK_RESP_CHECK_END);
                }
            }
            else if(TimeSpent(sockRespTick,SOCK_RESP_WAIT_TIMEOUT))
            {
                    sockRespTick =  GetStartTime();
                    LOG_ERR(CH_GSM,"TIMEOUT - NO Data Received from socket");
                    SocketRespSetState(SOCK_RESP_FAIL);                         //wait for 10 secs if no response received then return fail
            }
        break;

        case SOCK_RESP_CHECK_END:
            if(GsmSocketRead(socId,(uint8_t *)&c,1) == 1)
            {
                if(c != SOCK_RESP_END_BYTE)                                     //unless * is received rcv the data in the buffer
                {
                    if(sockRespIndex < sizeof(sockRespBuff))
                    {
                        sockRespBuff[sockRespIndex] = c;
                        sockRespIndex++;
                    }
                    else
                    {
                        // should not come here data is too much to hold
                        sockRespIndex--;
                    }
                }
                else
                {                                                               //as soon  as * is rcvd process the data check whether it is socket data packet reply or a command
                    // now process the received byte
                   //copy the '*' and check if CMD i.e. any command from server is received or not
                   //if CMD is received wait for 2 more * and then go to the state of SOCK_RESP_CHECK_DATA
                    respStarCnt++;  //here 1st * is received
                    sockRespBuff[sockRespIndex] = c;
                    sockRespIndex++;
                    
                   if(NULL != strstr((char *)sockRespBuff,"CMD="))
                   {
                      SocketRespSetState(SOCK_RESP_CHECK_CMD);
                   }
                   else //if no command is been received process the data
                   {
                      respStarCnt = 0;
                      sockRespBuff[sockRespIndex] = '\0';
                      LOG_DBG(CH_GSM, "SERVER ACK - %s",sockRespBuff);
                      SocketRespSetState(SOCK_RESP_CHECK_DATA);
                   }                 
                }
            }
            else if(TimeSpent(sockRespTick,SOCK_RESP_WAIT_TIMEOUT))
            {
                    sockRespTick = GetStartTime();
                    LOG_ERR(CH_GSM,"Start Byte $ rcvd but * Not rcvd - TIMEOUT");
                    SocketRespSetState(SOCK_RESP_FAIL);                         //if modem rcvd $ and failed to rcv * then after timeout  return failure
            }
        break;
        
       case SOCK_RESP_CHECK_CMD:
         if(GsmSocketRead(aisSock0,(uint8_t *)&c,1) == 1)
            {
                if(c != SOCK_RESP_END_BYTE)                                     //unless * is received rcv the data in the buffer
                {
                    if(sockRespIndex < sizeof(sockRespBuff))
                    {
                        sockRespBuff[sockRespIndex] = c;
                        sockRespIndex++;
                    }
                    else
                    {
                        // should not come here data is too much to hold
                        sockRespIndex--;
                    }
                }
                else
                {                                                               //as soon  as * is rcvd process the data check whether it is socket data packet reply or a command
                    // now process the received byte
                   //copy the '*' and check if CMD i.e. any command from server is received or not
                   //if CMD is received wait for 2 more * and then go to the state of SOCK_RESP_CHECK_DATA
                    respStarCnt++;  //here 1st * is received
                    sockRespBuff[sockRespIndex] = c;
                    sockRespIndex++;
                    
                    if(respStarCnt >= 3)
                    {
                      respStarCnt = 0;
                      sockRespBuff[sockRespIndex] = '\0';
                      LOG_DBG(CH_GSM, "SERVER ACK & GPRS Command - %s",sockRespBuff);
                      SocketRespSetState(SOCK_RESP_CHECK_DATA);
                    }
                 }       
            }
            else if(TimeSpent(sockRespTick,SOCK_RESP_WAIT_TIMEOUT))
            {
                    sockRespTick = GetStartTime();
                    LOG_ERR(CH_GSM,"Start Byte $ rcvd but * Not rcvd - TIMEOUT");
                    SocketRespSetState(SOCK_RESP_FAIL);                         //if modem rcvd $ and failed to rcv * then after timeout  return failure
            }
         break;
        
        case SOCK_RESP_CHECK_DATA:
            ackType =ProcessSockResp(sockRespBuff);
            if(ackType == SOCK_ACK_CHECK_SUCCESS)
            {
               SocketRespSetState(SOCK_RESP_SUCCESS);
            }
            else if(ackType == SOCK_ACK_CHECK_WAIT)
            {
               SocketRespSetState(SOCK_RESP_CHECK_START);
            }
            else if(ackType == SOCK_ACK_CHECK_FAIL )   //else if(ackType == SOCK_ACK_CHECK_FAIL If urc is still not rcvd for the sent packet then compulsory wait for rest of the time.
            {
                 LOG_ERR(CH_GSM,"ACK Packet Numbers Mismatch...");
                 SocketRespSetState(SOCK_RESP_FAIL); 
            }
          break;
        
        case SOCK_RESP_SUCCESS:
        break;
        
        case SOCK_RESP_FAIL:
        break;
    }
    
    return sockRespState;
}
//    char c = 0;
//    socketAck_et ackType;
//    
//    switch(sockRespState)
//    {
//        case SOCK_RESP_CHECK_START:
//            // read for one byte
//            if(GsmSocketRead(socId,&c,1) == 1)
//            {
//                if(c == SOCK_RESP_START_BYTE)
//                {
//                    //copy the '$'
//                    memset(sockRespBuff,0,sizeof(sockRespBuff));
//                    sockRespIndex = 0;
//                    sockRespBuff[sockRespIndex] = c;
//                    sockRespIndex++;
//                    SocketRespSetState(SOCK_RESP_CHECK_END);
//                }
//            }
//            else if(TimeSpent(sockRespTick,SOCK_RESP_WAIT_TIMEOUT))
//            {
//                    sockRespTick =  GetStartTime();
//                    LOG_ERR(CH_GSM,"TIMEOUT - NO Data Received from socket");
//                    SocketRespSetState(SOCK_RESP_FAIL);                         //wait for 10 secs if no response received then return fail
//            }
//        break;

//        case SOCK_RESP_CHECK_END:
//            if(GsmSocketRead(socId,&c,1) == 1)
//            {
//                if(c != SOCK_RESP_END_BYTE)                                     //unless * is received rcv the data in the buffer
//                {
//                    if(sockRespIndex < sizeof(sockRespBuff))
//                    {
//                        sockRespBuff[sockRespIndex] = c;
//                        sockRespIndex++;
//                    }
//                    else
//                    {
//                        // should not come here data is too much to hold
//                        sockRespIndex--;
//                    }
//                }
//                else
//                {                                                               //as soon  as * is rcvd process the data check whether it is socket data packet reply or a command
//                    // now process the received byte
//                   //copy the '*' and check if CMD i.e. any command from server is received or not
//                   //if CMD is received wait for 2 more * and then go to the state of SOCK_RESP_CHECK_DATA
//                    //respStarCnt++;  //here 1st * is received
//                    sockRespBuff[sockRespIndex] = c;
//                    sockRespIndex++;
//                    
//                
//                      sockRespBuff[sockRespIndex] = '\0';
//                      LOG_DBG(CH_GSM, "SERVER ACK - %s",sockRespBuff);
//                      SocketRespSetState(SOCK_RESP_CHECK_DATA);
//                }
//                                  
//                
//            }
//            else if(TimeSpent(sockRespTick,SOCK_RESP_WAIT_TIMEOUT))
//            {
//                    sockRespTick = GetStartTime();
//                    LOG_ERR(CH_GSM,"Start Byte $ rcvd but * Not rcvd - TIMEOUT");
//                    SocketRespSetState(SOCK_RESP_FAIL);                         //if modem rcvd $ and failed to rcv * then after timeout  return failure
//            }
//        break;
//        
//         
//        case SOCK_RESP_CHECK_DATA:
//            ackType =ProcessSockResp(sockRespBuff);
//            if(ackType == SOCK_ACK_CHECK_SUCCESS)
//            {
//               SocketRespSetState(SOCK_RESP_SUCCESS);
//            }
//            else if(ackType == SOCK_ACK_CHECK_WAIT)
//            {
//               SocketRespSetState(SOCK_RESP_CHECK_START);
//            }
//            else if(ackType == SOCK_ACK_CHECK_FAIL )   //else if(ackType == SOCK_ACK_CHECK_FAIL If urc is still not rcvd for the sent packet then compulsory wait for rest of the time.
//            {
//                 LOG_ERR(CH_GSM,"ACK Packet Numbers Mismatch...");
//                 SocketRespSetState(SOCK_RESP_FAIL); 
//            }
//          break;
//        
//        case SOCK_RESP_SUCCESS:
//        break;
//        
//        case SOCK_RESP_FAIL:
//        break;
//    }
//    
//    return sockRespState;


pvtSendHandler_et GetPvtSendState(void)
{
    return pvtSendState;
}

uint8_t PvtSendIsWaitForPacket(void)
{
   if(pvtSendState == PVT_WAIT_FOR_PACKET)
	{
		return True;
	}
	return False;
}

uint8_t PvtIsWaitForConnPacket(void)
{
//    if(pvtSendState == PVT_HTTP_SOCKET_WAIT_FOR_CP)
//    {
//		return True;
//	}
	return False;
}

uint8_t AisSendDataPacket(uint8_t sockId , char* buffPtr , uint16_t bufflen)
{
   if(GsmSocketWrite(sockId,(uint8_t *)buffPtr,bufflen) == (gsmSockWriteRet_et)True)
   {
        return (gsmSockWriteRet_et)True;
    }
    return (gsmSockWriteRet_et)False;
}

uint8_t NtripSendDataPacket(uint8_t sockId , char* buffPtr , uint16_t bufflen)
{
   if(GsmSocketWrite(sockId,(uint8_t *)buffPtr,bufflen) == (gsmSockWriteRet_et)True)
   {
        return (gsmSockWriteRet_et)True;
    }
    return (gsmSockWriteRet_et)False;
}

uint8_t NtripSendIsWaitForPacket(void)
{
   if(ntripSendState == NTRIP_WAIT_FOR_PACKET)
	{
		return True;
	}
	return False;
}

//POST /pktDump/v2.pl?UN=8001&PN=12227 HTTP/1.1
//Host: v2.cleanupmumbai.com
//Content-Type: text/plain
//Content-Length: 271
//Connection: Close
//
//$EPB,QDNET,1.1.4,NR,L,869867030126710,MH02CE6710,1,01082018,073456,0.000000,0,0.000000,0,0.0,0.0,0,0.00,0.00,0.00,Vodafone IN,0,1,27.7,4.1,2,12,404,20,6D6B,74BA,404,20,6D6B,6088,-89,404,20,6D6B,7154,-94,404,20,6D6B,74B9,-94,0,0,0,0,0,1,0,0,0,0,0,339.694,3669,012227,B7BD*
void MakeHttpHeader(uint8_t *hostPtr, uint8_t *filepath, uint8_t *type, uint32_t contentLength,uint32_t PacketNum)
{
     GetUnitIdFrmMemForAppFile(); //Get Unit Id from the memory
    
    memset(httpHeader, 0, sizeof(httpHeader));
	httpHeaderLen = snprintf((char *)httpHeader, sizeof(httpHeader),

	"%s %s?UN=%s&PN=%d HTTP/1.1\r\n"
	"Host: %s\r\n"   
	"Content-Length: %d\r\n"
	"Content-Type: text/plain\r\n"
	"Connection: alive\r\n"
	"\r\n",	type, filepath,aisDeviceDataPtr->unitId,PacketNum, hostPtr, contentLength);
	LOG_DBG(CH_PACKET, "MakeHttpHeader = %s, HeaderLength = %d, contentLength = %d",httpHeader, httpHeaderLen, contentLength);   
}
        
uint8_t PvtSendPacket(uint8_t* packetBuff , uint16_t packetlen,uint32_t packetNumber)
{ 
  if(PvtSendIsWaitForPacket())              //pvt is waiting for normal packet
  {
      MakeHttpHeader(host,path,server0ConfigPtr->serverReqType,packetlen,packetNumber);  //make the http header for this packetBuff of length packetlen
      memset(httpPacketBuff,0,sizeof(httpPacketBuff));                 //clear the final http header appended with the actual packet data
      
      //copy the http header of length httpHeaderLen in the httpPacketBuff
      strncpy((char*)httpPacketBuff, (const char*)httpHeader, httpHeaderLen);
      
      //append the device data with the http header in the httpPacketBuff
      memcpy(httpPacketBuff + httpHeaderLen, packetBuff, packetlen);
      
      totPacketLen = httpHeaderLen + packetlen;
      
      socId      = aisSock0;
      aisPacketNo = packetNumber;
      PvtSendSetState(PVT_WAIT_FOR_SOCK_OPEN);
    return 1;
  }
  return 0;
}

uint8_t PvtSendConnPacket(uint8_t* packetBuff , uint16_t packetlen)
{ 
//   if(PvtIsWaitForConnPacket())    //pvt is waiting for connect packet
//  {
//       MakeHttpHeader(host,path,server0ConfigPtr->serverReqType,packetlen);  //make the http header for this packetBuff of length packetlen
//       memset(httpPacketBuff,0,sizeof(httpPacketBuff));                     //clear the final http header appended with the actual packet data
//      
//      //copy the http header of length httpHeaderLen in the httpPacketBuff
//      strncpy((char*)httpPacketBuff, (const char*)httpHeader, httpHeaderLen);
//      
//      //append the device data with the http header in the httpPacketBuff
//      memcpy(httpPacketBuff + httpHeaderLen, packetBuff, packetlen);
//      
//      totPacketLen = httpHeaderLen + packetlen;
////      packetdata = packetBuff;
////      PacketLen  = packetlen;
//      socId      = aisSock0;
//      PvtSendSetState(PVT_CONNECT_MSG_SEND_START);
//    return 1;
//  }
  return 0;
}


void BuzzerEnableInit(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
	
    GPIO_InitStruct.Pin = BUZZER_EN_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	
    HAL_GPIO_Init(BUZZER_EN_GPIO_PORT,&GPIO_InitStruct);
    HAL_GPIO_WritePin(BUZZER_EN_GPIO_PORT, BUZZER_EN_GPIO_PIN, GPIO_PIN_RESET);   //Reset the gsm Enable pin 
	
}

void writeBuzzerPinState(uint8_t pinState)
{
     if(pinState == 1)
     {
			 //ToggleBuzzer();
			 if(BuzzerState == BUZZER_IDLE_STATE)
			 {
				flagBuzzerOff = 0;
			 BuzzerState = BUZZER_TOGGLE_STATE;
			buzzerPulseTick = GetStartTime();
       HAL_GPIO_WritePin(BUZZER_EN_GPIO_PORT, BUZZER_EN_GPIO_PIN, GPIO_PIN_SET);
			 }
     }
     else if(pinState == 0)
     {
			 BuzzerState = BUZZER_IDLE_STATE;
       HAL_GPIO_WritePin(BUZZER_EN_GPIO_PORT, BUZZER_EN_GPIO_PIN, GPIO_PIN_RESET);
     }
}

//void ToggleBuzzer()
//{
//	if(BuzzerState==BUZZER_TOGGLE_STATE)
//	{
//		return;
//	}
//    writeBuzzerPinState(1);
//	  BuzzerState = BUZZER_TOGGLE_STATE;
//	  buzzerPulseTick = GetStartTime();
//}


void BuzzerHandler()
{
    switch(BuzzerState)
    {
        case BUZZER_TOGGLE_STATE:
            if(flagBuzzerOff == 0)
						{
								if(TimeSpent(buzzerPulseTick, 200)) //1 sec On
								{
											//BuzzerState = BUZZER_IDLE_STATE;
											flagBuzzerOff = 1;
											HAL_GPIO_WritePin(BUZZER_EN_GPIO_PORT, BUZZER_EN_GPIO_PIN, GPIO_PIN_RESET);
											//buzzerOnTick = GetStartTime();
									    buzzerPulseTick = GetStartTime();
								}
					  }
						else if(flagBuzzerOff == 1)
						{
						   if(TimeSpent(buzzerPulseTick,200))
							 {
							   flagBuzzerOff = 0; 
								 HAL_GPIO_WritePin(BUZZER_EN_GPIO_PORT, BUZZER_EN_GPIO_PIN, GPIO_PIN_SET);
								// buzzerOnTick = GetStartTime();
								 buzzerPulseTick = GetStartTime();
							 }
						}
            break;
        
       case BUZZER_IDLE_STATE:
            break;
    }
}

void RelayEnableInit(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
	
    GPIO_InitStruct.Pin = RELAY_EN_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	
    HAL_GPIO_Init(RELAY_EN_GPIO_PORT,&GPIO_InitStruct);
    HAL_GPIO_WritePin(RELAY_EN_GPIO_PORT, RELAY_EN_GPIO_PIN, GPIO_PIN_SET);   //Reset the gsm Enable pin 
	
	
}

void writeRelayPinState(uint8_t pinState)
{
//     if(pinState == 1)
//     {
//			 if(flagRelayOn == 0)
//			 {
//				 flagStartRelay = 1;
//				 relayCount = 0;
//				 RelayStartTick = GetStartTime();
//				 HAL_GPIO_WritePin(RELAY_EN_GPIO_PORT, RELAY_EN_GPIO_PIN, GPIO_PIN_RESET);
//			 }
//     }
//     else if(pinState == 0)
//     {
//			 flagStartRelay = 0;
//			 relayCount = 0;
//			 flagRelayOn = 0;
//       HAL_GPIO_WritePin(RELAY_EN_GPIO_PORT, RELAY_EN_GPIO_PIN, GPIO_PIN_SET);
//     }

       if(pinState == 1)
			 {
			   HAL_GPIO_WritePin(RELAY_EN_GPIO_PORT, RELAY_EN_GPIO_PIN, GPIO_PIN_RESET);
			 }
			 else if(pinState == 0)
			 {
				 HAL_GPIO_WritePin(RELAY_EN_GPIO_PORT, RELAY_EN_GPIO_PIN, GPIO_PIN_SET);
			 }
}


void StartRelayHandler(void)
{
   if((flagStartRelay == 1) && (relayCount <= 1))
	 {
		  if(TimeSpent(RelayStartTick,1000))
			{
	      if(relayCount == 0)
				{
					//relayCount++;
				  HAL_GPIO_WritePin(RELAY_EN_GPIO_PORT, RELAY_EN_GPIO_PIN, GPIO_PIN_SET);
				}
				else if(relayCount == 1)
				{
				  //relayCount++;
				  HAL_GPIO_WritePin(RELAY_EN_GPIO_PORT, RELAY_EN_GPIO_PIN, GPIO_PIN_RESET);
				}
				else if(relayCount == 2)
				{
				  relayCount++;
				  HAL_GPIO_WritePin(RELAY_EN_GPIO_PORT, RELAY_EN_GPIO_PIN, GPIO_PIN_SET);
				}	
				else if(relayCount == 3)
				{
					flagStartRelay = 0;
					flagRelayOn = 1;
				  relayCount++;
				  HAL_GPIO_WritePin(RELAY_EN_GPIO_PORT, RELAY_EN_GPIO_PIN, GPIO_PIN_RESET);
				}	
					
				RelayStartTick = GetStartTime();
			}	 
	 }

}




void NtripSendHandler(void)
{
  socketResponse_et sockACKState;
     
	switch(ntripSendState)
    {
    case NTRIP_WAIT_FOR_PACKET:

      break;
      
    case NTRIP_WAIT_FOR_SOCK_OPEN:
         if(GsmSocketIsOpened(1))
				 {
					    //GsmSocketOpen(0);  //if gprs is activated then open the socket
							NtripSendSetState(NTRIP_IS_SOCK_OPENED);
				 }
      break;
      
    case NTRIP_IS_SOCK_OPENED:
        if(GsmSocketIsOpened(1))
        {
            NtripSendSetState(NTRIP_PACKET_WRITE_START);
        }
        else if(GsmNtripSockCloseFlagStatus()) //if failed to open the socket and socket is closed  
        {
              NtripSendSetState(NTRIP_SOCK_OPEN_FAIL);
        }
        else if(flagGsmGprsDeactivate)  //if tried to open the socket for 3 min then store the packet in the mem,
        { 
             NtripSendSetState(NTRIP_GPRS_DEACT_STATE);
        }  
      break;
      
     case NTRIP_SOCK_OPEN_FAIL:
          NtripSendSetState(NTRIP_CLOSE_SOCKET);
      break;
         
    case NTRIP_PACKET_WRITE_START:
         if(NtripSendDataPacket(ntripsocId,(char *)ntripPacketBuff,ntripPacketLen))//if socket started writing data return true
         {
               NtripSendSetState(NTRIP_GET_WRITE_STATUS);
         }
         else 
         {
              LOG_ERR(CH_PACKET,"SOCKET BUSY...SOCKET WRITE FAIL");         //This is a rarest case as we are checking socket connection and then only pvt state is changed to wait for packet
              NtripSendSetState(NTRIP_GPRS_DEACT_STATE);
         }    
      break;
      
    case NTRIP_GET_WRITE_STATUS:
      if(GsmSockGetWriteState(ntripsocId) == GSM_SOCK_WRITE_SUCCESS)
      {
	      NtripSendSetState(NTRIP_WAIT_FOR_PACKET);			
      }
      else if(GsmSockGetWriteState(ntripsocId) == GSM_SOCK_WRITE_FAIL)
      {
       NtripSendSetState(NTRIP_SOCK_WRITE_FAIL);   //PVT_SOCK_WRITE_FAIL //in the both the cases mcu sends socket close command to the gsm
        //NtripSendSetState(NTRIP_GET_ACK_FAIL);
			} 
      break;

////////////////////////////////////////////////////////////////////////////////////      
    case NTRIP_WAIT_FOR_ACK_URC:
//      if(GsmIsReadSockUrcRcvd() || TimeSpent(sockReadUrcTick,SOCK_ACK_URC_TIMEOUT)) //If URC received or timeout occured to get it set the state to read
//      {
//        sockReadUrcTick = GetStartTime();
////        GsmClearReadSockUrc();
//        GsmReadHttpSocket(); //change the socket read state to read start
//        NtripSendSetState(NTRIP_GET_SOCK_ACK); 
//      }
      break;
       
    case NTRIP_GET_SOCK_ACK:
       sockACKState = SocketResponseHandler();
			 
       if(sockACKState == SOCK_RESP_SUCCESS)
       {
         LOG_DBG(CH_PACKET,"SOCK ACK RECEIVED");
         NtripSendSetState(NTRIP_ACK_SUCCESS); 
         gsmLedStatusCb(PACKET_SENT_SUCCESS);
				 ResetAckReceivedTick();
       }
       else if(sockACKState == SOCK_RESP_FAIL)
       {
         LOG_ERR(CH_PACKET,"FAILED TO GET SOCK ACK");
         NtripSendSetState(NTRIP_GET_ACK_FAIL); 
       }
      break;
      
      case NTRIP_ACK_SUCCESS:
			
			  //PvtSendSetState(PVT_WAIT_FOR_PACKET);
			break;
//////////////////////////////////////////////////////////////////////////////////////////			
      case NTRIP_GET_ACK_FAIL: //failed to receive the sock ack , ERROR received in read command, ack mismatch,no data received in read command
       //ideally gsm module sends the 0, CLOSED URC with the sock ack only if it is not received immediatley close the socket
         if(GsmGetNtripSockUrcStatus())
         {
           GsmClearNtripSockUrcStatus();
           GsmCloseNtripSocket();
           NtripSendSetState(NTRIP_CLOSE_SOCKET); 
         }
         else 
         {
           LOG_DBG(CH_GSM,"0, CLOSED URC not received..Close the socket")
           GsmCloseNtripSocket();
           NtripSendSetState(NTRIP_CLOSE_SOCKET); 
         }
      break;
  
    case NTRIP_CLOSE_SOCKET:
    case NTRIP_SOCK_WRITE_FAIL:
      if(GsmNtripSockCloseFlagStatus() || TimeSpent(sockClosedTick,MAX_SOCK_CLOSED_TIMEOUT))  //this status is of the AT+QICLOSE gsm commnads status if ERROR/Timeout received from the modem then max timeout is 5seconds
     {
        GsmResetNtripSockCloseFlag();
        NtripSendSetState(NTRIP_WAIT_FOR_PACKET);
        ntrippacketSetState(NTRIP_PACKET_STATE_IDLE);  //change the state directly to idle
     }//add timespent
      break;
      
    case NTRIP_GPRS_DEACT_STATE:
        NtripSendSetState(NTRIP_WAIT_FOR_PACKET);
        ntrippacketSetState(NTRIP_PACKET_STATE_IDLE);  //change the state directly to idle
      break;
		
		case NTRIP_SOCK_OPEN_TIMEOUT:
			break;
    }
    
    //check whether gsm connectivity is lost in between
	if(ntripSendState != NTRIP_WAIT_FOR_PACKET)
	{
		if((GsmGprsIsActive() == 0) || (GsmSocketIsOpened(1) == 0))
		{
			// set the state to PVT_GPRS_DEACT_STATE
			NtripSendSetState(NTRIP_GPRS_DEACT_STATE);  
		}
	}
}



void NtripSendSetState(ntripSendHandler_et state)
{
	if(ntripSendState == state)
	{
		return; 
	}
    ntripSendState = state;
	switch(ntripSendState)
	{
    case NTRIP_WAIT_FOR_PACKET:
        LOG_DBGS(CH_GSM, "State - NTRIP_PWFP");
      break;
      
    case NTRIP_WAIT_FOR_SOCK_OPEN:
      //GsmClearReadSockUrc();  //Clear the socket data receive/socket closed urc before opening socket
      GsmClearNtripSockUrcStatus(); 
      //LOG_DBGS(CH_GSM, "State - PWFSO");
      break;
      
    case NTRIP_IS_SOCK_OPENED:
      //LOG_DBGS(CH_GSM, "State - PISO");
      break;
      
    case NTRIP_SOCK_OPEN_FAIL:
      LOG_DBGS(CH_GSM, "State - NTRIP_PSOF");
      break;
      
    case NTRIP_PACKET_WRITE_START:
      LOG_DBGS(CH_GSM, "State - NTRIP_PPWS");
      break;
      
    case NTRIP_GET_WRITE_STATUS:
      LOG_DBGS(CH_GSM, "State - NTRIP_PGWS");
      break;
      
    case NTRIP_SOCK_WRITE_FAIL:
      LOG_DBGS(CH_GSM, "State - NTRIP_PSWF");
      break;
      
    case NTRIP_WAIT_FOR_ACK_URC:
      sockReadUrcTick = GetStartTime();   //start time to wait for the ack
      LOG_DBGS(CH_GSM, "State - NTRIP_PWFAU");
      break;
     
    case NTRIP_GET_SOCK_ACK:
      SocketRespSetState(SOCK_RESP_CHECK_START);
      LOG_DBGS(CH_GSM, "State - NTRIP_PGSA");
      break;
      
    case NTRIP_ACK_SUCCESS:
      LOG_DBGS(CH_GSM, "State - NTRIP_PAS");
      break;

    case NTRIP_GET_ACK_FAIL:
      LOG_DBGS(CH_GSM, "State - NTRIP_PGAF");
      break;
    
    case NTRIP_CLOSE_SOCKET:
      sockClosedTick = GetStartTime();
      LOG_DBGS(CH_GSM, "State - NTRIP_PCS");
      break;
      
    case NTRIP_GPRS_DEACT_STATE:
       //LOG_DBGS(CH_GSM, "State - PGDS");
       break;
		
		case NTRIP_SOCK_OPEN_TIMEOUT:
			break;
        
	}
}


//void rtcm_header_Parse(void) 
//{
//    rtcm_parser();
//}

//void rtcm_parser(void) 
//{
//    uint8_t byte;
//    uint8_t bytes_read;
//    
//    const char *expected_header = "ICY 200 OK";
//    const uint8_t expected_length = 10;//strlen(expected_header);

//    static uint8_t header_index = 0;
//    static uint8_t header_started = 0;

//			if (rtcm_state == RTCM_STATE_WAIT_HEADER) 
//			{
//					bytes_read = NtripSocketRead(1, (char *)&byte, 1);
//					if (bytes_read == 1) 
//					{
//							if (!header_started) 
//							{
//									if (byte == 'I') 
//									{
//											header_started = 1;
//											header_index = 0;
//											header_buffer[header_index++] = byte;
//											LOG_DBG(CH_RTCM, "Different byte IIIIIIIIIIIII****************************=%c",byte);
//											//header_buffer[header_index] = '\0';
//									}
//									else
//									{
//									  LOG_DBG(CH_RTCM, "Different byte **************************************************=%c",byte);
//									
//									}
//							} 
//							else 
//							{
//									if (header_index < sizeof(header_buffer) - 1) 
//									{
//											header_buffer[header_index++] = byte;
//											//header_buffer[header_index] = '\0';
//                      LOG_DBG(CH_RTCM, "Different byte ##########################=%c",byte);
//											if (byte == 'K') 
//											{
//													// Once 'K' is received, compare the buffer
//													if (strncmp((char *)header_buffer, expected_header, expected_length) == 0) 
//													{
//															LOG_DBG(CH_RTCM, "NTRIP header detected: %s", header_buffer);
//															rtcm_state = RTCM_STATE_PROCESS_DATA;
//															setntripheaderflag();
//															rtcm_reset();
//													} 
//													else 
//													{
//															LOG_ERR(CH_RTCM, "Invalid NTRIP header: %s", header_buffer);
//													}

//													// Reset for next attempt
//													header_index = 0;
//													header_started = 0;
//											}
//									} 
//									else 
//									{
//											// Overflow protection
//											LOG_ERR(CH_RTCM, "Header buffer overflow");
//											header_index = 0;
//											header_started = 0;
//									}
//							}
//					}
////					else
////					{
////					  LOG_ERR(CH_RTCM, "else condition &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&");
////					
////					}
////					else if (TimeSpent(nontripresponsetick, 5000) && GsmSocketIsOpened(0)) 
////					{
////							nontripresponsetick = GetStartTime();
////					}
//			}
//    else if (rtcm_state == RTCM_STATE_PROCESS_DATA) 
//		{
//        // Process RTCM data frames
//        
//        // Try to read available data in chunks
//        uint8_t temp_buffer[256];
//        bytes_read = NtripSocketRead(1,(char *) temp_buffer, sizeof(temp_buffer));
//        
//        if (bytes_read > 0) 
//				{
//            for (int i = 0; i < bytes_read; i++) 
//						{
//                byte = temp_buffer[i];
//                
//                // Process each byte according to the frame state
//                switch (rtcm_frame_state) 
//								{
//                    case RTCM_FRAME_START:
//                        if (byte == 0xD3) {  // RTCM frame start byte
//                            //LOG_DBG(CH_RTCM, "Get start byte");
//                            rtcm_data_buffer[0] = byte;
//                            rtcm_data_index = 1;
//                            rtcm_frame_state = RTCM_FRAME_LENGTH;
//                        }
//                        break;
//                        
//                    case RTCM_FRAME_LENGTH:
//                        if (rtcm_data_index < 3) 
//												{
//                            rtcm_data_buffer[rtcm_data_index++] = byte;
//                            
//                            if (rtcm_data_index == 3) 
//														{
//                                // Extract message length (10 bits)
//                                rtcm_expected_length = ((rtcm_data_buffer[1] & 0x03) << 8) | rtcm_data_buffer[2];
//                                
//                                // Add header size (3 bytes) and CRC size (3 bytes)
//                                rtcm_expected_length += 6;
//                                
//                                //LOG_DBG(CH_RTCM, "Get Rtcm data length: %d", rtcm_expected_length);
//                                
//                                // Check if length is valid to prevent buffer overflow
//                                if (rtcm_expected_length > sizeof(rtcm_data_buffer)) 
//																{
//                                    LOG_ERR(CH_RTCM, "RTCM frame too large: %d", rtcm_expected_length);
//                                    rtcm_reset();
//                                    rtcm_frame_state = RTCM_FRAME_START;
//                                } 
//																
//																else 
//																{
//                                    rtcm_frame_state = RTCM_FRAME_DATA;
//                                }
//                            }
//                        }
//                        break;
//                        
//                    case RTCM_FRAME_DATA:
//                      
//                        if (rtcm_data_index < rtcm_expected_length - 3) 
//												{  
//                            rtcm_data_buffer[rtcm_data_index++] = byte;
//                            
//                            if (rtcm_data_index == rtcm_expected_length - 3) 
//														{
//                                //LOG_DBG(CH_RTCM, "Get Rtcm Data complete");
//                                rtcm_frame_state = RTCM_FRAME_CRC;
//                                rtcm_crc_index = 0;
//                            }
//                        }
//                        break;
//                        
//                    case RTCM_FRAME_CRC:
//                       
//                        rtcm_crc_bytes[rtcm_crc_index++] = byte;
//                        rtcm_data_buffer[rtcm_data_index++] = byte;
//                        
//                        if (rtcm_crc_index == 3) 
//												{
//                            //LOG_DBG(CH_RTCM, "Get Rtcm CRC byte complete");
//                            rtcm_crc = crc24q_check(rtcm_data_buffer,rtcm_data_index);
////                            
//                              if(rtcm_crc)
//															{
//															  SendRtcm(rtcm_data_buffer,rtcm_data_index);
//															  //LOG_DBG(CH_RTCM, "crc match");
//                                LOG_INFO(CH_GSM, "message length : %d", rtcm_expected_length - 6); 
//															}
//															else 
//														  {
//                                LOG_ERR(CH_RTCM, "crc mismatch");
//                              }
//                            rtcm_reset();
//                            rtcm_frame_state = RTCM_FRAME_START;
//                        }
//                        break;
//                }
//            }
//        }
//				nontripresponsetick=GetStartTime();
//    }
//}

//void rtcm_reset(void) 
//{
//    rtcm_frame_state = RTCM_FRAME_START;
//    rtcm_data_index = 0;
//    rtcm_data_length = 0;
//    rtcm_expected_length = 0;
//    rtcm_crc = 0;
//    rtcm_crc_index = 0;
//}


//    "GET /AUTO HTTP/1.0\r\n"
//    "User-Agent: NTRIP Client11/1.0\r\n"
//    "Authorization: Basic UWROZXRSb3ZlcjAxOlF1ZWN0ZWwyMDI0\r\n"
//    "\r\n");
	  
//	  "GET /IMAX HTTP/1.0\r\n"
//    "User-Agent: NTRIP Python1Client/1.0\r\n"
//    "Authorization: Basic c2lkaGFydGguMTIzOmNvcnNAMjAyMg==\r\n"
//    "\r\n");
//rover1:cm92ZXIxOmFkcGNtbUA4NTA=
//rover2:cm92ZXIyOmFkcGNtbUA4NTA=

//    "GET /AUTO HTTP/1.0\r\n"
//    "User-Agent: NTRIP Client11/1.0\r\n"
//    "Authorization: Basic cm92ZXIxOmFkcGNtbUA4NTA=\r\n"
//    "\r\n");

void MakeNtripHttpHeader(void)
{
  ntripcredPtr = GetNtripCred();
	ntripuser_ccid = getCCID();
  //encode_basic_auth_credentials(ntripcredPtr->ntripusername, ntripcredPtr->ntrippass, output_b64);
	encode_basic_auth_credentials(ntripuser_ccid, ntripcredPtr->ntrippass, output_b64);
  memset(ntriphttpHeader, 0, sizeof(ntriphttpHeader));
	ntriphttpHeaderLen = snprintf((char *)ntriphttpHeader, sizeof(ntriphttpHeader),

  "GET /%s HTTP/1.0\r\n"
  "User-Agent: NTRIP Client11/1.0\r\n"
  "Authorization: Basic %s\r\n"
  "\r\n",ntripcredPtr->mountpoint,output_b64);

	


	LOG_DBG(CH_PACKET, "NTRIP_MakeHttpHeader = %s, HeaderLength = %d",ntriphttpHeader, ntriphttpHeaderLen);   
}
//1: UWROZXRSb3ZlcjAxOlF1ZWN0ZWwyMDI0
//2: UWROZXRSb3ZlcjAyOlF1ZWN0ZWwyMDI0
//3: UWROZXRSb3ZlcjAzOlF1ZWN0ZWwyMDI0


void base64_encode(const unsigned char *input, int length, char *output) 
{
    int i, j;
    for (i = 0, j = 0; i < length;) 
		{
        uint32_t octet_a = i < length ? input[i++] : 0;
        uint32_t octet_b = i < length ? input[i++] : 0;
        uint32_t octet_c = i < length ? input[i++] : 0;

        uint32_t triple = (octet_a << 16) | (octet_b << 8) | octet_c;

        output[j++] = base64_table[(triple >> 18) & 0x3F];
        output[j++] = base64_table[(triple >> 12) & 0x3F];
        output[j++] = (i > length + 1) ? '=' : base64_table[(triple >> 6) & 0x3F];
        output[j++] = (i > length)     ? '=' : base64_table[triple & 0x3F];
    }
    output[j] = '\0';
}


void encode_basic_auth_credentials(uint8_t *username, uint8_t *password, char *output_b64) 
{
    char combined[128];  // Ensure this is large enough for username + ":" + password
		
    snprintf(combined, sizeof(combined), "%s:%s", username, password);
		
    base64_encode((const unsigned char *)combined, strlen(combined), output_b64);
}


uint8_t NtripSendPacket(uint8_t* packetBuff , uint16_t packetlen)
{ 
  if(NtripSendIsWaitForPacket())              //pvt is waiting for normal packet
  {
     if(getntripheaderflag()==0)
		 {
		 
		   rtcmbuffreset();
		   MakeNtripHttpHeader();
			 memset(ntripPacketBuff,0,sizeof(ntripPacketBuff));                
       strncpy((char*)ntripPacketBuff, (const char*)ntriphttpHeader, ntriphttpHeaderLen);
			 ntripPacketLen = ntriphttpHeaderLen;
			 //socId = aisSock0;
			 NtripSendSetState(NTRIP_WAIT_FOR_SOCK_OPEN);
		 }
     else if(getntripheaderflag()==1)
     {
		   memset(ntripPacketBuff,0,sizeof(ntripPacketBuff));                
       strncpy((char*)ntripPacketBuff, (const char*)packetBuff, packetlen);
			 ntripPacketLen = packetlen;
       //socId = aisSock0;
			 NtripSendSetState(NTRIP_WAIT_FOR_SOCK_OPEN);
     }		 


    return 1;
  }
  return 0;
}

ntripSendHandler_et GetntripSendState(void)
{
    return ntripSendState;
}

void rtcm_parser(void) 
{
    uint8_t Nbyte;
    int bytes_read;
    
    // Check current state
    if (rtcm_state == RTCM_STATE_WAIT_HEADER) {
        // Try to read data for header detection
        bytes_read = NtripSocketRead(1, &Nbyte, 1);
        if (bytes_read == 1) {
            // Add byte to header buffer
            if (header_index < sizeof(header_buffer) - 1) {
                header_buffer[header_index++] = Nbyte;
                header_buffer[header_index] = '\0';  // Null terminate
            }
            
            // Check if we have the ICY 200 OK header
            if (strstr((char *)header_buffer, "ICY 200 OK")) 
						{
                LOG_DBG(CH_RTCM, "NTRIP header detected: %s", header_buffer);
                rtcm_state = RTCM_STATE_PROCESS_DATA;
                header_index = 0;
								setntripheaderflag();
                rtcm_reset();
            } 
            else if (header_index >= sizeof(header_buffer) - 1) {
                // Buffer full but no header found, reset
                LOG_ERR(CH_RTCM, "Header buffer full, no header found");
                header_index = 0;
            }
						
        }
    } 
    else if (rtcm_state == RTCM_STATE_PROCESS_DATA) {
        // Process RTCM data frames
        
        // Try to read available data in chunks
        uint8_t temp_buffer[256];
        bytes_read = NtripSocketRead(1, temp_buffer, sizeof(temp_buffer));
        
        if (bytes_read > 0) {
            for (int i = 0; i < bytes_read; i++) {
                Nbyte = temp_buffer[i];
                
                // Process each byte according to the frame state
                switch (rtcm_frame_state) {
                    case RTCM_FRAME_START:
                        if (Nbyte == 0xD3) {  // RTCM frame start byte
                            //LOG_DBG(CH_RTCM, "Get start byte");
                            rtcm_data_buffer[0] = Nbyte;
                            rtcm_data_index = 1;
                            rtcm_frame_state = RTCM_FRAME_LENGTH;
                        }
                        break;
                        
                    case RTCM_FRAME_LENGTH:
                        if (rtcm_data_index < 3) {
                            rtcm_data_buffer[rtcm_data_index++] = Nbyte;
                            
                            if (rtcm_data_index == 3) {
                                // Extract message length (10 bits)
                                rtcm_expected_length = ((rtcm_data_buffer[1] & 0x03) << 8) | rtcm_data_buffer[2];
                                
                                // Add header size (3 bytes) and CRC size (3 bytes)
                                rtcm_expected_length += 6;
                                
                                //LOG_DBG(CH_RTCM, "Get Rtcm data length: %d", rtcm_expected_length);
                                
                                // Check if length is valid to prevent buffer overflow
                                if (rtcm_expected_length > sizeof(rtcm_data_buffer)) {
                                    LOG_ERR(CH_RTCM, "RTCM frame too large: %d", rtcm_expected_length);
                                    rtcm_reset();
                                    rtcm_frame_state = RTCM_FRAME_START;
                                } else {
                                    rtcm_frame_state = RTCM_FRAME_DATA;
                                }
                            }
                        }
                        break;
                        
                    case RTCM_FRAME_DATA:
                      
                        if (rtcm_data_index < rtcm_expected_length - 3) 
												{  
                            rtcm_data_buffer[rtcm_data_index++] = Nbyte;
                            
                            if (rtcm_data_index == rtcm_expected_length - 3) {
                                //LOG_DBG(CH_RTCM, "Get Rtcm Data complete");
                                rtcm_frame_state = RTCM_FRAME_CRC;
                                rtcm_crc_index = 0;
                            }
                        }
                        break;
                        
                    case RTCM_FRAME_CRC:
                       
                        rtcm_crc_bytes[rtcm_crc_index++] = Nbyte;
                        rtcm_data_buffer[rtcm_data_index++] = Nbyte;
                        
                        if (rtcm_crc_index == 3) 
												{
                            //LOG_DBG(CH_RTCM, "Get Rtcm CRC byte complete");
                            //rtcm_crc = (rtcm_crc_bytes[0] << 16) | (rtcm_crc_bytes[1] << 8) | rtcm_crc_bytes[2];
                            rtcm_crc = crc24q_check(rtcm_data_buffer,rtcm_data_index);
//                            if (rtcm_check_crc(rtcm_data_buffer, rtcm_data_index - 3, rtcm_crc)) 
//														{
//                                LOG_DBG(CH_RTCM, "crc match");
//                                LOG_INFO(CH_GSM, "message length : %d", rtcm_expected_length - 6);  
//                                
//                            } 
//														else 
//														{
//                                LOG_ERR(CH_RTCM, "crc mismatch");
//                            }
                              if(rtcm_crc)
															{
															  SendRtcm(rtcm_data_buffer,rtcm_data_index);
															  //LOG_DBG(CH_RTCM, "crc match");
                                LOG_INFO(CH_GSM, "message length : %d", rtcm_expected_length - 6); 
															}
															else 
														  {
                                LOG_ERR(CH_RTCM, "crc mismatch");
                              }
                            rtcm_reset();
                            rtcm_frame_state = RTCM_FRAME_START;
                        }
                        break;
                }
            }
        }
    }
}

// Call this function from your main loop or timer callback
void rtcm_header_Parse(void) 
{
    rtcm_parser();
}

void rtcm_reset(void) 
{
    rtcm_frame_state = RTCM_FRAME_START;
    rtcm_data_index = 0;
    rtcm_data_length = 0;
    rtcm_expected_length = 0;
    rtcm_crc = 0;
    rtcm_crc_index = 0;
}

void rtcmbuffreset(void)
{
   memset(header_buffer, 0, sizeof(header_buffer));
	 
	 memset(rtcm_data_buffer, 0, sizeof(rtcm_data_buffer));
	 
	 memset(rtcm_crc_bytes, 0, sizeof(rtcm_crc_bytes));
	 rtcm_crc_index=0;
	 header_index=0;
	 rtcm_data_index=0;
   rtcm_reset();
	 rtcm_state = RTCM_STATE_WAIT_HEADER;
}