/**
 *  \file sys_para.h
 *  \brief Includes all system configuration settings. This file is the user interface to alter any system configurations.
 *  		Timeouts for various functionalities, default device information and switch to enable or disable a functionality
 *  		are defined. 
 */

#ifndef __SYS_PARA_H__
#define __SYS_PARA_H__

//#define VODAFONE_VOICE		
//#define VODAFONE_M2M		
//#define VODAFONE_QDNET		
/*#define AIRTEL				
#define MTNL				
#define AIRCEL				
#define IDEA				
#define MOBILY	 */			 

//#define MEM_CLEAR_FLAG                  0
//#define DEFAULT_SERIAL_DATA_DISABLE     0
#define WATCHDOG_ENABLE                   0      //1   //Always keep watchdog enabled.

//#define DEFAULT_SENDPACKET_INTERVAL		(60*10)		//in milliseconds	=1 min
//#define GPRS_POLL_INTERVAL			(30*10)		//in milliseconds	=2 min
//#define READ_SMS_INTERVAL			(5*60*10)		//in milliseconds	(5*60*1000)msec = 5mins
//#define MAX_GSM_REG_TIMEOUT			(5*60*10)		//in milliseconds	(5*60*1000)msec = 5mins
//#define MAX_GSM_QUERY_WAIT_TIMEOUT		(20*10)		//in milliseconds
//#define GSM_CELLID_POLL_INTERVAL		(1*60*10)
//#define MAX_GPRS_REG_TIMEOUT			(10*60*10)	//in milliseconds
//#define MAX_CALL_PICKUP_TIMEOUT			(60*10)		//in milliseconds
//#define MAX_CALL_HANGUP_TIMEOUT			(5*60*10)		//in milliseconds
//#define MAX_SEND_PACKET_TIMEOUT			(3*60*10)	// 3 mins timout
//#define MAX_FOTA_UPGRADE_TIMEOUT		(5*60*10)	//5 mins timeout for fota
//#define SEND_MEM_PACKET_TRIAL_TIMEOUT	        (2*60*10)

//#define MAX_PACKET_BUFFLENGTH	3000
//#define SERIAL_RX_BUFFER_LEN	2048
//#define CONFIGPASSWORD		"1234"



//#define PHN_FUNC    	
//#define PHN_INIT		
//#define IMEI_EN			
//#define ENG_EN			
//#define NW_TIME_SYNC_EN			
//#define TIME_SYNC_EN	
//#define SMS_ENCODE_EN	
//#define SIM_STATE_EN	

//#define INIT_MAX_TRY_COUNT			3
//#define SEND_SMS_ENABLE				1

//#define DEFAULT_URL			"qdvts.com/cgi-bin/gprs/g2.pl"
//#define DEFAULT_URL 		"qdvts.com/cgi-bin/gprsRF/rf2.pl"		//"qdvts.com/cgi-bin/dl/d1.pl"
//#define DEFAULT_SOC_PORT 	80
//#define DEFAULT_REQ_TYPE 	"POST"

//#define DEFAULT_UNITID			100
//#define DEFAULT_PACKET_MSG_TYPE		23
//#define DEFAULT_NO_OF_POSITIONS		4
//#define DEFAULT_RUNMODE_INTERVAL	15	//MULTIPLE OF 100 MS
//#define DEFAULT_STOPMODE_INTERVAL	15
//
//#define DEFAULT_IGNITION_ON_INTERVAL    5000    
//#define DEFAULT_ONE_SEC_TASK_INTERVAL   1000
//
//
//#define DEFAULT_BOARD_RESET_INTERVAL    1440    //1day = 1440mins
//#define MAX_PACK_FOR_BULK_UPLOAD	350
//#define BULKFILE_CONFIG_PARA		1
//#define BULKFILE_UPLOAD_URL		"aq.qdvts.com"
//#define BULKFILE_UPLOAD_PORT		21
//#define BULKFILE_UPLOAD_USER		"packetdump@aq.qdvts.com"
//#define BULKFILE_UPLOAD_PASS		"pdp@1234"


//#define PACKET_WAIT_INTERVAL            12000

#endif