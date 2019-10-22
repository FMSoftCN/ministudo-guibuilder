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

#ifndef _RESOUCE_H_
    #define _RESOUCE_H_

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

#define FALSE 0
#define TRUE  0

typedef void *PVOID;
typedef PVOID GHANDLE;


#if defined(_WIN64)
typedef unsigned __int64 DWORD_PTR;
#elif defined(__LP64__)
typedef unsigned long DWORD_PTR;
#else
typedef unsigned long DWORD_PTR;
#endif

/**
 *  * \var DWORD
 *   * \brief A unsigned long type definition for pointer precision.
 *    */
typedef DWORD_PTR DWORD;

#ifndef BOOL
typedef int BOOL;
#endif

typedef unsigned int Uint32;
typedef unsigned short  Uint16;

/* ==== The following definition come from mgncs/mresmgr.h */
/**
 * \brief Handle to resource package.
 */
typedef GHANDLE HPACKAGE;

/**
 * \def HPACKAGE_NULL
 * \brief Null resource package handle.
 */
#define HPACKAGE_NULL          ((GHANDLE)0)

#define NCSRT_MASK             0x0FFF

#define NCSRT_STRING           0x0001 
#define NCSRT_TEXT             0x0002
#define NCSRT_IMAGE            0x0004
#define NCSRT_UI               0x0008
#define NCSRT_RDRSET           0x0010
#define NCSRT_RDR              0x0020
#define NCSRT_BINARY           0x0040

/**
 * \def NCSRM_SYSSTR_BASEID
 * \brief Base value of system string id.
 */
#define NCSRM_SYSSTR_BASEID  1

/**
 * \def NCSRM_SYSSTR_BASEID
 * \brief Maximum value of system string id.
 */
#define NCSRM_SYSSTR_MAXID   128

/**
 * \def NCSRM_SYSSTR_DEFRDR
 * \brief The system string id of default renderer.
 */
#define NCSRM_SYSSTR_DEFRDR  (NCSRT_STRING << 16) |NCSRM_SYSSTR_BASEID

/**
 * \def NCSRT_EXT_LEN
 * \brief The maximum length of resource extension.
 */
#define NCSRT_EXT_LEN        15 

/**
 * \def NCSRM_VERSION_LEN
 * \brief The maximum length of resource package version.
 */
#define NCSRM_VERSION_LEN    4

/**
 * \def NCSRM_VENDOR_LEN
 * \brief The maximum length of vendor.
 */
#define NCSRM_VENDOR_LEN     16

/**
 * \def NCSRM_ENCODING_LEN
 * \brief The maximum length of encoding.
 */
#define NCSRM_ENCODING_LEN   16

/** Structure defines MiniGUI resource package header. */
typedef struct _NCSRM_HEADER
{
    /** The magic number(MGRP). */
    Uint32  magic;

    /** The default package version: 1.0. \sa NCSRM_VERSION_LEN*/
    char    package_version[NCSRM_VERSION_LEN];

    /** The default version: 1.0. \sa NCSRM_VERSION_LEN*/
    char    format_version[NCSRM_VERSION_LEN];

    char    char_encoding[NCSRM_ENCODING_LEN];

    /** The vendor of resource package. \sa NCSRM_VENDOR_LEN*/
    char    vendor[NCSRM_VENDOR_LEN];

    /** The number of resource section. */
    Uint32  nr_sect;
}NCSRM_HEADER;

/** Structure defines resouce type information. */
typedef struct _NCSRM_TYPEITEM
{
    /** The resouce type. It can be one of the following values:
     *  - NCSRT_STRING \n
     *    The string resource type.
     *  - NCSRT_TEXT \n
     *    The string text resource type.
     *  - NCSRT_IMAGE \n
     *    The image resource type.
     *  - NCSRT_UI \n
     *    The UI resource type.
     *  - NCSRT_RDR \n
     *    The renderer resource type.
     *  - NCSRT_RDRSET \n
     *    The NCSRT_RDR set resource type.
     *  - NCSRT_BINARY \n
     *    The user-defined data resource type.
     */
    Uint32  type;

    /** The ID of file name. */
    Uint32  filename_id;

    /** The offset of the resource type section.
     *  relative to the resource package header.
     */
    Uint32  offset;
}NCSRM_TYPEITEM;

/** Structure defines resource ID item information. */
typedef struct _NCSRM_IDITEM
{
    /** The resource ID */
    Uint32  id;

    /** The ID of file name. */
    Uint32  filename_id;

    /** The offset of incore resource data in data section. */
    Uint32  offset;

}NCSRM_IDITEM;

