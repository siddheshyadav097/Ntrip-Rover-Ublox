#include "gsm_utility.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

cellIdStruct_st         gsmCellTowerData;
gsmNeighcellIdStruct_st gsmNeighCellTowerData;
GSMTrackingdata_st gsmTrackingData;

uint8_t neighCellTowerData = 0;
uint8_t homeCellTowerDatarcvd = 0;

uint8_t newMode = 0, newDump= 0, modeAndDumpRcvd =0;

static char strTmp[100]; 
static char hexData[20] = {0};


void GsmInitClearCellDataBuff(void)
{
   memset(&gsmCellTowerData,0,sizeof(cellIdStruct_st));
   memset(&gsmNeighCellTowerData,0,sizeof(gsmNeighcellIdStruct_st));
}

void ProcessQENGStart(void)
{
    neighCellTowerData = 0;
    homeCellTowerDatarcvd = 0;
    modeAndDumpRcvd =0;
    newMode = 0;
    newDump= 0;
}

uint8_t ProcessQENGResultGet(void)
{
    return neighCellTowerData;
}


uint32_t GSMUtilConvHexToDec(uint8_t* HexData)
{
    int i = 0,len =0 , val =0,decimal = 0;
    /* Find the length of total number of hex digit */
    len = strlen((char*)HexData);
    len--;

    /*
     * Iterate over each hex digit
     */
    for(i=0; HexData[i]!='\0'; i++)
    {
        /* Find the decimal representation of hex[i] */
        if(HexData[i]>='0' && HexData[i]<='9')
        {
            val = HexData[i] - 48;
        }
        else if(HexData[i]>='a' && HexData[i]<='f')
        {
            val = HexData[i] - 97 + 10;
        }
        else if(HexData[i]>='A' && HexData[i]<='F')
        {
            val = HexData[i] - 65 + 10;
        }

        decimal += val * pow(16, len);
        len--;
    }
//    LOG_DBG(CH_GSM,"Hexadecimal number = %s\r\n", HexData);
//    LOG_DBG(CH_GSM,"Decimal number = %ld\r\n", decimal);
    return decimal;
}

