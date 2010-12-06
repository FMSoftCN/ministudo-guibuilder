/**
 * \file mgutils.h
 * \date 2007/12/26
 * 
 *  This file includes interfaces of mGUtils. 
 *
 \verbatim
    Copyright (C) 2007 ~ 2008 Feynman Software

    All rights reserved by Feynman Software.

    This file is part of mGUtils, a component for MiniGUI.
 \endverbatim
 */

/*
 * $Id: mgutils.h 124 2008-08-26 07:50:19Z weiym $
 *
 * mGUtils: A MiniGUI component which contains miscellaneous 
 * utilities like ColorSelectionDialogBox, FileOpenDialogBox, and so on. 
 *
 * Copyright (C) 2007 ~ 2008 Feynman Software.
 */

#ifndef _MGUTILS_H
#define _MGUTILS_H

#ifdef  __cplusplus
extern "C" {
#endif

#if !defined(__NODLL__) && (defined (WIN32) || defined (__NUCLEUS_MNT__))
  #ifdef __MGUTILS_LIB__
  #define MGUTILS_EXPORT       __declspec(dllexport)
  #else
  #define MGUTILS_EXPORT       __declspec(dllimport) 
  #endif
#else
  #define MGUTILS_EXPORT
#endif

#define MGUTILS_MSG_BASE            MSG_USER + 0x40
#define MSG_FILESELOK               (MGUTILS_MSG_BASE + 1)
#define MSG_FILESELCANCEL           (MGUTILS_MSG_BASE + 2)
#define MSG_COLORSELOK              (MGUTILS_MSG_BASE + 3)
#define MSG_COLORSELCANCEL          (MGUTILS_MSG_BASE + 4)
#define MSG_FONTSELOK               (MGUTILS_MSG_BASE + 5)
#define MSG_FONTSELCANCEL           (MGUTILS_MSG_BASE + 6)

    /**
     * \addtogroup mywins_fns Interfaces of MyWins module of mGUtils library
     * @{
     */

    /**
     * \defgroup mywins_colorseldlg Color Selection Dialog Box
     * @{
     */

/**
 * The color data structure used by \a ColorSelDialog.
 * \sa ColorSelDialog.
 */
typedef struct _COLORDATA {
    /** Reserves, not used now. */
    DWORD           style;
    /** The value of the color  returned. */
    gal_pixel       pixel;
    /** The R, G, B value of the color returned. */
    Uint8           r, g, b;
    /** The H value of the color returned. */
    Uint16          h;
    /** The S, V value of the color returned. */
    Uint8           s, v;
    /** Indicates the controls is transparent or no?*/
    BOOL            is_trans;
} COLORDATA, *PCOLORDATA;

/* Internal use */
#define SELCOLOR_OK     IDOK
/* Internal use */
#define SELCOLOR_CANCEL IDCANCEL

/**
 * \fn int ColorSelDialog (HWND hWnd, int x, int y, \
 *               int w, int h, PCOLORDATA pClrData)
 * \brief Creates a Color Selection Dialog Box.
 *
 * This function creates a Color Selection Dialog Box, and returns 
 * the values of the color selected by the user.
 *
 * \param hWnd The hosting main window.
 * \param x  x,y,w,h: The default position and size of the dialog box.
 * \param y  x,y,w,h: The default position and size of the dialog box.
 * \param w  x,y,w,h: The default position and size of the dialog box.
 * \param h  x,y,w,h: The default position and size of the dialog box.
 * \param pClrData The pointer to the COLORDATA structure.
 *
 * \retval IDOK     The user choosed a color and OK button clicked.
 * \retval IDCANCEL The user clicked the Cancel button.
 * 
 * \sa COLORDATA, ColorSelDialog
 */
MGUTILS_EXPORT int ColorSelDialog (HWND hWnd, int x, int y, 
                int w, int h, PCOLORDATA pClrData);

    /** @} end of mywins_colorseldlg*/

    /** @} end of mywins_fns */

    /**
     * \addtogroup mywins_fns Interfaces of MyWins module of mGUtils library
     * @{
     */

    /**
     * \defgroup mywins_colorspace Color Space Conversion Functions
     *
     * This module defines some color space conversion functions.
     *
     * @{
     */

/**
 * \fn void RGB2HSV (Uint8 r, Uint8 g, Uint8 b, \
 *               Uint16 *hout, Uint8 *sout, Uint8 *vout)
 * \brief Converts r/g/b values of a color in RGB color space to h/s/v of 
 *        the color in HSV color space.
 *
 * This function converts r/g/b values of a color in RGB color space to 
 * h/s/v values of the color in HSV color space. 
 *
 * \param r     The red value of the color in RGB space to be converted.
 * \param g     The green value of the color in RGB space to be converted.
 * \param b     The blue value of the color in RGB space to be converted.
 * \param hout  The pointer to Uint16, returns the h value of the color
 *              in HSV color space.
 * \param sout  The pointer to Uint8, returns the s value of the color 
 *              in HSV color space returned.
 * \param vout  The pointer to Uint8, returns the v value of the color
 *              in HSV color space returned.
 *
 * \sa HSV2RGB, YUV2RGB, RGB2YUV 
 */

void RGB2HSV (Uint8 r, Uint8 g, Uint8 b, 
                Uint16 *hout, Uint8 *sout, Uint8 *vout);

/**
 * \fn void HSV2RGB (Uint16 hin, Uint8 sin, Uint8 vin, \
 *               Uint8 *rout, Uint8 *gout, Uint8 *bout)
 * \brief Converts h/s/v values of a color to r/g/b values of the color.
 *
 * This function converts h/s/v values of a color in HSV color space 
 * to r/g/b values of the color in RGB space. 
 *
 * \param hin   The h value of the HSV color space to be converted, 
 *              the range is between 0 to 359.
 * \param sin   The s value of the HSV color space to be converted, 
 *              the range is between 0 to 255.
 * \param vin   The v value of the HSV color space to be converted, 
 *              the range is between 0 to 255.
 * \param rout  The pointer to Uint8, returns the red value of the RGB 
 *              color space.
 * \param gout  The pointer to Uint8, returns the green value of the RGB 
 *              color space.
 * \param bout  The pointer to Uint8, returns the bout value of the RGB 
 *              color space.
 * 
 * \sa RGB2HSV, YUV2RGB, RGB2YUV 
 */

void HSV2RGB (Uint16 hin, Uint8 sin, Uint8 vin, 
                Uint8 *rout, Uint8 *gout, Uint8 *bout);

/**
 * \fn void YUV2RGB (int y, int u, int v, Uint8 *r, Uint8 *g, Uint8 *b)
 * \brief Converts y/u/v values of a color in YUV color space to 
 *        to r/g/b values of the color in RGB color space.
 *
 * This function converts y/u/v values of YUV color space to r/g/b values 
 * of RGB color space. 
 *
 * \param y     The y value of the YUV color space to be converted.
 * \param u     The u value of the YUV color space to be converted.
 * \param v     The v value of the YUV color space to be converted.
 * \param r     The pointer to Uint8, returns the red value of the 
 *              RGB color space.
 * \param g     The pointer to Uint8, returns the green value of the 
 *              RGB color space.
 * \param b     The pointer to Uint8, returns the bout value of the 
 *              RGB color space.
 * 
 * \sa RGB2HSV, HSV2RGB, RGB2YUV 
 */

void YUV2RGB (int y, int u, int v, Uint8 *r, Uint8 *g, Uint8 *b);

/**
 * \fn void RGB2YUV (Uint8 r, Uint8 g, Uint8 b, int *y, int *u, int *v)
 * \brief Converts r/g/b values of a color in RGB color space 
 *        to y/u/v values of the color in YUV color space.
 *
 * The function converts r/g/b values of a color in RGB color space 
 *        to y/u/v values of the color in YUV color space.
 *
 * \param r  The red value of the RGB color space to be converted.
 * \param g  The green value of the RGB color space to be converted.
 * \param b  The blue value of the RGB color space to be converted.
 * \param y  The pointer to int, returns the y value of the YUV color space.
 * \param u  The pointer to int, returns the u value of the YUV color space.
 * \param v  The pointer to int, returns the v value of the YUV color space.
 * 
 * \sa RGB2HSV, HSV2RGB, YUV2RGB 
 */
MG_EXPORT void RGB2YUV (Uint8 r, Uint8 g, Uint8 b, int *y, int *u, int *v);

    /** @} end of mywins_colorspace*/

    /** @} end of mywins_fns */

    /**
     * \addtogroup mgext_fns Interfaces of the MiniGUI extension 
     *             library (libmgext)
     * @{
     */

    /**
     * \addtogroup mywins_fns Interfaces of MyWins module of mGUtils library
     * @{
     */

    /**
     * \defgroup mywins_helpers Useful helpers
     *
     * This module defines some useful helpers, such as tool-tip window,
     * progress window, and so on.
     *
     * @{
     */

/**
 * \fn int myMessageBox (HWND hwnd, DWORD dwStyle, const char* title,\
                const char* text, ...)
 * \brief Creates a message box.
 *
 * This function creates a message box calling \a MessageBox and passing
 * \a hwnd, \a dwStyle and \a title to it. This function also receives \a 
 * printf-like arguments to format a string.
 *
 * \param hwnd The hosting main window.
 * \param dwStyle The style of the message box.
 * \param title The title of the message box.
 * \param text The format string.
 *
 * \return Identifier of the button which closes the message box.
 *
 * \sa MessageBox, printf(3)
 */
MGUTILS_EXPORT int myMessageBox (HWND hwnd, DWORD dwStyle, const char* title,
                const char* text, ...);

/**
 * \fn int myWinMessage (HWND hwnd, const char* title, \
                const char* button1, const char* text, ...)
 * \brief Creates a message box within only one button.
 *
 * This function creates a message box hosted to the main window \a hwnd,
 * displays a message and an application icon in the message box, and creates
 * a button which can be used to close the box. This function also 
 * receives \a printf-like arguments to format a string.
 *
 * This function can be used to display a information for the user.
 *
 * \param hwnd The hosting main window.
 * \param title The title of the message box.
 * \param button1 The text in the button.
 * \param text The format string.
 *
 * \return 0 indicates the message box was closed by clicking the only button.
 *
 * \sa myWinChoice, printf(3)
 */
MGUTILS_EXPORT int myWinMessage (HWND hwnd, const char* title, 
                const char* button1, const char* text, ...);

/**
 * \fn int myWinChoice (HWND hwnd, const char* title, \
                const char* button1, const char* button2, \
                const char* text, ...)
 * \brief Creates a message box within two buttons.
 *
 * This function creates a message box hosted to the main window \a hwnd, 
 * displays a message and an application icon in the message box, and creates
 * two buttons in it. This function also receives \a printf-like arguments 
 * to format a string.
 *
 * This function can be used to prompt the user to choose one item 
 * between two.
 *
 * \param hwnd The hosting main window.
 * \param title The title of the message box.
 * \param button1 The title of the first button.
 * \param button2 The title of the second button.
 * \param text The format string.
 *
 * \return Either 0 or 1, indicates the message box was closed by 
 *         the first or second button.
 *
 * \sa myWinTernary, printf(3)
 */
MGUTILS_EXPORT int myWinChoice (HWND hwnd, const char* title, 
                const char* button1, const char* button2, 
                const char* text, ...);

/**
 * \fn int myWinTernary (HWND hwnd, const char* title, \
                const char* button1, const char* button2, const char* button3, \
                const char* text, ...)
 * \brief Creates a message box within three buttons.
 *
 * This function creates a message box hosted to the main window \a hwnd, 
 * displays a message and an application icon in the message box, and creates
 * three buttons in it. This function also receives \a printf-like arguments 
 * to format a string.
 *
 * This function can be used to prompt the user to choose one item among 
 * three choices.
 *
 * \param hwnd The hosting main window.
 * \param title The title of the message box.
 * \param button1 The title of the first button.
 * \param button2 The title of the second button.
 * \param button3 The title of the third button.
 * \param text The format string.
 *
 * \return 0, 1, or 2, indicates the message box was closed by 
 *         the first, the second, or the third button.
 *
 * \sa myWinChoice, printf(3)
 */
MGUTILS_EXPORT int myWinTernary (HWND hwnd, const char* title, 
                const char* button1, const char* button2, const char* button3,
                const char* text, ...);

/**
 * \fn void errorWindow (HWND hwnd, const char* str, const char* title)
 * \brief A MiniGUI edition of \a perror.
 *
 * This function creates a message box by using \a myMessageBox, and display the
 * current system error message. You can consider it as an alternative of 
 * \a perror.
 *
 * \param hwnd The hosting main window.
 * \param str The string will be appeared before the system error message.
 * \param title The title of the message box.
 *
 * \sa myMessageBox, perror(3)
 */
MGUTILS_EXPORT void errorWindow (HWND hwnd, const char* str, const char* title);

/**
 * \fn HWND createStatusWin (HWND hParentWnd, int width, int height,\
                const char* title, const char* text, ...)
 * \brief Creates a status main window.
 *
 * This function creates a status main window and returns the handle to it.
 * You can call \a destroyStatusWin to destroy it. This function also 
 * receives \a printf-like arguments to format a string.
 *
 * \param hParentWnd The hosting main window.
 * \param width The width of the status window. 
 * \param height The height of the status window. 
 * \param title The title of the status window.
 * \param text The format string.
 *
 * \return The handle to the status window on success, HWND_INVALID on error.
 *
 * \sa destroyStatusWin
 */
MGUTILS_EXPORT HWND createStatusWin (HWND hParentWnd, int width, int height, 
                const char* title, const char* text, ...);

/**
 * \fn void destroyStatusWin (HWND hwnd)
 * \brief Destroies a status window.
 *
 * This function destroies the specified status window \a hwnd, which 
 * is returned by \a createStatusWin.
 *
 * \param hwnd The handle to the status window.
 * 
 * \sa createStatusWin
 */
MGUTILS_EXPORT void destroyStatusWin (HWND hwnd);

/* back-compatibility definitions */
#define createToolTipWin CreateToolTipWin
#define resetToolTipWin ResetToolTipWin
#define destroyToolTipWin DestroyToolTipWin

#ifdef _MGCTRL_PROGRESSBAR

/**
 * \fn HWND createProgressWin (HWND hParentWnd, const char *title,\
                const char *label, int id, int range)
 * \brief Creates a main window within a progress bar.
 *
 * This function creates a main window within a progress bar and 
 * returns the handle. You can call \a destroyProgressWin to destroy it. 
 *
 * Note that you can use \a SendDlgItemMessage to send a message to the 
 * progress bar in the main window in order to update the progress bar.
 *
 * \param hParentWnd The hosting main window.
 * \param title The title of the progress window.
 * \param label The text in the label of the progress bar.
 * \param id The identifier of the progress bar.
 * \param range The maximal value of the progress bar (minimal value is 0).
 *
 * \return The handle to the progress window on success, HWND_INVALID on error.
 *
 * \sa destroyProgressWin
 */
MGUTILS_EXPORT HWND createProgressWin (HWND hParentWnd, const char* title, 
                const char* label, int id, int range);

/**
 * \fn void destroyProgressWin (HWND hwnd)
 * \brief Destroies progress window.
 *
 * This function destroies the specified progress window \a hwnd, which 
 * is returned by \a createProgressWin.
 *
 * \param hwnd The handle to the progress window.
 * 
 * \sa createProgressWin
 */
MGUTILS_EXPORT void destroyProgressWin (HWND hwnd);

#endif /* _MGCTRL_PROGRESSBAR */

/**
 * Button info structure used by \a myWinMenu and \a myWinEntries function.
 * \sa myWinMenu, myWinEntries
 */
typedef struct _myWinButton
{
    /** Text of the button. */
    char*   text;
    /** Identifier of the button. */
    int     id;
    /** Styles of the button. */
    DWORD   flags;
} myWINBUTTON;

/* This is an internal structure. */
typedef struct _myWinMenuItems
{
    /* The pointer to the array of the item strings. */
    char**      items;
    /* The identifier of the listbox display the menu items. */
    int         listboxid;
    /* The pointer to the array of the selection status of the items. */
    int*        selected;
    /* The minimal button identifier. */
    int         minbuttonid;
    /* The maximal button identifier. */
    int         maxbuttonid;
} myWINMENUITEMS;

/**
 * \fn int myWinMenu (HWND hParentWnd, const char* title,\
                const char *label, int width, int listboxheight,\
                char **items, int *listItem, myWINBUTTON* buttons)
 * \brief Creates a menu main window for the user to select an item.
 *
 * This function creates a menu main window including a few buttons, 
 * and a list box with checkable item.
 * 
 * When the user click one of the buttons, this function will return the 
 * identifier of the button which leads to close the menu window, and 
 * the selections of the items via \a listItem.
 *
 * \param hParentWnd The hosting main window.
 * \param title The title of the menu main window.
 * \param label The label of the list box.
 * \param width The width of the menu main window.
 * \param listboxheight The height of the list box.
 * \param items The pointer to the array of the item strings.
 * \param listItem The pointer to the array of the check status of the items, 
 *        initial and returned.
 * \param buttons The buttons will be created.
 *
 * \return Returns the identifier of the button leading to close 
 *         the menu window on success, else on errors.
 *
 * \sa myWINBUTTON
 */
MGUTILS_EXPORT int myWinMenu (HWND hParentWnd, const char* title, 
                const char* label, int width, int listboxheight, 
                char ** items, int * listItem, myWINBUTTON* buttons);

/**
 * Entry info structure used by \a myWinEntries function.
 * \sa myWinEntries
 */
typedef struct _myWinEntry
{
    /** The label of the entry. */
    char*   text;
    /** The pointer to the string of the entry. */
    char**  value;
    /** The maximal length of the entry in bytes. */
    int     maxlen;
    /** The styles of the entry. */
    DWORD   flags;
} myWINENTRY;

/* This is an internal structure. */
typedef struct _myWinEntryItems
{
    myWINENTRY* entries;
    int         entrycount;
    int         firstentryid;
    int         minbuttonid;
    int         maxbuttonid;
} myWINENTRYITEMS;

/**
 * \fn int myWinEntries (HWND hParentWnd, const char* title,\
                const char* label, int width, int editboxwidth,\
                BOOL fIME, myWINENTRY* items, myWINBUTTON* buttons)
 * \brief Creates a entry main window for the user to enter something.
 *
 * This function creates a entry main window including a few buttons
 * and a few entries.
 * 
 * When the user click one of the buttons, this function will return the 
 * identifier of the button which leads to close the menu window, and 
 * the entered strings.
 *
 * \param hParentWnd The hosting main window.
 * \param title The title of the menu main window.
 * \param label The label of the entries.
 * \param width The width of the menu main window.
 * \param editboxwidth The width of the edit boxes.
 * \param fIME Whether active the IME window (obsolete).
 * \param items The pointer to the array of the entries, initial and returned.
 * \param buttons The buttons will be created.
 *
 * \return Returns the identifier of the button leading to close the 
 *         menu window on success, else on errors.
 *
 * \sa myWINBUTTON, myWINENTRY
 */
MGUTILS_EXPORT int myWinEntries (HWND hParentWnd, const char* title, 
                const char* label, int width, int editboxwidth, 
                BOOL fIME, myWINENTRY* items, myWINBUTTON* buttons);

/**
 * \fn int myWinHelpMessage (HWND hwnd, int width, int height,\
                const char* help_title, const char* help_msg)
 * \brief Creates a help message window.
 *
 * This function creates a help message window including a scrollable help 
 * message and a spin box. When the user click the OK button, this function 
 * will return.
 *
 * \param hwnd The hosting main window.
 * \param width The width of the help message window.
 * \param height The height of the help message window.
 * \param help_title The title of the window.
 * \param help_msg The help message.
 *
 * \return 0 on success, -1 on error.
 */
MGUTILS_EXPORT int myWinHelpMessage (HWND hwnd, int width, int height,
                const char* help_title, const char* help_msg);

    /** @} end of mywins_helpers */

    /** @} end of mywins_fns */

    /** @} end of mgext_fns */

    /**
     * \addtogroup mywins_fns Interfaces of MyWins module of mGUtils library
     * @{
     */

    /**
     * \defgroup mywins_newfiledlg Open File Dialog Box
     * @{
     */

#ifndef WIN32
#include <sys/types.h>
#include <dirent.h>
#endif
/**
 * \def FILE_ERROR_OK
 * \brief Open file success.
 */
#define FILE_ERROR_OK           0

/**
 * \def FILE_ERROR_PARAMERR
 * \brief Wrong parameters.
 */
#define FILE_ERROR_PARAMERR     -1

/**
 * \def FILE_ERROR_PARAMERR
 * \brief Path doesn't exist.
 */
#define FILE_ERROR_PATHNOTEXIST -2

/**
 * \def MAX_FILTER_LEN
 * \brief The maximum length of filter string.
 */
#define MAX_FILTER_LEN          255

/**
 * \def MY_NAMEMAX
 * \brief The maximum length of name.
 */
#define MY_NAMEMAX      127

/**
 * \def MY_PATHMAX
 * \brief The maximum length of path.
 */
#define MY_PATHMAX      255
/**
 * The file dialog box structure used by \a ShowOpenDialog.
 * \sa ShowOpenDialog.
 */
typedef struct _NEWFILEDLGDATA
{
    /** Indicates to create a Save File or an Open File dialog box. */
    BOOL    IsSave;
    /** Indicates the controls is transparent or no?*/
    BOOL    IsTrans;
    /** The full path name of the file returned. */
    char    filefullname[MY_NAMEMAX + MY_PATHMAX + 1];
    /** The name of the file to be opened. */
    char    filename[MY_NAMEMAX + 1];
    /** The initial path of the dialog box. */
    char    filepath[MY_PATHMAX + 1];
    /**
     * The filter string, for example: 
     * All file (*.*)|Text file (*.txt;*.TXT)
     */
    char    filter[MAX_FILTER_LEN + 1];
    /** The initial index of the filter*/
    int     filterindex;
} NEWFILEDLGDATA;
/** Data type of pointer to a NEWFILEDLGDATA */
typedef NEWFILEDLGDATA* PNEWFILEDLGDATA;

/**
 * \fn int ShowOpenDialog (HWND hWnd, int lx, int ty,\
                int w, int h, PNEWFILEDLGDATA pnfdd)
 * \brief Creates an Open File Dialog Box.
 *
 * This function creates an Open File Dialog Box, and returns 
 * the full path name of the file selected by user.
 *
 * \param hWnd The hosting main window.
 * \param lx lx,ty,w,h: The default position and size of the dialog box.
 * \param ty lx,ty,w,h: The default position and size of the dialog box.
 * \param w  lx,ty,w,h: The default position and size of the dialog box.
 * \param h  lx,ty,w,h: The default position and size of the dialog box.
 * \param pnfdd The pointer to the NEWFILEDLGDATA structure.
 *
 * \retval IDOK The user choosed a file and OK button clicked.
 * \retval IDCANCLE CANCEL button clicked.
 * 
 * \sa NEWFILEDLGDATA, ShowOpenDialog
 */
MGUTILS_EXPORT int ShowOpenDialog (HWND hWnd, int lx, int ty, 
                int w, int h, PNEWFILEDLGDATA pnfdd);

    /** @} end of mywins_newfiledlg */

    /** @} end of mywins_fns */


    /**
     * \addtogroup vcongui_fns Interfaces of mGUtils library (libmgutils)
     *
     * mGUtils provides a virtual console in a main window of MiniGUI.
     *
     * @{
     */

#ifndef __cplusplus
#ifndef __ECOS__
typedef	enum {false, true} bool;
#endif
#endif

#define VCONGUI_VERSION "Version 0.5 (Nov. 2001)"

#define MIN_COLS    10
#define MAX_COLS    100
#define MIN_ROWS    10
#define MAX_ROWS    60

#define GetCharWidth GetSysCharWidth
#define GetCCharWidth GetSysCCharWidth
#define GetCharHeight GetSysCharHeight

/**
 * Information of child process created.
 * \sa VCOnMiniGUI
 */
typedef struct _CHILDINFO
{
    /** Whether display start up messages. */
    bool startupMessage;
    /** Customized string will be displayed as a startup message. */
    const char* startupStr;
    /**
     * The program should be executed when startup
     * (If it is NULL, MiniGUI will try to execute the default shell).
     */
    const char* execProg;
    /** The argument of the startup program if \a execProg is NULL. */
    const char* execArgs;
    
    /**
     * The customized default window procedure of vcongui main window
     * (If it is NULL, MiniGUI will call the default main window procedure).
     */
    WNDPROC     DefWinProc;
    /** Whether display a menu */
    bool        fMenu;
    /** The initial position of the vcongui main window. */
    int         left, top;
    /** The number of terminal rows and columns. */
    int         rows, cols;
}CHILDINFO;
/** Data type of pointer to a CHILDINFO */
typedef CHILDINFO* PCHILDINFO;

/**
 * \fn void* VCOnMiniGUI (void* data)
 * \brief Creates a vcongui main window.
 *
 * This function creates a vcongui main window, and enter a message loop.
 * 
 * \param data The pointer to a CHILDINFO structure.
 */
void* VCOnMiniGUI (void* data);

#ifdef _MGRM_THREADS
void* NewVirtualConsole (PCHILDINFO pChildInfo);
#endif

    /** @} end of vcongui_fns */

    /**
     * \addtogroup templates_fns Interfaces of mGUtils library (libmgutils)
     *
     * mGUtils provides a dialog in a main window of MiniGUI.
     *
     * @{
     */

/**
* \fn int ShowCommonDialog (PDLGTEMPLATE dlg_template, HWND hwnd, \
WNDPROC proc, void* private_data)
* \ brief Creates a modal common dialog box from a dialog box
*          template in memory and other information.
*
* This function can be used for file choosing dialog, color selecting
* dialog, font selecting dialog, and information dialog.
*
* \param dlg_template The pointer to a DLGTEMPLATE structure.
* \param hwnd The handle to the hosting main window.
* \param proc The window procedure of the common dialog box.
* \param private_data The parameter will be passed to the window procedure.
*
* \return If the user clicks OK or CLOSE button of the dialog box, the
*         return value is TRUE, otherwise return FALSE.
*/
MGUTILS_EXPORT BOOL ShowCommonDialog (PDLGTEMPLATE dlg_template, HWND hwnd, 
        WNDPROC proc, void* private_data);


typedef struct _FILEDLGDATA
{
        /** Indicates to create a Save File or an Open File dialog box. */
        BOOL        is_save;
        
        /** Indicates the controls is transparent or no? */
        BOOL        is_trans;
        
        /** The full path name of the file returned. */
        char        filefullname[MY_NAMEMAX + MY_PATHMAX + 1];
        
        /** The name of the file to be opened. */
        char        filename[MY_NAMEMAX + 1];
        
        /** The initial path of the dialog box. */
        char        filepath[MY_PATHMAX + 1];
        
        /**
        * The filter string, for example: 
        * All file (*.*)|Text file (*.txt;*.TXT)
        */     
        char        filter[MAX_FILTER_LEN + 1];
        
        /** The initial index of the filter*/
        int         filterindex;
        
        WNDPROC     hook;
} FILEDLGDATA, *PFILEDLGDATA;

#define IDC_FOSD_BASE                   520
#define IDC_FOSD_PATH_NOTE              (IDC_FOSD_BASE + 1) 
#define IDC_FOSD_PATH                   (IDC_FOSD_BASE + 2) 
#define IDC_FOSD_UPPER                  (IDC_FOSD_BASE + 3) 
#define IDC_FOSD_FILELIST               (IDC_FOSD_BASE + 4) 
#define IDC_FOSD_FILENAME_NOTE          (IDC_FOSD_BASE + 5) 
#define IDC_FOSD_FILENAME               (IDC_FOSD_BASE + 6) 
#define IDC_FOSD_FILETYPE_NOTE          (IDC_FOSD_BASE + 7) 
#define IDC_FOSD_FILETYPE               (IDC_FOSD_BASE + 8) 
#define IDC_FOSD_ISHIDE                 (IDC_FOSD_BASE + 9) 
#define IDC_FOSD_OK                     (IDC_FOSD_BASE + 10)
#define IDC_FOSD_CANCEL                 (IDC_FOSD_BASE + 11)

/**
* \fn int FileOpenSaveDialog (PDLGTEMPLATE dlg_template, HWND hwnd, \
WNDPROC proc, PFILEDLGDATA pfdd)
* \ brief Creates a modal Open/Save File Dialog Box.
*
* This function creates an Open/Save File Dialog Box, and returns
* the full path name of the file selected by user.
*
* \param dlg_template The pointer to a DLGTEMPLATE structure.
* \param hwnd The handle to the hosting main window.
* \param proc The window procedure of the common dialog box.
* \param pfdd The pointer to the FILEDLGDATA structure.
*
* \return If the user clicks OK or CLOSE button of the dialog box, the
*         return value is TRUE, otherwise return FALSE.
*/
MGUTILS_EXPORT BOOL FileOpenSaveDialog  (PDLGTEMPLATE dlg_template, HWND hwnd, 
        WNDPROC proc, PFILEDLGDATA pfdd);


typedef struct _COLORDLGDATA {
        /** The value of the color  returned. */
        gal_pixel       pixel;
        
        /** The R, G, B value of the color returned. */
        Uint8           r, g, b;
        
        /** The H value of the color returned. */
        Uint16          h;
        
        /** The S, V value of the color returned. */
        Uint8           s, v;

        /** Indicates the controls is transparent or no?*/
        BOOL            is_trans;
        
        WNDPROC         hook;
} COLORDLGDATA, *PCOLORDLGDATA;

#define IDC_CSD_BASE                600 
#define IDC_CSD_BASIC_COLOR_NOTE    (IDC_CSD_BASE + 1) 
#define IDC_CSD_BASIC_COLOR          (IDC_CSD_BASE + 2)
#define IDC_CSD_BASIC_COLOR_0       (IDC_CSD_BASE + 2)
#define IDC_CSD_BASIC_COLOR_1       (IDC_CSD_BASE + 3)
#define IDC_CSD_BASIC_COLOR_2       (IDC_CSD_BASE + 4)
#define IDC_CSD_BASIC_COLOR_3       (IDC_CSD_BASE + 5)
#define IDC_CSD_BASIC_COLOR_4       (IDC_CSD_BASE + 6)
#define IDC_CSD_BASIC_COLOR_5       (IDC_CSD_BASE + 7)
#define IDC_CSD_BASIC_COLOR_6       (IDC_CSD_BASE + 8)
#define IDC_CSD_BASIC_COLOR_7       (IDC_CSD_BASE + 9)
#define IDC_CSD_BASIC_COLOR_8       (IDC_CSD_BASE + 10)  
#define IDC_CSD_BASIC_COLOR_9       (IDC_CSD_BASE + 11)
#define IDC_CSD_BASIC_COLOR_10      (IDC_CSD_BASE + 12)
#define IDC_CSD_BASIC_COLOR_11      (IDC_CSD_BASE + 13)
#define IDC_CSD_BASIC_COLOR_12      (IDC_CSD_BASE + 14)
#define IDC_CSD_BASIC_COLOR_13      (IDC_CSD_BASE + 15)
#define IDC_CSD_BASIC_COLOR_14      (IDC_CSD_BASE + 16)
#define IDC_CSD_BASIC_COLOR_15      (IDC_CSD_BASE + 17)
#define IDC_CSD_BASIC_COLOR_16      (IDC_CSD_BASE + 18)
#define IDC_CSD_BASIC_COLOR_17      (IDC_CSD_BASE + 19)
#define IDC_CSD_BASIC_COLOR_18      (IDC_CSD_BASE + 20)
#define IDC_CSD_BASIC_COLOR_19      (IDC_CSD_BASE + 21)
#define IDC_CSD_BASIC_COLOR_20      (IDC_CSD_BASE + 22)
#define IDC_CSD_BASIC_COLOR_21      (IDC_CSD_BASE + 23)
#define IDC_CSD_BASIC_COLOR_22      (IDC_CSD_BASE + 24)
#define IDC_CSD_BASIC_COLOR_23      (IDC_CSD_BASE + 25)
#define IDC_CSD_BASIC_COLOR_24      (IDC_CSD_BASE + 26)
#define IDC_CSD_BASIC_COLOR_25      (IDC_CSD_BASE + 27)
#define IDC_CSD_BASIC_COLOR_26      (IDC_CSD_BASE + 28)
#define IDC_CSD_BASIC_COLOR_27      (IDC_CSD_BASE + 29)
#define IDC_CSD_BASIC_COLOR_28      (IDC_CSD_BASE + 30)
#define IDC_CSD_BASIC_COLOR_29      (IDC_CSD_BASE + 31)
#define IDC_CSD_CUSTOM_COLOR_NOTE   (IDC_CSD_BASE + 32)
#define IDC_CSD_CUSTOM_COLOR_0      (IDC_CSD_BASE + 33)
#define IDC_CSD_CUSTOM              (IDC_CSD_BASE + 33)
#define IDC_CSD_CUSTOM_COLOR_1      (IDC_CSD_BASE + 34)
#define IDC_CSD_CUSTOM_COLOR_2      (IDC_CSD_BASE + 35)
#define IDC_CSD_CUSTOM_COLOR_3      (IDC_CSD_BASE + 36)
#define IDC_CSD_CUSTOM_COLOR_4      (IDC_CSD_BASE + 37)
#define IDC_CSD_CUSTOM_COLOR_5      (IDC_CSD_BASE + 38)
#define IDC_CSD_SPACE               (IDC_CSD_BASE + 39)
#define IDC_CSD_YSPACE              (IDC_CSD_BASE + 40)
#define IDC_CSD_COLOR               (IDC_CSD_BASE + 41)
#define IDC_CSD_NOTE_H              (IDC_CSD_BASE + 42)
#define IDC_CSD_NOTE_S              (IDC_CSD_BASE + 43)
#define IDC_CSD_NOTE_V              (IDC_CSD_BASE + 44)
#define IDC_CSD_NOTE_R              (IDC_CSD_BASE + 45)
#define IDC_CSD_NOTE_G              (IDC_CSD_BASE + 46)
#define IDC_CSD_NOTE_B              (IDC_CSD_BASE + 47)
#define IDC_CSD_VALUE_H             (IDC_CSD_BASE + 48)
#define IDC_CSD_VALUE_S             (IDC_CSD_BASE + 49)
#define IDC_CSD_VALUE_V             (IDC_CSD_BASE + 50)
#define IDC_CSD_VALUE_R             (IDC_CSD_BASE + 51)
#define IDC_CSD_VALUE_G             (IDC_CSD_BASE + 52)
#define IDC_CSD_VALUE_B             (IDC_CSD_BASE + 53)
#define IDC_CSD_ADD                 (IDC_CSD_BASE + 54)
#define IDC_CSD_OK                  (IDC_CSD_BASE + 55)
#define IDC_CSD_CANCEL              (IDC_CSD_BASE + 56)

/**
* \fn int ColorSelectDialog (PDLGTEMPLATE dlg_template, HWND hwnd, \
WNDPROC proc, PCOLORDLGDATA pcdd)
* \ brief Creates a modal Color Selection Dialog Box.
*
* This function creates an Color Selection Dialog Box, and returns
* the values of the color selected by the user.
*
* \param dlg_template The pointer to a DLGTEMPLATE structure.
* \param hwnd The handle to the hosting main window.
* \param proc The window procedure of the color selection dialog box.
* \param pcdd The pointer to the COLORDLGDATA structure.
*
* \return If the user clicks OK or CLOSE button of the dialog box, the
*         return value is TRUE, otherwise return FALSE.
*/
MGUTILS_EXPORT BOOL ColorSelectDialog  (PDLGTEMPLATE dlg_template, HWND hwnd, 
        WNDPROC proc, PCOLORDLGDATA pcdd);


typedef struct _FONTDLGDATA {
        /* The font minimize size. */
        int             min_size;
        /* The font maximize size. */
        int             max_size;
        /* Indicates the controls is transparent or no?*/
        BOOL            is_trans;       
        
        /* The font color. */
        RGB             color;
        PLOGFONT        logfont;
        
        WNDPROC         hook;
} FONTDLGDATA, *PFONTDLGDATA;

#define IDC_FSD_BASE            540
#define IDC_FSD_FONT_NOTE       (IDC_FSD_BASE + 1) 
#define IDC_FSD_FONT            (IDC_FSD_BASE + 2)
#define IDC_FSD_STYLE_NOTE      (IDC_FSD_BASE + 3)
#define IDC_FSD_STYLE           (IDC_FSD_BASE + 4)
#define IDC_FSD_SIZE_NOTE       (IDC_FSD_BASE + 5)
#define IDC_FSD_SIZE            (IDC_FSD_BASE + 6)
#define IDC_FSD_EFFECTS_NOTE    (IDC_FSD_BASE + 7)
#define IDC_FSD_FLIP_NOTE       (IDC_FSD_BASE + 8)
#define IDC_FSD_FLIP            (IDC_FSD_BASE + 9)
#define IDC_FSD_COLOR_NOTE      (IDC_FSD_BASE + 10)  
#define IDC_FSD_COLOR           (IDC_FSD_BASE + 11)
#define IDC_FSD_ITALIC          (IDC_FSD_BASE + 12)
#define IDC_FSD_STRIKEOUT       (IDC_FSD_BASE + 13)
#define IDC_FSD_UNDERLINE       (IDC_FSD_BASE + 14)
#define IDC_FSD_SAMPLE_NOTE     (IDC_FSD_BASE + 15)
#define IDC_FSD_SAMPLE          (IDC_FSD_BASE + 16)
#define IDC_FSD_CHARSET_NOTE    (IDC_FSD_BASE + 17)
#define IDC_FSD_CHARSET         (IDC_FSD_BASE + 18)
#define IDC_FSD_OK              (IDC_FSD_BASE + 19)
#define IDC_FSD_CANCEL          (IDC_FSD_BASE + 20)

/**
* \fn int FontSelectDialog (PDLGTEMPLATE dlg_template, HWND hwnd, \
WNDPROC proc, PFONTDLGDATA pfsd)
* \ brief Creates a modal Font Selection Dialog Box.
*
* This function creates an Font Selection Dialog Box, and returns
* the pointer to the logical font selected by user.
*
* \param dlg_template The pointer to a DLGTEMPLATE structure.
* \param hwnd The handle to the hosting main window.
* \param proc The window procedure of the font selection dialog box.
* \param pfsd The pointer to the FONTDLGDATA structure.
*
* \return If the user clicks OK or CLOSE button of the dialog box, the
*         return value is TRUE, otherwise return FALSE.
*/
MGUTILS_EXPORT BOOL FontSelectDialog  (PDLGTEMPLATE dlg_template, HWND hwnd, 
        WNDPROC proc, PFONTDLGDATA pfsd);


typedef struct _INFODLGDATA {
        const char*     msg;
        WNDPROC         hook;
        
        /* for internal usage.*/
        int             nr_lines;
        int             vis_lines;
        int             start_line;
        RECT            rc;
} INFODLGDATA, *PINFODLGDATA;

#define IDC_IFD_SPIN      580

/**
* \fn int InfoShowDialog (PDLGTEMPLATE dlg_template, HWND hwnd, \
WNDPROC proc, PINFODLGDATA pidd)
* \ brief Creates a modal Information Dialog Box.
*
* This function creates a Information Dialog Box.
*
* \param dlg_template The pointer to a DLGTEMPLATE structure.
* \param hwnd The handle to the hosting main window.
* \param proc The window procedure of the information dialog box.
* \param pfdd The pointer to the INFODLGDATA structure.
*
* \return If the user clicks OK or CLOSE button of the dialog box, the
*         return value is TRUE, otherwise return FALSE.
*/
MGUTILS_EXPORT BOOL InfoShowDialog  (PDLGTEMPLATE dlg_template, HWND hwnd, 
        WNDPROC proc, PINFODLGDATA pidd);

extern DLGTEMPLATE DefFileDlg;
extern DLGTEMPLATE DefColorDlg;
extern DLGTEMPLATE DefFontDlg;
extern DLGTEMPLATE DefInfoDlg;

extern DLGTEMPLATE DefSimpleFileDlg;
extern DLGTEMPLATE DefSimpleColorDlg;
extern DLGTEMPLATE DefSimpleFontDlg;
extern DLGTEMPLATE DefSimpleInfoDlg;

/** The default File Open/Save Dialog callback procedure. */
extern int DefFileDialogProc (HWND hDlg, int message, WPARAM wParam, LPARAM lParam);

/** The default Color Selection Dialog callback procedure. */
extern int DefColorDialogProc (HWND hDlg, int message, WPARAM wParam, LPARAM lParam);

/** The default Font Selection Dialog callback procedure. */
extern int DefFontDialogProc (HWND hDlg, int message, WPARAM wParam, LPARAM lParam);

/** The default Information Dialog callback procedure. */
extern int DefInfoDialogProc (HWND hDlg, int message, WPARAM wParam, LPARAM lParam);

    /** @} end of templates_fns */

#ifdef  __cplusplus
}
#endif

#endif /* _MGUTILS_H */
