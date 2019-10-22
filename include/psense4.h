/*
** This file is a part of miniStudio, which provides a WYSIWYG UI designer
** and an IDE for MiniGUI app developers.
**
** Copyright (C) 2010 ~ 2019, Beijing FMSoft Technologies Co., Ltd.
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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
