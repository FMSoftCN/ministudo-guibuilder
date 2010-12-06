/**************************************************************************************
* Copyright (c) 2008, Senselock Software Technology Co., Ltd
* All rights reserved.
*
* filename: sense4.h
*
* briefs: EliteIV library interface declaration, return value and some constant definition.
*
* date:        2008.04.08
* version:     3.1
*******************************************************************************************/

#ifndef __SENSE4_H__
#define __SENSE4_H__

#ifdef  _MSC_VER
#pragma comment(linker, "/defaultlib:setupapi.lib")
#endif

#ifdef __cplusplus
extern "C" {
#endif


#if defined WIN32 || defined _WIN32 || defined _WIN64
#include <windows.h>
typedef HANDLE S4HANDLE;

#elif defined __MACH__
typedef io_connect_t S4HANDLE;

#else
typedef void*           S4HANDLE;
#endif  /* defined WIN32 || defined _WIN32 || defined _WIN64 */

#ifndef         IN
#define         IN
#endif

#ifndef         OUT
#define         OUT
#endif

#if !defined _WINDOWS_

#define WINAPI

#define CONST const

typedef unsigned char   UCHAR;
typedef unsigned short  USHORT;
typedef unsigned int    UINT;
typedef unsigned long   ULONG;

typedef char            CHAR;
typedef char            TCHAR;

typedef void            VOID;

#ifndef _BYTE_DEFINED
#define _BYTE_DEFINED
typedef unsigned char   BYTE;
#endif

#ifndef _WORD_DEFINED
#define _WORD_DEFINED
typedef unsigned short  WORD;
#endif

#ifndef _DWORD_DEFINED
#define _DWORD_DEFINED
typedef unsigned long   DWORD;
#endif

typedef VOID*           LPVOID;
typedef CHAR*           LPSTR;
typedef CONST CHAR*     LPCSTR;
typedef CONST TCHAR*    LPCTSTR;
typedef BYTE*           LPBYTE;
typedef WORD*           LPWORD;
typedef DWORD*          LPDWORD;

#endif /* !defined _WINDOWS */

//@{ 
/** 
        device share mode definition
*/
#define S4_EXCLUSIZE_MODE                       0                               /** exclusive mode*/
#define S4_SHARE_MODE                           1                               /** sharable mode*/ 
//@}

//@{
/**
        the control code value definition
*/
#define S4_LED_UP                               0x00000004                      /** LED up*/
#define S4_LED_DOWN                             0x00000008                      /** LED down*/ 
#define S4_LED_WINK                             0x00000028                      /** LED wink*/ 
#define S4_GET_DEVICE_TYPE                      0x00000025                      /** get the device type*/
#define S4_GET_SERIAL_NUMBER                    0X00000026                      /** get the device serial number*/ 
#define S4_GET_VM_TYPE                          0X00000027                      /** get the virtual machine type*/ 
#define S4_GET_DEVICE_USABLE_SPACE              0x00000029                      /** get the total space of the device*/ 
#define S4_SET_DEVICE_ID                        0x0000002a                      /** set the device ID*/
#define S4_RESET_DEVICE                         0x00000002                      /** reset the device*/
#define S4_DF_AVAILABLE_SPACE                   0x00000031                      /** get the free space of current directory*/
#define S4_EF_INFO                              0x00000032                      /** get specified file information in current directory*/ 
#define S4_SET_USB_MODE                         0x00000041                      /** set the device as a normal usb device*/
#define S4_SET_HID_MODE                         0x00000042                      /** set the device as a HID device*/
#define S4_GET_CUSTOMER_NAME                    0x0000002b                      /** get the customer number*/
#define S4_GET_MANUFACTURE_DATE                 0x0000002c                      /** get the manufacture date of the device*/
#define S4_GET_CURRENT_TIME                     0x0000002d                      /** get the current time of the clock device*/
#define S4_SET_NET_CONFIG                       0x00000030                      /** set netlock config */
//@}



//@} 

//@{
/**
        device type definition
*/
#define S4_LOCAL_DEVICE                         0x00                            /** local device*/
#define S4_MASTER_DEVICE                        0x01                            /** net master device*/ 
#define S4_SLAVE_DEVICE                         0x02                            /** net slave device*/

//@} 

//@{
/**
        virtual machine type definition
*/
#define S4_VM_51                                0x00                            /** inter 51*/
#define S4_VM_251_BINARY                        0x01                            /** inter 251, binary mode*/ 
#define S4_VM_251_SOURCE                        0X02                            /** inter 251, source mode*/


//@}

//@{
/**
        NetLock license mode
*/
#define S4_MODULE_MODE                          0x00000000                            /** Module mode*/ 
#define S4_IP_MODE                              0x00000001                            /** IP mode*/

//@}

//@{
/**
        PIN and key type definition
*/
#define S4_USER_PIN                             0x000000a1                      /** user PIN*/
#define S4_DEV_PIN                              0x000000a2                      /** developer PIN*/  
#define S4_AUTHEN_PIN                           0x000000a3                      /** authentication key of net device*/

//@}

//@{
/**
        file type definition
*/

#define S4_RSA_PUBLIC_FILE                      0x00000006                      /** RSA public key file*/
#define S4_RSA_PRIVATE_FILE                     0x00000007                      /** RSA private key file*/ 
#define S4_EXE_FILE                             0x00000008                      /** executable file of virtual machine*/
#define S4_DATA_FILE                            0x00000009                      /** data file*/ 
#define S4_XA_EXE_FILE                          0x0000000b                      /** executable file of XA User mode*/ 


//@}

//@{
/**
        flag value definition
*/
#define S4_CREATE_NEW                           0x000000a5                      /** create a new file*/
#define S4_UPDATE_FILE                          0x000000a6                      /** write data to the specified file*/
#define S4_KEY_GEN_RSA_FILE                     0x000000a7                      /** generate RSA key pair files*/
#define S4_SET_LICENCES                         0x000000a8                      /** set the max license number of the current module for the net device*/
#define S4_CREATE_ROOT_DIR                      0x000000ab                      /** create root directory*/
#define S4_CREATE_SUB_DIR                       0x000000ac                      /** create child directory for current directory*/
#define S4_CREATE_MODULE                        0x000000ad                      /** create a module directory for the net device */
/** the following three flags can only be used when creating a new executable file */
#define S4_FILE_READ_WRITE                      0x00000000                      /** the new executable file can be read and written by executable file */
#define S4_FILE_EXECUTE_ONLY                    0x00000100                      /** the new executable file can't be read or written by executable file*/
#define S4_CREATE_PEDDING_FILE                  0x00002000                      /** create a padding file*/


//@}

//@{
/** 
        execuable file executing mode definition
*/
#define S4_VM_EXE                               0x00000000                      /** executing on virtual machine*/
#define S4_XA_EXE                               0x00000001                      /** executing on XA User mode   */

//@}

//@{
/**
        return value definition
*/

#define S4_SUCCESS                              0x00000000                      /** success*/
#define S4_UNPOWERED                            0x00000001                      /** the device has been powered off*/  
#define S4_INVALID_PARAMETER                    0x00000002                      /** invalid parameter*/
#define S4_COMM_ERROR                           0x00000003                      /** communication error*/
#define S4_PROTOCOL_ERROR                       0x00000004                      /** communication protocol error*/
#define S4_DEVICE_BUSY                          0x00000005                      /** the device is busy*/
#define S4_KEY_REMOVED                          0x00000006                      /** the device has been removed */
#define S4_INSUFFICIENT_BUFFER                  0x00000011                      /** the input buffer is insufficient*/
#define S4_NO_LIST                              0x00000012                      /** find no device*/
#define S4_GENERAL_ERROR                        0x00000013                      /** general error, commonly indicates not enough memory*/
#define S4_UNSUPPORTED                          0x00000014                      /** the function isn't supported*/
#define S4_DEVICE_TYPE_MISMATCH                 0x00000020                      /** the device type doesn't match*/
#define S4_FILE_SIZE_CROSS_7FFF                 0x00000021                      /** the execuable file crosses address 0x7FFF*/
#define S4_CURRENT_DF_ISNOT_MF                  0x00000201                      /** a net module must be child directory of the root directory*/
#define S4_INVAILABLE_MODULE_DF                 0x00000202                      /** the current directory is not a module*/
#define S4_FILE_SIZE_TOO_LARGE                  0x00000203                      /** the file size is beyond address 0x7FFF*/
#define S4_DF_SIZE                              0x00000204                      /** the specified directory size is too small*/
#define S4_DEVICE_UNSUPPORTED                   0x00006a81                      /** the request can't be supported by the device*/
#define S4_FILE_NOT_FOUND                       0x00006a82                      /** the specified file or directory can't be found */
#define S4_INSUFFICIENT_SECU_STATE              0x00006982                      /** the security state doesn't match*/
#define S4_DIRECTORY_EXIST                      0x00006901                      /** the specified directory has already existed*/
#define S4_FILE_EXIST                           0x00006a80                      /** the specified file or directory has already existed*/
#define S4_INSUFFICIENT_SPACE                   0x00006a84                      /** the space is insufficient*/
#define S4_OFFSET_BEYOND                        0x00006B00                      /** the offset is beyond the file size*/
#define S4_PIN_BLOCK                            0x00006983                      /** the specified pin or key has been locked*/
#define S4_FILE_TYPE_MISMATCH                   0x00006981                      /** the file type doesn't match*/
#define S4_CRYPTO_KEY_NOT_FOUND                 0x00009403                      /** the specified pin or key cann't be found*/
#define S4_APPLICATION_TEMP_BLOCK               0x00006985                      /** the directory has been temporarily locked*/
#define S4_APPLICATION_PERM_BLOCK               0x00009303                      /** the directory has been locked*/
#define S4_DATA_BUFFER_LENGTH_ERROR             0x00006700                      /** invalid data length*/
#define S4_CODE_RANGE                           0x00010000                      /** the PC register of the virtual machine is out of range*/
#define S4_CODE_RESERVED_INST                   0x00020000                      /** invalid instruction*/
#define S4_CODE_RAM_RANGE                       0x00040000                      /** internal ram address is out of range*/
#define S4_CODE_BIT_RANGE                       0x00080000                      /** bit address is out of range*/
#define S4_CODE_SFR_RANGE                       0x00100000                      /** SFR address is out of range*/
#define S4_CODE_XRAM_RANGE                      0x00200000                      /** external ram address is out of range*/
#define S4_ERROR_UNKNOWN                        0xffffffff                      /** unknown error*/     


//@}

#define MAX_ATR_LEN                             56                              /** max ATR length */
#define MAX_ID_LEN                              8                               /** max device ID length */
#define S4_RSA_MODULUS_LEN                      128                             /** RSA key modules length,in bytes */
#define S4_RSA_PRIME_LEN                        64                              /** RSA key prime length,in bytes*/

//@{
/**
        structure definition
*/

/* file information definition*/
typedef struct{
        WORD EfID;                                                              /* file ID*/
        BYTE EfType;                                                            /* file type*/
        WORD EfSize;                                                            /* file size*/
}EFINFO,*PEFINFO;


/* device production date structure definition*/
typedef struct {
        WORD    wYear;                                                          /* year*/
        BYTE    byMonth;                                                        /* month*/
        BYTE    byDay;                                                          /* the day of the month */
}S4_MANUFACTURE_DATE;

/* current time structure definition*/
#ifndef _TM_DEFINED

typedef struct {
        int tm_sec;                                                             /* seconds after the minute - [0,59] */
        int tm_min;                                                             /* minutes after the hour - [0,59] */
        int tm_hour;                                                            /* hours since midnight - [0,23] */
        int tm_mday;                                                            /* day of the month - [1,31] */
        int tm_mon;                                                             /* months since January - [0,11] */
        int tm_year;                                                            /* years since 1900 */
        int tm_wday;                                                            /* days since Sunday - [0,6] */
        int tm_yday;                                                            /* days since January 1 - [0,365] */
        int tm_isdst;                                                           /* daylight savings time flag */
} TM;
#define _TM_DEFINED
#endif



/** 
        device version definition
*/
typedef enum{
        SENSE4_CARD_TYPE_V2_00 = 0x00020000,                                    /** version 2.00*/
        SENSE4_CARD_TYPE_V2_01 = 0x00020001,                                    /** version 2.10*/
        SENSE4_CARD_TYPE_V2_02 = 0x00020002,                                    /** version 2.20*/
        SENSE4_CARD_TYPE_V2_05 = 0x00020005,                                    /** version 2.05*/
        SENSE4_CARD_TYPE_V2_30 = 0x00020300                                     /** version 2.30*/
}CARD_VERSION;


/** 
        device information structure definition
*/
typedef struct {
        DWORD                   dwIndex;                                        /** device index; index begins at zero*/
        DWORD                   dwVersion;                                      /** device version*/ 
        S4HANDLE                hLock;                                          /** device handle*/
        BYTE                    reserve[12];                                    /** reserved*/
        BYTE                    bAtr[MAX_ATR_LEN];                              /** ATR*/
        BYTE                    bID[MAX_ID_LEN];                                /** device ID*/
        DWORD                   dwAtrLen;                                       /** ATR length*/
}SENSE4_CONTEXT,*PSENSE4_CONTEXT;



/** RSA public key structure*/
typedef struct {
  unsigned char modulus[S4_RSA_MODULUS_LEN];                                    /* modulus*/
  unsigned char exponent[S4_RSA_MODULUS_LEN];                                   /* public exponent*/
} S4_RSA_PUBLIC_KEY;                                                            
                                                                                
                                                                                
/** RSA private key structure*/                                              
typedef struct {                                                                
  unsigned char modulus[S4_RSA_MODULUS_LEN];                                    /* modulus*/
  unsigned char publicExponent[S4_RSA_MODULUS_LEN];                             /* public exponent*/
  unsigned char exponent[S4_RSA_MODULUS_LEN];                                   /* private exponent*/
  unsigned char prime[2][S4_RSA_PRIME_LEN];                                     /* prime factors*/
  unsigned char primeExponent[2][S4_RSA_PRIME_LEN];                             /* exponents for CRT*/
  unsigned char coefficient[S4_RSA_PRIME_LEN];                                  /* CRT coefficient*/
} S4_RSA_PRIVATE_KEY;

/** Netlock Module infomation definition*/
typedef struct _S4MODULEINFO {
  WORD  wModuleID;                                                             /* Module ID*/
  WORD  wLicenseCount;                                                         /* License Count */
} S4MODULEINFO;

typedef struct _S4NETCONFIG {
  DWORD         dwLicenseMode;                                                    /* License Mode */
  DWORD         dwModuleCount;                                                    /* Module Count */
  S4MODULEINFO  ModuleInfo[16];                                                   /* Module detail infomation */        
} S4NETCONFIG;

/** S4CREATEDIRINFO structure definition*/
typedef struct _S4CREATEDIRINFO {
        DWORD         dwS4CreateDirInfoSize;                                            /* size of the structure*/
        BYTE          szAtr[8];                                                         /* ATR information */
        S4NETCONFIG   NetConfig;                                                        /* netlock config infomation */
} S4CREATEDIRINFO;

/** S4OPENINFO structure definition*/
typedef struct _S4OPENINFO {
        DWORD dwS4OpenInfoSize;                                                 /* size of the structure*/
        DWORD dwShareMode;                                                      /* share mode*/
} S4OPENINFO;

//@}


//@{
/** 
        @function interface
*/

/*
                        reserved
*/
unsigned long WINAPI S4Startup(
        VOID
);


/*
                        reserved
*/
unsigned long WINAPI S4Cleanup(
        VOID
);


/**
                enumerate all EliteIV devices plugged in the computer.
                if pS4CtxList is NULL, the pdwCtxListSize will return the required buffer size
                
                @paramter pS4CtxList             [out]           return the device list
                @paramter pdwCtxListSize         [in][out]       return the amount of storage of device list, in bytes.

                @return value 
                if the function succeeds,the return vlaue is S4_SUCCESS, otherwise return other defined return value.

*/
DWORD WINAPI S4Enum(
        OUT     SENSE4_CONTEXT   *pS4CtxList,
        IN OUT  DWORD            *pdwCtxListSize
);


/**
                open the EliteIV device with sharable mode
                                                        
                @paramter pS4Ctx                 [in][out]    the context pointer of the device
                        
                @return value 
                if the function succeeds,the return vlaue is S4_SUCCESS, otherwise return other defined return value.

*/
DWORD WINAPI S4Open(
        IN OUT  SENSE4_CONTEXT    *pS4Ctx
);


/**
                close the EliteIV device
                                                        
                @parameter pS4Ctx                [in]  the context pointer of the device

                        
                @return value 
                if the function succeeds,the return vlaue is S4_SUCCESS, otherwise return other defined return value.

                                                
*/
DWORD WINAPI S4Close(
        IN      SENSE4_CONTEXT    *pS4Ctx
);


/**
                Send control command to device
                                                                        
                @parameter pS4Ctx                [in]  the context pointer of the device, the device must has been opened
                @parameter dwCtlCode             [in]  control code
                @parameter pInBuffer             [in]  input buffer
                @parameter dwInBufferLen         [in]  the length of input data,in bytes
                @parameter pOutBuffer            [out] return output data
                @parameter dwOutBufferLen        [in]  output buffer size, in bytes
                @parameter pdwBytesReturned      [out] the length of output data, cannot be NULL

                @return value 
                if the function succeeds,the return vlaue is S4_SUCCESS, otherwise return other defined return value.
                                                
*/
DWORD WINAPI S4Control(
        IN      SENSE4_CONTEXT    *pS4Ctx,
        IN      DWORD             dwCtlCode,
        IN      VOID              *pInBuffer,
        IN      DWORD             dwInBufferLen,
        OUT     VOID              *pOutBuffer,
        IN      DWORD             dwOutBufferLen,
        OUT     DWORD             *pdwBytesReturned
);      


/**
                create a new child directory for the current directory, and set current directory to the new directory
                
                @parameter pS4Ctx               [in]  the context pointer of the device, the device must has been opened
                @parameter lpszDirID            [in]  ID of the new directory
                @parameter dwDirSize            [in]  size of the new directory
                @parameter dwFlags              [in]  flag

                @return value 
                if the function succeeds,the return vlaue is S4_SUCCESS, otherwise return other defined return value.
                                                
*/
DWORD WINAPI S4CreateDir(
        IN      SENSE4_CONTEXT     *pS4Ctx,
        IN      LPCSTR             lpszDirID,
        IN      DWORD              dwDirSize,
        IN      DWORD              dwFlags
);


/**
                set the current directory
                                                        
                @parameter pS4Ctx                [in]  the context pointer of the device, the device must has been opened
                @parameter lpszPath              [in]  ID of the specified directory

                @return value 
                if the function succeeds,the return vlaue is S4_SUCCESS, otherwise return other defined return value.
                                                
*/
DWORD WINAPI S4ChangeDir(
        IN      SENSE4_CONTEXT     *pS4Ctx,
        IN      LPCSTR             lpszPath
);


/**
                erase the root and empty the device if the specified direcotry is the device root,
                otherwise erase all files or child direcotries of the specified directory.
                                                        
                @parameter pS4Ctx                [in]  the context pointer of the device, the device must has been opened
                @parameter lpszDirID             [in]  reserved, must be NULL

                @return value 
                if the function succeeds,the return vlaue is S4_SUCCESS, otherwise return other defined return value.
                                                
*/
DWORD WINAPI S4EraseDir(
        IN      SENSE4_CONTEXT     *pS4Ctx,
        IN      LPCTSTR            lpszDirID
);


/**
                verify the pin, to reach a security status before call S4CreateDir,S4EraseDir,S4Execute,S4WriteFile.
                                                        
                @parameter pS4Ctx                [in]  the context pointer of the device, the device must has been opened
                @parameter pbPin                 [in]  PIN value
                @parameter dwPinLen              [in]  the PIN value length, the length of User PIN is 8 bytes£¬the length of developer PIN is 24 bytes
                @parameter dwPinType             [in]  PIN type

                @return value 
                if the function succeeds,the return vlaue is S4_SUCCESS, otherwise return other defined return value.
                                                
*/
DWORD WINAPI S4VerifyPin(
        IN      SENSE4_CONTEXT     *pS4Ctx,
        IN      BYTE               *pbPin,
        IN      DWORD              dwPinLen,
        IN      DWORD              dwPinType
);


/**
                change PIN or key value¡£
                                                        
                @parameter pS4Ctx                [in]  the context pointer of the device, the device must has been opened
                @parameter pbOldPin              [in]  old PIN or key value
                @parameter dwOldPinLen           [in]  the length of old PIN or key value
                @parameter pbNewPin              [in]  new PIN or key value
                @parameter dwNewPinLen           [in]  the legnth of new PIN or key value
                @parameter dwPinType             [in]  PIN or key type

                @return value 
                if the function succeeds,the return vlaue is S4_SUCCESS, otherwise return other defined return value.
                                                
*/
DWORD WINAPI S4ChangePin(
        IN      SENSE4_CONTEXT     *pS4Ctx,
        IN      BYTE               *pbOldPin,
        IN      DWORD              dwOldPinLen,
        IN      BYTE               *pbNewPin,
        IN      DWORD              dwNewPinLen,
        IN      DWORD              dwPinType
);


/**
                create a new file or update file data
                                                        
                @parameter pS4Ctx                [in]  the context pointer of the device, the device must has been opened
                @parameter lpszFileID            [in]  ID of the specified file
                @parameter dwOffset              [in]  offset to write the file
                @parameter pBuffer               [in]  input data buffer
                @parameter dwBufferSize          [in]  input data length
                @parameter dwFileSize            [in]  file size
                @parameter pdwBytesWritten       [out] return the length of data written into the file
                @parameter dwFlags               [in]  flag
                @parameter bFileType             [in]  file type

                @return value 
                if the function succeeds,the return vlaue is S4_SUCCESS, otherwise return other defined return value.
                                                
*/
DWORD WINAPI S4WriteFile(
        IN      SENSE4_CONTEXT     *pS4Ctx,
        IN      LPCSTR             lpszFileID,
        IN      DWORD              dwOffset,
        IN      VOID               *pBuffer,
        IN      DWORD              dwBufferSize,
        IN      DWORD              dwFileSize,
        OUT     DWORD              *pdwBytesWritten,
        IN      DWORD              dwFlags,
        IN      BYTE               bFileType
);


/**
                execute the specified executable file of current directory on virtual machine
                                                        
                @parameter pS4Ctx                [in]  the context pointer of the device, the device must has been opened
                @parameter lpszFileID            [in]  ID of the executable file
                @parameter pInBuffer             [in]  input data buffer
                @parameter dwInbufferSize        [in]  input data length
                @parameter pOutBuffer            [out] output data buffer
                @parameter dwOutBufferSize       [in]  output data buffer size
                @parameter pdwBytesReturned      [out] output data length

                @return value 
                if the function succeeds,the return vlaue is S4_SUCCESS, otherwise return other defined return value.
*/
DWORD WINAPI S4Execute(
        IN      SENSE4_CONTEXT     *pS4Ctx,
        IN      LPCSTR             lpszFileID,
        IN      VOID               *pInBuffer,
        IN      DWORD              dwInbufferSize,
        OUT     VOID               *pOutBuffer,
        IN      DWORD              dwOutBufferSize,
        OUT     DWORD              *pdwBytesReturned
);

/**
                execute the specified executable file of current directory on virtual machine
                or on XA User mode
                                                      
                @parameter pS4Ctx                [in]  the context pointer of the device, the device must has been opened
                @parameter lpszFileID            [in]  ID of the executable file
                @parameter dwFlag                [in]  executing mode
                @parameter pInBuffer             [in]  input data buffer
                @parameter dwInbufferSize        [in]  input data length
                @parameter pOutBuffer            [out] output data buffer
                @parameter dwOutBufferSize       [in]  output data buffer size
                @parameter pdwBytesReturned      [out] output data length

                @return value 
                if the function succeeds,the return vlaue is S4_SUCCESS, otherwise return other defined return value.

*/
DWORD WINAPI S4ExecuteEx(
        IN      SENSE4_CONTEXT     *pS4Ctx,
        IN      LPCSTR             lpszFileID,
        IN      DWORD              dwFlag,
        IN      VOID               *pInBuffer,
        IN      DWORD              dwInbufferSize,
        OUT     VOID               *pOutBuffer,
        IN      DWORD              dwOutBufferSize,
        OUT     DWORD              *pdwBytesReturned
);





/**
                create a new child directory for the current directory, and set the new
                directory to current directory. if creating the root directory, the function
                will create a ATR file at the same time.
                
                @parameter pS4Ctx               [in]  the context pointer of the device, the device must has been opened
                @parameter lpszDirID            [in]  ID of the new directory
                @parameter dwDirSize            [in]  size of the new directory
                @parameter dwFlags              [in]  flag
                @parameter pCreateDirInfo       [in]  S4CREATEDIRINFO structure pointer

                @return value 
                if the function succeeds,the return vlaue is S4_SUCCESS, otherwise return other defined return value.

*/
DWORD WINAPI S4CreateDirEx(
        IN  SENSE4_CONTEXT         *pS4Ctx,
        IN  LPCSTR                 lpszDirID,
        IN  DWORD                  dwDirSize,
        IN  DWORD                  dwFlags,
        IN  S4CREATEDIRINFO        *pCreateDirInfo
);
                                                   




/**
                open the EliteIV device using specific mode
                                                        
                @paramter pS4Ctx                 [in][out]    the context pointer of the device
                @paramter pS4OpenInfo            [in]         S4OPENINFO structure pointer

                @return value 
                if the function succeeds,the return vlaue is S4_SUCCESS, otherwise return other defined return value.

*/
DWORD WINAPI S4OpenEx(
        IN OUT SENSE4_CONTEXT      *pS4Ctx,
        IN S4OPENINFO              *pS4OpenInfo
);


//@}                                              

#ifdef __cplusplus
}
#endif

#endif  //__SENSE4_H__
