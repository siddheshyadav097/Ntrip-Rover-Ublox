#include "gsm_common.h"
#include <string.h>

char* GsmFindString(char *line, uint32_t len,char *str)
{
    int32_t i;
    int32_t gsmStrlen;
    char *p;

    if ((NULL == line) || (NULL == str))
        return NULL;
    
    gsmStrlen = strlen(str);
    if(gsmStrlen > len)
    {
        return NULL;
    }

    p = line;
    for (i = 0;i < len - gsmStrlen + 1; i++)
    {
        if (0 == strncmp (p, str, gsmStrlen))
        {
            return p;
        }
        else
        {
            p++;
        }
    }
    return NULL;
}

/**
 *  @brief GSMStrPrefixMatch() This function Checks whether required GSM AT Cmd prefix is present in string passed to str.
 *  @param: str - Pointer to the string in which prefix is to be searched.
 *  @param: prefix - GSM AT Command prefix which needs to searched.
 *  @return : returns 0 if prefix not found else 1.
 */
uint8_t GSMStrPrefixMatch(const char* str, const char *prefix)
{
    for ( ; *str != '\0' && *prefix != '\0' ; str++, prefix++) {
        if (*str != *prefix) {
            return 0;
        }
    }
    return *prefix == '\0';
}

/*
// NAME   : pucUTL_eSearchChar_Exe
// ROLE   : Search a Character present in a Buffer for given number of occurrence
//		    and return the location as pointer.
//
// INPUT  : ucSearchChar - Character to be searched
//		ucPos      - Number of occurrence (Freq)
//		ucStartAdd - Start Address of Buffer
//		ucEndAdd   - End Address of Buffer
// OUTPUT : Not Found - 0
//	    Found     - Address of Desired Character in String
*/

uint8_t *pucUTL_eSearchChar_Exe (uint8_t ucSearchChar, uint8_t ucPos, uint8_t *ucStartAdd, uint8_t *ucEndAdd)
{
	uint8_t ucTemp ;

	for(ucTemp = 0; ((ucTemp < ucPos) && (ucStartAdd <= ucEndAdd)); ucStartAdd++)
	{
		if (*ucStartAdd == ucSearchChar)
		{
	 		ucTemp++;
		}
    }
	ucStartAdd--;
	if(ucPos != ucTemp)
	{
		ucStartAdd = 0x00;
	}
	return(ucStartAdd);
}

/*
// NAME   : ucUTL_eGetLen_Exe
// ROLE   : Get Length of String.
//
// INPUT  : String
// OUTPUT : No of character in String.
*/
uint8_t ucUTL_eGetLen_Exe (uint8_t * ucString)
{
	uint8_t ucTemp;
	for(ucTemp = 0; (*(ucString + ucTemp) != '\0'); ucTemp++)
	{
		;
	}
	
	return (ucTemp);
}

/*
// NAME   : ucUTL_eCompareStrings
// ROLE   : Compare Two Strings.
//
// INPUT  : ucCompareNoBytes    - Number of Bytes to compare
//			ucAddString1 - Address of First String
//			ucAddString2 - Address of Second String
// OUTPUT :  0 - Strings Not Matching
//	     1 - Strings Matching
*/
uint8_t ucUTL_eCompareStrings (uint8_t ucCompareNoBytes, uint8_t* ucAddString1, uint8_t* ucAddString2)
{
	uint8_t ucTemp = 0, ucFlag = 1;

	for(ucTemp = 0; (ucFlag == 1) && (ucTemp < ucCompareNoBytes); ucTemp++, ucAddString1++,ucAddString2++)
	{
		if(*(ucAddString1) != *(ucAddString2))
		{
			ucFlag = 0;
		}
	}	
	return (ucFlag);
}
