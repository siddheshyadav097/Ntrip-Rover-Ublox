/**
 *  \file digio_api.h
 *  \brief Header file containing constants required for digital i/o processing APIs and their function declarations.
 */

#ifndef __DIGIO_API_H__
#define __DIGIO_API_H__


 /*----includes-----*/
#include "digio_port.h"
#include "dig_ip_handler.h"

 /*----constants-----*/

#define DIGIO_ON_TIME_MS     	500
#define DIGIO_OFF_TIME_MS       500
#define DIGIO_FILTER_MS		    20


#define DIGIO_SWITCH_ON_TIME_MS     	    5000
#define DIGIO_SWITCH_OFF_TIME_MS          5000


typedef struct
{
  uint8_t ignitionState;
	uint8_t switchIpState;
}DigitalInput_t;

//typedef struct
//{
//  uint8_t digitalOutput1;
//  uint8_t digitalOutput2;  
//}DigitalOutput_t;

 /*----public function declarations-----*/
void InitDigitalIOConfig(void);
DigitalInput_t* GetDigitalInputStatus(void);
//DigitalOutput_t* GetDigitalOutputStatus(void);
#endif