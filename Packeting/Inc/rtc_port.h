/**
 *  \file rtc_port.h
 *  \brief Includes public function declarations setting acquiring the RTC time along with type definition of time parameters.
 */

#ifndef __RTC_PORT_H__
#define __RTC_PORT_H__


 /*----includes-----*/
#include "stm32g0xx_hal.h"

#include "lib_port.h"
//#include "mcu_port.h"
extern RTC_HandleTypeDef hrtc;


 /*----constants-----*/
#define DEFAULT_YEAR				18


/* Defines  ------------------------------------------------------------------*/ 
//Backup RAM storage locations. Ensure that the addresses of related parameters are consequtive since their 
//checksum is calculated together. Only the addresses of RTC_BKP_DR10 and RTC_BKP_DR11 are not consqeutive 
//#define RTC_YEAR_ADDR           RTC_BKP_DR1 //RTC_YEAR_ADDR, RTC_MONTH_ADDR and RTC_DAY_ADDR should be consequtive 
//#define RTC_MONTH_ADDR          RTC_BKP_DR2 
//#define RTC_DAY_ADDR            RTC_BKP_DR3
//#define RTC_DATE_CHECKSUM_ADDR  RTC_BKP_DR12 //Did not use RTC_BKP_DR4 because it keeps getting reset for unknown reasons...
//#define RTC_STATUS_REG      	RTC_BKP_DR5 /* Status Register */


 /*----typedefs-----*/
typedef struct 
{
    int32_t year;    
    int32_t month;
    int32_t day;
    int32_t hour;
    int32_t minute;
    int32_t second;
    int32_t timezone;  
}TimePara_t;


 /*----public function declarations-----*/
BOOL Get_RTC_time(TimePara_t *rtctime_ptr);
BOOL Set_RTC_time(TimePara_t *rtctime_ptr);
void InitRTC(void);
//void RTC_UpdateDate(void);
#endif