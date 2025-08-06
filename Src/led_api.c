/**
 *  @file			:	led_api.c
 *  @author   		:	Aakash/Ratna
 *  @date    		:	18/4/2017
 *  @brief    		: 	Handler for LED states, comprises of state machine for the available LED's.
 *  					Init_LED_Config must be called at the start of application.
 *  					Led_State_Handler should be called continuously. It includes the state machines for the 3 LED's,
 * 						GPRS_Led_Handler, GPS_Led_Handler & Power_Led_Handler.
 *						Set state functions are provided to the application to alter the state of the LEDs.
 *  @filerevision	:	1.0
 */
 
 /*----includes-----*/
#include "led_api.h"
#include "qtimespent.h"
#include "debug_log.h"


/*----constants-----*/
#define GSMREG_LEDON_TOGGLE_INTERVAL	(100)
#define GSMREG_LEDOFF_TOGGLE_INTERVAL	(1000)
#define GPRSREG_LED_TOGGLE_INTERVAL		(1000)
#define PACKET_LED_TOGGLE_INTERVAL		(200)

#define GPS_LED_TOGGLE_INTERVAL			(2000)//(1000)

#define POWER_LED_TOGGLE_INTERVAL		(2000)

#define BATMODE_LEDOFF_INTERVAL			(1000)
#define BATMODE_LEDON_INTERVAL			(100)   //600

#define BATLOW_LEDOFF_INTERVAL			(100)//(1*1000)
#define BATLOW_LEDON_INTERVAL			(100)

 /*----variables-----*/

uint32_t gprs_led_starttime, gps_led_starttime, power_led_starttime , rf_gprs_led_starttime,rf_gps_led_starttime;
uint32_t gps_gsm_display_time;
uint8_t gprsled_flag = 0, gpsled_flag = 0, powerled_flag = 0 , rfUnitStatusLed = 0;
uint8_t batt_led_flag = 0, gsmreg_led_flag = 0, packet_led_flag = 0 , rf_gsmreg_led_flag;

uint8_t rfUnitGetStatus = 0;

gprsLedState_et gprs_led_state = GSM_NOT_REG;
gpsLedState_et gps_led_state = GPS_INVALID;
battLedState_et batt_led_state = BATT_MODE;

//enum to check the functionality of the peripherals of the rfid unit
rfidUniteStates_et rfUnitWorkState = WAIT_FOR_RF_TEST_TAG;


 /*----function prototypes-----*/
static void Toggle_led(Enum_PinName pinid);
static void GPRS_Led_Handler(void);
static void GPS_Led_Handler(void);
static void Power_Led_Handler(void);


 /*----private functions-----*/
 
/**
 *  @brief This function toggles the state of pinid depending on value of their respective flags
 *  for e.g. if gprsled_flag == 0 then Call Led_State_ON & set its flag to 1 or else call Led_State_OFF
 *  & clear its respective flag. For other 2 conditions same procedure is followded
 *  @param [in] pinid one value of @ref Enum_PinName
 *  @return void
 */
static void Toggle_led(Enum_PinName pinid)
{
	if(pinid == GPRS_LED)
	{
		if(gprsled_flag == 0)
		{
			Led_State_ON(pinid);
			gprsled_flag = 1;
		}
		else 
		{
			Led_State_OFF(pinid);
			gprsled_flag = 0;
		}
	}
	else if(pinid == GPS_LED)
	{
		if(gpsled_flag == 0)
		{
			Led_State_ON(pinid);
			gpsled_flag = 1;
		}
		else 
		{
			Led_State_OFF(pinid);
			gpsled_flag = 0;
		}
	}
	else if(pinid == PWR_LED)
	{
		if(powerled_flag == 0)
		{
			Led_State_ON(pinid);
			powerled_flag = 1;
		}
		else 
		{
			Led_State_OFF(pinid);
			powerled_flag = 0;
		}
	}
//    else if(pinid == RFID_LED)
//    {
//      if(rfUnitStatusLed == 0)
//      { 
//         Led_State_ON(pinid);
//         rfUnitStatusLed = 1;
//      }
//      else if(rfUnitStatusLed == 1)
//      {
//        Led_State_OFF(pinid);
//        rfUnitStatusLed = 0; 
//      }
//    }
}

