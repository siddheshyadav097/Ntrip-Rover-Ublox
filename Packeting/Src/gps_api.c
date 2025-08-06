/**
 *  @file          :  gps_api.c
 *  @author        :  Aakash/Ratna
 *  @date          :  02/04/2017
 *  @brief         :  APIs to process and decode the incoming GPS serial data are defined.
 *  				  The decoded data is collected together as GpsData_t and GpsTimePara_t which is provided to the application. 
 *  @filerevision  :  1.0
 *  
 */

 
 /*----includes-----*/
#include "gps_api.h"
#include "lib_api.h"
#include "debug_log.h"
#include <math.h>


 /*----variables-----*/
uint8_t gpggapacketBuffer[254];
static uint8_t strTmpBuf[150];
uint8_t commaIdx;
uint8_t gpspara_Idx[20]= {0};//[10]= {0};
uint8_t flagNmeaLineReceived = 0;
uint8_t gpsNmeaLineStart = 0;

// circular buff that holds the serial data received from GSM
ringBuffer_st gpsNmeaRespRing;

// received data is store in this buffer
uint8_t gpsSerialRingBuf[MAX_GPS_RX_BUFFER_SIZE]= {0};

// the received serial data from gps is sepearated line by line and is stored in gsmSerialLineBuf
uint8_t gpsNmeaLineBuf[MAX_GPS_RX_BUFFER_SIZE]= {0};

// the number of bytes stored in the Nmea line buff
uint16_t gpsNmeaLineIndex = 0;

// the length of the received line from GPS
uint16_t gsmNmeaLineLength = 0;

// this flag is set when 0x0d is received
uint8_t flagGpsNmeaCarriageRetRcvd = 0;

//gpsmNmeaState_et gpsNmeaState = WAIT_FOR_NMEA_LINE;

GpsTimePara_t utc_timestruct = {0};
GpsData_t gps_coordinates = {0};

static void DecodeRMCdata(void);
static void DecodeGGAdata(void);
static void DecodeGSAdata(void);

uint32_t getGpsDataTick = 0;

uint16_t totalGpsDistanceCountInMem =0;

struct
{
    double PrevLatitude;  /*!< Latitude of starting point. */
    double PrevLongitude; /*!< Longitude of starting point. */
    float Distance;   /*!< Distance between 2 points which will be calculated. */
} GPS_Distance;

double latdeg = 0.0;
double latmin = 0.0;
double londeg = 0.0;
double lonmin = 0.0;
double distance = 0;
float totalGpsDistanceInKm = 0;

/*----public functions-----*/
/**
 *  @brief This function calls the GPS_Init function & clears utc_timestruct & gps_coordinates structures
 *  @return void
 */
void InitGPSConfig(void)
{
    InitGPS();
    memset(&utc_timestruct, 0, sizeof(GpsTimePara_t));
    getGpsDataTick = 0;
}
void GpsRingBuffInit(void)
{
  RingBufferInit(&gpsNmeaRespRing,gpsSerialRingBuf,sizeof(gpsSerialRingBuf));
//  gpsNmeaState = WAIT_FOR_NMEA_LINE;
}

void GpsUartReceiveDataCb(uint8_t receiveGpsData)
{
    RingBufferFill(&gpsNmeaRespRing,receiveGpsData);
}

