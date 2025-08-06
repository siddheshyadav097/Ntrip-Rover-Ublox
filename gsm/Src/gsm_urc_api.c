#include "gsm_urc_api.h"
#include "gsm_sim_api.h"
#include "gsm_init_cmd.h"
#include "gsm_statemachine.h"
#include "gsm_socket_api.h"
#include "gsm_sms.h"
#include "gsm_gprs_api.h"
#include <string.h>
#include <stdlib.h>
#include "debug_log.h"


static uint8_t strTmp[50];
gsmTimePara_t nwTimeByUrc;

extern uint8_t flagRtcTimeUpdate;

void UrcCallReadyCb(uint8_t *lineBuff, uint16_t len)
{
    GsmInitUpdateModuleReady();
}

void UrcSmsReadyCb(uint8_t *lineBuff, uint16_t len)
{
    
}

void UrcCpinCb(uint8_t *lineBuff, uint16_t len)
{
    char *p1 = NULL;
    char *p2 = NULL;
    
    p1 = (char *)lineBuff + 7;
    p2 = strchr(p1, '\0');
    if (p1 && p2)
    {
        memset(strTmp, 0x0, sizeof(strTmp));
        memcpy(strTmp, p1, p2 - p1);
        if(GsmGetSimStateByName((char *)strTmp, p2 - p1) == SIM_STAT_READY)
        {
            GsmInitUpdateSimReady();
        }
        else
        {
            GsmInitUpdateSimNotReady();
            GsmSetState(SIM_CARD_NOT_PRESENT);
        }
    }
}

void UrcCfunCb(uint8_t *lineBuff, uint16_t len)
{
    char* p1 = NULL; 
    uint8_t cfun;

    p1 = (char *)lineBuff+7;  //+CFUN: 1
    memset(strTmp, 0x0, sizeof(strTmp));
    memcpy(strTmp, p1, 1);
    cfun = atoi((const char *)strTmp);
    if(cfun == 1)          //GSM Modem is set to Full functionality (Default)
    {
      GsmInitFunctionalityReady();
    }
    //here in below cases we need to init module again as the functionality is lost
    //here set the gsm modem state to init state and init all the flags
    else if(cfun == 4)     //Disable phone both transmit and receive RF circuits
    {
      GsmInitFunctionalityNotReady();
    }
    else if(cfun == 0)     //0-Minimum functionality
    {
      GsmInitFunctionalityNotReady();
    }
    else
    {
      GsmInitFunctionalityNotReady();
    }   
}

void UrcQnitzCb(uint8_t *lineBuff, uint16_t len)
{
    char tempBuff[20];
    static uint8_t tempYear = 0;
    
    memset(tempBuff,0,sizeof(tempBuff));    //+QNITZ: "18/06/23,11:42:28+22,0"
    
    strncpy(tempBuff, (char*)lineBuff+9, 2);
    //year check is included bcoz when gsm is not registered, +qnitz urc sometimes give below mentioned reply 
    //+QNITZ: "80/01/06,00:00:14+22" which is wrong for current year
    tempYear = (uint8_t)atoi(tempBuff);           
    if(tempYear == 80)
      nwTimeByUrc.year = 0;
    else
      nwTimeByUrc.year =  (uint8_t)atoi(tempBuff);
    strncpy(tempBuff, (char*)lineBuff+12, 2);
    nwTimeByUrc.month = (uint8_t)atoi(tempBuff);
    strncpy(tempBuff, (char*)lineBuff+15, 2);
    nwTimeByUrc.day = (uint8_t)atoi(tempBuff);
    strncpy(tempBuff, (char*)lineBuff+18, 2);
    nwTimeByUrc.hour = (uint8_t)atoi(tempBuff);
    strncpy(tempBuff, (char*)lineBuff+21, 2);
    nwTimeByUrc.minute = (uint8_t)atoi(tempBuff);
    strncpy(tempBuff, (char*)lineBuff+24, 2);
    nwTimeByUrc.second = (uint8_t)atoi(tempBuff);
    if(nwTimeByUrc.year >= CURRENT_DEFAULT_YEAR)
		{
      flagRtcTimeUpdate = 1;
		}
    else
		{
      flagRtcTimeUpdate = 0;
		}
}

void UrcGprsRegistrationCb(uint8_t *lineBuff, uint16_t len)
{
    char* p1 = NULL;
    gsmNetworkState_et networkState;
    
    p1 = strchr((const char *)lineBuff,':'); //+CGREG: 1
    p1 = p1+2;
    if (p1 != NULL)
    {
        memset(strTmp, 0x0, sizeof(strTmp));
        memcpy(strTmp,p1,1);
        networkState = (gsmNetworkState_et)atoi((const char *)strTmp);
    }
        
    //Comapre the network state value with the expected valuess
    if(networkState == NETWORK_STAT_REGISTERED || networkState == NETWORK_STAT_REGISTERED_ROAMING)
    {
        GsmUpdateGprsRegistration(1, networkState);
    }
    else
    {
        GsmUpdateGprsRegistration(0, networkState);
    }
}

