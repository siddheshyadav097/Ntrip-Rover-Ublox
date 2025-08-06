/**
 *  @file          :  rtc_port.c
 *  @author        :  Aakash/Ratna
 *  @date          :  26/05/2017
 *  @brief         :  Provides interface to set or read the RTC time which is used to maintain the current date-time.
 *  @filerevision  :  1.0
 *  
 */

 
 /*----includes-----*/
#include "rtc_port.h"
#include "debug_log.h"
//#include "stm32f3xx_hal_pwr.h"
//#include "mbcrc.h"


//#define RTC_STATUS_INIT_OK              0x1234       /* RTC initialised */
//#define RTC_STATUS_TIME_OK              0xA5A5       /* RTC time OK */
//#define	RTC_STATUS_ZERO                 0x0000
   
 /*----variables-----*/
//int32_t ret_val;
//TimePara_t rtctime_struct;
//TimePara_t time11_config;
RTC_HandleTypeDef hrtc;

//RTC_TimeTypeDef sTime;
//RTC_DateTypeDef DateToUpdate;

/* Private variable definitions  ---------------------------------------------*/ 
//static  uint8_t daySavedInBackupRam   = 0;
//static  uint8_t monthSavedInBackupRam = 0;
//static  uint8_t yearSavedInBackupRam  = 0;

//RTC_DateTypeDef sdatestructureget;
//RTC_TimeTypeDef stimestructureget;

//RTC_DateTypeDef updatedsdatestructure;
//RTC_TimeTypeDef updatedstimestructure;        //Modified on 12/1/17 

/* Private function definitions  ---------------------------------------------*/ 
// static void RTC_ReadDateFromBackupRAM(RTC_DateTypeDef * sdate);
// static void RTC_SaveDateInBackupRAM(RTC_DateTypeDef * sdate);
// static HAL_StatusTypeDef RTC_WriteDate(RTC_HandleTypeDef *hrtc, RTC_DateTypeDef *sDate);

/*----public functions-----*/
/**
 *  @brief This function queries to get the current local date & time. Here we are checking condition 
 *  for the current year compared with default year which is set as 2017 in rtc_port.h file. If current year
 *  is less than default year we have to clear localtime_struct structure, else the queried current time data
 *  is copied in rtctime_ptr pointer
 *  @param [in] rtctime_ptr Parameter_Description
 *  @return void
 */
BOOL Get_RTC_time(TimePara_t *rtctime_ptr)
{
    RTC_DateTypeDef sdatestructureget;
    volatile  RTC_TimeTypeDef stimestructureget;
  
  /* Get the RTC current Date */
  
    /* Get the RTC current Time */
    if(HAL_RTC_GetTime(&hrtc, &stimestructureget, RTC_FORMAT_BIN) == HAL_OK)
    {
      if(HAL_RTC_GetDate(&hrtc, &sdatestructureget, RTC_FORMAT_BIN) == HAL_OK)  
      {
      if(sdatestructureget.Year >= DEFAULT_YEAR)
      {
          rtctime_ptr->day = sdatestructureget.Date;
          rtctime_ptr->month = sdatestructureget.Month;
          rtctime_ptr->year = sdatestructureget.Year;
          rtctime_ptr->hour = stimestructureget.Hours;
          rtctime_ptr->minute = stimestructureget.Minutes;
          rtctime_ptr->second = stimestructureget.Seconds;
//        LOG_DBG(CH_GSM,"TIME: %02d:%02d:%02d",stimestructureget.Hours,stimestructureget.Minutes,stimestructureget.Seconds);
          return True;
      }
      else
      {
        return False;
      }
    }
    else
    {
//       LOG_DBG(CH_GSM,"READ RTC FAILED1");
      return False;
    }
  }
  else
  {
//    LOG_DBG(CH_GSM,"READ RTC FAILED2");
    return False;
  }
}

/**
 *  @brief This function queries to get the current local date & time. Here we are checking condition 
 *  for the current year compared with default year which is set as 2017 in rtc_port.h file. If current year
 *  is less than default year we have to clear localtime_struct structure, else the queried current time data
 *  is copied in rtctime_ptr pointer
 *  @param [in] rtctime_ptr Parameter_Description
 *  @return void
 */
//void Get_RTC_time_OnInit(TimePara_t *rtctime_ptr)
//{
//    RTC_ReadDateFromBackupRAM(&sdatestructureget);   //sdatestructure
    
