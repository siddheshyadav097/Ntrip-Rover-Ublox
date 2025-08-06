#ifndef __GSM_POWER_H
#define __GSM_POWER_H

#include "gsm_port.h"

#define GSM_POWER_OFF_WAIT_MS       2000//5000        // 8 seconds
#define GSM_POWER_ON_WAIT_MS        2000//5000        // 8 seconds

#define WAIT_FOR_GSM_PWRKEY         1000
#define GSM_POWERKEY_ON_WAIT_MS     1000
#define GSM_POWERKEY_OFF_WAIT_MS    10000

typedef enum
{
    GSM_POWER_RESET_INIT,
    GSM_POWER_OFF_WAIT,
    GSM_POWER_ON_WAIT,
    GSM_POWER_RESET_DONE,
    GSM_POWERKEY_ON_WAIT,
    GSM_POWERKEY_OFF_WAIT,
    GSM_POWERKEY_DONE
      
}powerResetState_et;

/**
 *  @brief  : set the gsm power state 
 *  @param  :[in] power 
 *  @return :none
 */
void GsmResetPowerSetState(powerResetState_et power);

/**
 *  @brief  : It will power off the module and wait for some time and then power ON the module
 *              after power reset is done successfully
 *  @return :Return_Description
 */
powerResetState_et GsmResetPowerHandler(void);


#endif