/**
 *  @brief ProcessQENGData processes the respective cell id received data
 *  @return void
 *  e.g.
 *  AT+QENG?
 *  +QENG: 1,4
 *  +QENG: 0,404,20,3a9a,1429,1,63,-68,147,147,5,5,x,x,x,x,x,x,x
 *  +QENG: 1,1,8,-71,32,134,134,404,20,3a9a,142a,2,12,-77,19,110,110,404,20,3a9a,142b,
 *         3,9,-85,35,79,79,404,20,3a9a,8c83,4,5,-95,61,39,39,404,20,3a9a,8b4e,
 *         5,3,-96,18,35,35,404,20,3a9a,8b4d,6,6,-101,19,15,15,404,20,3a9a,2cf9
 *  +QENG: 2,21,88
 *
 *  +QENG: 4,12,(8,-71,32),(12,-77,19),(9,-85,35),(5,-95,61),(3,-96,18),(6,-101,19),(10,-103,x),
 *         (4,-103,2),(11,-95,x),(7,-89,x),(2,-84,x),(13,-94,x)
 *
 *   OK
**/
uint8_t ProcessQENGData(uint8_t *QengLinedata ,uint8_t QengLinelen)
{
    char* p1 = NULL;
    char* p2 = NULL;
    uint8_t* cBuffAdd = NULL;
    uint8_t* cBuffAdd1 = NULL , *cBuffAdd2 = NULL , *cBuffAdd3 = NULL;
  
   if((strncmp((char*)QengLinedata, (char *)"+QENG: 1,1", 10) == 0) && (modeAndDumpRcvd != 1))
    {
         memset(strTmp, 0x0, sizeof(strTmp));
         sprintf((char *)strTmp, "+QENG: ");
        if (GSMStrPrefixMatch((char *)QengLinedata,strTmp))
        {
           p1 = strstr((char *)QengLinedata,":");
           p1++;
           p2 = strchr(p1,',');
           if(p1 && p2)
           {
              memset(strTmp, 0x0, sizeof(strTmp));
              memcpy(strTmp, p1, p2 - p1);
              newMode = atoi(strTmp);        //copy the mode which is set
              p2 += 1;                         //now get the dump type here serv cell
              memset(strTmp, 0x0, sizeof(strTmp));
              memcpy(strTmp, p2, 1);
              newDump = atoi(strTmp);  
           }
        }
   }
   else if((strncmp((char*)QengLinedata, (char *)"+QENG: 0,", 9) == 0) && (homeCellTowerDatarcvd != 1))
   {
          gsmCellTowerData.mcc =0;
          gsmCellTowerData.mnc =0;
          gsmCellTowerData.lac =0;
          gsmCellTowerData.cellid =0;
          gsmCellTowerData.dbm =0;
          gsmTrackingData.GSMLacInDec =0;
          gsmTrackingData.GSMCellIdInDec =0;
            
            cBuffAdd1 = pucUTL_eSearchChar_Exe (':', 1, &QengLinedata[0], &QengLinedata[QengLinelen - 1]);
            if(cBuffAdd1 != 0x00)
            {
                    //get the mobile country code
                    cBuffAdd2 = pucUTL_eSearchChar_Exe (',', 1, cBuffAdd1, &QengLinedata[QengLinelen - 1]);
                    cBuffAdd2++ ;
                    if(cBuffAdd2 != 0x00)
                    {
                        cBuffAdd3 = pucUTL_eSearchChar_Exe (',', 1, cBuffAdd2, &QengLinedata[QengLinelen - 1]);
                        if( (cBuffAdd2 != 0x00) && (cBuffAdd3 != 0x00))
                        {
                            memset(strTmp, 0x0, sizeof(strTmp));
                            memcpy(strTmp, cBuffAdd2, cBuffAdd3 - cBuffAdd2);
                            if((strncmp(strTmp,"x",1) != 0)){gsmCellTowerData.mcc = atoi(strTmp);
//                            LOG_DBG(CH_GSM,"mcc = %d\r\n",gsmCellTowerData.mcc);
                            }
                            else{gsmCellTowerData.mcc = 0;}
                        }
                    }
                    else{gsmCellTowerData.mcc = 0;}
                    
                    //get the mobile network code
                    cBuffAdd2 = pucUTL_eSearchChar_Exe (',', 2, cBuffAdd1, &QengLinedata[QengLinelen - 1]);
                    cBuffAdd2++ ;
                    if(cBuffAdd2 != 0x00)
                    {
                        cBuffAdd3 = pucUTL_eSearchChar_Exe (',', 1, cBuffAdd2, &QengLinedata[QengLinelen - 1]); 
                        if( (cBuffAdd2 != 0x00) && (cBuffAdd3 != 0x00))
                        {
                            memset(strTmp, 0x0, sizeof(strTmp));
                            memcpy(strTmp, cBuffAdd2, cBuffAdd3 - cBuffAdd2);
                            if((strncmp(strTmp,"x",1) != 0)) {gsmCellTowerData.mnc = atoi(strTmp);        
//                              LOG_DBG(CH_GSM,"mnc = %d\r\n",gsmCellTowerData.mnc);
                            }
                            else{gsmCellTowerData.mnc = 0; }
                        }
                    }
                    else{gsmCellTowerData.mnc = 0;}
                    
                    //get the location area code(LAC)
                    cBuffAdd2 = pucUTL_eSearchChar_Exe (',', 3, cBuffAdd1, &QengLinedata[QengLinelen - 1]);
                    cBuffAdd2++ ;
                    if(cBuffAdd2 != 0x00)
                    {
                        cBuffAdd3 = pucUTL_eSearchChar_Exe (',', 1, cBuffAdd2, &QengLinedata[QengLinelen - 1]); 
                        if( (cBuffAdd2 != 0x00) && (cBuffAdd3 != 0x00))
                        {
                             memset(strTmp, 0x0, sizeof(strTmp));
                            memcpy(strTmp, cBuffAdd2, cBuffAdd3 - cBuffAdd2);
                            if((strncmp(strTmp,"x",1) != 0)){gsmCellTowerData.lac = GSMUtilConvHexToDec((unsigned char*)strTmp);        
//                              LOG_DBG(CH_GSM,"LAC = %d\r\n",gsmCellTowerData.lac);
                            }
                            else{gsmCellTowerData.lac = 0;}
                        }
                    }
                    else{gsmCellTowerData.lac = 0;}
                    
                    //get the cell id data
                    cBuffAdd2 = pucUTL_eSearchChar_Exe (',', 4, cBuffAdd1, &QengLinedata[QengLinelen - 1]);
                    cBuffAdd2++ ;
                    if(cBuffAdd2 != 0x00)
                    {
                        cBuffAdd3 = pucUTL_eSearchChar_Exe (',', 1, cBuffAdd2, &QengLinedata[QengLinelen - 1]); 
                        if( (cBuffAdd2 != 0x00) && (cBuffAdd3 != 0x00))
                        {
                             memset(strTmp, 0x0, sizeof(strTmp));
                             memcpy(strTmp, cBuffAdd2, cBuffAdd3 - cBuffAdd2);
                             if((strncmp(strTmp,"x",1) != 0)){gsmCellTowerData.cellid = GSMUtilConvHexToDec((unsigned char*)strTmp); 
//                             LOG_DBG(CH_GSM,"Cell ID = %d\r\n",gsmCellTowerData.cellid);
                             }
                             else{gsmCellTowerData.cellid =0;}
                        }
                    }
                    else{gsmCellTowerData.cellid =0;}
                    
                    //Get the rssi in dBm
                    cBuffAdd2 = pucUTL_eSearchChar_Exe (',', 7, cBuffAdd1, &QengLinedata[QengLinelen - 1]);
                    cBuffAdd2++ ;
                    if(cBuffAdd2 != 0x00)
                    {
                        cBuffAdd3 = pucUTL_eSearchChar_Exe (',', 1, cBuffAdd2, &QengLinedata[QengLinelen - 1]);
                        if( (cBuffAdd2 != 0x00) && (cBuffAdd3 != 0x00))
                        {
                             memset(strTmp, 0x0, sizeof(strTmp));
                             memcpy(strTmp, cBuffAdd2, cBuffAdd3 - cBuffAdd2);
                             if((strncmp(strTmp,"x",1) != 0)){gsmCellTowerData.dbm = atoi(strTmp); 
//                             LOG_DBG(CH_GSM,"RSSI In dBm = %d\r\n",gsmCellTowerData.dbm);
                             }
                             else{gsmCellTowerData.dbm =0;}
                        }
                    }
                    else{gsmCellTowerData.dbm = 0;}
                    homeCellTowerDatarcvd =1;   
                    modeAndDumpRcvd =1;
            }
   }

   else if((strncmp((char*)QengLinedata, (char *)"+QENG: 1,1", 10) == 0) && (modeAndDumpRcvd) && (neighCellTowerData  != 1))
   {
       cBuffAdd = pucUTL_eSearchChar_Exe (':', 1, &QengLinedata[0], &QengLinedata[QengLinelen - 1]);  
       cBuffAdd2 = pucUTL_eSearchChar_Exe(',',1,cBuffAdd,&QengLinedata[QengLinelen - 1]);             
       cBuffAdd2++;
       if(*cBuffAdd2 == '1')
       {
              //Neighbouring cell id 1 - rssi in dbm
              cBuffAdd3 = pucUTL_eSearchChar_Exe (',', 2, cBuffAdd2, &QengLinedata[QengLinelen - 1]);
              cBuffAdd3++;
              cBuffAdd2 = pucUTL_eSearchChar_Exe(',',1,cBuffAdd3,&QengLinedata[QengLinelen - 1]);
              if((cBuffAdd2 != 0) && (cBuffAdd3 != 0))
              {
                memset(strTmp, 0x0, sizeof(strTmp));
                memcpy(strTmp, cBuffAdd3, cBuffAdd2 - cBuffAdd3);
                if((strncmp(strTmp,"x",1) != 0)){gsmNeighCellTowerData.nc1dbm = atoi(strTmp);}
                else{gsmNeighCellTowerData.nc1dbm =0;}
              }
              //Neighbouring cell id 1 - mcc
              cBuffAdd2++;
              cBuffAdd3 = pucUTL_eSearchChar_Exe (',', 3, cBuffAdd2, &QengLinedata[QengLinelen - 1]);
              cBuffAdd3++;
              cBuffAdd2 = pucUTL_eSearchChar_Exe(',',1,cBuffAdd3,&QengLinedata[QengLinelen - 1]);
              if((cBuffAdd2 != 0) && (cBuffAdd3 != 0))
              {
                memset(strTmp, 0x0, sizeof(strTmp));
                memcpy(strTmp, cBuffAdd3, cBuffAdd2 - cBuffAdd3);
                if((strncmp(strTmp,"x",1) != 0)){gsmNeighCellTowerData.nc1Mcc = atoi(strTmp);}
                else{gsmNeighCellTowerData.nc1Mcc =0;}
              }
              //Neighbouring cell id 1 - mnc
              cBuffAdd2++;
              cBuffAdd3 = pucUTL_eSearchChar_Exe (',', 1, cBuffAdd2, &QengLinedata[QengLinelen - 1]);
              if((cBuffAdd2 != 0) && (cBuffAdd3 != 0))
              {
                memset(strTmp, 0x0, sizeof(strTmp));
                memcpy(strTmp, cBuffAdd2, cBuffAdd3 - cBuffAdd2);
                if((strncmp(strTmp,"x",1) != 0)){gsmNeighCellTowerData.nc1Mnc = atoi(strTmp);}
                else{gsmNeighCellTowerData.nc1Mnc =0;}
              }
              //Neighbouring cell id 1 - LAC
              cBuffAdd3++;
              cBuffAdd2 = pucUTL_eSearchChar_Exe (',', 1, cBuffAdd3, &QengLinedata[QengLinelen - 1]);
              if((cBuffAdd2 != 0) && (cBuffAdd3 != 0))
              {
                memset(strTmp, 0x0, sizeof(strTmp));
                memcpy(strTmp, cBuffAdd3, cBuffAdd2 - cBuffAdd3);
                if((strncmp(strTmp,"x",1) != 0)){gsmNeighCellTowerData.nc1Lac = GSMUtilConvHexToDec((unsigned char*)strTmp);}
                else{gsmNeighCellTowerData.nc1Lac =0;}
              }
              //Neighbouring cell id 1 - Cell Id
              cBuffAdd2++;
              cBuffAdd3 = pucUTL_eSearchChar_Exe (',', 1, cBuffAdd2, &QengLinedata[QengLinelen - 1]);
              if((cBuffAdd2 != 0) && (cBuffAdd3 != 0))
              {
                memset(strTmp, 0x0, sizeof(strTmp));
                memcpy(strTmp, cBuffAdd2, cBuffAdd3 - cBuffAdd2);
                if((strncmp(strTmp,"x",1) != 0)){gsmNeighCellTowerData.nc1Cellid = GSMUtilConvHexToDec((unsigned char*)strTmp);}
                else{gsmNeighCellTowerData.nc1Cellid =0;}
          }
          else
          {
            gsmNeighCellTowerData.nc1dbm =0;
            gsmNeighCellTowerData.nc1Mcc =0;
            gsmNeighCellTowerData.nc1Mnc =0;
            gsmNeighCellTowerData.nc1Lac =0;
            gsmNeighCellTowerData.nc1Cellid =0;
          }
          //NC2 - dbm rssi
          cBuffAdd2 = pucUTL_eSearchChar_Exe(',',11,cBuffAdd,&QengLinedata[QengLinelen - 1]);
          cBuffAdd2++;
          if(*cBuffAdd2 == '2')
          {
                cBuffAdd3 = pucUTL_eSearchChar_Exe (',',2, cBuffAdd2, &QengLinedata[QengLinelen - 1]);
                cBuffAdd3++;
                cBuffAdd2 = pucUTL_eSearchChar_Exe(',',1,cBuffAdd3,&QengLinedata[QengLinelen - 1]);
                if((cBuffAdd2 != 0) && (cBuffAdd3 != 0))
               {
                  memset(strTmp, 0x0, sizeof(strTmp));
                  memcpy(strTmp, cBuffAdd3, cBuffAdd2 - cBuffAdd3);
                  if((strncmp(strTmp,"x",1) != 0)){gsmNeighCellTowerData.nc2dbm = atoi(strTmp);}
                  else{gsmNeighCellTowerData.nc2dbm =0;}
               }
               //nc2 mcc
                cBuffAdd2++;
                cBuffAdd3 = pucUTL_eSearchChar_Exe (',',3, cBuffAdd2, &QengLinedata[QengLinelen - 1]);
                cBuffAdd3++;
                cBuffAdd2 = pucUTL_eSearchChar_Exe(',',1,cBuffAdd3,&QengLinedata[QengLinelen - 1]);
                if((cBuffAdd2 != 0) && (cBuffAdd3 != 0))
               {
                  memset(strTmp, 0x0, sizeof(strTmp));
                  memcpy(strTmp, cBuffAdd3, cBuffAdd2 - cBuffAdd3);
                  if((strncmp(strTmp,"x",1) != 0)){gsmNeighCellTowerData.nc2Mcc = atoi(strTmp);}
                  else{gsmNeighCellTowerData.nc2Mcc =0;}
               }
               //Nc2 - mnc
              cBuffAdd2++;
              cBuffAdd3 = pucUTL_eSearchChar_Exe (',', 1, cBuffAdd2, &QengLinedata[QengLinelen - 1]);
              if((cBuffAdd2 != 0) && (cBuffAdd3 != 0))
              {
                memset(strTmp, 0x0, sizeof(strTmp));
                memcpy(strTmp, cBuffAdd2, cBuffAdd3 - cBuffAdd2);
                if((strncmp(strTmp,"x",1) != 0)){gsmNeighCellTowerData.nc2Mnc = atoi(strTmp);}
                else{gsmNeighCellTowerData.nc2Mnc =0;}
              }
               //Nc2 - LAC
              cBuffAdd3++;
              cBuffAdd2 = pucUTL_eSearchChar_Exe (',', 1, cBuffAdd3, &QengLinedata[QengLinelen - 1]);
              if((cBuffAdd2 != 0) && (cBuffAdd3 != 0))
              {
                memset(strTmp, 0x0, sizeof(strTmp));
                memcpy(strTmp, cBuffAdd3, cBuffAdd2 - cBuffAdd3);
                if((strncmp(strTmp,"x",1) != 0)){gsmNeighCellTowerData.nc2Lac = GSMUtilConvHexToDec((unsigned char*)strTmp);}
                else{gsmNeighCellTowerData.nc2Lac =0;}
              }
              //Nc2- Cell Id
              cBuffAdd2++;
              cBuffAdd3 = pucUTL_eSearchChar_Exe (',', 1, cBuffAdd2, &QengLinedata[QengLinelen - 1]);
              if((cBuffAdd2 != 0) && (cBuffAdd3 != 0))
              {
                memset(strTmp, 0x0, sizeof(strTmp));
                memcpy(strTmp, cBuffAdd2, cBuffAdd3 - cBuffAdd2);
                if((strncmp(strTmp,"x",1) != 0)){gsmNeighCellTowerData.nc2Cellid = GSMUtilConvHexToDec((unsigned char*)strTmp);}
                else{gsmNeighCellTowerData.nc2Cellid =0;}
              }
          }
          else
          {
            gsmNeighCellTowerData.nc2dbm =0;
            gsmNeighCellTowerData.nc2Mcc =0;
            gsmNeighCellTowerData.nc2Mnc =0;
            gsmNeighCellTowerData.nc2Lac =0;
            gsmNeighCellTowerData.nc2Cellid =0;
          }
          //NC3 - dbm rssi
          cBuffAdd2 = pucUTL_eSearchChar_Exe(',',21,cBuffAdd,&QengLinedata[QengLinelen - 1]);
          cBuffAdd2++;
          if(*cBuffAdd2 == '3')
          {
                cBuffAdd3 = pucUTL_eSearchChar_Exe (',',2, cBuffAdd2, &QengLinedata[QengLinelen - 1]);
                cBuffAdd3++;
                cBuffAdd2 = pucUTL_eSearchChar_Exe(',',1,cBuffAdd3,&QengLinedata[QengLinelen - 1]);
                if((cBuffAdd2 != 0) && (cBuffAdd3 != 0))
               {
                  memset(strTmp, 0x0, sizeof(strTmp));
                  memcpy(strTmp, cBuffAdd3, cBuffAdd2 - cBuffAdd3);
                  if((strncmp(strTmp,"x",1) != 0)){gsmNeighCellTowerData.nc3dbm = atoi(strTmp);}
                  else{gsmNeighCellTowerData.nc3dbm =0;}
               }
               //nc3 mcc
                cBuffAdd2++;
                cBuffAdd3 = pucUTL_eSearchChar_Exe (',',3, cBuffAdd2, &QengLinedata[QengLinelen - 1]);
                cBuffAdd3++;
                cBuffAdd2 = pucUTL_eSearchChar_Exe(',',1,cBuffAdd3,&QengLinedata[QengLinelen - 1]);
                if((cBuffAdd2 != 0) && (cBuffAdd3 != 0))
               {
                  memset(strTmp, 0x0, sizeof(strTmp));
                  memcpy(strTmp, cBuffAdd3, cBuffAdd2 - cBuffAdd3);
                  if((strncmp(strTmp,"x",1) != 0)){gsmNeighCellTowerData.nc3Mcc = atoi(strTmp);}
                  else{gsmNeighCellTowerData.nc3Mcc =0;}
               }
               //Nc3 - mnc
              cBuffAdd2++;
              cBuffAdd3 = pucUTL_eSearchChar_Exe (',', 1, cBuffAdd2, &QengLinedata[QengLinelen - 1]);
              if((cBuffAdd2 != 0) && (cBuffAdd3 != 0))
              {
                memset(strTmp, 0x0, sizeof(strTmp));
                memcpy(strTmp, cBuffAdd2, cBuffAdd3 - cBuffAdd2);
                if((strncmp(strTmp,"x",1) != 0)){gsmNeighCellTowerData.nc3Mnc = atoi(strTmp);}
                else{gsmNeighCellTowerData.nc3Mnc =0;}
              }
               //Nc3 - LAC
              cBuffAdd3++;
              cBuffAdd2 = pucUTL_eSearchChar_Exe (',', 1, cBuffAdd3, &QengLinedata[QengLinelen - 1]);
              if((cBuffAdd2 != 0) && (cBuffAdd3 != 0))
              {
                memset(strTmp, 0x0, sizeof(strTmp));
                memcpy(strTmp, cBuffAdd3, cBuffAdd2 - cBuffAdd3);
                if((strncmp(strTmp,"x",1) != 0)){gsmNeighCellTowerData.nc3Lac = GSMUtilConvHexToDec((unsigned char*)strTmp);}
                else{gsmNeighCellTowerData.nc3Lac =0;}
              }
              //Nc3- Cell Id
              cBuffAdd2++;
              cBuffAdd3 = pucUTL_eSearchChar_Exe (',', 1, cBuffAdd2, &QengLinedata[QengLinelen - 1]);
              if((cBuffAdd2 != 0) && (cBuffAdd3 != 0))
              {
                memset(strTmp, 0x0, sizeof(strTmp));
                memcpy(strTmp, cBuffAdd2, cBuffAdd3 - cBuffAdd2);
                if((strncmp(strTmp,"x",1) != 0)){gsmNeighCellTowerData.nc3Cellid = GSMUtilConvHexToDec((unsigned char*)strTmp);}
                else{gsmNeighCellTowerData.nc3Cellid =0;}
              }
          }
          else
          {
            gsmNeighCellTowerData.nc3dbm =0;
            gsmNeighCellTowerData.nc3Mcc =0;
            gsmNeighCellTowerData.nc3Mnc =0;
            gsmNeighCellTowerData.nc3Lac =0;
            gsmNeighCellTowerData.nc3Cellid =0;
          }
          //NC4 - dbm rssi
          cBuffAdd2 = pucUTL_eSearchChar_Exe(',',31,cBuffAdd,&QengLinedata[QengLinelen - 1]);
          cBuffAdd2++;
          if(*cBuffAdd2 == '4')
          {
                cBuffAdd3 = pucUTL_eSearchChar_Exe (',',2, cBuffAdd2, &QengLinedata[QengLinelen - 1]);
                cBuffAdd3++;
                cBuffAdd2 = pucUTL_eSearchChar_Exe(',',1,cBuffAdd3,&QengLinedata[QengLinelen - 1]);
                if((cBuffAdd2 != 0) && (cBuffAdd3 != 0))
               {
                  memset(strTmp, 0x0, sizeof(strTmp));
                  memcpy(strTmp, cBuffAdd3, cBuffAdd2 - cBuffAdd3);
                  if((strncmp(strTmp,"x",1) != 0)){gsmNeighCellTowerData.nc4dbm = atoi(strTmp);}
                  else{gsmNeighCellTowerData.nc4dbm =0;}
               }
               //nc4 mcc
                cBuffAdd2++;
                cBuffAdd3 = pucUTL_eSearchChar_Exe (',',3, cBuffAdd2, &QengLinedata[QengLinelen - 1]);
                cBuffAdd3++;
                cBuffAdd2 = pucUTL_eSearchChar_Exe(',',1,cBuffAdd3,&QengLinedata[QengLinelen - 1]);
                if((cBuffAdd2 != 0) && (cBuffAdd3 != 0))
               {
                  memset(strTmp, 0x0, sizeof(strTmp));
                  memcpy(strTmp, cBuffAdd3, cBuffAdd2 - cBuffAdd3);
                  if((strncmp(strTmp,"x",1) != 0)){gsmNeighCellTowerData.nc4Mcc = atoi(strTmp);}
                  else{gsmNeighCellTowerData.nc4Mcc =0;}
               }
               //Nc4 - mnc
              cBuffAdd2++;
              cBuffAdd3 = pucUTL_eSearchChar_Exe (',', 1, cBuffAdd2, &QengLinedata[QengLinelen - 1]);
              if((cBuffAdd2 != 0) && (cBuffAdd3 != 0))
              {
                memset(strTmp, 0x0, sizeof(strTmp));
                memcpy(strTmp, cBuffAdd2, cBuffAdd3 - cBuffAdd2);
                if((strncmp(strTmp,"x",1) != 0)){gsmNeighCellTowerData.nc4Mnc = atoi(strTmp);}
                else{gsmNeighCellTowerData.nc4Mnc =0;}
              }
               //Nc4 - LAC
              cBuffAdd3++;
              cBuffAdd2 = pucUTL_eSearchChar_Exe (',', 1, cBuffAdd3, &QengLinedata[QengLinelen - 1]);
              if((cBuffAdd2 != 0) && (cBuffAdd3 != 0))
              {
                memset(strTmp, 0x0, sizeof(strTmp));
                memcpy(strTmp, cBuffAdd3, cBuffAdd2 - cBuffAdd3);
                if((strncmp(strTmp,"x",1) != 0)){gsmNeighCellTowerData.nc4Lac = GSMUtilConvHexToDec((unsigned char*)strTmp);}
                else{gsmNeighCellTowerData.nc4Lac =0;}
              }
              //Nc4- Cell Id
              cBuffAdd2++;
              cBuffAdd3 = pucUTL_eSearchChar_Exe (',', 1, cBuffAdd2, &QengLinedata[QengLinelen - 1]);
              if((cBuffAdd2 != 0) && (cBuffAdd3 != 0))
              {
                memset(strTmp, 0x0, sizeof(strTmp));
                memcpy(strTmp, cBuffAdd2, cBuffAdd3 - cBuffAdd2);
                if((strncmp(strTmp,"x",1) != 0)){gsmNeighCellTowerData.nc4Cellid = GSMUtilConvHexToDec((unsigned char*)strTmp);}
                else{gsmNeighCellTowerData.nc4Cellid =0;}
              }
          }
          else
          {
            gsmNeighCellTowerData.nc4dbm =0;
            gsmNeighCellTowerData.nc4Mcc =0;
            gsmNeighCellTowerData.nc4Mnc =0;
            gsmNeighCellTowerData.nc4Lac =0;
            gsmNeighCellTowerData.nc4Cellid =0;
          }
          //Neighbouring cell Id 5 parameters
          //   *+QENG: 1,1,8,-71,32,134,134,404,20,3a9a,142a,2,12,-77,19,110,110,404,20,3a9a,142b,
          //   *       3,9,-85,35,79,79,404,20,3a9a,8c83,4,5,-95,61,39,39,404,20,3a9a,8b4e,
          //   *       5,3,-96,18,35,35,404,20,3a9a,8b4d,6,6,-101,19,15,15,404,20,3a9a,2cf9
          //NC5 - dbm rssi
          cBuffAdd2 = pucUTL_eSearchChar_Exe(',',41,cBuffAdd,&QengLinedata[QengLinelen - 1]);
          cBuffAdd2++;
          if(*cBuffAdd2 == '5')
          {
                cBuffAdd3 = pucUTL_eSearchChar_Exe (',',2, cBuffAdd2, &QengLinedata[QengLinelen - 1]);
                cBuffAdd3++;
                cBuffAdd2 = pucUTL_eSearchChar_Exe(',',1,cBuffAdd3,&QengLinedata[QengLinelen - 1]);
                if((cBuffAdd2 != 0) && (cBuffAdd3 != 0))
               {
                  memset(strTmp, 0x0, sizeof(strTmp));
                  memcpy(strTmp, cBuffAdd3, cBuffAdd2 - cBuffAdd3);
                  if((strncmp(strTmp,"x",1) != 0)){gsmNeighCellTowerData.nc5dbm = atoi(strTmp);}
                  else{gsmNeighCellTowerData.nc5dbm =0;}
               }
               //nc5 mcc
                cBuffAdd2++;
                cBuffAdd3 = pucUTL_eSearchChar_Exe (',',3, cBuffAdd2, &QengLinedata[QengLinelen - 1]);
                cBuffAdd3++;
                cBuffAdd2 = pucUTL_eSearchChar_Exe(',',1,cBuffAdd3,&QengLinedata[QengLinelen - 1]);
                if((cBuffAdd2 != 0) && (cBuffAdd3 != 0))
               {
                  memset(strTmp, 0x0, sizeof(strTmp));
                  memcpy(strTmp, cBuffAdd3, cBuffAdd2 - cBuffAdd3);
                  if((strncmp(strTmp,"x",1) != 0)){gsmNeighCellTowerData.nc5Mcc = atoi(strTmp);}
                  else{gsmNeighCellTowerData.nc5Mcc =0;}
               }
               //Nc5 - mnc
              cBuffAdd2++;
              cBuffAdd3 = pucUTL_eSearchChar_Exe (',', 1, cBuffAdd2, &QengLinedata[QengLinelen - 1]);
              if((cBuffAdd2 != 0) && (cBuffAdd3 != 0))
              {
                memset(strTmp, 0x0, sizeof(strTmp));
                memcpy(strTmp, cBuffAdd2, cBuffAdd3 - cBuffAdd2);
                if((strncmp(strTmp,"x",1) != 0)){gsmNeighCellTowerData.nc5Mnc = atoi(strTmp);}
                else{gsmNeighCellTowerData.nc5Mnc =0;}
              }
               //Nc5 - LAC
              cBuffAdd3++;
              cBuffAdd2 = pucUTL_eSearchChar_Exe (',', 1, cBuffAdd3, &QengLinedata[QengLinelen - 1]);
              if((cBuffAdd2 != 0) && (cBuffAdd3 != 0))
              {
                memset(strTmp, 0x0, sizeof(strTmp));
                memcpy(strTmp, cBuffAdd3, cBuffAdd2 - cBuffAdd3);
                if((strncmp(strTmp,"x",1) != 0)){gsmNeighCellTowerData.nc5Lac = GSMUtilConvHexToDec((unsigned char*)strTmp);}
                else{gsmNeighCellTowerData.nc5Lac =0;}
              }
              //Nc5- Cell Id
              cBuffAdd2++;
              cBuffAdd3 = pucUTL_eSearchChar_Exe (',', 1, cBuffAdd2, &QengLinedata[QengLinelen - 1]);
              if((cBuffAdd2 != 0) && (cBuffAdd3 != 0))
              {
                memset(strTmp, 0x0, sizeof(strTmp));
                memcpy(strTmp, cBuffAdd2, cBuffAdd3 - cBuffAdd2);
                if((strncmp(strTmp,"x",1) != 0)){gsmNeighCellTowerData.nc5Cellid = GSMUtilConvHexToDec((unsigned char*)strTmp);}
                else{gsmNeighCellTowerData.nc5Cellid =0;}
              }
          }
          else
          {
            gsmNeighCellTowerData.nc5dbm =0;
            gsmNeighCellTowerData.nc5Mcc =0;
            gsmNeighCellTowerData.nc5Mnc =0;
            gsmNeighCellTowerData.nc5Lac =0;
            gsmNeighCellTowerData.nc5Cellid =0;
          }
          //Neighbouring cell Id 6 parameters
          //   *+QENG: 1,1,8,-71,32,134,134,404,20,3a9a,142a,2,12,-77,19,110,110,404,20,3a9a,142b,
          //   *       3,9,-85,35,79,79,404,20,3a9a,8c83,4,5,-95,61,39,39,404,20,3a9a,8b4e,
          //   *       5,3,-96,18,35,35,404,20,3a9a,8b4d,6,6,-101,19,15,15,404,20,3a9a,2cf9
          //NC6 - dbm rssi
          cBuffAdd2 = pucUTL_eSearchChar_Exe(',',51,cBuffAdd,&QengLinedata[QengLinelen - 1]);
          cBuffAdd2++;
          if(*cBuffAdd2 == '6')
          {
                cBuffAdd3 = pucUTL_eSearchChar_Exe (',',2, cBuffAdd2, &QengLinedata[QengLinelen - 1]);
                cBuffAdd3++;
                cBuffAdd2 = pucUTL_eSearchChar_Exe(',',1,cBuffAdd3,&QengLinedata[QengLinelen - 1]);
                if((cBuffAdd2 != 0) && (cBuffAdd3 != 0))
               {
                  memset(strTmp, 0x0, sizeof(strTmp));
                  memcpy(strTmp, cBuffAdd3, cBuffAdd2 - cBuffAdd3);
                  if((strncmp(strTmp,"x",1) != 0)){gsmNeighCellTowerData.nc6dbm = atoi(strTmp);}
                  else{gsmNeighCellTowerData.nc6dbm =0;}
               }
               //nc6 mcc
                cBuffAdd2++;
                cBuffAdd3 = pucUTL_eSearchChar_Exe (',',3, cBuffAdd2, &QengLinedata[QengLinelen - 1]);
                cBuffAdd3++;
                cBuffAdd2 = pucUTL_eSearchChar_Exe(',',1,cBuffAdd3,&QengLinedata[QengLinelen - 1]);
                if((cBuffAdd2 != 0) && (cBuffAdd3 != 0))
               {
                  memset(strTmp, 0x0, sizeof(strTmp));
                  memcpy(strTmp, cBuffAdd3, cBuffAdd2 - cBuffAdd3);
                  if((strncmp(strTmp,"x",1) != 0)){gsmNeighCellTowerData.nc6Mcc = atoi(strTmp);}
                  else{gsmNeighCellTowerData.nc6Mcc =0;}
               }
               //Nc6 - mnc
              cBuffAdd2++;
              cBuffAdd3 = pucUTL_eSearchChar_Exe (',', 1, cBuffAdd2, &QengLinedata[QengLinelen - 1]);
              if((cBuffAdd2 != 0) && (cBuffAdd3 != 0))
              {
                memset(strTmp, 0x0, sizeof(strTmp));
                memcpy(strTmp, cBuffAdd2, cBuffAdd3 - cBuffAdd2);
                if((strncmp(strTmp,"x",1) != 0)){gsmNeighCellTowerData.nc6Mnc = atoi(strTmp);}
                else{gsmNeighCellTowerData.nc6Mnc =0;}
              }
               //Nc6 - LAC
              cBuffAdd3++;
              cBuffAdd2 = pucUTL_eSearchChar_Exe (',', 1, cBuffAdd3, &QengLinedata[QengLinelen - 1]);
              if((cBuffAdd2 != 0) && (cBuffAdd3 != 0))
              {
                memset(strTmp, 0x0, sizeof(strTmp));
                memcpy(strTmp, cBuffAdd3, cBuffAdd2 - cBuffAdd3);
                if((strncmp(strTmp,"x",1) != 0)){gsmNeighCellTowerData.nc6Lac = GSMUtilConvHexToDec((unsigned char*)strTmp);}
                else{gsmNeighCellTowerData.nc6Lac =0;}
              }
              //Nc6- Cell Id
              cBuffAdd2++;
              cBuffAdd3 = pucUTL_eSearchChar_Exe (',', 1, cBuffAdd2, &QengLinedata[QengLinelen - 1]);
              if((cBuffAdd2 != 0) && (cBuffAdd3 != 0))
              {
                memset(strTmp, 0x0, sizeof(strTmp));
                memcpy(strTmp, cBuffAdd2, cBuffAdd3 - cBuffAdd2);
                if((strncmp(strTmp,"x",1) != 0)){gsmNeighCellTowerData.nc6Cellid = GSMUtilConvHexToDec((unsigned char*)strTmp);}
                else{gsmNeighCellTowerData.nc6Cellid =0;}
              }
          }
          else
          {
            gsmNeighCellTowerData.nc6dbm =0;
            gsmNeighCellTowerData.nc6Mcc =0;
            gsmNeighCellTowerData.nc6Mnc =0;
            gsmNeighCellTowerData.nc6Lac =0;
            gsmNeighCellTowerData.nc6Cellid =0;
          }

           neighCellTowerData =1;   
        }
    }
    return neighCellTowerData;
}

