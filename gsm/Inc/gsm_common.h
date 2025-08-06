#ifndef __GSM_COMMON_H
#define __GSM_COMMON_H

#include "gsm_port.h"

#define NUM_ELEMS(x) (sizeof(x)/sizeof(x[0]))
char* GsmFindString(char *line, uint32_t len,char *str);
uint8_t GSMStrPrefixMatch(const char* str, const char *prefix);

uint8_t *pucUTL_eSearchChar_Exe (uint8_t ucSearchChar, uint8_t ucPos, uint8_t *ucStartAdd, uint8_t *ucEndAdd);
uint8_t ucUTL_eGetLen_Exe (uint8_t * ucString);
uint8_t ucUTL_eCompareStrings (uint8_t ucCompareNoBytes, uint8_t* ucAddString1, uint8_t* ucAddString2);
#endif