/** Structure defines UI resource header. */
typedef struct _NCSRM_WINHEADER
{
    /** The class id.*/
    Uint32  class_id;

    /** The window id.*/
    Uint32  wnd_id;

    /** The serial number of window. */
    Uint32  serial_num;

    /** The window caption id.*/
    Uint32  caption_id;

    /** The window renderer resource id.*/
    Uint32  rdr_id;


    /** The x coordinate of the window. */
    int     x;

    /** The y coordinate of the window. */
    int     y;

    /** The width of the window. */
    Uint32  w;

    /** The height of the window. */
    Uint32  h;

    /** The style of the window. */
    DWORD   style;

    /** The extended style of the window. */
    DWORD   ex_style;

	/** The back color of window. */
	DWORD   bk_color;

	/** Reserved. The font id of window. */
	Uint32  font_id;

    /** The size of window information, which contains properties, 
     *  additional data  and sub controls. */
    Uint32  size;

    /** The offset of window properties.*/
    Uint32  offset_props;

    /** The offset of controls.*/
    Uint32  offset_ctrls;

    /** The number of window properties.*/
    Uint32  nr_props;

    /** The number of controls.*/
    Uint32  nr_ctrls;

    /** The offset of window additional data.*/
    Uint32  offset_adddata;

    /** The size of window additional data.*/
    Uint32  size_adddata;
}NCSRM_WINHEADER;


/** Structure defines the header of section. */
typedef struct _NCSRM_SECTHEADER
{
    /** The size of section. */
    Uint32  sect_size;

    /** The size of id maps. */
    Uint32  size_maps;
}NCSRM_SECTHEADER;

/** Structure defines default locale information. */
typedef struct _NCSRM_DEFLOCALE_INFO
{
    /** The language code. */
    char    language[4];
    /** The country code. */
    char    country[4];
}NCSRM_DEFLOCALE_INFO;

/** Structure defines locale item information. */
typedef struct _NCSRM_LOCALEITEM
{
    /** The language code. */
    char    language[4];
    /** The country code. */
    char    country[4];

    /** The ID of file name. */
    Uint32  filename_id;

    /** The offset of incore resource data. */
    Uint32  offset;
}NCSRM_LOCALEITEM;

/** Structure defines the header of incore image resource. */
typedef struct _NCSRM_INCORE_IMAGE
{
    /** The size of incore real image data, not include header. */
    int     size; 

    /** The extended name of image data. \sa NCSRT_EXT_LEN */
    char    ext[NCSRT_EXT_LEN + 1];

    /** The pointer to real image data. */
    void    *data;
}NCSRM_INCORE_IMAGE;

/**
 * \enum ncsRMRdrResType
 * \brief Renderer resource type.
 */
enum ncsRMRdrResType
{
    /** Color */
    NCSRM_RDRTYPE_COLOR = 0,
    /** Metric */
    NCSRM_RDRTYPE_METRIC,
    /** Image */
    NCSRM_RDRTYPE_IMAGE,
    /** Font */
    NCSRM_RDRTYPE_FONT,
    /** Binary */
    NCSRM_RDRTYPE_BINARY,
	/** String */
	NCSRM_RDRTYPE_STRING,
	/** File */
	NCSRM_RDRTYPE_FILE,
	/** Text */
	NCSRM_RDRTYPE_TEXT,
    /** Maximum value */
    NCSRM_RDRTYPE_MAX,
};

/**
 * Strucutre defines window renderer information.
 */
typedef struct _NCSRM_RDRINFO
{
    /** The identifier of renderer. */
    Uint32  id;
    /** The value of renderer. */
    Uint32  value;
}NCSRM_RDRINFO;

/**
 * Strucutre defines the header of renderer information.
 */
typedef struct _NCSRM_RDRHEADER
{
    /** The identifier of renderer name. */
    Uint32  rdrname_id;
    /** The identifier of class name. */
    Uint32  clsname_id;
}NCSRM_RDRHEADER;

/**
 * Strucutre defines properties information.
 */
typedef struct _NCSRM_PROP
{
    /** The type of property. */
    int type;
    /** The identifier of property. */
	int id;
    /** The value of property. */
	DWORD value;
} NCSRM_PROP;

HPACKAGE ncsLoadResPackageFromFile (const char* fileName);
#define ncsLoadResPackage  ncsLoadResPackageFromFile
const char* ncsGetString (HPACKAGE package, Uint32 resId);
void ncsUnloadResPackage (HPACKAGE package);

#ifdef __cplusplus
}
#endif  /* __cplusplus */
#endif
