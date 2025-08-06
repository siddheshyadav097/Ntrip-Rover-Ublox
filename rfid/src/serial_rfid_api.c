
#include "serial_rfid_api.h"
//#include "serial_fuel_port.h"
#include "mem_config_api.h"
#include "bat_api.h"
#include "digio_api.h"
#include "debug_log.h"
#include "buzzer_app.h"
#include "led_api.h"
#include "fota_api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//callback to handle the response recived from the rfid
rfidRespCb_et RfidRespCb(rfidReaderCmdCode_et cmdCode, uint8_t *lineBuff, uint16_t len);

// the current init command is in progress
rfidCMD_et rfidReaderCmdIndex = START_AUTO_READ;

//rfid send command state
rfidCmdState_et rfCmdState = RFID_CMD_SEND;

//rfid command handler set state
rfidHandlerCmdState_et rfHandlerState = READER_CMD_WAIT;

//rfid reader power handler to switch on/off the pwr supply for the  reader
rdrPwrState_et readerPowerState = READER_OFF;

uint8_t getRfVtgCnt = 0;

// retry count to be incremented when cmd failure is received
uint8_t rfCmdRetryCount = 0;

// tick timer used for waiting during retry interval
uint32_t rfRetryCmdTick = 0;

//struct where rfid readers configurations are stored
rfidConfigStruct_st RfConfigStruct ={0};

//uint8_t epcArrayIndex = 0;

//status of the response received from the rfid
//uint8_t readStatus = 0; 

//time stamp to send the scan inventory tags command to the reader
uint32_t getRfTagStartTick = 0;

//counter for number of tags available in the rfid stack to sent
uint8_t epcTagCntr = 0;

//buffer to copy the tags received from the reader
uint8_t epcList[1024]= {0};

//epcId Buff to copy the single tag from epcList buff
uint8_t epcId[15]= {0};

//variable to copy the value of num of tags received from the reader
uint8_t tagCount = 0;

//index to copy  the single data from the epcList buff
uint8_t lenIndex  =0;
uint8_t epcLen =0;

//variable to store the previous tag count where more num of tags are received from the reader
uint8_t prevTagCount =0;
uint8_t totalTagCnt = 0;

//flag is set the 0x03 is re-cmd received from the buffer as more data is pending to be received from the reader
uint8_t flagMoretag = 0;
uint8_t rfCnt =0;

//response int value which stores the result for duplicate tag search
int  dupIndex = 0;

uint8_t testTagIndex =0;

float rfCurrLowrTh , rfCurrUpperTh = 0;

//array of struct to store the EPC id Tags
epcId_st EpcIdStruct[MAX_EPC_IDS_AVAILABLE]; 

//default states for veh batt status
//inputState_e prevVehIgnState =  INPUT_OFF;
//inputState_e currVehIgnState =  INPUT_OFF;

//reader power sense tick
uint32_t rdrSenseVehBattTick =0;

//flag to monitor and then only check the cmds when the reader is online
uint8_t readerOnline = 0;

DigitalInput_t* digitalInputState;
SupplyInfo_st* rfVoltagePtr;
rfReaderPwrTh_st* memRfReaderThPtr;

uint32_t rfidLedStarttime = 0;

uint8_t firstTimeHighFlag  = 0;
uint8_t firstTimeLowFlag   = 0;

//rfidLedState_et rdrLedSetState = LED_IDLE_STATE;
rfidLedState_et rfidLedState = LED_IDLE_STATE;

//06 00 22 4E 00 E5 61 
//11-Aug-18 13:44:30.490 [RX] - 05 00 22 00 F5 7D 

static char hexchars[] = "0123456789ABCDEF";

                                 /*Format of RFID Command
                                 * Len Adr  Cmd  Data[] LSB-CRC16	MSB-CRC16 */
//uint8_t ScanTagsBuff[5]        = {0x04 ,0x00, 0x01, 0xDB, 0x4B};
//uint8_t SetReaderBaudrate[6]   = {0x05 ,0x00, 0x28, 0x05, 0x28, 0xD7}; //0x05 in data means the baudrate is 57600bps
//uint8_t SetReaderAddress[6]    = {0x05 ,0x00, 0x24, 0x00, 0x25, 0x29}; //0x00 in data means readers address to set is 0x00
//uint8_t GetReaderInfoBuff[5]   = {0x04 ,0x00, 0x21, 0xD9, 0x6A};
//uint8_t SetReaderPower[6]      = {0x05 ,0x00, 0x2F, 0x1E, 0x72, 0x34}; //0x1E in Data means Reader pwr - 30 dBm
//uint8_t GetReaderWorkMode[5]   = {0x04 ,0x00, 0x36, 0xE7, 0x0E};
//uint8_t SetReadersWorkMode[11] = {0x0A ,0x00, 0x35, 0x00, 0x00, 0x04, 0x02, 0x01 ,0x01,0xC4,0x43};
//uint8_t SetReadersScanTime[6]  = {0x05 ,0x00, 0x25, 0x05,0x50,0x67};  //0x05 in Data means Inventory Scan Time to serach Tags is 500 msec
//uint8_t SetReadersRegion[7]    = {0x06, 0x00, 0x22, 0x4E, 0x00,0xE5, 0x61};  //The host sends this command to change the current region of the reader.


/***************************Phychip 4s commands*********************
                            Format of RFID Command
                                 
-----------------------------------------------------------------------------------------------------|                                 
Preamble  |                       Header 4   byte                  |Payload      End Mask     CRC-16 |
----------|--------------------------------------------------------|------------|-----------|----------|
   1 byte |                 |               |                      |            |           |          |
          |   1 byte        |     1 byte    |      2 byte          |  N Bytes   | 1Byte     | 2 byte   |
          |   Message Type  |      Code     |     Payload Length   |            |           |          |
          |                 |               | (unit of byte : N)   |            |           |          |
--------------------------------------------------------------------------------------------------------
*********************************************************************/
            
