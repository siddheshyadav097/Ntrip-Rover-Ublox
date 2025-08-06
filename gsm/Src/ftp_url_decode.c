#include "ftp_url_decode.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

ftpUrlDecodeRet_t FtpUrlDecode(uint8_t *urlString, ftpFile_st *ftpResult)
{
    uint8_t hstr[7];
    uint32_t j,i;
    uint8_t *phostnamehead;
    uint8_t *phostnameTail;
    char portStr[8];
    ftpUrlDecodeRet_t ret = FTP_URL_DECODE_FAIL;
    uint8_t *purl=urlString;
    uint32_t datalen = strlen((const char *)urlString);
    uint32_t filePathLen,binfilenameLen;
    
    // clear the ftp result structure
    memset(ftpResult,0,sizeof(ftpFile_st));
    // store the default FTP service port number
    ftpResult->portNum = FTP_SERVICE_PORT;

    do
    {    
        /* Resolve ftp:// */
        memset(hstr,0,7);
        // if length is less than 7 then break
        if((datalen) < 7)
            break;
        
        memcpy(hstr,urlString,7);
        
        for(i=0;i<6;i++)
        {
            if( (hstr[i]>= 'A') && (hstr[i] <= 'Z'))
                hstr[i] = tolower(hstr[i]);
        }
        if(NULL != strstr((char *)hstr,"ftp://"))
        {
            purl = urlString + 6;
            datalen -= 6;
        }
        else
        {
            break;
        }
        i=0;
        
        /* retrieve host name */
        phostnamehead = purl;
        phostnameTail = purl;
        while(i<datalen && purl[i] != '/' && purl[i] != ':')
        {
            i++;
            phostnameTail++;
        }
        if(i > FTP_SERVERADD_LEN)
        {
            LOG_DBG(CH_GSM,"<--server Addess is too long!!! (the buffer limit size is %d)->\r\n",FTP_SERVERADD_LEN);
            break;
        }
        memcpy((char*)ftpResult->host,(char*)phostnamehead, i);  // here is server ip or domain name.
        
        if(datalen >= i)
            datalen -= i;
        else
            break;
		
	/* retrieve  file path ,  image file name and ,port      eg /filepath/file/xxx.bin:8021@user......  or   /filepath/file/xxx.bin@user......  */
		
        purl+=i;
        i = 0;
        phostnamehead = purl;
        phostnameTail = purl;
        while(purl[i] !=':' && purl[i] !='@' && i<datalen )
        {
            i++;
            phostnameTail++;
        }		
        if(datalen >= i)
        {
            datalen -= i;
        }
        else // no @username:password     eg :  ftp://192.168.10.10/file/test.bin 
        {
            j = i;
            while(purl[j] !='/' && j>0) // the last '/' char 
            {
                j--;
            }
            binfilenameLen = (i-j-1);
            filePathLen = i-(i-j);
            if((binfilenameLen > FTP_FILENAME_LEN) ||(filePathLen > FTP_FILEPATH_LEN))
            {
                LOG_DBG(CH_GSM,"<--!! bin file name len(%d) or filePath(lne %d) is to long ! limit len(binfilename=%d, filePath=%d)->\r\n",binfilenameLen,filePathLen,FTP_FILENAME_LEN,FTP_FILEPATH_LEN);
                break;
            }
            memcpy((char *)ftpResult->fileName, (char *)(purl+j+1),binfilenameLen);
            memcpy((char *)ftpResult->filePath, (char *)phostnamehead, filePathLen);
            break;
        }

        /* retrieve  file path ,  image file name  /filepath/file/xxx.bin@user......  */
        if(purl[i] =='@' ) // no port number , it means port number is 21
        {
            j = i;
            while(purl[j] !='/' && j>0) // the last '/' char 
            {
                j--;
            }
            binfilenameLen = (i-j-1);
            filePathLen = i-(i-j);
            if((binfilenameLen > FTP_FILENAME_LEN) ||(filePathLen > FTP_FILEPATH_LEN))
            {
                LOG_DBG(CH_GSM,"<--@@ bin file name len(%d) or filePath(lne %d) is to long ! limit len(binfilename=%d, filePath=%d)->\r\n",binfilenameLen,filePathLen,FTP_FILENAME_LEN,FTP_FILEPATH_LEN);
                break;
            }
            memcpy((char *)ftpResult->fileName, (char *)(purl+j+1),binfilenameLen);
            memcpy((char *)ftpResult->filePath, (char *)phostnamehead, filePathLen); 
            //LOG_DBG(CH_GSM,"<--@@binFile=%s filePath=%s datalen=%d-->\r\n",binFile,filePath,datalen);       
        }
        /* retrieve file path , image file name and port   /filepath/file/xxx.bin:8021@user......  */
        else// else if (purl[i] ==':')     ftp port number.  
        {
            j = i;
            while(purl[j] !='/' && j>0) // the last '/' char 
            {
                j--;
            }
            binfilenameLen = (i-j-1);
            filePathLen = i-(i-j);
            if((binfilenameLen > FTP_FILENAME_LEN) ||(filePathLen > FTP_FILEPATH_LEN))
            {
                LOG_DBG(CH_GSM,"<--## bin file name len(%d) or filePath(lne %d) is to long ! limt len(binfilename=%d, filePath=%d)->\r\n",binfilenameLen,filePathLen,FTP_FILENAME_LEN,FTP_FILEPATH_LEN);
                break;
            }
            memcpy((char *)ftpResult->fileName, (char *)(purl+j+1),binfilenameLen);
            memcpy((char *)ftpResult->filePath, (char *)phostnamehead, filePathLen); 

            purl+=i; // puri = :port@username....
            i = 0;
            phostnamehead = purl;
            phostnameTail = purl;
            while(i<datalen && purl[i] != '@' )
            {
                i++;
                phostnameTail++;
            }
            if(datalen >= i)
                datalen -= i;
            else
                break;
            memset(portStr, 0x00,sizeof(portStr));
            memcpy(portStr, phostnamehead+1, i-1);
            ftpResult->portNum =  atoi(portStr);
            //   UPGRADE_APP_DEBUG(FOTA_DBGBuffer,"<--&&portStr=%s server port=%d-->\r\n",portStr,*serverPort);    
        }

        /* retrieve the ftp username and password      eg  @username:password  */
        purl+=i;
        i = 0;
        phostnamehead = purl;
        phostnameTail = purl;
        while(purl[i] !=':' && i<datalen )
        {
            i++;
            phostnameTail++;
        }
        if(datalen >= i)
            datalen -= i;
        else
            break;
        if(0 == datalen)// no user password
        {
            if(i > FTP_USERNAME_LEN)
            {
                LOG_DBG(CH_GSM,"<--@@ ftp user name(len %d) is to long ! limit len(%d)->\r\n",i,FTP_USERNAME_LEN);
                break;
            }
            memcpy(ftpResult->userName,phostnamehead+1,i); // ftp user name          
        }
        else
        {
            if((i-1 > FTP_USERNAME_LEN) || (datalen > FTP_PASSWORD_LEN))
            {
                LOG_DBG(CH_GSM,"<--@@ ftp user name(len %d)  or password(len=%d)is to long ! limit len(username=%d, pwd =%d)->\r\n",i-1,datalen,FTP_USERNAME_LEN,FTP_PASSWORD_LEN);
                break;
            }
            memcpy(ftpResult->userName,phostnamehead+1,i-1); // ftp user name 
            memcpy(ftpResult->password,phostnamehead+i+1,datalen); //user  password
        }

        ret = FTP_URL_DECODE_SUCCESS;
    }
    while(0);

    memcpy((void*)(ftpResult->filePath+strlen((const char*)ftpResult->filePath)), (const void*)"/", 1);  //  file path end with '/'
    
    LOG_DBG(CH_FTP,"\r\nFTP hostname=%s\r\n FTP filepath=%s\r\n FTP filename=%s\r\n",ftpResult->host,ftpResult->filePath,ftpResult->fileName);
    LOG_DBG(CH_FTP,"FTP username=%s\r\n FTP password=%s\r\n FTP portNumber=%d-->\r\n",ftpResult->userName, ftpResult->password, ftpResult->portNum);
    return ret;
}
