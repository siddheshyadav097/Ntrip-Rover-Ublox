/**
 *  @file          :  digio_api.c
 *  @author        :  Aakash/Ratna
 *  @date          :  18/4/2017
 *  @brief         :  Includes initialization of digital inputs and outputs,
 *  				  along with commands for reading digital i/ps and controlling the digital o/ps as per application.
 *  @filerevision  :  1.0
 *  
 */

 
 /*----includes-----*/
#include "digio_api.h"
//#include "qtimespent.h"
//#include "debug_log.h"


 /*----variables-----*/
//BOOL retvalue;

DigitalInput_t digitalInputs;
//DigitalOutput_t digitalOutputs;

static uint8_t keyId[NUM_TOTAL_INPUTS];

input_s inputHandle[NUM_TOTAL_INPUTS] = 
{
  {&getIgnitionPinState,DIGIO_ON_TIME_MS,DIGIO_OFF_TIME_MS,DIGIO_FILTER_MS},
	{&getSwitchEnablePinState,DIGIO_SWITCH_ON_TIME_MS,DIGIO_SWITCH_OFF_TIME_MS,DIGIO_FILTER_MS},
  
};
 /*----public functions-----*/

static void SetDigInputSenseInterval(uint16_t senseInterval)
{
  static uint8_t i;
  
  for(i = 0; i<NUM_TOTAL_INPUTS; i++)
  {
    inputHandle[i].onTimeInMs = senseInterval;
    inputHandle[i].offTimeInMs = senseInterval;
    keyId[i] = AddInput(&inputHandle[i]); 
  }
}
 void InitDigitalIOConfig(void)
 {
   memset(&digitalInputs, 0, sizeof(DigitalInput_t));
   //memset(&digitalOutputs, 0, sizeof(DigitalOutput_t));
   InitializeDigitalIOPins();
   SetDigInputSenseInterval(5000);  
 }
 
DigitalInput_t* GetDigitalInputStatus(void)
{
  digitalInputs.ignitionState = GetInputState(keyId[0]);
	digitalInputs.switchIpState = GetInputState(keyId[1]);
//  digitalInputs.input1State = GetInputState(keyId[1]);
//  digitalInputs.input2State = GetInputState(keyId[2]);
//  digitalInputs.input3State = GetInputState(keyId[3]);
//  digitalInputs.input4State = GetInputState(keyId[4]);
  
  return(&digitalInputs);  
}

//DigitalOutput_t* GetDigitalOutputStatus(void)
//{
//  digitalOutputs.digitalOutput1 = GetOutput1State();
//  digitalOutputs.digitalOutput2 = GetOutput2State();
//  
//  return (&digitalOutputs);
//}