uint8_t SetSystemReset[8]             = {0xBB, 0x00, 0x08, 0x00, 0x00, 0x7E, 0x7A , 0x11};
uint8_t GetAntiCollisionMode[8]       = {0xBB, 0x00, 0x34, 0x00, 0x00, 0x7E, 0x68, 0x4D};
uint8_t GetTypeCA_IQueryParameters[8] = {0xBB, 0x00, 0x0D, 0x00, 0x00, 0x7E, 0xB7, 0xD3};
//uint8_t GetReaderInformationModel[9]  = {0xBB, 0x00, 0x03, 0x00, 0x01, 0x00, 0x7E, 0x48, 0xAB}; //Get Model (0x00) Type
uint8_t GetReaderInformationSN[9]     = {0xBB, 0x00, 0x03, 0x00, 0x01, 0x01, 0x7E, 0x7B, 0x9A};
uint8_t GetReaderPower[8]             = {0xBB, 0x00, 0x15, 0x00, 0x00, 0x7E, 0x29, 0xB7};
uint8_t SetReaderPower[10]            = {0xBB, 0x00, 0x16, 0x00, 0x02, 0x01, 0x0E, 0x7E, 0x00, 0xA5};
uint8_t GetRegion[8]                  = {0xBB, 0x00, 0x06, 0x00, 0x00, 0x7E, 0xA9, 0xCC};  //europe
uint8_t SetRegion[9]                  = {0xBB, 0x00, 0x07, 0x00, 0x01, 0x31, 0x7E,0xF7, 0x09};
uint8_t GetFHLBTParam[8]              = {0xBB, 0x00, 0x13, 0x00, 0x00, 0x7E, 0x0E, 0x2E};
uint8_t StartAutoRead2[13]            = {0xBB, 0x00, 0x36, 0x00, 0x05, 0x02, 0x00, 0x00, 0x00, 0x00, 0x7E, 0x22, 0x0D};
uint8_t StopAutoRead2[8]              = {0xBB, 0x00, 0x37, 0x00, 0x00, 0x7E, 0xF3, 0x91};



//"Max Frequency – 867.9 MHz
//Min Frequency – 865.1 MHz"


uint8_t RfidTestTagEpc[5] = {0x55 ,0x66 ,0x77 ,0x88, };   //test tag epc id to display the gps and gsm status on the led
 

rfidReaderCmd_st rfidReaderCmdList[] = {
  
  {SetSystemReset,               8,3,1000,100,0},      //set system reset 
  {GetAntiCollisionMode,         8,3,1000,100,0},
  {GetTypeCA_IQueryParameters,   8,3,1000,100,0},
//  {GetReaderInformationModel,    9,3,1000,100,0},
  {GetReaderInformationSN,       9,3,1000,100,0},
  {GetReaderPower,               8,3,1000,100,0},      //get reader power
  {SetReaderPower,               10,3,1000,100,0},     //set reader power
  {GetRegion,                    8,3,1000,100,0},
  {SetRegion,                    9,3,1000,100,0},
  {GetFHLBTParam,                8,3,1000,100,0},
  {StartAutoRead2,               13,3,1000,100,0},
  {StopAutoRead2,                8,3,1000,100,0},
};

rfidStateMachine_et  rfidState = RFID_POWER_RESET;

uint32_t readerPowerStateStartTime = 0;

uint8_t rfReadTagStatus = 0;

uint8_t rfCommandStatus = 0;
static void RfidToggleLED();
static void RfidLedHandler();

/******** RFID DATA HANDLERS AND ITS CB STARTS HERE ***----------*/

/**
 *  @brief  :RfidInit()- This function handles the initialization of the rfid ring
 *           buffers and rfid serial uart port i.e. UART5 
 *           This function is called in the packet_api. c in InitPacketConfig() function.
 *  @param  : void
 *  @return : void         
 */
void RfidInit(void)
{
  RfidCmdHandlerInit();
  RfidGetReaderPwrTh();                   //update the thresholds for rfid reader pwr vtg
  RfidSetCurrentThresholds();
}

/**
 *  @brief  :RfidReaderHandler()- This function is called in the while(1) loop
 *           of the main file. It calls RfidStateMachineHandler() - which handles the statemachine 
 *           for rfid , RfidResponseHandler() which is called in the rfid_data_handler.c which handles 
 *           the data received from the rfid port,RfidEpcDataClearHandler() - which clears the stored epc id's 
 *           based on their timestamp. 
 *  @param  : void
 *  @return : void         
 */
void RfidReaderHandler(void)
{
  RfidPowerStateCheckHandler(); 
  
  if(readerOnline)  // If Vehicles Ignition is ON then only reader will be online
  
	    if(FotaGetState() == FOTA_WAIT_FOR_START)
        {
      RfidResponseHandler();       //this handler handles the data received from the rfid
      RfidStateMachineHandler();   //this handler handles the statemachines for rfid
      RfidEpcDataClearHandler();   //independantly clears the data received from the rfid port
      RfidLedHandler();             //handles the led blinking when noral tag as well as test tag is detected
				}
  
}

void RfidToggleLED()
{
//	if(rfidLedState==TOGGLE_STATE)
//	{
//		return;
//	}
//  Led_State_ON(RFID_LED);
//	rfidLedState = TOGGLE_STATE;
//	rfidLedStarttime = GetStartTime();
}

void RfidLedHandler()
{
//    switch(rfidLedState)
//    {
//        case TOGGLE_STATE: 
//            if(TimeSpent(rfidLedStarttime, RF_TAG_DETECT_ON_TIME)) //1 sec On
//            {
//                  rfidLedState = LED_IDLE_STATE;
//							    Led_State_OFF(RFID_LED);
//            }
//            break;
//        
//       case LED_IDLE_STATE:
//            break;
//    }
}

