/**
 *  \file digio_port.h
 *  \brief Includes for platform specific dependencies and public function declarations 
 *			for Digital i/o port initialization & digital i/o handles.
 */

#ifndef __DIGIO_PORT_H
#define __DIGIO_PORT_H


 /*----includes-----*/
#include "lib_port.h"
#include "debug_log.h"
#include "dig_ip_handler.h"


#define VEHICLE_IGNITION_PORT    GPIOA           
#define VEHICLE_IGNITION_PIN     GPIO_PIN_9  


#define  SWITCH_ENABLE_PORT      GPIOC
#define  SWITCH_ENABLE_PIN       GPIO_PIN_6

   

typedef enum
{
    OUTPUT_OFF,
    OUTPUT_ON
}outputState_e;


 /*----public function declarations-----*/
void InitializeDigitalIOPins(void);
inputState_e getIgnitionPinState(void);
inputState_e getSwitchEnablePinState(void);
void ChangeSwitchPinToOutPut(void);
void PutOffCpuSupply(void);

#endif