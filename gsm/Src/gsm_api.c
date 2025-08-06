#include "gsm_api.h"
#include "gsm_gprs_api.h"
#include "mem_config_api.h"

/**
 *  @brief  : Initialises the uart port and the statemachine
 *  @return :  none
 */
void GsmInit(void)
{
    GsmAtHandlerInit();
    GsmPwrEnableInit();
    GsmPwrKeyInit();
    GsmStateMachineInit();
//    LoadDefaultGprsSim0Config();
}

/**
 *  @brief  : This function to be called in the main loop
 *            gsm statemachine and the reponse handler functions are called here
 *  @return : none
 */
void GsmHandler(void)
{
    GsmStateMachine();
    GsmResponseHandler();
}