/**
 *  @brief  :RfidGetAvailableTagsCnt()- This function checks for num of epc tag id's 
 *           pending the EpcIdStruct which needs to be sent on the server.
 *  @param  : void
 *  @return : epcTagCntr - Num of unsent Epc Tag id's available in the buffer         
 */
uint8_t RfidGetAvailableTagsCnt(void)
{
  epcTagCntr =0;
  for(uint8_t i = 0; i < MAX_EPC_IDS_AVAILABLE ;i++)
  {
       if(EpcIdStruct[i].epcIdLength > 0)  //if epcLen is > 0 and flag whether the flag is sent on the server is not set
       {
          if(!EpcIdStruct[i].epcSentFlag)
          {
            epcTagCntr++;
          }
       }
  }
 return epcTagCntr; 
}


//finding whether the tag read is a test tag 
uint8_t RfidSearchForTestTag(uint8_t* epcId , uint8_t epcIdLen)
{
    if(memcmp(epcId,RfidTestTagEpc,epcIdLen) == 0) 
    {
       return 1;
    }
    else
    {
      return 0;
    }
  
}
/**
 *  @brief  :RfidSearchDuplicateTag()- This function checks whether the tag received from the rfid
 *           reader is already present in the EpcIdStruct.
 *  @param  : epcId -  Epc Id which needs to be searched in the EpcIdStruct.
 *          : epcIdLen - Length of the epcId
 *  @return : int  - which can be of a value of index of the array of struct where the match for this tag is found.
 *            or if no match is found it returns the value -1.
 */
int RfidSearchDuplicateTag(uint8_t* epcId , uint8_t epcIdLen)
{
  int j;
  for( j =0; j < MAX_EPC_IDS_AVAILABLE; j++) 
  {
    if(EpcIdStruct[j].epcIdLength > 0 )  //if the epc id is present at the array index
    {
        if(memcmp(EpcIdStruct[j].epcId,epcId,epcIdLen) != 0)  
        {
           continue;
        }
        /* this tag is a match */
        break;
    }
  }
  return (j < MAX_EPC_IDS_AVAILABLE) ? j : -1;  
}

/**
 *  @brief  :RfidEpcDataClearHandler()- This function runs independantly and keeps on scanning the EpcIdStruct.
 *          :If any of the Tags epcIdLength is greater than 0 then it checks the timestamp of that epc id 
 *           that is tag is been hold for 1 minute or not, If timestamp matches with the MAX_EPC_ID_RETAIN_TIME i.e.
 *           1 minute then it clears that Tag id from the struct.
 *  @param  : void
 *  @return : void
 */
void RfidEpcDataClearHandler(void)
{
   uint8_t rfArrayIndex = 0;
   //Check any of the epc id is sent on the server clear its epcSentFlag flag and make the timestamp to 0
   for(rfArrayIndex=0 ; rfArrayIndex < MAX_EPC_IDS_AVAILABLE  ; rfArrayIndex++ )
   {
     if(EpcIdStruct[rfArrayIndex].epcIdLength  > 0)
     {
         if((EpcIdStruct[rfArrayIndex].epcSentFlag) && \
           TimeSpent(EpcIdStruct[rfArrayIndex].epcTimeStamp,MAX_EPC_ID_RETAIN_TIME))  //if this epcid is sent on the server then reset the flag and it is retained in the buffer for 60 secs
         {
           EpcIdStruct[rfArrayIndex].epcSentFlag = 0;   //clear the epcSentFlag 
           memset(EpcIdStruct[rfArrayIndex].epcId,0,sizeof(EpcIdStruct[rfArrayIndex].epcId)); //clear the epc id
           EpcIdStruct[rfArrayIndex].epcIdLength  = 0; //clear the length of the epc id
           EpcIdStruct[rfArrayIndex].epcTimeStamp = 0; 
         }
     }
   }
}

/**
 *  @brief  :RfidGetEPCData()- This function is called from packet_api.c to get the data from the EpcIdStruct
 *           It checks for whether the respective epc id's sent flag is 0 or not.Then only it copies the epc Id
 *           and it its length in the passed struct.
 *           After copying the unsent epc id it sets the sent flag.
 *  @param  : getDataCntr -  Index to be checked in the EpcIdStruct.
 *          : RfDataPtr of type RfEpcData_st struct
 *  @return : void
 */
uint8_t RfidGetEPCData(uint8_t getDataCntr,RfEpcData_st *RfDataPtr)
{
   if(EpcIdStruct[getDataCntr].epcIdLength > 0)  //If the length of the epc id > 0
   {
      if(!EpcIdStruct[getDataCntr].epcSentFlag)  //check the flag whether the sent flag is 0 or not
      {
        memcpy(RfDataPtr->RfepcId,EpcIdStruct[getDataCntr].epcId,EpcIdStruct[getDataCntr].epcIdLength);
        RfDataPtr->RfepcIdLen = EpcIdStruct[getDataCntr].epcIdLength;
        EpcIdStruct[getDataCntr].epcSentFlag = 1;
        return  1;
      }
   }
   return 0;
}

/**
 *  @brief  :RfidAddTagToStruct()- This function adds the new epc id's received from the rfid reader.
 *          After adding the epc id in the struct the timestamp for that epc id is updated and resets the 
 *          sent flag for that epc id.
 *  @param  : epcId    -  Epc Id to be added in the EpcIdStruct.
 *          : epcIdLen -  Length of the epcId
 *  @return : void
 */