void GpsNmeaResponseHandler(void)
{
   uint8_t rcvByte;
   while((RingBufferDrain(&gpsNmeaRespRing,&rcvByte) == 1))
   {
      if(gpsNmeaLineStart == 0)
      {
          if(rcvByte == '$')
          {
            gpsNmeaLineBuf[gpsNmeaLineIndex] = rcvByte;
            gpsNmeaLineIndex++;
            gpsNmeaLineStart = 1;
          }
      }
      else
      {
           if (rcvByte == 0x0D)
            {  
                if(gpsNmeaLineIndex != 0)
                {
                    gpsNmeaLineBuf[gpsNmeaLineIndex] = rcvByte;
                    gpsNmeaLineIndex++;
                    flagGpsNmeaCarriageRetRcvd = 1;
                }
            }
            else if(rcvByte == 0x0A)
            {
                if(flagGpsNmeaCarriageRetRcvd)
                {
                    gpsNmeaLineBuf[gpsNmeaLineIndex] = rcvByte;
                    gpsNmeaLineIndex++;
                    gpsNmeaLineBuf[gpsNmeaLineIndex] = '\0';
                    gsmNmeaLineLength = gpsNmeaLineIndex;
                    flagNmeaLineReceived = 1;
                    gpsNmeaLineIndex = 0;
                    flagGpsNmeaCarriageRetRcvd = 0;
//                    gpsNmeaState = PROCESS_NMEA_LINE;
                    break;
                }
            }
            else
            {
                gpsNmeaLineBuf[gpsNmeaLineIndex] = rcvByte;
                gpsNmeaLineIndex++;
                if (gpsNmeaLineIndex >= MAX_GPS_RX_BUFFER_SIZE)
                {
                    // should not come here this can happen only if the line buffer size is smaller
                    // than the line received
                    gpsNmeaLineIndex = gpsNmeaLineIndex - 1;
                }
            }
      }
   
   }
   if(flagNmeaLineReceived)
   {
//   LOG_DBG(CH_GPS,"%s",gpsNmeaLineBuf);
     GetGpsData(gpsNmeaLineBuf,gsmNmeaLineLength) ;
		 
		//	GetGpsData("$GNRMC,151001.000,A,1906.845779,N,07252.831986,E,0.27,127.71,300822,,,A,V*03\r\n",78);
     gpsNmeaLineStart = 0;
     flagNmeaLineReceived = 0;
   }
   //If no data is received from the gps for 5 seconds then clear the gps struct
   if(TimeSpent(getGpsDataTick,MAX_GPS_NOT_RCVD_TIMEOUT))
   {
       getGpsDataTick = GetStartTime();
       ClearGpsPacketData();  
   }
}

/**
 *  @brief This function will collect all the data from GPRMC gps string into its respective variables defined 
 *  in @ref GpsData_t structure. As the GPS string seprates 2 parameters with ',' we will also copy the particular
 *  parameter into its respective variables using ',' seprator. Once this data is copied in a global buffer called
 *  gpspara_Idx, we have to decode that parameter in DecodeRMCdata function. After decoding all the parameters
 *  copy that data from gps_coordinates structure to the GpsData_t type of pointer i.e. gps_data_ptr 
 *  @param [in] gps_data_ptr Points to @ref GpsData_t type of structure
 *  @return void
 */

