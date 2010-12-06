/**************************************************************************************
* Copyright (c) 2008, Senselock Software Technology Co., Ltd
* All rights reserved.
* 
* File Name£ºPSense4.h
*
* briefs:Declare PW4WriteFile function and define some parameters
*
* date:        2008.04.08
* version:     3.1
*******************************************************************************************/

#ifndef _INCLUDE_S4WF_H_
#define _INCLUDE_S4WF_H_

#ifdef __cplusplus
extern "C" {
#endif

#define S4WF_INVALID_S4CONTEXT          0x00000101
#define S4WF_INVALID_FILE_ID            0x00000102
#define S4WF_INVALID_PC_FILE            0x00000103
#define S4WF_INVALID_FLAGS              0x00000104
#define S4WF_INVALID_FILE_SIZE          0x00000105
#define S4WF_INVALID_FILE_TYPE          0x00000106

//Supplement parameter to download HEX file
#define S4_HEX_FILE                     0x0000000a
//Supplement parameter to download XA HEX file
#define S4_XA_HEX_FILE                  0x0000000c


DWORD WINAPI PS4WriteFile(
        IN      CONST SENSE4_CONTEXT *pS4Ctx,
        IN      LPCSTR  lpszFileID,
        IN      LPCSTR  lpszPCFilePath,
        IN OUT  DWORD   *pdwFileSize,//attention:this parameter is a pointer
        IN      DWORD   dwFlags,
        IN      DWORD   dwFileType,
        OUT     DWORD   *pdwBytesWritten
        );
#ifdef __cplusplus
}
#endif

#endif

/*------------------------------------------ END OF FILE ----------------------------------*/
