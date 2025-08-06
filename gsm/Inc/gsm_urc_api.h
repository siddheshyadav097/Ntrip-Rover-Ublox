#ifndef __GSM_URC_API_H
#define __GSM_URC_API_H

#include "gsm_port.h"
#include "gsm_at_handler.h"


/*********************************************/
/*            GSM URC RESPONSES          */
/*********************************************/
typedef enum
{
  URC_CALL_READY =0,
  URC_SMS_READY,
  URC_SIM_STATUS,
  URC_GSM_FUN,
  URC_TIME_ZONE_REPORT,
  URC_RCV_SMS, 
  URC_GSM_REG ,
  URC_GPRS_REG,
  URC_SOCKET_CLOSED,
  URC_PDP_DEACTIVATED,
//  RETRIEVE_SOCKET_DATA,
  URC_READ_SOCKET_DATA,
  URC_UNDER_VOLTAGE_WARN,
  URC_UNDER_VOLTAGE_PD,
  URC_OVER_VOLATGE_WARN,
  URC_OVER_VOLTAGE_PD,
  URC_GSM_NW_TIME,
  URC_OTHER
}gsmUrcResponseCode_et;

typedef struct
{
    char const *expectedUrc;
	gsmUrcResponseCode_et  respCode;
    void (*fnPtr)(uint8_t *lineBuff,uint16_t len);
    uint8_t flagToBeProcessed;
}gsmUrcResp_st;


/**
 *  @brief It checks the received response with all the valid URC response
 *  @param [in] resp pointer that holds the line data
 *  @param [in] respLen length of the response data
 *  @return will return the valid response code if found
 *  			will return URC_OTHER if no match is found
 */
gsmUrcResponseCode_et CheckUrcResonse(uint8_t *resp,uint16_t respLen);

/**
 *  @brief  : It called from at handler when URC is received
 *  @param  :[in] gsmResp  - urc response type
 *  @param  :[in] lineBuff response line data
 *  @param  :[in] len      length of response
 *  @return : none
 */
//void UrcProcessCb(gsmUrcResponseCode_et gsmResp, uint8_t *lineBuff, uint16_t len);

void GetNWTimeByUrc(gsmTimePara_t* gsmTimeParaPtr);

#endif