void GetGpsData(uint8_t* gpslineBuff , uint16_t gpsLineLen)//(GpsData_t *gps_data_ptr)
{
	uint8_t *p=NULL;
	uint8_t *q=NULL;
	uint8_t *r=NULL;
	uint8_t val_checksum;
    uint8_t latlogBuff[20]= {0};
		
	//	LOG_DBG(CH_GSM,"%s",gpslineBuff);
    	
	if(((p = (uint8_t*)strstr((const char*)gpslineBuff,"$GPRMC")) != NULL) || ((p =(uint8_t*)strstr((const char*)gpslineBuff,"$GNRMC")) != NULL))
	{
		q = (uint8_t*)strstr((const char*)p,"*");
		if(q)
		{
			r = (uint8_t*)strstr((const char*)q,"\r\n");
            if(r != NULL)
            {
                memset(strTmpBuf, 0, sizeof(strTmpBuf));
                strncpy((char*)strTmpBuf, (const char*)q+1, r-q-1);
                val_checksum = m_atoh(strTmpBuf);
                memset(strTmpBuf, 0, sizeof(strTmpBuf));
                strncpy((char*)strTmpBuf, (const char*)p+1, q-p-1);//To calculate checksum parse all characters between $ and * from NMEA sentence
               

             
							//GetGpsData("$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A\r\n",81);
                if(Verify_Checksum(strTmpBuf, strlen((const char*)strTmpBuf), val_checksum))
                {
                    //GPRMC,220516,A,5133.82,N,00042.24,W,173.8,231.8,130694,004.2,W
                    //$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A
                   

                    getGpsDataTick = GetStartTime(); //update the gps data received tick
                    
                    p = strTmpBuf;
                    for(commaIdx=0; commaIdx < 11; commaIdx++)
                    {
                        q = (uint8_t*)strstr((const char*)p, ",");
                        if(q == NULL)
                        break;
                        memset(gpspara_Idx,0,sizeof(gpspara_Idx));
                        strncpy((char*)gpspara_Idx, (const char*)p, q - p);
                        p = q +1;
                        if(commaIdx == 3 || commaIdx == 5)
                        {
                          memset(latlogBuff,0,sizeof(latlogBuff));
                          strcpy((char*)latlogBuff, (const char*)gpspara_Idx);
                        }
    //                  else if(commaIdx == 4 || commaIdx == 6)
    //                  {
    //                   memset(latlogDir,0,sizeof(latlogDir));
    //                   strcpy((char*)latlogDir, (const char*)gpspara_Idx);
    //                  }
                        else if(commaIdx == 4)
                        {
                          gps_coordinates.latDir = gpspara_Idx[0];
                          if(gps_coordinates.latDir == 0)
                          {
                             gps_coordinates.latDir = '0';
                          }
                          gps_coordinates.latitude = ConvertNmeaToDecimal(latlogBuff, 1, gps_coordinates.latDir);
                        }
                        else if(commaIdx == 6)  
                        {
                          gps_coordinates.longDir = gpspara_Idx[0];
                          if(gps_coordinates.longDir == 0)
                          {
                             gps_coordinates.longDir = '0';
                          }
                          gps_coordinates.longitude = ConvertNmeaToDecimal(latlogBuff, 2,gps_coordinates.longDir);
                        }
                        else if (commaIdx <= 2 || commaIdx >= 7)
                        {
                          DecodeRMCdata();
                        }
												//LOG_DBG(CH_GSM,"GPS LAT LONG - %0.2f, %0.2f",gps_coordinates.latitude,gps_coordinates.longitude);
                    }
    //				memcpy(gps_data_ptr, &gps_coordinates, sizeof(GpsData_t));
                }
                else   //if checksum is not verified then clear the gps struct
                {
                   ClearGpsPacketData();
                }
          }
		}
	}
    if(((p = (uint8_t*)strstr((const char*)gpslineBuff,"$GPGGA")) != NULL) || ((p =(uint8_t*)strstr((const char*)gpslineBuff,"$GNGGA")) != NULL))
	{
		    memset(gpggapacketBuffer,0,sizeof(gpggapacketBuffer));
	      strcpy((char *)gpggapacketBuffer,(char *)gpslineBuff);
		
		
//		    if(strstr((const char*)gpslineBuff, "$GNGGA") != NULL) 
//			  {
//           strcpy((char *)gpggapacketBuffer, (const char *)gpslineBuff);
//           memcpy(gpggapacketBuffer + 1, "GPGGA", 5);  // Replace GNGGA ? GPGGA at position 1

//				}
//				else 
//				{
//           strcpy((char *)gpggapacketBuffer, (const char *)gpslineBuff);
//        }
		
		
		
		
        q = (uint8_t*)strstr((const char*)p,"*");
		if(q)
		{
			r = (uint8_t*)strstr((const char*)q,"\r\n");
            if(r != NULL)
            {
                memset(strTmpBuf, 0, sizeof(strTmpBuf));
                strncpy((char*)strTmpBuf, (const char*)q+1, r-q-1);
                val_checksum = m_atoh(strTmpBuf);
                memset(strTmpBuf, 0, sizeof(strTmpBuf));
                strncpy((char*)strTmpBuf, (const char*)p+1, q-p-1);
                if(Verify_Checksum(strTmpBuf, strlen((const char*)strTmpBuf), val_checksum))
                {
                  //GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,
                     getGpsDataTick = GetStartTime(); //update the gps data received tick
                     
                     p = strTmpBuf;
                    for(commaIdx=0; commaIdx < 11; commaIdx++)
                    {
                        q = (uint8_t*)strstr((const char*)p, ",");
                        if(q == NULL)
                        break;
                        memset(gpspara_Idx,0,sizeof(gpspara_Idx));
                        strncpy((char*)gpspara_Idx, (const char*)p, q - p);
//                      LOG_INFO(CH_GPS,"<--gpspara_Idx = %s-->\r\n", gpspara_Idx);
                        p = q +1;
                        if(commaIdx >= 6)
                        {
                          DecodeGGAdata();
                        }
                    }
    //				memcpy(gps_data_ptr, &gps_coordinates, sizeof(GpsData_t));
			     }//If checksum is invalid then all the parameters are cleared
                else
                {
                   gps_coordinates.gpsFix   = 0;
                   gps_coordinates.noOfSat  = 0;
                   gps_coordinates.altitude = 0.0;
                }
	 	     }
        }
	}
    if(((p = (uint8_t*)strstr((const char*)gpslineBuff,"$GPGSA")) != NULL) || ((p =(uint8_t*)strstr((const char*)gpslineBuff,"$GLGSA")) != NULL))
	{
	  q = (uint8_t*)strstr((const char*)p,"*");
		if(q)
		{
			r = (uint8_t*)strstr((const char*)q,"\r\n");
            if(r != NULL)
            {
			memset(strTmpBuf, 0, sizeof(strTmpBuf));
			strncpy((char*)strTmpBuf, (const char*)q+1, r-q-1);
			val_checksum = m_atoh(strTmpBuf);
			memset(strTmpBuf, 0, sizeof(strTmpBuf));
			strncpy((char*)strTmpBuf, (const char*)p+1, q-p-1);
			if(Verify_Checksum(strTmpBuf, strlen((const char*)strTmpBuf), val_checksum))
			{
				//GPGSA,A,3,04,05,,09,12,,,24,,,,,2.5,1.3,2.1
				//GPGSA,A,3,13,20,11,29,01,25,07,04,,,,,1.63,0.94,1.33
                getGpsDataTick = GetStartTime(); //update the gps data received tick
                
				p = strTmpBuf;
				for(commaIdx=0; commaIdx < 17; commaIdx++)
				{
					q = (uint8_t*)strstr((const char*)p, ",");
					if(q == NULL)
				    break;
					memset(gpspara_Idx,0,sizeof(gpspara_Idx));
					strncpy((char*)gpspara_Idx, (const char*)p, q - p);
//					LOG_INFO(CH_GPS,"<--gpspara_Idx = %s-->\r\n", gpspara_Idx);
					p = q +1;
					if(commaIdx >= 15)
					{
					  DecodeGSAdata();
					}
				}
//				memcpy(gps_data_ptr, &gps_coordinates, sizeof(GpsData_t));
//				memset(cpy_gpsdata_buff,0,sizeof(cpy_gpsdata_buff));
//				return True;
			}
            else{//If checksum is invalid then all the parameters are cleared
              gps_coordinates.pDop = 0.0; 
			  gps_coordinates.hDop = 0.0; 
            }
	 	  } 
        }
	}  
//    if((p = (uint8_t*)strstr((const char*)gpslineBuff,"$PMTK")) != NULL)
//	{
//       LOG_DBG(CH_GPS,"GPS RESTARTED,%s",gpslineBuff);
//    }
//	memset(cpy_gpsdata_buff,0,sizeof(cpy_gpsdata_buff));
//	return False;
}