rfidUniteStates_et GetRfTestTagDetectState(void)
{
   return rfUnitWorkState;
}
uint8_t blinkCount = 0,flagCheckState = 0;
void RfidUnitLedSetState(rfidUniteStates_et rfUnitCurrState)
{
   if(rfUnitWorkState == rfUnitCurrState )
   {
      return;
   }
   rfUnitWorkState = rfUnitCurrState;
   switch(rfUnitWorkState)
   {
        case WAIT_FOR_RF_TEST_TAG:   //by default this handler will  wait for user to show the test tag
            flagCheckState = 0;
//          LOG_DBGS(CH_GSM, "State - WAIT_FOR_RF_TEST_TAG");
         break;
         
       case DISPLAY_STATUS_IDLE_STATE:
         rf_gps_led_starttime = GetStartTime();
//          LOG_DBGS(CH_GSM, "State - DISPLAY_STATUS_IDLE_STATE");
         break;
         
        case VALID_RFID_TEST_TAG_DETECTED:
//          LOG_DBGS(CH_GSM, "State - VALID_RFID_TEST_TAG_DETECTED");
         break;
   
       case GPRS_STATUS_DISPLAY:
         rf_gsmreg_led_flag = 0;
         blinkCount = 0;
//         rf_gprs_led_starttime =  GetStartTime();
         gps_gsm_display_time = GetStartTime();
//         LOG_DBGS(CH_GSM, "State - GPRS_STATUS_DISPLAY");
         break;
       
       case GPS_STATUS_DISPLAY:
         flagCheckState = 0;
         blinkCount = 0;
         rf_gps_led_starttime = GetStartTime();
         gps_gsm_display_time = GetStartTime();
//         LOG_DBGS(CH_GSM, "State - GPS_STATUS_DISPLAY");
        break;
       
   }

}
 
static void RfidUnitGPRSHandler(void)
{
   
//    switch(rfUnitWorkState)
//    {
//        case WAIT_FOR_RF_TEST_TAG:
//          break;
//          
//        case VALID_RFID_TEST_TAG_DETECTED:
//             RfidUnitLedSetState(GPS_STATUS_DISPLAY);
//          break;
//          
//         case DISPLAY_STATUS_IDLE_STATE:
//              Led_State_OFF(RFID_LED);
//              if(TimeSpent(rf_gps_led_starttime,1000))
//              {
//                if(flagCheckState == 0)
//                {
//                   RfidUnitLedSetState(GPRS_STATUS_DISPLAY); 
//                }
//                else
//                {
//                   RfidUnitLedSetState(WAIT_FOR_RF_TEST_TAG);
//                }
//              }
//         break;
//          
//        case GPS_STATUS_DISPLAY:
//              if(GetGpsLEDState() == GPS_VALID)
//              {
//                if(TimeSpent(rf_gps_led_starttime, 300))  //3 blinks of on off
//                 {
//                    if(blinkCount <= 3) 
//                    {
//                       blinkCount++;
//                       Toggle_led(RFID_LED);
//                       rf_gps_led_starttime = GetStartTime();
//                    }
//                    else
//                   {
//                     blinkCount = 0;
//                     Led_State_OFF(RFID_LED);
//                     rf_gps_led_starttime = GetStartTime();
//                     RfidUnitLedSetState(DISPLAY_STATUS_IDLE_STATE); 
//                   }
//                 }
//              }
//              else if(GetGpsLEDState() == GPS_INVALID)
//              { 
//                 Led_State_ON(RFID_LED);
//                 if(TimeSpent(gps_gsm_display_time,MAX_GPS_VALIDITY_DISPLAY_TIME))
//                 {
//                  Led_State_OFF(RFID_LED);
//                  gps_gsm_display_time = GetStartTime();
//                  RfidUnitLedSetState(DISPLAY_STATUS_IDLE_STATE); 
//                 }
//              }
//        break;
//        
//        case GPRS_STATUS_DISPLAY:
//          flagCheckState = 1;
//          if(GetGprsLEDState() == GSM_NOT_REG)
//          {
//             Led_State_ON(RFID_LED);   //continuous on
//             if(TimeSpent(gps_gsm_display_time,MAX_GSM_VALIDITY_DISPLAY_TIME))
//             {
//               Led_State_OFF(RFID_LED);
//               gps_gsm_display_time = GetStartTime();
////               RfidUnitLedSetState(DISPLAY_STATUS_IDLE_STATE); 
//               RfidUnitLedSetState(WAIT_FOR_RF_TEST_TAG);
//             }
//          }
//          else if(GetGprsLEDState() == GSM_REG)
//          {
//            if(TimeSpent(rf_gprs_led_starttime, 300))
//			{
//                if(blinkCount <= 3)   //2 blinks of on/off  is gsm is only registered
//				{
//					blinkCount++;
//					Toggle_led(RFID_LED);
//					rf_gprs_led_starttime = GetStartTime();
//				}
//                else
//                {
//                   blinkCount =0;
//                   Led_State_OFF(RFID_LED);	
//                   rf_gprs_led_starttime = GetStartTime();
////                   RfidUnitLedSetState(DISPLAY_STATUS_IDLE_STATE); 
//                   RfidUnitLedSetState(WAIT_FOR_RF_TEST_TAG);
//                }
//            }
//          }
//          else if((GetGprsLEDState() == GPRS_ACT) || (GetGprsLEDState() == PACKET_SEND_SUCCESS))
//          {
//            if(TimeSpent(rf_gprs_led_starttime, 300))
//			{
//				if(blinkCount <= 5)    //3 blinks of on/off is gprs is activated or packet is sent on the server
//				{
//					blinkCount++;
//					Toggle_led(RFID_LED);
//					rf_gprs_led_starttime = GetStartTime();
//				}
//                else
//                {
//                   blinkCount =0;
//                   Led_State_OFF(RFID_LED);	
//                   rf_gprs_led_starttime = GetStartTime();
////                   RfidUnitLedSetState(DISPLAY_STATUS_IDLE_STATE); 
//                   RfidUnitLedSetState(WAIT_FOR_RF_TEST_TAG);
//                }
//			}
//            
//          }
//          
//          break;
//    }
}