void UrcGsmRegistrationCb(uint8_t *lineBuff, uint16_t len)
{
    char* p1 = NULL;
    gsmNetworkState_et networkState;
    
    p1 = strchr((const char *)lineBuff,':'); //+CREG: 1
    p1 = p1+2;
    if (p1 != NULL)
    {
        memset(strTmp, 0x0, sizeof(strTmp));
        memcpy(strTmp,p1,1);
        networkState = (gsmNetworkState_et)atoi((const char *)strTmp);
    }
        
    //Comapre the network state value with the expected valuess
    if(networkState == NETWORK_STAT_REGISTERED || networkState == NETWORK_STAT_REGISTERED_ROAMING)
    {
        GsmUpdateNetworkRegistration(1,networkState);
    }
    else 
    {
        GsmUpdateNetworkRegistration(0,networkState);
    }
}

void UrcVoltageMonitorCb(uint8_t *lineBuff, uint16_t len)
{
    
}

const static gsmUrcResp_st gsmURCResponsePrefixEntry[]=
{
   {"Call Ready",            URC_SMS_READY,            UrcSmsReadyCb},
   {"SMS Ready",             URC_CALL_READY ,          UrcCallReadyCb},
   {"+CPIN:",                     URC_SIM_STATUS,           UrcCpinCb},
   {"+CFUN:",                     URC_GSM_FUN,              UrcCfunCb},
//   {"+CTZE:",                     URC_TIME_ZONE_REPORT,     UrcCtzeCb},
   {"+CMTI:",                     URC_RCV_SMS,              UrcSmsCb},
   {"+CREG:",                     URC_GSM_REG,              UrcGsmRegistrationCb},
   {"+CGREG:",                    URC_GPRS_REG,             UrcGprsRegistrationCb},
//   {"+QIRDI:",                    URC_READ_SOCKET_DATA,     UrcUnsecuredSocketReadCb},
   {"+QIURC:",                    URC_READ_SOCKET_DATA,     UrcUnsecuredSocketReadCb},
//   {"+QSSLURC:",                  URC_READ_SOCKET_DATA,     UrcSecuredSocketCb},
   {"0, CLOSED",                  URC_SOCKET_CLOSED,        UrcUnsecuredS0ClosedCb},
   {"1, CLOSED",                  URC_SOCKET_CLOSED,        UrcUnsecuredS1ClosedCb},
//   {"2, CLOSED",                  URC_SOCKET_CLOSED,        UrcUnsecuredS2ClosedCb},
//   {"3, CLOSED",                  URC_SOCKET_CLOSED,        UrcUnsecuredS3ClosedCb},
//   {"4, CLOSED",                  URC_SOCKET_CLOSED,        UrcUnsecuredS4ClosedCb},
   {"+PDP DEACT",                 URC_PDP_DEACTIVATED,      UrcGprsDeactiveCb},
  // {"+QNITZ",                     URC_GSM_NW_TIME,          UrcQnitzCb},
   {"UNDER_VOLTAGE WARNING",      URC_UNDER_VOLTAGE_WARN,   UrcVoltageMonitorCb},
   {"UNDER_VOLTAGE POWER DOWN",   URC_UNDER_VOLTAGE_PD,     UrcVoltageMonitorCb},
   {"OVER_VOLTAGE WARNING",       URC_OVER_VOLATGE_WARN,    UrcVoltageMonitorCb},
   {"OVER_VOLTAGE POWER DOWN",    URC_OVER_VOLTAGE_PD,      UrcVoltageMonitorCb}
};


gsmUrcResponseCode_et CheckUrcResonse(uint8_t *resp,uint16_t respLen)
{
	uint8_t i;
	gsmUrcResponseCode_et respCode = URC_OTHER;
    
    //LOG_DBGSN(CH_GSM,(char *)resp,respLen);
    
	for(i = 0; i < NUM_ELEMS(gsmURCResponsePrefixEntry); i++)
	{
		if(strncmp((const char*)resp,(const char *)gsmURCResponsePrefixEntry[i].expectedUrc,strlen((const char *)gsmURCResponsePrefixEntry[i].expectedUrc)) == 0)
		{
			LOG_DBG(CH_GSM,"%s",(char *)resp);
            respCode = gsmURCResponsePrefixEntry[i].respCode;
            gsmURCResponsePrefixEntry[i].fnPtr(resp,respLen);
            break;
        }
	}
	return respCode;
}

void GetNWTimeByUrc(gsmTimePara_t* gsmTimeParaPtr)
{
    memset(gsmTimeParaPtr,0,sizeof(gsmTimePara_t));
	memcpy(gsmTimeParaPtr, &nwTimeByUrc, sizeof(gsmTimePara_t));
}