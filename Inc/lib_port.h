/**
 *  \file lib_port.h
 *  \brief Macro definitions for standard library functions for specific CPU platform.
 *  		General function return typedefs are defined.
 */

#ifndef __LIB_PORT__H
#define __LIB_PORT__H

#define ST_CPU
//#define QUECTEL_CPU


#ifdef ST_CPU
	#include "stm32g0xx_hal.h"
        #include <string.h>
        #include <stdlib.h>
#endif

//#ifdef QUECTEL_CPU
//#include "ql_type.h"
//
//#define atoi    Ql_atoi
//#define atof	Ql_atof
//#define memset  Ql_memset
//#define memcpy  Ql_memcpy
//
//#define strcmp   Ql_strcmp
//#define strcpy   Ql_strcpy
//#define strncpy  Ql_strncpy
//#define strstr   Ql_strstr
//#define strlen   Ql_strlen
//
//#define sprintf	 Ql_sprintf
//	
//	typedef u8		uint8_t;	
//	typedef u16 	        uint16_t;	
//	typedef u32		uint32_t;	
//	typedef u64		uint64_t;	
//
//	typedef s8		int8_t;		
//	typedef s16		int16_t;		
//	typedef s32		int32_t;	
//	typedef s64		int64_t;		
//	
//#endif

typedef unsigned char UCHAR;
typedef char    CHAR;

typedef uint16_t USHORT;
typedef int16_t SHORT;

typedef uint32_t ULONG;
typedef int32_t LONG;


typedef enum
{
	False,
	True
}BOOL;

typedef enum
{
	RET_SUCCESS,
	RET_CONTINUE,
	RET_FAILURE
}retState_t;

void FeedWatchdog(void);

#endif		