/**
 *  @brief Brief
 *  
 *  @return void
 */
static void GPRS_Led_Handler(void)
{
	static uint8_t count = 0;
	switch(gprs_led_state)
	{
		case GSM_NOT_REG:					//Continuous ON
			Led_State_ON(GPRS_LED);
			break;
			
		case GSM_REG:
			if(gsmreg_led_flag == 0)		//100ms ON
			{
				Led_State_ON(GPRS_LED);
				if(TimeSpent(gprs_led_starttime, GSMREG_LEDON_TOGGLE_INTERVAL))
				{
					gsmreg_led_flag = 1;
					gprs_led_starttime = GetStartTime();
				}
			}
			else
			{
				Led_State_OFF(GPRS_LED);		//1s OFF
				if(TimeSpent(gprs_led_starttime, GSMREG_LEDOFF_TOGGLE_INTERVAL))
				{
					gsmreg_led_flag = 0;
					gprs_led_starttime = GetStartTime();
				}
			}
			break;
	
		case GPRS_ACT:		//1s ON, 1s OFF
			if(TimeSpent(gprs_led_starttime, GPRSREG_LED_TOGGLE_INTERVAL))
			{//APP_DEBUG("Toggle GPRSREGLed_State\r\n");
				Toggle_led(GPRS_LED);
				gprs_led_starttime = GetStartTime();
			}
			break;
			
		case PACKET_SEND_SUCCESS:		//BLINK at 200ms for 3sec
			//APP_DEBUG("ENTER Packet send SUCCESS\r\n");
			packet_led_flag = 1;
			if(TimeSpent(gprs_led_starttime, PACKET_LED_TOGGLE_INTERVAL))
			{//APP_DEBUG("TIME TO TOGGLE PACKET LED\r\n");
				if(((PACKET_LED_TOGGLE_INTERVAL)*count) <= 3000)//*100
				{//APP_DEBUG("Toggle for Packet send Led_State\r\n");
					count++;
					Toggle_led(GPRS_LED);
					Toggle_led(PWR_LED);
					gprs_led_starttime = GetStartTime();
				}
				else
				{
					count = 0;
					packet_led_flag = 0;
					Set_gprsLEDState(GPRS_ACT);
					Led_State_OFF(PWR_LED);
				}
			}
			break;

		default: 
			break;	
	}
}

/**
 *  @brief Brief
 *  
 *  @return void
 */
static void GPS_Led_Handler(void)
{
	switch(gps_led_state)
	{
		case GPS_VALID:			//1s ON, 1s OFF
			if(TimeSpent(gps_led_starttime, GPS_LED_TOGGLE_INTERVAL))
			{
				Toggle_led(GPS_LED);
				gps_led_starttime = GetStartTime();
			}
			break;
			
			
			case GPS_FIX_VALID:			
			if(TimeSpent(gps_led_starttime, 100))
			{
				Toggle_led(GPS_LED);
				gps_led_starttime = GetStartTime();
			}
			break;
			
			case GPS_FLOAT_VALID:			//1s ON, 1s OFF
			if(TimeSpent(gps_led_starttime, 500))
			{
				Toggle_led(GPS_LED);
				gps_led_starttime = GetStartTime();
			}
			break;
					
		case GPS_INVALID:		//Continuous ON
			Led_State_ON(GPS_LED);
			break;

		default: 
			break;	
	}
}



