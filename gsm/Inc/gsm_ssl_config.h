#ifndef __SSL_CONFIG_H
#define __SSL_CONFIG_H

#include "gsm_port.h"
#include "gsm_at_handler.h"

#define SSL_CONFIG_CMD_DEFAULT_TIMEOUT      300

//Configure the CipherSuite values
#define  TLS_RSA_WITH_AES_256_CBC_SHA          0X0035
#define  TLS_RSA_WITH_AES_128_CBC_SHA          0X002F
#define  TLS_RSA_WITH_RC4_128_SHA              0X0005
#define  TLS_RSA_WITH_RC4_128_MD5              0X0004
#define  TLS_RSA_WITH_3DES_EDE_CBC_SHA         0X000A
#define  TLS_RSA_WITH_AES_256_CBC_SHA256       0X003D
#define  ALL_CIPHER_SUIT_SUPPORT               0XFFFF

//Configure whether to ignore the RTC time
#define  DO_NOT_IGNORE_RTC_TIME            0
#define  IGNORE_RTC_TIME                   1

//<seclevel> Configure the authentication mode
#define NO_AUTH                             0 
#define MANAGE_SERVER_AUTH                  1
#define MANAGE_SERVER_CLIENT_AUTH           2

//<sslversion> Configure the SSL version
// currently we are using TLS1.2
#define  SSL3_0                              0
#define  TLS1_0                              1
#define  TLS1_1                              2
#define  TLS1_2                              3
#define  ALL_SSL_VERSION_SUPPORT             4

//these enums are for for gsm SSL config command set described below.
typedef enum 
{
   CONFIG_SSL_VERSION=0,
   CONFIG_AUTH_MODE,
   CONFIG_CIPHERSUITE,                        
   CONFIG_RTC_TIME,
   SSL_NUM_CMDS
}sslConfigCmd_et;

typedef enum
{
    SSL_CONFIG_WAIT_FOR_START,
    SSL_CONFIG_IN_PROGRESS,
    SSL_CONFIG_SUCCESS,
    SSL_CONFIG_FAIL
}sslConfigState_et;

typedef struct
{
	char *cmd;
	uint16_t cmdTimeoutInMs;
	GsmCmdResponseCbFnPtr_t respCb;
}sslConfigCmd_st;

void GsmSslConfigStart(void);

sslConfigState_et GsmSslConfigHandler(void);

#endif