void RfidAddTagToStruct(uint8_t* epcId , uint8_t epcIdLen)
{
  char epcStr[30];      
  uint8_t rfArrayIndex = 0;

   if(epcIdLen <= MAX_EPC_ID_LENGTH_ASCII)
   {
       for(rfArrayIndex =0 ; rfArrayIndex < MAX_EPC_IDS_AVAILABLE ; rfArrayIndex++)  //check for empty index where we can insert the epc id in the array of struct
       {
           if(EpcIdStruct[rfArrayIndex].epcIdLength == 0) //If epc id length is 0 then the array is clear
           {
               memset(EpcIdStruct[rfArrayIndex].epcId,0,sizeof(EpcIdStruct[rfArrayIndex].epcId));
               memset(epcStr,0,sizeof(epcStr));
               memcpy(EpcIdStruct[rfArrayIndex].epcId,epcId,epcIdLen);         //get the Epc id of the received Tag
               EpcIdStruct[rfArrayIndex].epcIdLength      = epcIdLen;          //actual length of the Epc Id
               EpcIdStruct[rfArrayIndex].epcTagArrayIndex = rfArrayIndex;      //array index at which the tag is stored
               EpcIdStruct[rfArrayIndex].epcTimeStamp     = GetStartTime();    //timestamp when the tag is received
               EpcIdStruct[rfArrayIndex].epcSentFlag      = 0;                 //epcId Sent Flag -  This flag is chnaged at api side, when the tag is sent or stored the memory 
               
               RfidBytesToHex(EpcIdStruct[rfArrayIndex].epcId,epcIdLen,epcStr); //only to see the tag on debug remove it later
               LOG_INFO(CH_RFID,"EPC ID : %s , Tag Index : %d",epcStr , EpcIdStruct[rfArrayIndex].epcTagArrayIndex);   //remove

               break;  //break the for loop inserting the tag into the array
           }
       }
   }
   else   //should never come here as epc id length id is always max 15 Bytes
   {
       LOG_INFO(CH_RFID,"EPC ID Len More than 15");
   }
}

/**
 *  @brief  :RfidGetTags()- This function is called from the RfidRespCb() whenevr response for
 *           Scan Tags Command is received/ or any other command related to the reader is sent.
 *           Status of the read tag command :
 *           
 *  @param  : lineBuff - Data received from the rfid port
 *          : len - Length of the data received from the rfid port
 *  @return : 1 if succefully matched with the status codes
 *          : 0  if any error code is received from the reader.
EPC id will be received as follows for phychip

BB022300123000E2000017941801762580129884833F317E49CE
 */
   
uint8_t getRfidReadTagStatus(void)
{
    return rfReadTagStatus;
}

 uint8_t RfidGetTags(uint8_t* lineBuff,uint16_t len)
 {   
   lenIndex  =0;
   
   if( (lineBuff[2] == 0x22) && (lineBuff[1] == 0x02))  //status of epc id Notification
   {
//       RfidToggleLED();  //Toggle the output buzzer and led when a valid tag is received
         memset(epcList,0,sizeof(epcList));
         memcpy(epcList,(char*)&lineBuff[7],sizeof(epcList));
         
         if(RfidGetPayloadLen() == 0x0E) //as the length is always PC 0x30 00 plus 12 bytes of epc data
         {
          epcLen = RfidGetPayloadLen() - 2;
         }
           memset(epcId,0,sizeof(epcId));
           memcpy(epcId,(char*)&epcList[0],epcLen);
           
//           testTagIndex = RfidSearchForTestTag(epcId,epcLen);   //check whether it is a test tag which is used to check the rfid,gps, gsm status
//           if(testTagIndex == 1)
//           {
//              testTagIndex = 0;
//              if(GetRfTestTagDetectState() == WAIT_FOR_RF_TEST_TAG)
//              {
//                  RfidUnitLedSetState(VALID_RFID_TEST_TAG_DETECTED);
//              }
//           }
//           else//here check if it is a QDnet's Bin tag then only toggle the LED
//           {
                //If it is not a rfid test tag then only toggle the led
               RfidToggleLED();  //Toggle the output buzzer and led when a valid tag is received
				       SetBuzzerBeep(SINGLE_BEEP);
//           }
           dupIndex    = RfidSearchDuplicateTag(epcId,epcLen); //search for duplicate tag entry in the struct array
           if(dupIndex == -1)
           {
             LOG_INFO(CH_RFID,"No Duplicate Tag Found");    //No duplicate found
             RfidAddTagToStruct(epcId,epcLen);              //add the tag in the array struct
            
           }
           else
           {
//             LOG_INFO(CH_RFID,"Duplicate Tag Found Skip this Tag"); 
           }
      return 1;
   }
   //any of the error code is received while reading the tag
   else 
   {
     return 0;
   }
  
}

//struct where rfid readers configurations are stored
rfidConfigStruct_st* RfidGetReaderConfigrations(void)
{
   return (&RfConfigStruct);
}

//uint8_t RfidCheckReadersConfig(void)
//{
//   if((RfConfigStruct.readersAdd ==  0x00)   && \
//     (RfConfigStruct.rfPower    ==  0x1E)  && \
//       (RfConfigStruct.rfScanTime ==  0x05)  && \
//       (RfConfigStruct.rfMaxFrequency  == 0x4E) && \
//       (RfConfigStruct.rfMinFrequency  == 0x00))
//   {
//     return 1;
//   }
//   
//   return 0;
//
//
//}

/**
 *  @brief  :RfidRespCb()- Callback  to process the command response and data received from the rfid.
 *  @param  : cmdCode of type rfidReaderCmdCode_et to process the data according to the command for which resp is received
            : lineBuff - Data received from the rfid port
 *          : len - Length of the data received.
 *  @return : of type rfidRespCb_et RFID_RESP_CB_OK_COMPLETE - If response is processed successfully
 *           RFID_RESP_CB_WAIT -  If more data is pending to be processed.
 *           RFID_RESP_CB_ERROR_COMPLETE - If failed  to get the data from the reader i.e. timeout  or error code is received.
 */