GpsData_t* GetGpsDataForPacketing(void)
{
//   if(Is_Gps_Valid())
//   {
     return(&gps_coordinates);
//   }
}

GpsData_t* ClearGpsPacketData(void)
{
  memset(&utc_timestruct, 0, sizeof(GpsTimePara_t));
  memset(&gps_coordinates, 0, sizeof(GpsData_t));
  gps_coordinates.gpsFix = 0;
  gps_coordinates.latDir = '0';
  gps_coordinates.longDir = '0';
  gps_coordinates.validity = 'V';
  return(&gps_coordinates);
}
/*----private functions-----*/
double ConvertNmeaToDecimal(uint8_t *nmea, uint8_t type, uint8_t dir)
{
	  uint8_t idx, dot = 0;
	  uint8_t cdd[5], cmm[10]= {0};
	  uint16_t degree;
	  double dec = 0, minute;
	  
	  for (idx = 0; idx < strlen((char const*)nmea); idx++)
	  {
		  if (nmea[idx] == '.')
		  {
			  dot = idx;
			  break;
		  }
	  }

	  if (dot < 3)
		  return 0;

	  memset(&cdd, 0, 5);
	  memset(&cmm, 0, 10);
	  
	  strncpy((char*)cdd, (char const*)nmea, dot-2);
	  strcpy((char*)cmm, (char const*)nmea+dot-2);
	  
	  degree = atoi((char const*)cdd);    
	  minute = atof((char const*)cmm);

	  dec = degree + (minute/60.00);
    LOG_DBG(CH_GPS,"degree = %d,minute = %f, decimal = %f", degree,minute,dec);
	  if (type == 1 && dec > MAX_LATITUDE)
		  return 0;
	  else if (type == 2 && dec > MAX_LONGITUDE)
		  return 0;

	  if (dir == 'N'|| dir == 'E')
		return dec;
	  else
		return -1 * dec;
}
 
/**
 *  @brief This function converts gps data which are in string format into integer/float type data format
 *  After converting it copy each parameter into its respective structure like for date & time stamp gets 
 *  copied to utc_timestruct & other gps data like lat,long,speed,heading etc gets copied into gps_coordinates
 *  structure
 *  @return void
 */
