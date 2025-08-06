#include "gsm_reset_power.h"

static uint32_t gsmPowerResetTick = 0;
static powerResetState_et powerResetState = GSM_POWER_RESET_INIT;

    
/**
 *  @brief  : set the gsm power state 
 *  @param  :[in] power 
 *  @return :none
 */

void GsmResetPowerSetState(powerResetState_et power)
{
    powerResetState = power;
    gsmPowerResetTick = GetStartTime();
    
    switch(powerResetState)
    {
			  case GSM_POWER_RESET_INIT:
					break;
				
        case GSM_POWER_OFF_WAIT:
            TurnOffGsmPowerRegulatorSwitch();
            LOG_DBGS(CH_GSM,"POWER OFF");
        break;
        
        case GSM_POWER_ON_WAIT:
            TurnOnGsmPowerRegulatorSwitch();
            LOG_DBGS(CH_GSM,"POWER ON WAIT");
        break;
        
        case GSM_POWER_RESET_DONE:
            LOG_DBGS(CH_GSM,"POWER RESET DONE");
        break;
            
        case GSM_POWERKEY_ON_WAIT:
            GsmPowerkeyPinOn();
            LOG_DBGS(CH_GSM,"POWERKEY ON WAIT");
            break;
    
        case GSM_POWERKEY_OFF_WAIT:
            GsmPowerkeyPinOff();
            LOG_DBGS(CH_GSM,"POWERKEY OFF WAIT");
          break;
            
        case GSM_POWERKEY_DONE:    
            LOG_DBGS(CH_GSM,"GSM_POWERKEY_DONE");
          break;    
    }
}

powerResetState_et GsmResetPowerHandler(void)
{
    switch(powerResetState)
    {
			  case GSM_POWER_RESET_INIT:
					break;
				
        case GSM_POWER_OFF_WAIT:
            if(TimeSpent(gsmPowerResetTick,GSM_POWER_OFF_WAIT_MS))
            {
                GsmResetPowerSetState(GSM_POWER_ON_WAIT);
            }
        break;
        
        case GSM_POWER_ON_WAIT:
            if(TimeSpent(gsmPowerResetTick,GSM_POWER_ON_WAIT_MS))
            {
                GsmResetPowerSetState(GSM_POWER_RESET_DONE);
            }
        break;
        
        case GSM_POWER_RESET_DONE:
            if(TimeSpent(gsmPowerResetTick,WAIT_FOR_GSM_PWRKEY))
            {
                GsmResetPowerSetState(GSM_POWERKEY_ON_WAIT);
            }
        break;

        case GSM_POWERKEY_ON_WAIT:
            if(TimeSpent(gsmPowerResetTick,GSM_POWERKEY_ON_WAIT_MS))
            {
                GsmResetPowerSetState(GSM_POWERKEY_OFF_WAIT);
            }
        break;
    
        case GSM_POWERKEY_OFF_WAIT:
            if(TimeSpent(gsmPowerResetTick,GSM_POWERKEY_OFF_WAIT_MS))
            {
                GsmResetPowerSetState(GSM_POWERKEY_DONE);
            }
        break;
            
        case GSM_POWERKEY_DONE:    
        break;
    }
    
    return powerResetState;
}