rfidRespCb_et RfidRespCb(rfidReaderCmdCode_et cmdCode, uint8_t *lineBuff, uint16_t len)
{
       uint8_t readTagStatus = 0;
       rfidRespCb_et rfRespRetVal = RFID_RESP_CB_WAIT;
      
   
      switch(cmdCode)
      {
          case  SET_SYSTEM_RESET_CMD: 
          case  SET_READER_POWER_CMD:
          case  SET_REGION_CMD:
          case  START_AUTO_READ_CMD:
          case  STOP_AUTO_READ_CMD:
                if(lineBuff[5] == 0x00)  //status bit is success
                {
                  rfRespRetVal  = RFID_RESP_CB_OK_COMPLETE;
                  rfCmdState    = RFID_CMD_SUCCESS;
                }
                else   //if command response status is not success
                {
                  rfRespRetVal  = RFID_RESP_CB_ERROR_COMPLETE;
                  rfCmdState    = RFID_CMD_FAILURE;
                }
          break;
        
        
      case  GET_ANTI_COL_MODE_CMD:
          if(lineBuff[4] == 0x04)  //status bit is success
          {
            rfRespRetVal  = RFID_RESP_CB_OK_COMPLETE;
            rfCmdState    = RFID_CMD_SUCCESS;
          }
          else   //if command response status is not success
          {
            rfRespRetVal  = RFID_RESP_CB_ERROR_COMPLETE;
            rfCmdState    = RFID_CMD_FAILURE;
          }
         
      break;
      
      case  GET_TYPE_CA_QUERY_CMD:
         if((lineBuff[5] == 0xC1) && (lineBuff[6] == 0x20))  //status bit is success
          {
            rfRespRetVal  = RFID_RESP_CB_OK_COMPLETE;
            rfCmdState    = RFID_CMD_SUCCESS;
          }
          else   //if command response status is not success
          {
            rfRespRetVal  = RFID_RESP_CB_ERROR_COMPLETE;
            rfCmdState    = RFID_CMD_FAILURE;
          }
      break;
         
        
//      case  GET_READER_INFO_MODE_CMD:
      case  GET_READER_INFO_SN_CMD:
      case  GET_FHLBT_PARAM_CMD:
          if(RfidGetPayloadLen() > 0)  //status bit is success
          {
             //copy the data here
            rfRespRetVal  = RFID_RESP_CB_OK_COMPLETE;
            rfCmdState    = RFID_CMD_SUCCESS;
          }
          else   //if command response status is not success
          {
            rfRespRetVal  = RFID_RESP_CB_ERROR_COMPLETE;
            rfCmdState    = RFID_CMD_FAILURE;
          }
        break;
     
      case  GET_READER_POWER_CMD:
        if((lineBuff[5] == 0x01) && (lineBuff[6] == 0x0E))  //status bit is success
        {
          rfRespRetVal  = RFID_RESP_CB_OK_COMPLETE;
          rfCmdState    = RFID_CMD_SUCCESS;
        }
        else   //if command response status is not success
        {
          rfRespRetVal  = RFID_RESP_CB_ERROR_COMPLETE;
          rfCmdState    = RFID_CMD_FAILURE;
        }
      break;
        
      
      case  GET_REGION_CMD:
          if(lineBuff[5] == 0x31)  //Europe (0x31)
          {
            rfRespRetVal  = RFID_RESP_CB_OK_COMPLETE;
            rfCmdState    = RFID_CMD_SUCCESS;
          }
          else   //if command response status is not success
          {
            rfRespRetVal  = RFID_RESP_CB_ERROR_COMPLETE;
            rfCmdState    = RFID_CMD_FAILURE;
          }
      break; 
      
       case  START_AUTO_READ_NOTIFICATION:  
         readTagStatus =  RfidGetTags(lineBuff,len);
         if(readTagStatus)
         { 
          rfRespRetVal  = RFID_RESP_CB_OK_COMPLETE;
          rfCmdState    = RFID_CMD_SUCCESS;
         }
         else
         {
           rfRespRetVal  = RFID_RESP_CB_ERROR_COMPLETE;
           rfCmdState    = RFID_CMD_FAILURE;
         }
        break;
        
        case RF_RESP_TIMEOUT:
        rfRespRetVal  = RFID_RESP_CB_ERROR_COMPLETE;
        rfCmdState    = RFID_CMD_FAILURE;
        break;
      }
      return rfRespRetVal;
}

rfidStateMachine_et GetRfidReaderSendState(void)
{
   return rfidState;
}

/**
 *  @brief  :RfidReaderSetState()- This function is handled by the RfidStateMachineHandler() to set the state of the reader.
 *  @param  : rfidStateMachine_et - state of the reader to set
 *  @return : void
 */
