/**
 *  \file led_api.h
 *  \brief Includes typedefs for LED states and public function declarations for LED handling.
 */

#ifndef __LED_API_H__
#define __LED_API_H__


 /*----includes-----*/
#include "lib_port.h"
#include "led_port.h"

#define MAX_GPS_VALIDITY_DISPLAY_TIME    2000

#define MAX_GSM_VALIDITY_DISPLAY_TIME    2000

 /*----typedefs-----*/
typedef enum 
{
	GSM_NOT_REG,
	GSM_REG,
	GPRS_ACT,
	PACKET_SEND_SUCCESS
}gprsLedState_et;

typedef enum 
{
	GPS_VALID,
	GPS_FIX_VALID,
	GPS_FLOAT_VALID,
	GPS_INVALID
}gpsLedState_et;

typedef enum
{
    BATT_MODE,
	CHARGING_MODE,
	CHARGE_COMPLETE,
    BATT_LOW
}battLedState_et;

typedef enum
{
  WAIT_FOR_RF_TEST_TAG,
  VALID_RFID_TEST_TAG_DETECTED,
  DISPLAY_STATUS_IDLE_STATE,
  GPRS_STATUS_DISPLAY,
  GPS_STATUS_DISPLAY

}rfidUniteStates_et;

 /*----public function declarations-----*/
void Init_LED_Config(void);
rfidUniteStates_et GetRfTestTagDetectState(void);
void RfidUnitLedSetState(rfidUniteStates_et rfUnitCurrState);
void Set_gprsLEDState(gprsLedState_et gprs_state);
gprsLedState_et GetGprsLEDState(void);
void Set_gpsLEDState(gpsLedState_et gps_state);
gpsLedState_et GetGpsLEDState(void);
void Set_batteryLEDState(battLedState_et batt_state);
void Led_State_Handler(void);

#endif