//    /* Get the RTC current Time */
//    if(HAL_RTC_GetTime(&hrtc, &stimestructureget, RTC_FORMAT_BIN) == HAL_OK)   //RTC_FORMAT_BIN
//    {
//      if(sdatestructureget.Year >= DEFAULT_YEAR)   //sdatestructure
//	{
//            rtctime_ptr->day = sdatestructureget.Date;   ////sdatestructure
//            rtctime_ptr->month = sdatestructureget.Month;  ////sdatestructure
//            rtctime_ptr->year = sdatestructureget.Year;//2004;//2017;  ////sdatestructure
//            rtctime_ptr->hour = stimestructureget.Hours;
//            rtctime_ptr->minute = stimestructureget.Minutes;
//            rtctime_ptr->second = stimestructureget.Seconds;
//            
//             //If date is valid the Write the Backum RAM date to the RTC (without changing the time)
//             RTC_WriteDate(&hrtc,&sdatestructureget);    //sdatestructure
//            
////            LOG_INFO(CH_MAN,"<--Get_RTC_time = %d-%d-%d:%d-%d-%d-->\r\n", sdatestructureget.Date, sdatestructureget.Month, 
////            sdatestructureget.Year, stimestructureget.Hours, stimestructureget.Minutes, stimestructureget.Seconds);
//        }
//    }
//}

/**
 *  @brief Copies data from location of rtctime_ptr to rtctime_struct structure
 *  @param [in] rtctime_ptr points to the starting of rtc time
 *  @return void
 */
BOOL Set_RTC_time(TimePara_t *rtctime_ptr)
{
      RTC_DateTypeDef sdatestructure;
      RTC_TimeTypeDef stimestructure;
     
      sdatestructure.Year = rtctime_ptr->year;
      sdatestructure.Month = rtctime_ptr->month;
      sdatestructure.Date = rtctime_ptr->day ;
      sdatestructure.WeekDay = RTC_WEEKDAY_MONDAY;
      
      if(HAL_RTC_SetDate(&hrtc,&sdatestructure,RTC_FORMAT_BIN) == 0)
      {
        stimestructure.Hours = rtctime_ptr->hour;
        stimestructure.Minutes = rtctime_ptr->minute;
        stimestructure.Seconds = rtctime_ptr->second;
        stimestructure.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
        stimestructure.StoreOperation = RTC_STOREOPERATION_RESET;
        if(HAL_RTC_SetTime(&hrtc,&stimestructure,RTC_FORMAT_BIN) == 0)
        {
            return True;
        }
          return False;
    }
    return False;
	//ret_val = Ql_SetLocalTime((ST_Time*)&rtctime_struct);
}
        
void InitRTC(void)
{
 /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  hrtc.Init.OutPutPullUp = RTC_OUTPUT_PULLUP_NONE;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    //Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */
    sDate.WeekDay = RTC_WEEKDAY_MONDAY;
		sDate.Month = RTC_MONTH_JANUARY;
		sDate.Date = 0x1;
		sDate.Year = 0x0;

		if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
		{
			//Error_Handler();
		}
  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0x0;
  sTime.Minutes = 0x0;
  sTime.Seconds = 0x0;
  sTime.SubSeconds = 0x0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    //Error_Handler();
  }
 
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */
	

}
/**
 *  @brief Read the date, and its checksum from the backup RAM. If the checksum is not correct, the date
 *  is reset to 1st Jan, 2001 and saved in the backup RAM. 
 *  
 *  @param sdate Pointer to the RTC_DateTypeDef structure in which the date must be saved
 */
//static void RTC_ReadDateFromBackupRAM(RTC_DateTypeDef * sdate)
//{
//    RTC_HandleTypeDef hrtc;
//    uint16_t savedChecksum; 
//    uint16_t calculatedChecksum; 
//    
//    //Read the date and its' checksum from the Backup RAM
//    //Note that the hrtc variable is not actually used in the HAL_RTCEx_BKUPRead function
//    sdate->Month    = (uint8_t)HAL_RTCEx_BKUPRead(&hrtc, RTC_MONTH_ADDR);
//    sdate->Date     = (uint8_t)HAL_RTCEx_BKUPRead(&hrtc, RTC_DAY_ADDR);
//    sdate->Year     = (uint8_t)HAL_RTCEx_BKUPRead(&hrtc, RTC_YEAR_ADDR);  
//    savedChecksum   =  HAL_RTCEx_BKUPRead(&hrtc, RTC_DATE_CHECKSUM_ADDR);                        
//    daySavedInBackupRam = sdate->Date; 
//    monthSavedInBackupRam = sdate->Month;
//    yearSavedInBackupRam  = sdate->Year;    
//    
//
//    //If the checksum that was saved in the Backup RAM is not valid, reset the date
//    //This should only happen the very first time the program is run (i.e. nothing has been written to the backup RAM)
//    calculatedChecksum = usMBCRC16((uint8_t *)&(sdate->Month),sizeof(RTC_DateTypeDef)-1); // '-1' because weekday is not included    
//
//    /*if it is S/W reset then read the date from backup RAM and update it in RTC*/
//    
//    //If date is valid then Write the Backum RAM date to the RTC (without changing the time)
//     RTC_WriteDate(&hrtc,sdate);    //sdatestructure , get the date from rtc
//             
//    if (calculatedChecksum != savedChecksum)
//    {
//        //Reset to Jan 1st, 2001
//        sdate->Month = 1;       
//        sdate->Date  = 1;      
//        sdate->Year  = 0;     
//        RTC_SaveDateInBackupRAM(sdate); 
//        
//        
//        /*IF backUp RAM is corrupted then write the default values in the Backup as well as in the RTC*/
//        RTC_WriteDate(&hrtc,sdate);    //sdatestructure , get the date from rtc
//    }    
//}

