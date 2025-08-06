#ifndef __GSM_API_H
#define __GSM_API_H

#include "gsm_port.h"
#include "gsm_common.h"
#include "gsm_at_handler.h"
#include "gsm_statemachine.h"
#include "gsm_socket_api.h"
#include "gsm_sms.h"
#include "gsm_utility.h"
#include "debug_log.h"
/* *  @brief  : Initialises the uart port and the statemachine
 *  @return :  none*/

/**
 */
void GsmInit(void);

/**
 *  @brief  : This function to be called in the main loop
 *            gsm statemachine and the reponse handler functions are called here
 *  @return : none
 */
void GsmHandler(void);




#endif