/**
 *  @brief Brief
 *  
 *  @return void
 */
static void Power_Led_Handler(void)
{
//	switch(batt_led_state)
//	{
//		case BATT_MODE:				
//			if(batt_led_flag == 0)		//100ms ON
//			{
//				Led_State_ON(PWR_LED);
//				if(TimeSpent(power_led_starttime, BATMODE_LEDON_INTERVAL))
//				{
//					batt_led_flag = 1;
//					power_led_starttime = GetStartTime();
//				}
//			}
//			else						//1s OFF
//			{	
//				Led_State_OFF(PWR_LED);		
//				if(TimeSpent(power_led_starttime, BATMODE_LEDOFF_INTERVAL))
//				{
//					batt_led_flag = 0;
//					power_led_starttime = GetStartTime();
//				}
//			}
//			break;
//			
//		case CHARGING_MODE:				//2s ON, 2s OFF
//			if(TimeSpent(power_led_starttime, POWER_LED_TOGGLE_INTERVAL))
//			{//APP_DEBUG("Toggle POWEREGLed_State\r\n");
//				Toggle_led(PWR_LED);
//				power_led_starttime = GetStartTime();
//			}
//			break;

//		case CHARGE_COMPLETE:			//Continuous ON
//			Led_State_ON(PWR_LED);
//			break;
//		
//		case BATT_LOW:				
//			if(batt_led_flag == 0)		//100ms ON
//			{
//				Led_State_ON(PWR_LED);
//				if(TimeSpent(power_led_starttime, BATLOW_LEDON_INTERVAL))
//				{
//					batt_led_flag = 1;
//					power_led_starttime = GetStartTime();
//				}
//			}
//			else						//100msec OFF
//			{
//				Led_State_OFF(PWR_LED);
//				if(TimeSpent(power_led_starttime, BATLOW_LEDOFF_INTERVAL))
//				{
//					batt_led_flag = 0;
//					power_led_starttime = GetStartTime();
//				}
//			}
//			break;
//			
//		default: 
//			break;	
//	}
}


 /*----public functions-----*/
 
/**
 *  @brief Initialize Led configurations to init the gpio pins
 *  
 *  @return void
 */
void Init_LED_Config(void)
{
	Init_LED_port();
}

/**
 *  @brief This function is used to set the current value of gprs_state. If the value of
 *  packet_led_flag = 1, then reset the gprs led starttime or else return & set gprs_state 
 *  with current state value & reset the start time
 *  @param [in] gprs_state is a 1 value of @ref gprsledState_t enum
 *  @return void
 */
void Set_gprsLEDState(gprsLedState_et gprs_state)
{
	if(packet_led_flag == 1)
	{
		gprs_led_starttime = GetStartTime();
	}
	else
	{
		if(gprs_state==gprs_led_state)
		{
			return;
		}
		gprs_led_state = gprs_state;
		//APP_DEBUG("Set GPRSLed_State = %d\r\n",gprs_led_state);
		gprs_led_starttime = GetStartTime();
	}
}

gprsLedState_et GetGprsLEDState(void)
{
   return gprs_led_state;
}

/**
 *  @brief This function is used to set the current value of gps_state. If gps_state==gps_led_state
 *  return, otherwise set the new value of gps_state & reset the gps_led_starttime
 *  @param [in] gps_state is a 1 value of @ref gpsledState_t
 *  @return void
 */
void Set_gpsLEDState(gpsLedState_et gps_state)
{
	if(gps_state==gps_led_state)
	{
		return;
	}
	gps_led_state = gps_state;
	gps_led_starttime = GetStartTime();
}

gpsLedState_et GetGpsLEDState(void)
{
  return gps_led_state;
}

/**
 *  @brief This function is used to set the current value of batt_state. If batt_state==batt_led_state
 *  return, otherwise set the new value of batt_state & reset the power_led_starttime
 *  @param [in] batt_state is a 1 value of @ref battledState_t
 *  @return void
 */
void Set_batteryLEDState(battLedState_et batt_state)
{
	if(batt_state==batt_led_state)
	{
		return;
	}
	batt_led_state = batt_state;
	power_led_starttime = GetStartTime();
}

/**
 *  @brief Brief
 *  
 *  @return void
 */
void Led_State_Handler(void)
{
	GPRS_Led_Handler();
	GPS_Led_Handler();
	Power_Led_Handler();
 // RfidUnitGPRSHandler();
}