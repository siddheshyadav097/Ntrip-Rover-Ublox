#include "gsm_sim_api.h"
#include <string.h>


/**
 *  @brief This function gsmGetSimStateByErrCode() gets the status of the sim card
 *         according to the error string receive with tthe prefix +CPIN: 
 *   @return :EnumSIMState_et ss enum value.
 * 
 */
simState_et GsmGetSimStateByErrCode(char* errCode, uint8_t len)
{
    simState_et ss = SIM_STAT_UNSPECIFIED; 
    if (strncmp(errCode, "SIM not inserted", len) == 0)
    {
        ss = SIM_STAT_NOT_INSERTED;
    }
    else if (strncmp(errCode, "SIM PIN required", len) == 0)
    {
        ss = SIM_STAT_PIN_REQ;
    }
    else if (strncmp(errCode, "SIM PUK required", len) == 0)
    {
        ss = SIM_STAT_PUK_REQ;
    }
    else if (strncmp(errCode, "PH-SIM PIN required", len) == 0)
    {
        ss = SIM_STAT_PH_PIN_REQ;
    }
    else if (strncmp(errCode, "PH-FSIM PUK required", len) == 0)
    {
        ss = SIM_STAT_PH_PUK_REQ;
    }
    else if (strncmp(errCode, "SIM PIN2 required", len) == 0)
    {
        ss = SIM_STAT_PIN2_REQ;
    }
    else if (strncmp(errCode, "SIM PUK2 requird", len) == 0)
    {
        ss = SIM_STAT_PUK2_REQ;
    }
    else if (strncmp(errCode, "SIM busy", len) == 0)
    {
        ss = SIM_STAT_BUSY;
    }
    else if (strncmp(errCode, "SIM failure", len) == 0)
    {
        ss =  SIM_STAT_FAILURE;
    }
    else if (strncmp(errCode, "NOT READY", len) == 0)
    {
        ss = SIM_STAT_NOT_READY;
    }
    return ss;
}

/**
 *  @brief This function gsmGetSimStateByName() gets the status of the sim card
 *         according to the string receive with the prefix +CPIN: 
 *   @return :EnumSIMState_et ss enum value.
 * 
 */
simState_et GsmGetSimStateByName(char* simStat, uint8_t len)
{
    simState_et ss = SIM_STAT_UNSPECIFIED;
    
    if (strncmp(simStat, "READY", len) == 0)
    {
        ss = SIM_STAT_READY;
    }
    else if (strncmp(simStat, "NOT INSERTED", len) == 0)
    {
        ss = SIM_STAT_NOT_INSERTED;
    }
    else if (strncmp(simStat, "SIM PIN", len) == 0)
    {
        ss = SIM_STAT_PIN_REQ;
    }
    else if (strncmp(simStat, "SIM PUK", len) == 0)
    {
        ss = SIM_STAT_PUK_REQ;
    }
    else if (strncmp(simStat, "PH-SIM PIN", len) == 0)
    {
        ss = SIM_STAT_PH_PIN_REQ;
    }
    else if (strncmp(simStat, "PH-SIM PUK", len) == 0)
    {
        ss = SIM_STAT_PH_PUK_REQ;
    }
    else if (strncmp(simStat, "SIM PIN2", len) == 0)
    {
        ss = SIM_STAT_PIN2_REQ;
    }
    else if (strncmp(simStat, "SIM PUK2", len) == 0)
    {
        ss = SIM_STAT_PUK2_REQ;
    }
    else if (strncmp(simStat, "SIM BUSY", len) == 0)
    {
        ss = SIM_STAT_BUSY;
    }
    else if (strncmp(simStat, "SIM FAILURE", len) == 0)
    {
        ss = SIM_STAT_FAILURE;
    }
    else if (strncmp(simStat, "NOT READY", len) == 0)
    {
        ss = SIM_STAT_NOT_INSERTED;   //SIM_STAT_NOT_READY
    }
    return ss;
}