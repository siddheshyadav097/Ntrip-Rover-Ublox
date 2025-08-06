/**
 *  \file gps_api.h
 *  \brief Includes macros for constants used by GPS APIs, initialization of GPS,
 *  	   type definitions for GPS data and time parameters and APIs to process GPS data acquired.
 */

#ifndef __GPS_API_H__
#define __GPS_API_H__

 /*----includes-----*/
#include "gps_port.h"
#include "ring_buffer.h" 
#include "qtimespent.h"


 /*----constants-----*/
#define GPS_EARTH_RADIUS                6371
/* Degrees to radians converter */
#define GPS_DEGREES2RADIANS(x)  ((x) * (float)0.01745329251994)
/* Radians to degrees */
#define GPS_RADIANS2DEGREES(x)  ((x) * (float)57.29577951308232)

#define METER_PER_COUNT              500
   
#define MAX_GPS_RX_BUFFER_SIZE      1024

#define MAX_LATITUDE                 90
#define MAX_LONGITUDE                180

#define MAX_GPS_NOT_RCVD_TIMEOUT     5000


//#define	MAX_FIELD_SIZE	        12
//#define	MAX_FIELD_BUFFERS	8
//#define	MAX_GPS_RX_BUFFER	50
//#define LEADING_POS_LEN         55
//#define CONSECUTIVE_POS_LEN     43
//#define POS_COUNT_INDEX         4           //Unit ID starts from 0


 /*----typedefs-----*/
typedef  struct 
{
	uint8_t gpsFix;
	uint8_t validity;
	//float latitude;
	double latitude;
	uint8_t latDir;
	//float longitude;
	double longitude;
	uint8_t longDir;
	float speed;
	float heading;
	float altitude;
	uint8_t noOfSat;
	float pDop;
	float hDop;
}GpsData_t;

typedef struct 
{
    int32_t year;    
    int32_t month;
    int32_t day;
    int32_t hour;
    int32_t minute;
    int32_t second;
    int32_t timezone;  
}GpsTimePara_t;

//typedef enum
//{
//   WAIT_FOR_NMEA_LINE =0,
//   PROCESS_NMEA_LINE
//}gpsmNmeaState_et;

//typedef struct 
//{
//        unsigned Timeout:1;
//        unsigned LineStart:1;
//        unsigned FieldFound:1;
//        unsigned StoreField:1;
//        unsigned ChecksumField:1;
//        unsigned ValidFlag:1;
//      unsigned LatitudeDirection:1;
//      unsigned LongitudeDirection:1;
//} GPSFlags_st;


 /*----public function declarations-----*/
void InitGPSConfig(void);
void GpsRingBuffInit(void);
void GpsNmeaResponseHandler(void);
// this function is called by gps UART reception ISR
void GpsUartReceiveDataCb(uint8_t receiveGpsData);
void GetGpsData(uint8_t* gpslineBuff , uint16_t gpsLineLen);
GpsData_t* GetGpsDataForPacketing(void);
GpsData_t* ClearGpsPacketData(void);
BOOL Is_Gps_Valid(void);
BOOL Get_Gps_Data(GpsData_t *gps_data_ptr);
void Get_Gps_time(GpsTimePara_t *gpstime_ptr);
double ConvertNmeaToDecimal(uint8_t *nmea, uint8_t type, uint8_t dir);
extern uint8_t gpggapacketBuffer[254];
#endif