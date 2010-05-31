#!/bin/bash

TOP=../..
OUTFILE=${TOP}/mainframe/dlgtmpls.c
HDRFILES=${TOP}/include/dlgtmpls.h
NCS2ICS=ncs2ics

echo > $OUTFILE

SRCS=( "font_management FontManager" \
	 "import_image ImportImage" \
	 "new_file NewFile" \
	 "new_renderer NewRenderer" \
	 "new_renderer_set NewRendererSet" \
	 "add_renderer AddRenderer" \
	 "set_default_renderer SetDefaultRenderer" \
	 "set_screen_size SetScreenSize" \
	 "text_profile AddLang TextProfile" \
	 "connect_events ConnectEvent SelectEvent InputEventName" \
	 "font_select FontSelect" \
	 "idrangemanager IDRangeEditor NewIDRange ExtendIDRange" \
	 "about About" )

cat > $HDRFILES <<_ACEOF
/**
 * define the dialog template
 * for guibuilder.
*/
#ifndef DLG_TEMPLATE_H
#define DLG_TEMPLATE_H


_ACEOF

cat > $OUTFILE <<_ACEOF

#include <minigui/common.h>                                   
#include <minigui/minigui.h>        
#include <minigui/gdi.h>      
#include <minigui/window.h>     
#include <minigui/control.h>     

_ACEOF

i=0
j=0
while [ $i -lt ${#SRCS[@]} ]
do
	INFO=( ${SRCS[$i]} )
	NAMES=( ${INFO[@]:1} )
	#echo "$NCS2ICS ${INFO[0]} $OUTFILE ${NAMES[@]}"
	$NCS2ICS ${INFO[0]} $OUTFILE ${NAMES[@]}
	echo  >> $OUTFILE
	
	k=0
	while [ $k -lt ${#NAMES[@]} ]
	do
		ID_NAME=ID_`echo ${NAMES[$k]} | tr a-z A-Z`
		echo "#define $ID_NAME  `expr $j + 10000`" >> $HDRFILES
		k=`expr $k + 1`
		j=`expr $j + 1`
	done
	i=`expr $i + 1`
done

cat >> $HDRFILES <<_ACEOF

#ifdef __cplusplus
extern "C" {
#endif

extern DLGTEMPLATE * GetDlgTemplate(int id);

extern int AutoCenterDlgProc(HWND hwnd, int message, WPARAM wParam, LPARAM lParam);

extern PCTRLDATA GetControlData(DLGTEMPLATE *tmpl, int id);

#ifdef __cplusplus
}
#endif

#endif /*DLG_TEMPLATE_H*/

_ACEOF

sed -i 's/DLGTEMPLATE/static DLGTEMPLATE/g' $OUTFILE


cat >>$OUTFILE <<_ACEOF

static DLGTEMPLATE* templs[] = {
_ACEOF

i=0
while [ $i -lt ${#SRCS[@]} ]
do
	INFO=( ${SRCS[$i]} )
	NAMES=( ${INFO[@]:1} )
	k=0
	while [ $k -lt ${#NAMES[@]} ]
	do
		echo "	&_${NAMES[$k]}_templ," >> $OUTFILE
		k=`expr $k + 1`
	done
	i=`expr $i + 1`
done

cat >> $OUTFILE <<_ACEOF
};


DLGTEMPLATE * GetDlgTemplate(int id)
{
	int count = sizeof(templs) / sizeof(DLGTEMPLATE*);
	id -= 10000;
	if(id < 0 || id >= count)
		return NULL;

	if(templs[id])
		templs[id]->dwStyle &= ~WS_VISIBLE;
	
	return templs[id];
}

int AutoCenterDlgProc(HWND hwnd, int message, WPARAM wParam, LPARAM lParam)
{
	int ret = DefaultDialogProc(hwnd, message, wParam, lParam);

	if(message == MSG_CREATE)
	{
		RECT rcSelf, rcParent;
		HWND hMain;
		int x, y;
		GetWindowRect(hwnd, &rcSelf);
		hMain = GetMainWindowHandle(hwnd);
		if(hMain == hwnd){
			rcParent = g_rcScr;
		}
		else
		{
			GetWindowRect(hwnd, &rcParent);
		}
		
		x = rcParent.left + (RECTW(rcParent) - RECTW(rcSelf)) / 3;
		y = rcParent.top + (RECTH(rcParent) - RECTH(rcSelf)) / 3;
		MoveWindow(hwnd, x, y, RECTW(rcSelf), RECTH(rcSelf), TRUE);
	}
	return ret;
}


PCTRLDATA GetControlData(DLGTEMPLATE *tmpl, int id)
{
	PCTRLDATA pctrl;
	int i;
	if(!tmpl)
		return NULL;

	for(i = 0; i < tmpl->controlnr; i++)
	{
		pctrl = &tmpl->controls[i];
		if(pctrl->id == id)
			return pctrl;
	}
	return NULL;
}

_ACEOF