static void DecodeRMCdata(void)
{
	switch(commaIdx)
	{
		case 1:
			if(strlen((char *)gpspara_Idx) > 0)
			{
				utc_timestruct.hour = (gpspara_Idx[0] - '0')*10 + (gpspara_Idx[1] - '0');
				utc_timestruct.minute = (gpspara_Idx[2] - '0')*10 + (gpspara_Idx[3] - '0');
				utc_timestruct.second = (gpspara_Idx[4] - '0')*10 + (gpspara_Idx[5] - '0');
//				LOG_DBG(CH_GPS,"GPStime = %d:%d:%d",utc_timestruct.hour, utc_timestruct.minute, utc_timestruct.second);
			}
			else
			{
				utc_timestruct.hour = 0;
				utc_timestruct.minute = 0;
				utc_timestruct.second = 0;
			}
			break;
			
		case 2:
			gps_coordinates.validity = gpspara_Idx[0];
//			LOG_DBG(CH_GPS,"GPSValidity = %c-->", gps_coordinates.validity);
			break;

		case 3:
			//gps_coordinates.Latitude = (uint32_t)(atof((const char*)gpspara_Idx) * 100000);
			//APP_DEBUG("<--GPSLatitude = %f-->\r\n", gps_coordinates.Latitude);
			break;		
			
		case 4:
			//gps_coordinates.LatDir = gpspara_Idx[0];
			//APP_DEBUG("<--GPSLatDir = %c-->\r\n", gps_coordinates.LatDir);
			break;	

		case 5:
			//gps_coordinates.Longitude = (uint32_t)(atof((const char*)gpspara_Idx) * 100000);
			//APP_DEBUG("<--GPSLongitude = %f-->\r\n", gps_coordinates.Longitude);
			break;	

		case 6:
			//gps_coordinates.LongDir = gpspara_Idx[0];
			//APP_DEBUG("<--GPSLongDir = %c-->\r\n", gps_coordinates.LongDir);
			break;	

		case 7:
			gps_coordinates.speed = (float)(atof((const char*)gpspara_Idx) * 1.852);  //in km/hr
//			LOG_DBG(CH_GPS,"GPSSpeed = %f", gps_coordinates.speed);
			break;	
			
		case 8:
			if(strlen((char *)gpspara_Idx) > 0)		
			{
				gps_coordinates.heading = (float)(atof((const char*)gpspara_Idx));
//				LOG_DBG(CH_GPS,"GPSHeading = %f", gps_coordinates.heading);
			}
			else
				gps_coordinates.heading = 0;
			break;	

		case 9:
			if(strlen((char *)gpspara_Idx) > 0)
			{
				utc_timestruct.day = (gpspara_Idx[0] - '0')*10 + (gpspara_Idx[1] - '0');
				utc_timestruct.month = (gpspara_Idx[2] - '0')*10 + (gpspara_Idx[3] - '0');
				utc_timestruct.year = (gpspara_Idx[4] - '0')*10 + (gpspara_Idx[5] - '0');
				//utc_timestruct.year -= 17;
				//APP_DEBUG("<--GPSDate = %d/%d/%d-->\r\n", utc_timestruct.day, utc_timestruct.month, utc_timestruct.year);
			}
			else
			{
				utc_timestruct.day = 0;
				utc_timestruct.month = 0;
				utc_timestruct.year = 0;
			}
			break;
			
		default:
            break;	
	}
}

static void DecodeGGAdata(void)
{
	switch(commaIdx)
	{
//		case 1:
//		case 2:
//		case 3:
//		case 4:
//		case 5:
//      break;
		case 6:
            gps_coordinates.gpsFix = atoi((const char*)gpspara_Idx); 
			break;	

		case 7:
			gps_coordinates.noOfSat = atoi((const char*)gpspara_Idx); 
			break;	
			
		case 8:
			break;	

		case 9:
			gps_coordinates.altitude = atof((const char*)gpspara_Idx);
			//LOG_INFO(CH_PAC,"<--GPSAltitude = %d-->\r\n", gps_coordinates.Altitude);
			break;
			
		default:
            break;	
	}
}

static void DecodeGSAdata(void)
{
	switch(commaIdx)
	{
//		case 1:
//		case 2:
//		case 3:
//		case 4:
//		case 5:
//      break;
		case 15:
            gps_coordinates.pDop = atof((const char*)gpspara_Idx); 
			break;	

		case 16:
			gps_coordinates.hDop = atof((const char*)gpspara_Idx); 
			break;	
			
//		case 8:
//			break;	
//
//		case 9:
//			gps_coordinates.altitude = atof((const char*)gpspara_Idx);
//			//LOG_INFO(CH_PAC,"<--GPSAltitude = %d-->\r\n", gps_coordinates.Altitude);
//			break;
			
		default:
            break;	
	}
}

