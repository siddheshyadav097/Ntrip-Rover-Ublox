#ifndef __GSM_PROCESS_RESP_H
#define __GSM_PROCESS_RESP_H
//Remove D1 n D2 make lm2576 as 4.2v
//use 2576 change divider n/w to generate 4.2 volts
//use 2575 for 1A current
#include "gsm_port.h"
#include "gsm_common.h"

/*********************************************/
/*            GSM COMMAND RESPONSES          */
/*********************************************/
typedef enum
{
  NO_RESPONSE       =0,
  RESP_OK           =1,
  RESP_ERROR        =2,
  RESP_CME_ERROR    =3,
  RESP_CMS_ERROR    =4,
  RESP_NO_CARRIER   =5,
  RESP_BUSY         =6,
  RESP_RING         =7,
  RESP_IGNORE       =8,
  RESP_NEWSMS       =9,
  RESP_NO_WAIT      =10,
  RESP_OTHER        =11,          
  RESP_TIMEOUT      =12,
  RESP_SOCKET_OPEN  =13,
  RESP_SEND_OK      =14,
  RESP_DEACT_OK     =15,
  RESP_SIM_STATUS   =16,
  RESP_FUNCTIONALITY =17,
  RESP_GSM_NW       =18,
  RESP_GPRS_NW      =19,
  RESP_CELLID_DATA  =20,
  RESP_SIGNAL_DATA  =21,
  RESP_OPS_DATA     =22,
  RESP_RCV_SOCKET_DATA =23,
  RESP_IP_INITIAL   =24,
  RESP_IP_START     =25,
  RESP_IP_CONFIG    =26,
  RESP_IP_IND       =27,
  RESP_IP_GPRSACT   =28,
  RESP_IP_STATUS    =29,
  RESP_IP_PROCESSING =30,
  RESP_TCP_CONNECTING =31,
  RESP_GPRS_DEACT   =32,
  RESP_SCONNECT_OK  =33,
  RESP_IP_CLOSE     =34,
  RESP_PDP_DEACT    =35,
  RESP_SOCK_STATE    =36,
  RESP_CONECT_FAIL  =37,
  RESP_S0_CLOSE_OK  =38,
  RESP_S1_CLOSE_OK  =39,
  RESP_SHUT_OK      =40,
  RESP_SEND_FAIL    =41,
  RESP_CLOSED       =42,
  RESP_SPDP_DEACT   =43,
  RESP_ALREADY_CONNECT =44,
  RESP_NO_GPRS_CMD  =45,
  RESP_GPRS_CMD     =46,
  RESP_NO_ANSWER    =47,
  RESP_GPRS_DATA             = 48,
  SSL_SOC_OPEN_REPLY         = 49,
  RESP_URC                   = 50,
  RESP_GSM_TIME              = 51,
  RESP_QDNET_SOC_CONNECT_OK  = 52,
  RESP_QD_DATA_SENT_OK       = 53,
  RESP_SEND_SMS              = 54,
  RESP_READ_SMS              = 55,
  RESP_LIST_SMS              = 56,
  RESP_SERVICE_PROVIDER_NAME = 57,
  RESP_UFS_SIZE              = 58,
  RESP_PROMPT_CHAR  =59,   
  RESP_QUERY_GPRS_ACT    =60,
  RESP_FTP_CONFIG   =61,
  RESP_FTP_OPEN   =62,
  RESP_FTP_PATH   =63,
  RESP_FTP_SIZE   =64,
  RESP_FTP_GETFILE   =65,
  RESP_FTP_CLOSE   =66,
  RESP_FILE_LIST   =67,
  RESP_FILE_OPEN   =68,
  RESP_FILE_READ   =69,
  RESP_FILE_CLOSE  =70,
  RESP_NW_INFORMATION = 71,
  RESP_SOFT_SHUT_CMD = 74  
  
}gsmCmdResponseCode_et;


//Response and their matching return values
typedef struct
{
    char const *response;
	gsmCmdResponseCode_et  respCode;
}gsmCmdResp_st;


/**
 *  @brief It checks the received response with all the valid command response
 *  @param [in] resp pointer that holds the line data
 *  @param [in] respLen length of the response data
 *  @return will return the valid response code if found
 *  			will return RESP_OTHER if no match is found
 */
gsmCmdResponseCode_et CheckCmdResponse(uint8_t *resp, uint16_t respLen);





#endif