void RfidReaderSetState(rfidStateMachine_et state)
{    
    rfidState = state;
    rfCmdRetryCount = 0;
    if(rfidState != RFID_READER_WAIT)
    {
       rfHandlerState = READER_CMD_IN_PROGRESS;
    }
    switch(rfidState)
    {
       case RFID_POWER_RESET:
          LOG_INFO(CH_RFID,"State - RFID_POWER_RESET"); //rfid power reset state
          break;
          
       case RFID_READER_WAIT:    //this state is to wait for 100 msec and then again the set reader in the RFID_GET_TAGS state
          break;
          
       case RFID_SET_SYSTEM_RESET://rfid set reader reset
        LOG_INFO(CH_RFID,"State - RFID_SET_SYSTEM_RESET");
        rfidReaderCmdIndex = SET_SYSTEM_RESET;
        rfidReaderCmdList[rfidReaderCmdIndex].flagCmdSuccess = 0;
        break;
        
       case RFID_GET_ANTI_COLL_MODE:
         LOG_INFO(CH_RFID,"State - RFID_GET_ANTI_COLL_MODE");
         rfidReaderCmdIndex = GET_ANTI_COLL_MODE;
         rfidReaderCmdList[rfidReaderCmdIndex].flagCmdSuccess = 0;
         break;
         
       case RFID_GET_C_AI_PARAM:
         LOG_INFO(CH_RFID,"State - RFID_GET_C_AI_PARAM");
         rfidReaderCmdIndex = GET_C_AI_PARAM;
         rfidReaderCmdList[rfidReaderCmdIndex].flagCmdSuccess = 0; 
         break;
          
//        case RFIS_GET_RDR_INFO:  //set readers baud rate - 57600bps
//          LOG_INFO(CH_RFID,"State - RFIS_GET_RDR_INFO");
//          rfidReaderCmdIndex = GET_RDR_INFO;
//          rfidReaderCmdList[rfidReaderCmdIndex].flagCmdSuccess = 0;
//          break;
          
        case RFID_GET_RDR_INFO_SN:
          LOG_INFO(CH_RFID,"State - RFID_GET_RDR_INFO_SN");
          rfidReaderCmdIndex = GET_RDR_INFO_SN;
          rfidReaderCmdList[rfidReaderCmdIndex].flagCmdSuccess = 0;
          break;
          
        case RFID_GET_RDR_POWER: //Get Tx Power level - 01 0E - 270 DBM
          LOG_INFO(CH_RFID,"State - RFID_GET_RDR_POWER");
          rfidReaderCmdIndex = GET_RDR_POWER;
          rfidReaderCmdList[rfidReaderCmdIndex].flagCmdSuccess = 0;
          break;
          
       case RFID_SET_RDR_POWER: //Set TX Power level  - 270 dbm
          LOG_INFO(CH_RFID,"State - RFID_SET_RDR_POWER");
          rfidReaderCmdIndex = SET_RDR_POWER;
          rfidReaderCmdList[rfidReaderCmdIndex].flagCmdSuccess = 0;
          break;
          
        case RFID_GET_REGION: // Get Region
          LOG_INFO(CH_RFID,"State - RFID_GET_REGION");
          rfidReaderCmdIndex = GET_REGION;
          rfidReaderCmdList[rfidReaderCmdIndex].flagCmdSuccess = 0;
          break;
          
        case RFID_SET_REGION: //Set Region
          LOG_INFO(CH_RFID,"State - RFID_SET_REGION");
          rfidReaderCmdIndex = SET_REGION;
          rfidReaderCmdList[rfidReaderCmdIndex].flagCmdSuccess = 0;
          break;
          
        case RFID_GET_FHLBT_PARAM:
//          R>BB 01 13 00 0B 01 7C 00 64 00 05 FD 1C 01 00 00 7E 445E
//
//          Read Time = 380 [ms]
//          Idle Time = 100 [ms]
//          CW Sense Time = 5 [ms]
//          LBT RF Level = -74 [dBm]
//          Frequency Hopping(Only) Enable
          
          LOG_INFO(CH_RFID,"State - RFID_GET_FHLBT_PARAM");
          rfidReaderCmdIndex = GET_FHLBT_PARAM;
          rfidReaderCmdList[rfidReaderCmdIndex].flagCmdSuccess = 0;
        break;
        
            
      case RFID_START_AUTO_READ:      //rfid start reading tags command
        LOG_DBG(CH_RFID,"State - RFID_START_AUTO_READ");
        rfidReaderCmdIndex = START_AUTO_READ;
        rfidReaderCmdList[rfidReaderCmdIndex].flagCmdSuccess = 0;
        break;
        
       case RFID_STOP_AUTO_READ:
          LOG_DBG(CH_RFID,"State - RFID_STOP_AUTO_READ");
          rfidReaderCmdIndex = STOP_AUTO_READ;
          rfidReaderCmdList[rfidReaderCmdIndex].flagCmdSuccess = 0;
      break;
          
        default:
          break;
    }   
}

void RfidSetCurrentThresholds(void)
{
   rfCurrLowrTh  = memRfReaderThPtr->rfRdrLowerTh_24V;
   rfCurrUpperTh = memRfReaderThPtr->rfRdrUpperTh_24V;
}

void RfidGetReaderPwrTh(void)
{
    //get the main ip supply voltage on the power up
     memRfReaderThPtr = GetRfidPwrThValues(); //Get the default stored values from the memory
}

void RfidApiDataReset(void)
{
    rfCmdState = RFID_CMD_SEND;
    rfHandlerState = READER_CMD_WAIT;
    rfidState = RFID_POWER_RESET;
    rfReadTagStatus = 0;
    rfCmdRetryCount = 0;  
}

/**
 *  @brief  :SetReaderPwrState()- This function is handled by the RfidReaderPwrHandler() to set the power state of the reader.
 *  @param  : rdrPwrState_et - power state of the reader to set
 *  @return : void
 */

static void SetReaderPwrState(rdrPwrState_et state)
{
//    if(state == readerPowerState)
//    {
//        return;
//    }
//    readerPowerState = state;
//    
//    switch(readerPowerState)
//    {
//        case READER_ON:
//            LOG_DBG(CH_RFID,"***************10 Seconds over READER ON***************");
//				    SetBuzzerBeep(BEEP_ON);
//            SerialControlConfig(0);   //inverted logic 
//            readerOnline = 1;
//            RfidDataHandlerReset();
//            RfidApiDataReset();
//          break;
//          
//       case  READER_OFF:
//            LOG_DBG(CH_RFID,"***************120 Seconds over READER OFF***************");
//            SerialControlConfig(1);
//            readerOnline = 0;
//            RfidDataHandlerReset();
//            RfidApiDataReset();
//            rfidLedState = LED_IDLE_STATE;
//            //writeOutput1PinState(0);
//			      HAL_GPIO_WritePin(TEST_TAG_DETECT_PORT,TEST_TAG_DETECT_PIN1,GPIO_PIN_RESET);
//         break;
//    }
}