/**
 *  @brief Save the specified date, and its checksum in the backup RAM
 *  
 *  @param sdate Pointer to the RTC_DateTypeDef structure which contains the date to be saved
 */
//static void RTC_SaveDateInBackupRAM(RTC_DateTypeDef * sdate)
//{
//    RTC_HandleTypeDef hrtc;
//    uint16_t checksum; 
//    
//    //Note: hrtc variable is not used in HAL_RTCEx_BKUPWrite function
//    HAL_RTCEx_BKUPWrite(&hrtc, RTC_MONTH_ADDR, sdate->Month);   
//    HAL_RTCEx_BKUPWrite(&hrtc, RTC_DAY_ADDR, sdate->Date);   
//    HAL_RTCEx_BKUPWrite(&hrtc, RTC_YEAR_ADDR, sdate->Year);   
//    
//    checksum = usMBCRC16((uint8_t *)&(sdate->Month),sizeof(RTC_DateTypeDef)-1);  // '-1' because weekday is not included    
//    HAL_RTCEx_BKUPWrite(&hrtc, RTC_DATE_CHECKSUM_ADDR, checksum); 
//
//    daySavedInBackupRam   = sdate->Date;
//    monthSavedInBackupRam = sdate->Month;
//    yearSavedInBackupRam  = sdate->Year;
    
//}

/**
 *  @brief Same as the HAL_RTC_SetDate function, but does not 
 *         - update the counter, alarm or weekday 
 *         - accept BCD format
 *  
 * @param   hrtc    Pointer to a RTC_HandleTypeDef structure that contains the configuration information for RTC.
 * @param   sDate   Pointer to date structure
 */
//static HAL_StatusTypeDef RTC_WriteDate(RTC_HandleTypeDef *hrtc, RTC_DateTypeDef *sDate)
//{
//    //Check input parameters
//    if((hrtc == NULL) || (sDate == NULL)){
//     return HAL_ERROR;
//    }
//
//    //Check the parameters
//    assert_param(IS_RTC_FORMAT(Format));
//
//    //Process Locked 
//    __HAL_LOCK(hrtc);
//    hrtc->State = HAL_RTC_STATE_BUSY; 
//
//    assert_param(IS_RTC_YEAR(sDate->Year));
//    assert_param(IS_RTC_MONTH(sDate->Month));
//    assert_param(IS_RTC_DATE(sDate->Date)); 
//
//    //Change the current date */
//    hrtc->DateToUpdate.Year  = sDate->Year;
//    hrtc->DateToUpdate.Month = sDate->Month;
//    hrtc->DateToUpdate.Date  = sDate->Date;
//    
//    //Process Unlocked
//    hrtc->State = HAL_RTC_STATE_READY;
//    __HAL_UNLOCK(hrtc);
//
//    return HAL_OK;      
//}


/**
 *  @brief If the RTC date has changed, the value is saved in the Backup RAM.
 *  This function has to be called reularly after initialization. 
 */
//RTC_TimeTypeDef stime;
//void RTC_UpdateDate(void)
//{
//    RTC_DateTypeDef sdate;
//    RTC_TimeTypeDef stime;
//
//    //Get the RTC current Time, and update the date of 24 hours have passed
//    HAL_RTC_GetTime(&hrtc, &stime, RTC_FORMAT_BIN);
//    
//    //Get the RTC current Date
//    HAL_RTC_GetDate(&hrtc, &sdate, RTC_FORMAT_BIN);
//
//    //Update the Backup RAM if the date has changed
////    if (daySavedInBackupRam != sdate.Date || monthSavedInBackupRam != sdate.Month || yearSavedInBackupRam != sdate.Year){       
////        RTC_SaveDateInBackupRAM(&sdate); 
////    }
//}
