#ifndef __SIM_API_H
#define __SIM_API_H

#include "gsm_port.h"
#include "gsm_at_handler.h"

typedef enum 
{
    SIM_STAT_NOT_INSERTED = 0,
    SIM_STAT_READY,
    SIM_STAT_PIN_REQ,
    SIM_STAT_PUK_REQ,
    SIM_STAT_PH_PIN_REQ,
    SIM_STAT_PH_PUK_REQ,
    SIM_STAT_PIN2_REQ,
    SIM_STAT_PUK2_REQ,
    SIM_STAT_BUSY,
    SIM_STAT_FAILURE,
    SIM_STAT_NOT_READY,
    SIM_STAT_UNSPECIFIED
}simState_et;


simState_et GsmGetSimStateByErrCode(char* errCode, uint8_t len);

simState_et GsmGetSimStateByName(char* simStat, uint8_t len);



#endif