void RfidPowerStateCheckHandler(void)
{
    rfVoltagePtr = GetSupplyInfo(); //get the current main input volatge
    
    //Set the rfid reader ON/OFF Voltage based on the main input voltage
    if(rfVoltagePtr->mainIPVolt >= 18 && rfVoltagePtr->mainIPVolt <= 30)
    {
      rfCurrLowrTh  = memRfReaderThPtr->rfRdrLowerTh_24V;
      rfCurrUpperTh = memRfReaderThPtr->rfRdrUpperTh_24V;
    }
    else if(rfVoltagePtr->mainIPVolt >= 6 && rfVoltagePtr->mainIPVolt <= 17)
    {
      rfCurrLowrTh  = memRfReaderThPtr->rfRdrLowerTh_12V;
      rfCurrUpperTh = memRfReaderThPtr->rfRdrUpperTh_12V;
    }
//    else
//    {
//       rfCurrLowrTh  = 12.5;//memRfReaderThPtr->rfRdrLowerTh_12V;
//       rfCurrUpperTh = 12.6;//memRfReaderThPtr->rfRdrUpperTh_12V;
//    }
    
	switch(readerPowerState)
	{
	     case(READER_ON):	
              if(rfVoltagePtr->mainIPVolt <= rfCurrLowrTh )
                {
                    if(firstTimeLowFlag == 1)
                    {
                        firstTimeLowFlag = 0;
                        readerPowerStateStartTime = GetStartTime();
											  SetReaderPwrState(READER_OFF); //put off the reader immediately dont wait for 2 minutes
                    }
                    else if(TimeSpent(readerPowerStateStartTime, memRfReaderThPtr->ReaderOnToOffTime))
                    {
                        //SetReaderPwrState(READER_OFF);
                    }
                }
                else
                {
                    firstTimeLowFlag = 1;
                }
           break;
			
	case(READER_OFF):	
              if(rfVoltagePtr->mainIPVolt >= rfCurrUpperTh)
                {
                    if(firstTimeHighFlag == 1)
                    {
                        firstTimeHighFlag = 0;
                        readerPowerStateStartTime = GetStartTime();
                    }
                    else if(TimeSpent(readerPowerStateStartTime, memRfReaderThPtr->ReaderOffToOnTime))
                    {
                        SetReaderPwrState(READER_ON);
                    }
                }
                else
                {
                    firstTimeHighFlag = 1;
                }
            break;
	}
	
}
/**
 *  @brief  :RfidStateMachineHandler()- This function handles the change in the state of the reader
 *  @param  : void
 *  @return : void
 */
void RfidStateMachineHandler(void)
{
    rfidHandlerCmdState_et rfState;
    switch(rfidState)
    {
        case RFID_POWER_RESET:
             RfidReaderSetState(RFID_SET_SYSTEM_RESET);//as no power reset is done now get the readers working mode at the start
        break;
        
        case RFID_SET_SYSTEM_RESET:
             rfState = RfidCommandHandler();
             if(rfState == READER_CMD_SUCCESS)
             {
               RfidReaderSetState(RFID_GET_ANTI_COLL_MODE);
             }
             else if(rfState == READER_CMD_FAIL)
             {
               RfidReaderSetState(RFID_GET_ANTI_COLL_MODE);  //RFID_POWER_RESET
             }     
        break;
        
        case  RFID_GET_ANTI_COLL_MODE:
             rfState = RfidCommandHandler();
             if(rfState == READER_CMD_SUCCESS)
             {
               RfidReaderSetState(RFID_GET_C_AI_PARAM);
             }
             else if(rfState == READER_CMD_FAIL)
             {
               RfidReaderSetState(RFID_GET_C_AI_PARAM);  //RFID_POWER_RESET
             }   
        break;
          
        case RFID_GET_C_AI_PARAM:
          rfState = RfidCommandHandler();
         if(rfState == READER_CMD_SUCCESS)
         {
           RfidReaderSetState(RFID_GET_RDR_INFO_SN);
         }
         else if(rfState == READER_CMD_FAIL)
         {
           RfidReaderSetState(RFID_GET_RDR_INFO_SN);  //RFID_POWER_RESET
         }   
       break;
       
//         case RFIS_GET_RDR_INFO:
//          rfState = RfidCommandHandler();
//         if(rfState == READER_CMD_SUCCESS)
//         {
//           RfidReaderSetState(RFID_GET_RDR_INFO_SN);
//         }
//         else if(rfState == READER_CMD_FAIL)
//         {
//           RfidReaderSetState(RFID_GET_RDR_INFO_SN);
//         } 
//        break;
         
       case RFID_GET_RDR_INFO_SN:
          rfState = RfidCommandHandler();
         if(rfState == READER_CMD_SUCCESS)
         {
           RfidReaderSetState(RFID_GET_RDR_POWER);
         }
         else if(rfState == READER_CMD_FAIL)
         {
           RfidReaderSetState(RFID_GET_RDR_POWER);
         } 
       break;
       
      case RFID_GET_RDR_POWER:
        rfState = RfidCommandHandler();
         if(rfState == READER_CMD_SUCCESS)
         {
           RfidReaderSetState(RFID_GET_REGION);
         }
         else if(rfState == READER_CMD_FAIL)
         {
           RfidReaderSetState(RFID_SET_RDR_POWER);
         } 
      break;
      
     case RFID_SET_RDR_POWER:
         rfState = RfidCommandHandler();
         if(rfState == READER_CMD_SUCCESS)
         {
           RfidReaderSetState(RFID_GET_REGION);
         }
         else if(rfState == READER_CMD_FAIL)
         {
           RfidReaderSetState(RFID_GET_REGION);
         } 
       break;
       
       
     case  RFID_GET_REGION:
         rfState = RfidCommandHandler();
         if(rfState == READER_CMD_SUCCESS)
         {
           RfidReaderSetState(RFID_GET_FHLBT_PARAM);
         }
         else if(rfState == READER_CMD_FAIL)
         {
           RfidReaderSetState(RFID_SET_REGION);
         } 
       break;
       
    case RFID_SET_REGION:
          rfState = RfidCommandHandler();
         if(rfState == READER_CMD_SUCCESS)
         {
           RfidReaderSetState(RFID_GET_FHLBT_PARAM);
         }
         else if(rfState == READER_CMD_FAIL)
         {
           RfidReaderSetState(RFID_GET_FHLBT_PARAM);
         } 
      break;
      
    case RFID_GET_FHLBT_PARAM:
          rfState = RfidCommandHandler();
         if(rfState == READER_CMD_SUCCESS)
         {
           RfidReaderSetState(RFID_START_AUTO_READ);
         }
         else if(rfState == READER_CMD_FAIL)
         {
           RfidReaderSetState(RFID_STOP_AUTO_READ);
         } 
      break;
       
       case RFID_READER_WAIT:
         if(TimeSpent(getRfTagStartTick,SCAN_RF_TAG_TIME)) //SCAN_RF_TAG_TIME - 100 msec - elapsed then again check for tags
         {
            //RfidReaderSetState(RFID_GET_TAGS);
            getRfTagStartTick =  GetStartTime();
         }
         break;
      
        case RFID_START_AUTO_READ:
         rfState = RfidCommandHandler();
         if(rfState == READER_CMD_SUCCESS)
         {
           RfidReaderSetState(RFID_READER_WAIT);  //change the reader state to the wait state after reading the tags
           getRfTagStartTick = GetStartTime();
         }
         else if(rfState == READER_CMD_FAIL )
         {
           RfidReaderSetState(RFID_STOP_AUTO_READ);  //if failure in reading the tag then stop reading and start reading again
         }
        break;
          
       case RFID_STOP_AUTO_READ:
          rfState = RfidCommandHandler();
         if(rfState == READER_CMD_SUCCESS)
         {
           RfidReaderSetState(RFID_START_AUTO_READ);
         }
         else if(rfState == READER_CMD_FAIL)
         {
           RfidReaderSetState(RFID_START_AUTO_READ);
         } 
         break;
    }   
}