/**
 *  @brief This function checks the Gps Validity by checking the value of gps_coordinates.Validity, it returns 1
 *  if the value is 'A' it returns 1 else returns 0 if it is 'V'
 *   validity - A-ok, V-invalid
 *  @return Boolean 1 or 0
 */
BOOL Is_Gps_Valid(void)
{
    if((gps_coordinates.validity == 'A')&& (gps_coordinates.gpsFix != 0))   //condition added to check gps fix
    return True;
    else
    return False;
}



/**
 *  @brief Copies data from utc_timestruct to the gpstime_ptr pointer to get the updated GPS time in 
 *  new packet
 *  @param [in] gpstime_ptr it is a pointer which points to GpsTimePara_t structure 
 *  @return void
 */
void Get_Gps_time(GpsTimePara_t *gpstime_ptr)
{
	memset(gpstime_ptr,0,sizeof(GpsTimePara_t));
	memcpy(gpstime_ptr, &utc_timestruct, sizeof(GpsTimePara_t));
}

void CalDistance(double latitude1,double longitude1,double latitude2,double longitude2)
{
  float lat1, lat2, difflat, difflon, a= 0;
         
  if(gps_coordinates.latDir=='S')
  {latitude2=latitude2*(-1);
  }
  if(gps_coordinates.longDir=='W')
  {longitude2=longitude2*(-1);
  }
   GPS_Distance.PrevLatitude = latitude2;
   GPS_Distance.PrevLongitude= longitude2;
   /* Calculate distance between 2 points */
   lat1 = GPS_DEGREES2RADIANS(latitude1);
   lat2 = GPS_DEGREES2RADIANS(latitude2);
   difflat = GPS_DEGREES2RADIANS(latitude2 - latitude1);
   difflon = GPS_DEGREES2RADIANS(longitude2 - longitude1);

   a = sin(difflat * (float)0.5) * sin(difflat * (float)0.5) + cos(lat1) * cos(lat2) * sin(difflon * (float)0.5) * sin(difflon * (float)0.5);
   /* Get distance in meters */
   GPS_Distance.Distance = GPS_EARTH_RADIUS * 2 * atan2(sqrt(a), sqrt(1 - a)) * 1000;
}

float GpsGetDistance(void)
{
  return totalGpsDistanceInKm;
}

void UpdateGpsDistnceFromMem(void)
{
//  gpsMiscDataPtr = GetAisMiscData();
//  totalGpsDistanceCountInMem = gpsMiscDataPtr->gpsDistanceCount;
//  totalGpsDistanceInKm = totalGpsDistanceCountInMem *(float)((float)METER_PER_COUNT/(float)1000);
}

void Update_Distance(void)
{
//    distance = (uint8_t)GPS_Distance.Distance;
//    finaldistance += distance;
   if(distance >= METER_PER_COUNT) //if distance us >= 500 meters
   {
     distance -= METER_PER_COUNT;
     totalGpsDistanceCountInMem++;
     totalGpsDistanceInKm += (float)((float)METER_PER_COUNT/(float)1000);
//     gpsMiscDataPtr->gpsDistanceCount = totalGpsDistanceCountInMem;
//   
//     SetMiscDataToFlash(gpsMiscDataPtr);
   } 
}

void ResetGpsDistance(void)
{
   distance = 0;
   totalGpsDistanceCountInMem = 0;
   totalGpsDistanceInKm = 0;
//   gpsMiscDataPtr->gpsDistanceCount = 0;
//   SetMiscDataToFlash(gpsMiscDataPtr); //update gps distance to 0 in memory
}

uint8_t Distance_Task(void)
{
  if((GPS_Distance.PrevLatitude != 0.0) && (GPS_Distance.PrevLongitude != 0.0))
  {
    CalDistance(GPS_Distance.PrevLatitude,GPS_Distance.PrevLongitude,gps_coordinates.latitude,gps_coordinates.longitude);
    distance += (uint8_t)GPS_Distance.Distance;
    return 1;
  }
  else  //previous lat long are 0 
  {
    GPS_Distance.PrevLatitude = gps_coordinates.latitude;
    GPS_Distance.PrevLongitude= gps_coordinates.longitude;
  }
  return 0;
}
