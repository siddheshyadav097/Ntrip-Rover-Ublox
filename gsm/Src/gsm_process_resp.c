#include "gsm_process_resp.h"
#include <string.h>

const static gsmCmdResp_st gsmResponsePrefixEntry[] =
{
  {"AT",                    RESP_IGNORE},
  {"OK",                    RESP_OK },
  {"ERROR" ,                RESP_ERROR},
  {"+CMS ERROR",            RESP_CMS_ERROR},
  {"+CME ERROR",            RESP_CME_ERROR},
  {"+QIOPEN: ",             RESP_SOCKET_OPEN},
  {"+QIACT: ",              RESP_QUERY_GPRS_ACT},
  {"SOC_ESTB",              RESP_QDNET_SOC_CONNECT_OK},
  {"SRV_ACK",               RESP_QD_DATA_SENT_OK},
  {"SEND OK",               RESP_SEND_OK},
  {"DEACT OK",              RESP_DEACT_OK},
  {"+QSSLOPEN: ",           SSL_SOC_OPEN_REPLY},
  {"+CPIN:",                RESP_SIM_STATUS},
  {"+COPS:",                RESP_OPS_DATA},
  {"+QSPN:",                RESP_SERVICE_PROVIDER_NAME},
  {"+QNWINFO:",             RESP_NW_INFORMATION},
  {"+CFUN:",                RESP_FUNCTIONALITY},
  {"+CREG:",                RESP_GSM_NW},
  {"+CGREG:",               RESP_GPRS_NW},
  {"+QENG:",                RESP_CELLID_DATA},
  {"+CSQ:",                 RESP_SIGNAL_DATA},
  {"+QIRD:",                RESP_RCV_SOCKET_DATA},
//  {"+QSSLRECV:",            RESP_RCV_SOCKET_DATA},
//  {"+QNITZ:"   ,            RESP_IGNORE},
  {"+CCLK:"   ,             RESP_GSM_TIME},
  {"STATE: IP INITIAL",     RESP_IP_INITIAL},
  {"STATE: IP START",       RESP_IP_START},
  {"STATE: IP CONFIG",      RESP_IP_CONFIG},
  {"STATE: IP IND",         RESP_IP_IND},
  {"STATE: IP GPRSACT",     RESP_IP_GPRSACT},
  {"STATE: IP STATUS",      RESP_IP_STATUS},
  {"STATE: IP PROCESSING",  RESP_IP_PROCESSING},
  {"STATE: PDP DEACT",      RESP_PDP_DEACT},
  {"+QISTATE:"       ,      RESP_SOCK_STATE},
  {"CONNECT FAIL",          RESP_CONECT_FAIL},
  {"0, CONNECT FAIL",       RESP_CONECT_FAIL},
  {"1, CONNECT FAIL",       RESP_CONECT_FAIL},
  {"0, CLOSE OK",           RESP_S0_CLOSE_OK},
  {"1, CLOSE OK",           RESP_S1_CLOSE_OK},
  {"+CMGS:",                RESP_SEND_SMS},
  {"+CMGR:",                RESP_READ_SMS},
  {"+CMGL:",                RESP_LIST_SMS},
  {"SHUT OK",               RESP_SHUT_OK},
  {"SEND FAIL",             RESP_SEND_FAIL},
  {"ALREADY CONNECT",       RESP_ALREADY_CONNECT},
  {"BUSY",                  RESP_BUSY},
  {"RING",                  RESP_RING},
  {"NO CARRIER",            RESP_NO_CARRIER},
  {"+PPSTIR",               RESP_IGNORE},
  {"PN",                    RESP_NO_GPRS_CMD},
  {"PY",                    RESP_GPRS_CMD},
  {"NO ANSWER",             RESP_NO_ANSWER},
  {"PD",                    RESP_GPRS_DATA},
  {"STATE: CONNECT OK",     RESP_SCONNECT_OK},
  {"STATE: IP CLOSE",       RESP_IP_CLOSE},
  {"+QFLDS: ",              RESP_UFS_SIZE},
  {"+QFTPCFG:",             RESP_FTP_CONFIG},
  {"+QFTPOPEN:",            RESP_FTP_OPEN},
//  {"+QFTPPATH:",            RESP_FTP_PATH},
  {"+QFTPCWD:",            RESP_FTP_PATH},
  {"+QFTPSIZE:",            RESP_FTP_SIZE},
  {"+QFTPGET:",             RESP_FTP_GETFILE},
  {"+QFTPCLOSE:",           RESP_FTP_CLOSE},
  {"+QFLST: ",              RESP_FILE_LIST},
  {"+QFOPEN: ",             RESP_FILE_OPEN},
  {"CONNECT ",              RESP_FILE_READ},
  {"POWERED DOWN",          RESP_SOFT_SHUT_CMD}
};

gsmCmdResponseCode_et CheckCmdResponse(uint8_t *resp, uint16_t respLen)
{
	uint8_t i;
	gsmCmdResponseCode_et respCode = RESP_OTHER;
    
	for(i = 0; i < NUM_ELEMS(gsmResponsePrefixEntry); i++)
	{
		if(strncmp((const char*)resp,(const char *)gsmResponsePrefixEntry[i].response,strlen((const char *)gsmResponsePrefixEntry[i].response)) == 0)
		{
            if((gsmResponsePrefixEntry[i].respCode == RESP_GSM_NW) || (gsmResponsePrefixEntry[i].respCode == RESP_GPRS_NW))
            {
              if(NULL == strstr((char *)resp,","))
              {
                respCode = RESP_OTHER;
                break;
              }
            }
			respCode = gsmResponsePrefixEntry[i].respCode;
			break;
        }
	}
	return respCode;
}