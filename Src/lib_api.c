/**
 *  @file          :  lib_api.c
 *  @author        :  Aakash/Ratna
 *  @date          :  18/4/2017
 *  @brief         :  Library file that includes functions for data conversion one type to another,
 *  				  such as character, hex, ascii string, integer, binary.
 *  @filerevision  :  1.0
 *  
 */
 

 /*----includes-----*/
#include <string.h>
#include <stdlib.h>
#include "lib_api.h"

/**
 *  @brief This function calculates the checksum value of the data that needs to be written
 *  in the file along with this byte at the end of data
 *  @param [in] WtBuffPtr contains data whose checksum needs to be calculated
 *  @param [in] WtBuffLen contains length of data
 *  @return Returns calculated checksum byte
 */
//uint8_t CalculateLRC(uint8_t *WtBuffPtr, uint16_t WtBuffLen)
//{
//	uint8_t calcWtFileChksum = 0;
//	
//	while(WtBuffLen--)	
//	{	
//		calcWtFileChksum = (uint8_t)(calcWtFileChksum + (uint8_t)*WtBuffPtr++);
//	}
//	calcWtFileChksum = ~calcWtFileChksum;
//	return (uint8_t)calcWtFileChksum;
//}

//uint16_t GetSum(uint8_t *data,uint16_t count)
//{
//    uint16_t result = 0;
//	uint16_t i;
//	for(i = 0; i < count; i++)
//	{
//		result += data[i];
//	}
//	return result;
//}


 /*----public functions-----*/
 

/*
	Converts the ASCII character to corresponding hex number.
*/
uint8_t m_ctoh(uint8_t ch)
{
	if (ch >= 0x30 && ch <= 0x39)
	{
		ch = ch - 0x30;
	}
	else if (ch >= 'A' && ch <= 'F')
	{
		ch = ch - 'A' + 0xa;
	}
	else if (ch >= 'a' && ch <= 'f')
	{
		ch = ch - 'a' + 0xa;
	}
	else
	{
		ch = 0x0;
	}
	return(ch);
}

/*
	Converts the string to hex value.
*/
uint32_t m_atoh(uint8_t *ptr)
{
	uint32_t val=0;
	uint8_t ch;
	uint8_t len=0,i=0;
	
	len =(strlen((char*)ptr));
	i = len;
	while(i>0)
	{
		ch = m_ctoh(ptr[i-1]);
		val |= (ch & 0x000F) << (4*(len-i));
		i--;
	}
	return(val);
}


/*
	Converts the hex number to string.
*/
//void m_htoa(uint8_t *ptr, uint8_t mch)
//{
//	uint8_t ch;
//	ch = mch >> 4;
//	ch = ch & 0x0F;
//	//if (!((ch >= 0) && (ch <= 9)))
//	if (!(ch <= 9))
//	{
//		ch = ch + '0' + 7;
//	}
//	else
//	{
//		ch = ch + '0';
//	}
//	ptr[0] = ch;
//
//	ch = mch & 0x0F;
//	if (!(ch <= 9))
//	{
//		ch = ch + '0' + 0x7;
//	}
//	else
//	{
//		ch = ch + '0';
//	}
//	ptr[1] = ch;
//	ptr[2] = '\0';
//}


//uint16_t getInteger(uint8_t *ptr)
//{
//	uint8_t i;
//	uint8_t str[6];
//
//	while(*ptr != 0)
//	{
//		if ((*ptr == ' ') || (*ptr == '\t'))
//		{
//			ptr = ptr + 1;
//		}
//		else
//			break;
//	}
//
//	for (i=0; i<5; ++i)
//	{
//		if (ptr[i] == 0)
//			break;
//	
//		if ((ptr[i] >= '0') && (ptr[i] <= '9'))
//		{
//			str[i] = ptr[i];
//		}
//		else
//			break;
//	}
//	str[i] = '\0';
//	if (i != 0)
//		return(atoi((char*)str));
//	else
//		return(0);
//	
//}

/*convert int to ascii */
//void m_itoa (uint16_t i,char *ptr)		//30nov
//{
//	int divisor = 10000;
//	
//	while(divisor!=0)
//	{
//		*ptr = ((i/divisor) + '0');
//		i = (int)(i%divisor);
//		ptr++;
//		divisor = divisor/10;
//	}
//	*ptr = '\0';
//}

//void m_i2a(uint16_t val,uint8_t *ptr)
//{
//	unsigned long int divisor=100000;
//	uint16_t j;
//	uint8_t i,k=0;
//	
//	for(i=0;i<6;i++)
//	{
//		j=val/divisor;
//		if(j!=0)
//		{
//			k=1;
//			*ptr=(uint8_t)j+'0';
//			 ptr++;
//		}
//
//		if(j==0 && k==1)
//		{
//			*ptr=(uint8_t)j+'0';
//			 ptr++;
//		}
//		val=val%divisor;
//		divisor=divisor/10;
//	}
//	*ptr='\0';		
//}

/********************************************************************************************	
Function:			m_a2l()	
Description(Use):	Ascii long integer as input and output is Long int hex
Input:				ASCII Long Integer 
Output:				Long int Hex
********************************************************************************************/

//unsigned long int m_a2l(uint8_t *ptr)
//{
//	
//	uint8_t i;
//	unsigned long int Longint_Val=0;
//	
//			for(i=0;i<=11;i++)
//			{
//				if(ptr[i]=='\0')
//				{
//					break;
//				}
//		
//				if((ptr[i] >= '0') && (ptr[i]<='9'))			
//				{
//					Longint_Val=(Longint_Val*10)+(ptr[i]-0x30);
//				}
//				
//			}
//				
//	return (Longint_Val);		
//}

//uint8_t m_a2b(uint8_t *ptr)
//{
//	uint8_t i;
//	uint8_t hex_val=0;
//
//
//for(i=0;i<4;i++)
//	{
//		if(ptr[i]=='\0')
//		{
//			break;
//		}
//		if((ptr[i] >= '0') && (ptr[i]<='9'))			
//		{
//			hex_val= (hex_val*10)+(ptr[i]-0x30);
//		}
//	}
//	return(hex_val);
//
//}