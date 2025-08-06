/****************FOR FSRO***************/

/**
 *  @file    timespent.c
 *  @author  Gajanan & Gopal
 *  @version V
 *  @date    
 *  @brief 
 */
#include "qtimespent.h"
#define FULL_SCALE_VALUE		0xffffffff

// the counter tick is incremented in interrupt every 1 milliseconds
//extern uint32_t uwTick;
extern __IO uint32_t uwTick;

//configure timer4 for system tick.
//void InitQTimeSpent(void)
//{
//}

uint32_t GetStartTime(void)
{
	return uwTick;
}

uint8_t TimeSpent(uint32_t startTick, uint32_t totalTimeInMs)
{
	if(uwTick >= startTick)
	{
		if((uwTick - startTick) < totalTimeInMs)
		{
			return 0;
		}
		else
		{
			return 1;
		}
		//return ((counterTick - startTick) < totalTimeInMs)?0:1;
	}
	else
	{
		if(((FULL_SCALE_VALUE - (startTick - uwTick)) < totalTimeInMs))
		{
			return 0;
		}
		else
		{
			return 1;
		}
	}
}