/**
 *  @brief  :RfidCommandHandler()- This function sends the command to the reader and based to response 
 *           received returns handlerstate to the statemachine
 *  @param  : void
 *  @return : rfidHandlerCmdState_et
 *           READER_CMD_SUCCESS - If success is received from the callback.
 *           RFID_CMD_FAILURE   - If timeout or no response is received from the reader.
 */   
rfidHandlerCmdState_et RfidCommandHandler(void)
{
  switch(rfHandlerState)
    {
        case READER_CMD_WAIT:   
        break;
        
        case READER_CMD_IN_PROGRESS:
            switch(rfCmdState)
            {
                case RFID_CMD_SEND:
                    // check whether is rfid command is idle
                    if(RfidIsCmdIdle())
                    {
                        if(rfidReaderCmdList[rfidReaderCmdIndex].flagCmdSuccess == 0)
                        {
                            // send the at command
                            SendDataToRfidPort(rfidReaderCmdList[rfidReaderCmdIndex].cmd,rfidReaderCmdList[rfidReaderCmdIndex].cmdLen,
                                    rfidReaderCmdList[rfidReaderCmdIndex].cmdTimeoutInMs,RfidRespCb);
                            rfCmdState = RFID_WAIT_REPLY;
                        }
                        else
                        {
                            rfCmdState = RFID_CMD_SUCCESS;
                        }
                    }
                break;
                
                case RFID_WAIT_REPLY:
                    // the callback function will change the state of the rfHandlerState
                break;
                
                case RFID_CMD_SUCCESS:
                    LOG_INFO(CH_RFID,"RFID cmd %d Success",rfidReaderCmdIndex);
                    rfidReaderCmdList[rfidReaderCmdIndex].flagCmdSuccess = 1;
                    rfCmdRetryCount = 0;                    
                    rfHandlerState = READER_CMD_SUCCESS;
                    rfCmdState = RFID_CMD_SEND;
                break;
                
                case RFID_CMD_FAILURE:
                    rfCmdRetryCount++;
                    if(rfCmdRetryCount < rfidReaderCmdList[rfidReaderCmdIndex].numRetries)
                    {
                        rfRetryCmdTick = GetStartTime();
                        rfCmdState = RFID_CMD_RETRY_WAIT; 
                    }
                    else
                    {
                        // maximum retry is done so set the cmd to failure
                        rfHandlerState = READER_CMD_FAIL;
                        rfCmdState = RFID_CMD_SEND;
                    }
                break;
                
                case RFID_CMD_RETRY_WAIT:
                    if(TimeSpent(rfRetryCmdTick,rfidReaderCmdList[rfidReaderCmdIndex].retryWaitIntervalInIms))
                    {
                        rfCmdState = RFID_CMD_SEND;
                    }
                break;
            }
        break;
        
        case READER_CMD_FAIL:
            
        break;
        
        case READER_CMD_SUCCESS:
            
        break;
    }
    return rfHandlerState;
}

/*
 *   brief: This function converts asciiHex data to hex data
 *    param : bytes - hexAscii data to convert
 *          : size  - length of the data to convert
*           : hex -   converted hex data is copied here  
*/
void RfidBytesToHex(const uint8_t *bytes, uint32_t size, char *hex)
{
  for(;size!= 0;size--)
  {
    *hex++ = hexchars[*bytes >> 4];
    *hex++ = hexchars[*bytes & 15];
    bytes++;
  }
  *hex = '\0';
}