uint8_t GsmGetSMSIndex(uint8_t *ListSmsLinedata ,uint8_t ListSmsLinelen)
{
    uint8_t smsBuff[3];
    uint8_t smsIndex = 0;
    char *cBuffAdd1,*cBuffAdd2 =  NULL;  
     //+CMGL: 1,"REC UNREAD","+8615021012496","","2010/09/25 15:06:37+32",145,4,0,241,"+8
     //613800210500",145,27
     //Read Sender's mobile number
     if((cBuffAdd1 = strstr((const char *)ListSmsLinedata, ":")) != NULL)
     {
       if((cBuffAdd2 =  strstr((char*)ListSmsLinedata,",")) != NULL)
       {
         memset(smsBuff,0,sizeof(smsBuff));
         strncpy((char*)smsBuff,cBuffAdd1+1, cBuffAdd2 - cBuffAdd1-1);
         smsIndex = atoi((const char*)smsBuff);
         LOG_INFO(CH_GSM,"New SMS Index = %d",smsIndex);
       }
     }
     return smsIndex;
}

/**
 *   @brief ProcessCSQData processes the signal strength data
 *   @return void
 *   e.g.
 *   AT+CSQ
 *   +CSQ: 28,0 //Query the current signal strength indication is 28 and
 *   the bit error rate is 0
 *   OK
**/
void ProcessCSQData(uint8_t *CSQLinedata ,uint8_t CSQLinelen)
{
    uint8_t *cBuffAdd;
    memset(strTmp, 0x0, sizeof(strTmp));
    sprintf((char *)strTmp, "+CSQ: ");
    if (GSMStrPrefixMatch((char *)CSQLinedata,strTmp))
    {
       // Check for Strength Level
        cBuffAdd = pucUTL_eSearchChar_Exe (':', 1, &CSQLinedata[0], &CSQLinedata[CSQLinelen - 1]);

        // Check for RSSI
        if (*(cBuffAdd + 3) == ',')
        {
            gsmCellTowerData.rssi	        = *(cBuffAdd + 2) & 0x0F;   //Integer conversion
        }
        else
        {
            gsmCellTowerData.rssi			= ((*(cBuffAdd + 2) & 0x0F) * 10) + (*(cBuffAdd + 3) & 0x0F);
        }

        if((gsmCellTowerData.rssi < 98) && (gsmCellTowerData.rssi > 7))
        {
//                    ucFlag = 1;
        }
    }
    else
    {
        //ELSE MAKE IT 0 OR RESERVE THE PRV STRENGTH
//           gsmCellTowerData.rssi =0;
    }
}

cellIdStruct_st* GetServingNeighCellInfo(void)
{
  return (&gsmCellTowerData);
}

gsmNeighcellIdStruct_st* GetNeighbouringCellInfo(void)
{
   return(&gsmNeighCellTowerData);
}


//char* decimal_to_hexadecimal(int tempDecimal,uint8_t* decToHexArray)
void decimal_to_hexadecimal(int tempDecimal,char* decToHexArray)
{
    char HEXVALUE[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    char hex[65];
    
    int index, rem;
    uint16_t len =0,count =0;
    int i,j =0;
    index = 0;
    
    /* Decimal to hexadecimal conversion */
      for(count = 0; tempDecimal > 0; count++)
      {
        rem = tempDecimal % 16;

        hex[index] = HEXVALUE[rem];

        tempDecimal /= 16;

        index++;
      }
     hex[index] = '\0';

    //find the length of the string using strlen()
	len = strlen((const char*)hex);
    memset(hexData,0,sizeof(hexData));
	
	//loop through the string and print it backwards
	for(i=len-1,j=0; (i>=0) ; i--,j++)
    {
		hexData[j] =  hex[i];	
	}
    hexData[j]  = '\0';
    //copy the hex data to the array
    strncpy((char*)decToHexArray,hexData,len);
//  LOG_DBG(CH_GSM,"Hexadecimal number = %s", hexData);
//    return hexData;
}