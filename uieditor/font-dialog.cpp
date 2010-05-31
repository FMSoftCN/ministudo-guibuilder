/*
** $Id:$
**
** fontdlg.c: Font Select Dialog.
**
** Copyright (C) 2004 ~ 2008 Feynman Software.
**
** Current maintainer:
**
** Create date: 2008/04/08
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <list>
#include <vector>
#include "mapex.h"

#include "mgheads.h"
#include "mgfcheads.h"
#include <mgutils/mgutils.h>
#include "msd_intl.h"
using namespace std;

#include "resenv.h"
#include "font-dialog.h"

#include "dlgtmpls.h"

#ifndef WIN32
#include <unistd.h>
#include <sys/mman.h>
#else
#include "func-win.h"
#endif

#define IDC_NCSFD_BASE            140
#define IDC_NCSFD_FONT_NOTE       (IDC_NCSFD_BASE + 1)
#define IDC_NCSFD_FONT            (IDC_NCSFD_BASE + 2)
#define IDC_NCSFD_STYLE_NOTE      (IDC_NCSFD_BASE + 3)
#define IDC_NCSFD_STYLE           (IDC_NCSFD_BASE + 4)
#define IDC_NCSFD_SIZE_NOTE       (IDC_NCSFD_BASE + 5)
#define IDC_NCSFD_SIZE            (IDC_NCSFD_BASE + 6)
#define IDC_NCSFD_EFFECTS_NOTE    (IDC_NCSFD_BASE + 7)
#define IDC_NCSFD_FLIP_NOTE       (IDC_NCSFD_BASE + 8)
#define IDC_NCSFD_FLIP            (IDC_NCSFD_BASE + 9)
#define IDC_NCSFD_ITALIC          (IDC_NCSFD_BASE + 10)
#define IDC_NCSFD_STRIKEOUT       (IDC_NCSFD_BASE + 11)
#define IDC_NCSFD_UNDERLINE       (IDC_NCSFD_BASE + 12)
#define IDC_NCSFD_SAMPLE          (IDC_NCSFD_BASE + 13)
#define IDC_NCSFD_CHARSET_NOTE    (IDC_NCSFD_BASE + 14)
#define IDC_NCSFD_CHARSET         (IDC_NCSFD_BASE + 15)
#define IDC_NCSFD_OK              (IDC_NCSFD_BASE + 16)
#define IDC_NCSFD_CANCEL          (IDC_NCSFD_BASE + 17)

#define ARRAY_LEN(array) (sizeof(array)/sizeof(array[0]))

/* default templates */
CTRLDATA DefFontCtrl [] =
{
    { CTRL_STATIC,
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        7, 5, 60, 20,
        IDC_NCSFD_FONT_NOTE,
        _("Family:"),
        0, WS_EX_TRANSPARENT | WS_EX_USEPARENTRDR
    },
    { CTRL_COMBOBOX,
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST
        | CBS_NOTIFY | CBS_READONLY | WS_BORDER | CBS_EDITNOBORDER,
        7, 27, 123, 20,
        IDC_NCSFD_FONT,
        "",
        0,  WS_EX_USEPARENTRDR
    },
    { CTRL_STATIC,
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        140, 5, 50, 20,
        IDC_NCSFD_STYLE_NOTE,
        _("Style:"),
        0,  WS_EX_USEPARENTRDR
    },
    { CTRL_COMBOBOX,
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | CBS_NOTIFY
        | CBS_READONLY | WS_BORDER | CBS_EDITNOBORDER,
        140, 27, 100, 20,
        IDC_NCSFD_STYLE,
        "",
        0,  WS_EX_USEPARENTRDR
    },
    { CTRL_STATIC,
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        250, 5, 40, 20,
        IDC_NCSFD_SIZE_NOTE,
        _("Size:"),
        0, WS_EX_TRANSPARENT | WS_EX_USEPARENTRDR
    },
    { CTRL_COMBOBOX,
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | CBS_NOTIFY
        | CBS_READONLY | WS_BORDER | CBS_EDITNOBORDER,
        250, 27, 55, 20,
        IDC_NCSFD_SIZE,
        "",
        0,  WS_EX_USEPARENTRDR
    },
    { CTRL_STATIC,
        WS_CHILD | WS_VISIBLE | SS_GROUPBOX ,
        7, 50, 123, 90,
        IDC_NCSFD_EFFECTS_NOTE,
        _("Effects"),
        0, WS_EX_USEPARENTRDR | WS_EX_TRANSPARENT
    },
    { CTRL_BUTTON,
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
        10 , 70, 100, 20,
        IDC_NCSFD_ITALIC,
        _("Italic"),
        0, WS_EX_USEPARENTRDR
    },
    { CTRL_BUTTON,
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
        10 , 90, 100, 20,
        IDC_NCSFD_STRIKEOUT,
        _("Strikeout"),
        0, WS_EX_USEPARENTRDR
    },
    { CTRL_BUTTON,
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
        10 , 110, 100, 20,
        IDC_NCSFD_UNDERLINE,
        _("Underline"),
        0, WS_EX_USEPARENTRDR
    },
    { CTRL_STATIC,
        WS_CHILD | WS_VISIBLE |SS_LEFT,
        140, 65, 60, 20,
        IDC_NCSFD_CHARSET_NOTE,
        _("Charset"),
        0, WS_EX_TRANSPARENT | WS_EX_USEPARENTRDR
    },
    { CTRL_COMBOBOX,
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | CBS_NOTIFY
        | CBS_READONLY | WS_BORDER | CBS_EDITNOBORDER,
        205, 65, 100, 20,
        IDC_NCSFD_CHARSET,
        "",
        0, WS_EX_USEPARENTRDR
    },
    { CTRL_STATIC,
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        10, 150, 40, 20,
        IDC_NCSFD_FLIP_NOTE,
        _("Flip"),
        0, WS_EX_TRANSPARENT | WS_EX_USEPARENTRDR
    },
    { CTRL_COMBOBOX,
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | CBS_NOTIFY
        | CBS_READONLY | WS_BORDER | CBS_EDITNOBORDER,
        50, 150, 80, 20,
        IDC_NCSFD_FLIP,
        "",
        0,0
    },
    { CTRL_STATIC,
        WS_CHILD | WS_VISIBLE | WS_BORDER | SS_LEFT,
        140, 100, 165, 75,
        IDC_NCSFD_SAMPLE,
        _("Sample"),
        0, WS_EX_TRANSPARENT | WS_EX_USEPARENTRDR
    },
    { CTRL_BUTTON,
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
        80 , 180, 100, 25,
        IDC_NCSFD_CANCEL,
        _("Cancel"),
        0, WS_EX_USEPARENTRDR
    },
    { CTRL_BUTTON,
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
        200, 180, 100, 25,
        IDC_NCSFD_OK,
        _("Ok"),
        0, WS_EX_USEPARENTRDR
    }
};

DLGTEMPLATE DefFontDlg =
{
    WS_DLGFRAME | WS_BORDER | WS_CAPTION,
    WS_EX_USEPARENTRDR,
    0, 0, 320, 240,
    _("Font Selecting"), 0, 0,
    TABLESIZE(DefFontCtrl),
    DefFontCtrl
};


/////////////////////////////////////////


static NCS_PFONT InsertFont (NCS_PFONT head, const char* family)
{
    NCS_PFONT cur = NULL;
    NCS_PFONT pre = NULL;

    pre = head;
    cur = head->next;
    while (cur)
    {
        if (strcasecmp (cur->name, family) == 0)
            return cur;
        pre = cur;
        cur = cur->next;
    }

    cur = (NCS_PFONT)calloc(1,sizeof(*cur));
    strncpy (cur->name, family, LEN_FONT_NAME);
    cur->name [LEN_FONT_NAME] = '\0';
    cur->_ncs_chset.next = NULL;
    cur->next = NULL;

    pre->next = cur;
    return cur;
}

static NCS_PFONT FindFont (NCS_PFONT head, const char* family)
{
    NCS_PFONT cur = head->next;

    while (cur)
    {
        if (strcasecmp (cur->name, family) == 0)
            return cur;
        cur = cur->next;
    }
    return NULL;
}

static void InsertCharset (NCS_PFONT font, const char* chset_name)
{
	int len = 0;
    NCS_PCHARSET cur = font->_ncs_chset.next;
    NCS_PCHARSET pre = &(font->_ncs_chset);

	//strip chset_name
	while(chset_name[0] && (chset_name[0] == ' ' || chset_name[0] == '\t'))
		chset_name ++;

	if(!chset_name[0])
		return;

	len = strlen(chset_name);
	while(len > 0 && (chset_name[len-1] == ' ' || chset_name[len-1] == '\t'))
		len --;

	if(len <= 0)
		return;

    while (cur)
    {
        if (strcasecmp (cur->name, chset_name) == 0)
            return;
        pre = cur;
        cur = cur->next;
    }

	if(len > LEN_FONT_NAME)
		len = LEN_FONT_NAME;

    cur = (NCS_PCHARSET)malloc(sizeof(NCS_CHARSET));
    strncpy (cur->name, chset_name, len);
    cur->name [len] = '\0';
    cur->next = NULL;

    pre->next = cur;
}

static int isTrueTypeFont(const char* type)
{
	unsigned int i;

	static const char* ttf_types[] = {"ttf", "type1"};

	for(i=0;i<sizeof(ttf_types)/sizeof(char*); i++)
	{
		if(strcmp(type, ttf_types[i]) == 0)
			return 1;
	}
	return 0;
}

static int * insertIntArray(int * tarr, int tn, int nin, int * newlen, int *insertAt)
{
	int i;
	if(tarr == NULL)
	{
		tarr = (int*)malloc(sizeof(int)*1);
		tarr[0] = nin;
		if(newlen)
			*newlen = 1;
		if(insertAt)
			*insertAt = 0;
		return tarr;
	}

	if(newlen) *newlen = tn;
	for(i=(insertAt?*insertAt:0); i<tn; i++)
	{
		if(tarr[i] == nin)
			return tarr;
		else if(tarr[i] > nin)
			break;
	}

	tarr = (int*)realloc(tarr, sizeof(int)*(tn+1));

	if(i<tn)
	{
		for(;tn>i;tn--)
			tarr[tn] = tarr[tn-1];
	}
	tarr[i] = nin;
	if(newlen)
		(*newlen) ++;
	if(insertAt)
		(*insertAt) = i;
	return tarr;
}

#define GetFontSize(font_ops)   ((int(*)(PLOGFONT,const DEVFONT*,int))(((int*)(font_ops))[4]))
static void EnumSupportSize(NCS_PFONT pfont, const DEVFONT* dev_font)
{
	LOGFONT font;
	const char* str, *str2;
	int i;
	int oldsize = 0;
	int insertAt = 0;

	memset(&font, 0, sizeof(font));
	str = strchr(dev_font->name,'-');
	//type
	strncpy(font.type, dev_font->name, str - dev_font->name);
	if((pfont->is_ttf = isTrueTypeFont(font.type)))
		return;
	//family
	str ++;
	str2 = strchr(str, '-');
	strncpy(font.family, str, str2-str);

	for(i=FONT_MIN_SIZE; i<=FONT_MAX_SIZE; i++)
	{
		int size = GetFontSize(dev_font->font_ops)(&font, dev_font, i);
		if( size != oldsize)
		{
			pfont->supportSize = insertIntArray(pfont->supportSize, pfont->sizeCount, size, &pfont->sizeCount, &insertAt);
			oldsize = size;
		}
	}
}

static NCS_PFONT CreateFontAttrList (void)
{
    int  i;
    char *sn_start, *sn_sep;
    char *fn_start, *fn_end;
	char font_name[LEN_UNIDEVFONT_NAME + 2] = "";
    NCS_PFONT font_head, cur_font;
    const DEVFONT* dev_font = NULL;

    font_head = (NCS_PFONT) calloc (1,sizeof (NCS_FONT));
    font_head->next = NULL;
    font_head->_ncs_chset.next = NULL;

    while ((dev_font = GetNextDevFont (dev_font)))
    {
		memset(font_name,0,sizeof(font_name));
        strncpy (font_name, dev_font->name, LEN_DEVFONT_NAME);
        strcat (font_name, ",");

        fn_start = strchr (font_name, '-') + 1;
        fn_end = strchr (fn_start, '-');
        *fn_end = '\0';

        cur_font = InsertFont (font_head, fn_start);

        sn_start = fn_end + 1;
        for (i = 0; i < 3; i++)
            sn_start = strchr (sn_start, '-') + 1;

        while (*sn_start)
        {
            sn_sep = strchr (sn_start, ',');
            *sn_sep = '\0';
            InsertCharset (cur_font, sn_start);

            sn_start = sn_sep + 1;
        };

		EnumSupportSize(cur_font, dev_font);
    }

    return font_head;
}

static void FreeFontAttrList (NCS_PFONT font_head)
{
    NCS_PFONT    font_tmp, font_cur;
    NCS_PCHARSET chset_cur, chset_tmp;

    font_cur = font_head->next;
    while (font_cur)
    {
        chset_cur = font_cur->_ncs_chset.next;
        while (chset_cur)
        {
            chset_tmp = chset_cur->next;
            free (chset_cur);
            chset_cur = chset_tmp;
        }

		if(font_cur->supportSize)
			free(font_cur->supportSize);

        font_tmp = font_cur->next;
        free (font_cur);
        font_cur = font_tmp;
    }

    free (font_head);
}


static void InsertItem (HWND hWnd, const char** strs, int len)
{
    int i;

    for (i = 0; i < len; i++)
    {
        SendMessage (hWnd, CB_INSERTSTRING, i, (LPARAM)strs[i]);
    }

    SendMessage (hWnd, CB_SETCURSEL, 0, 0);

    return;
}

static inline NCS_PFONT GetSelectFont(HWND hWnd)
{
    char buff [LEN_FONT_NAME + 1];
	int cur_sel = SendDlgItemMessage (hWnd, IDC_NCSFD_FONT, CB_GETCURSEL, 0, 0);
    NCS_PFONTDIA pfdia = (NCS_PFONTDIA)GetWindowAdditionalData (hWnd);
    SendDlgItemMessage (hWnd, IDC_NCSFD_FONT,  CB_GETLBTEXT, cur_sel, (LPARAM)buff);
    return FindFont (pfdia->_ncs_font, buff);
}

static void RefreshSize(HWND hWnd, int size)
{
	int i, sel = 0;
	HWND hctrl = GetDlgItem(hWnd, IDC_NCSFD_SIZE);
	NCS_PFONT cur_font = GetSelectFont(hWnd);
	if(cur_font == NULL)
		return ;

    SendMessage (hctrl, CB_RESETCONTENT, 0, 0);

	if(cur_font->is_ttf){
		for(i = 5; i<= 72; i++)
		{
			char szbuf[10];
			sprintf(szbuf, "%d", i);
			SendMessage(hctrl, CB_ADDSTRING, 0, (LPARAM) szbuf);
			if (size == i)
				sel = i-5;
		}
	}
	else {
		for(i = 0; i < cur_font->sizeCount; i++)
		{
			char szbuf[10];
			sprintf(szbuf, "%d", cur_font->supportSize[i]);
			SendMessage(hctrl, CB_ADDSTRING,0,(LPARAM) szbuf);
			if (size == cur_font->supportSize[i])
				sel = i;
		}
	}
	SendMessage(hctrl, CB_SETCURSEL, sel, 0);
}

static int RefreshCharset (HWND hWnd, const char *sel)
{
	int i = 0, sel_idx = 0;
    HWND         combo;
    NCS_PFONT    cur_font;
    NCS_PCHARSET chset_cur;

    cur_font = GetSelectFont(hWnd);
    if (!cur_font)
        return -1;

	if(sel == NULL)
		sel = "UTF-8";

    combo = GetDlgItem (hWnd, IDC_NCSFD_CHARSET);
    SendMessage (combo, CB_RESETCONTENT, 0, 0);
    chset_cur = cur_font->_ncs_chset.next;
    while (chset_cur)
    {
        SendMessage (combo, CB_ADDSTRING, 0, (LPARAM)chset_cur->name);
        if (sel && strcmp(chset_cur->name, sel) == 0)
        	sel_idx = i;
        chset_cur = chset_cur->next;
        i++;
    }
    SendMessage (combo, CB_SETCURSEL, sel_idx, 0);

    return 0;
}

#define NR_LOOP_FOR_STYLE   2
#define NR_LOOP_FOR_WIDTH   3
#define NR_LOOP_FOR_HEIGHT  4
#define NR_LOOP_FOR_CHARSET 5

static BOOL fontGetFamilyFromName (const char* name, char* family)
{
    int i = 0;
    const char* family_part;

    if ((family_part = strchr (name, '-')) == NULL)
        return FALSE;
    if (*(++family_part) == '\0')
        return FALSE;

    while (family_part [i] && i <= LEN_FONT_NAME) {
        if (family_part [i] == '-') {
            family [i] = '\0';
            break;
        }

        family [i] = family_part [i];
        i++;
    }

    return TRUE;
}

static BOOL fontGetCharsetFromName (const char* name, char* charset)
{
    int i;
    char* delim;
    const char* charset_part = name;

    for (i = 0; i < NR_LOOP_FOR_CHARSET; i++) {
        if ((charset_part = strchr (charset_part, '-')) == NULL)
            return FALSE;
        if (*(++charset_part) == '\0')
            return FALSE;
    }

    if ((delim = (char *)strchr (charset_part, ','))) {
        int len;
        len = delim - charset_part;
        strncpy (charset, charset_part, len);
        charset [len] = '\0';
        return TRUE;
    }

    strncpy (charset, charset_part, LEN_FONT_NAME);
    charset [LEN_FONT_NAME] = '\0';
    return TRUE;
}

static int fontGetHeightFromName (const char* name)
{
    int i;
    const char* height_part = name;
    char height [LEN_FONT_NAME + 1];

    for (i = 0; i < NR_LOOP_FOR_HEIGHT; i++) {
        if ((height_part = strchr (height_part, '-')) == NULL)
            return -1;
        if (*(++height_part) == '\0')
            return -1;
    }

    i = 0;
    while (height_part [i]) {
        if (height_part [i] == '-') {
            height [i] = '\0';
            break;
        }

        height [i] = height_part [i];
        i++;
    }

    if (height_part [i] == '\0')
        return -1;

    return atoi (height);
}

static DWORD fontConvertStyle (const char* style_part)
{
    DWORD style = 0;

    switch (style_part [0]) {
    case FONT_WEIGHT_BLACK:
        style |= FS_WEIGHT_BLACK;
        break;
    case FONT_WEIGHT_BOLD:
        style |= FS_WEIGHT_BOLD;
        break;
    case FONT_WEIGHT_BOOK:
        style |= FS_WEIGHT_BOOK;
        break;
    case FONT_WEIGHT_DEMIBOLD:
        style |= FS_WEIGHT_DEMIBOLD;
        break;
    case FONT_WEIGHT_LIGHT:
        style |= FS_WEIGHT_LIGHT;
        break;
    case FONT_WEIGHT_MEDIUM:
        style |= FS_WEIGHT_MEDIUM;
        break;
    case FONT_WEIGHT_REGULAR:
        style |= FS_WEIGHT_REGULAR;
        break;
	case FONT_WEIGHT_SUBPIXEL:
		style |= FS_WEIGHT_SUBPIXEL;
		break;
    case FONT_WEIGHT_ALL:
        style |= FS_WEIGHT_MASK;
		break;
    default:
        return 0xFFFFFFFF;
    }

    switch (style_part [1]) {
    case FONT_SLANT_ITALIC:
        style |= FS_SLANT_ITALIC;
        break;
    case FONT_SLANT_OBLIQUE:
        style |= FS_SLANT_OBLIQUE;
        break;
    case FONT_SLANT_ROMAN:
        style |= FS_SLANT_ROMAN;
        break;
    case FONT_SLANT_ALL:
        style |= FS_SLANT_MASK;
        break;
    default:
        return 0xFFFFFFFF;
    }

    switch (style_part [2]) {
    case FONT_FLIP_HORZ:
        style |= FS_FLIP_HORZ;
        break;
    case FONT_FLIP_VERT:
        style |= FS_FLIP_VERT;
        break;
    case FONT_FLIP_HORZVERT:
        style |= FS_FLIP_HORZVERT;
        break;
    default:
        break;
    }

    switch (style_part [3]) {
    case FONT_OTHER_AUTOSCALE:
        style |= FS_OTHER_AUTOSCALE;
        break;
    case FONT_OTHER_TTFNOCACHE:
        style |= FS_OTHER_TTFNOCACHE;
        break;
    case FONT_OTHER_TTFKERN:
        style |= FS_OTHER_TTFKERN;
        break;
    case FONT_OTHER_TTFNOCACHEKERN:
        style |= FS_OTHER_TTFNOCACHEKERN;
        break;
    case FONT_OTHER_LCDPORTRAIT:
        style |= FS_OTHER_LCDPORTRAIT;
        break;
    case FONT_OTHER_LCDPORTRAITKERN:
        style |= FS_OTHER_LCDPORTRAITKERN;
        break;
    default:
        break;
    }

    switch (style_part [4]) {
    case FONT_UNDERLINE_LINE:
        style |= FS_UNDERLINE_LINE;
        break;
    case FONT_UNDERLINE_ALL:
        style |= FS_UNDERLINE_MASK;
        break;
    case FONT_UNDERLINE_NONE:
        style &= ~FS_UNDERLINE_MASK;
        break;
    default:
        return 0xFFFFFFFF;
    }

    switch (style_part [5]) {
    case FONT_STRUCKOUT_LINE:
        style |= FS_STRUCKOUT_LINE;
        break;
    case FONT_STRUCKOUT_ALL:
        style |= FS_STRUCKOUT_MASK;
        break;
    case FONT_STRUCKOUT_NONE:
        style &= ~FS_STRUCKOUT_MASK;
        break;
    default:
        return 0xFFFFFFFF;
    }

    return style;
}

static DWORD fontGetStyleFromName (const char* name)
{
    int i;
    const char* style_part = name;
    char style_name[7];

    for (i = 0; i < NR_LOOP_FOR_STYLE; i++) {
        if ((style_part = strchr (style_part, '-')) == NULL)
            return 0xFFFFFFFF;

        if (*(++style_part) == '\0')
            return 0xFFFFFFFF;
    }

    strncpy (style_name, style_part, 6);
    style_name[6] = '\0';

    return fontConvertStyle (style_name);
}

static int InitDialog (HWND hWnd)
{
	unsigned int i = 0, sel = 0;
    HWND         hctrl;
    NCS_PFONT    cur_font ;
    NCS_PFONTDIA pfdia ;
    const char   *def_font = NULL;

    char family[LEN_FONT_NAME + 1];
    char charset[LEN_FONT_NAME + 1];
    DWORD style;
    int height;

    pfdia = (NCS_PFONTDIA)GetWindowAdditionalData(hWnd);
    cur_font = pfdia->_ncs_font->next;
    if (pfdia->pfdd->font_name && strlen(pfdia->pfdd->font_name) > 0)
    	def_font = pfdia->pfdd->font_name;

    if (def_font){
		fontGetFamilyFromName (def_font, family);
		style = fontGetStyleFromName(def_font);
		fontGetCharsetFromName (def_font, charset);
		height = fontGetHeightFromName (def_font);
    }

    hctrl = GetDlgItem (hWnd, IDC_NCSFD_FONT);
    while (cur_font)
    {
        SendMessage (hctrl, CB_ADDSTRING, 0, (LPARAM)cur_font->name);
        if (def_font && strcmp(cur_font->name, family) == 0)
        	sel = i;
        cur_font = cur_font->next;
        i++;
    }
    SendMessage(hctrl, CB_SETCURSEL, sel, 0);

    i = 0; sel = 0;
    hctrl = GetDlgItem (hWnd, IDC_NCSFD_STYLE);
    InsertItem (hctrl, style_str, ARRAY_LEN(style_str));
    for (i = 0; i < ARRAY_LEN(style_value); i++)
    {
    	if (def_font && (style & FS_WEIGHT_MASK) == style_value[i]){
    		sel = i;
    		break;
    	}
    }
    SendMessage(hctrl, CB_SETCURSEL, sel, 0);

    i = 0; sel = 0;
    hctrl = GetDlgItem (hWnd, IDC_NCSFD_FLIP);
    InsertItem (hctrl, flip_str, ARRAY_LEN(flip_str));
    for (i = 0; i < ARRAY_LEN(flip_value); i++)
    {
    	if (def_font && (style & FS_FLIP_MASK) == flip_value[i]){
    		sel = i;
    		break;
    	}
    }
    SendMessage(hctrl, CB_SETCURSEL, sel, 0);

    if (def_font && (style & FS_UNDERLINE_LINE))
    {
    	hctrl = GetDlgItem (hWnd, IDC_NCSFD_UNDERLINE);
    	SendMessage(hctrl, BM_SETCHECK, BST_CHECKED, 0);
    }

    if (def_font && (style & FS_STRUCKOUT_LINE))
    {
    	hctrl = GetDlgItem (hWnd, IDC_NCSFD_STRIKEOUT);
    	SendMessage(hctrl, BM_SETCHECK, BST_CHECKED, 0);
    }

    if (def_font && (style & FS_SLANT_ITALIC))
    {
    	hctrl = GetDlgItem (hWnd, IDC_NCSFD_ITALIC);
    	SendMessage(hctrl, BM_SETCHECK, BST_CHECKED, 0);
    }

    RefreshCharset (hWnd, def_font ? charset : NULL);
    RefreshSize (hWnd, def_font ? height : -1);

	return 0;
}

static LOGFONT* CreateFont(HWND hWnd, char *name)
{
    HWND hctrl;
    int  sel, checked, size;
    char chset [LEN_FONT_NAME +1];
    char family [LEN_FONT_NAME +1];
	char szSize[10];
    char flip, slant, weight, strikeout, underline;

    hctrl = GetDlgItem (hWnd, IDC_NCSFD_FONT);
    sel = SendMessage (hctrl, CB_GETCURSEL, 0, 0);
    SendMessage (hctrl, CB_GETLBTEXT, sel, (LPARAM)family);

    hctrl = GetDlgItem (hWnd, IDC_NCSFD_CHARSET);
    sel = SendMessage (hctrl, CB_GETCURSEL, 0, 0);
    SendMessage (hctrl, CB_GETLBTEXT, sel, (LPARAM)chset);

    hctrl = GetDlgItem (hWnd, IDC_NCSFD_STYLE);
    sel = SendMessage (hctrl, CB_GETCURSEL, 0, 0);
    weight = style_arg [sel];

    hctrl = GetDlgItem (hWnd, IDC_NCSFD_ITALIC);
    checked = SendMessage (hctrl, BM_GETCHECK, 0, 0);
    if (BST_CHECKED == checked)
        slant = FONT_SLANT_ITALIC;
    else
        slant = FONT_SLANT_ROMAN;

    hctrl = GetDlgItem (hWnd, IDC_NCSFD_STRIKEOUT);
    checked = SendMessage (hctrl, BM_GETCHECK, 0, 0);
    if (BST_CHECKED == checked)
        strikeout = FONT_STRUCKOUT_LINE;
    else
        strikeout = FONT_STRUCKOUT_NONE;

    hctrl = GetDlgItem (hWnd, IDC_NCSFD_UNDERLINE);
    checked = SendMessage (hctrl, BM_GETCHECK, 0, 0);
    if (BST_CHECKED == checked)
        underline = FONT_UNDERLINE_LINE;
    else
        underline = FONT_UNDERLINE_NONE;

    sel = SendMessage (GetDlgItem (hWnd, IDC_NCSFD_FLIP), CB_GETCURSEL, 0, 0);
    flip = flip_arg [sel];

	hctrl = GetDlgItem(hWnd, IDC_NCSFD_SIZE);
    sel = SendMessage (hctrl, CB_GETCURSEL, 0, 0);
	SendMessage(hctrl, CB_GETLBTEXT, sel, (LPARAM)szSize);
	size = atoi(szSize);
	if (name)
		sprintf(name, "*-%s-%c%c%c%c%c%c-*-%d-%s",
			family, weight, slant, flip, '*', underline, strikeout, size, chset);

    return CreateLogFont (FONT_TYPE_NAME_ALL, family,
                chset, weight, slant, flip,
                FONT_OTHER_AUTOSCALE, underline,
                strikeout, size, 0);
}

static LOGFONT* UpdateFont(HWND hDlg, PLOGFONT old_font, char *name)
{
	if (old_font)
		DestroyLogFont(old_font);

	LOGFONT* font = CreateFont(hDlg, name);

	if (font)
	{
		SetWindowFont(GetDlgItem(hDlg, IDC_NCSFD_SAMPLE), font);
		return font;
	}
	return (LOGFONT*)NULL;
}


static int SaveFontData (HWND hDlg, PLOGFONT font, char * name)
{
    NCS_PFONTDIA pfdia = NULL;

    pfdia = (NCS_PFONTDIA)GetWindowAdditionalData (hDlg);
    strcpy(pfdia->pfdd->font_name, name);

    return 0;
}

static int FontSelProc (HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{
    static PLOGFONT font = NULL;
    static char font_name[256];
    NCS_PFONTDIA    pfdia = NULL;

    if (message != MSG_INITDIALOG)
        pfdia = (NCS_PFONTDIA)GetWindowAdditionalData(hDlg);

    switch (message)
    {
        case MSG_INITDIALOG:
            {
                pfdia = (NCS_PFONTDIA)malloc(sizeof(NCS_FONTDIA));
                if (pfdia == NULL){
                    fprintf(stderr, "FontSelProc: malloc error.\n");
                    return -1;
                }
                pfdia->_ncs_font = CreateFontAttrList ();
                pfdia->pfdd = (PFONTDATA)lParam;
                SetWindowAdditionalData(hDlg, (DWORD)pfdia);

                InitDialog (hDlg);

                font = UpdateFont(hDlg, NULL, font_name);

                break;
            }
        case MSG_CLOSE:
            {
                if (font)
                    DestroyLogFont (font);

                EndDialog (hDlg, IDCANCEL);
                break;
            }
        case MSG_DESTROY:
            {
                FreeFontAttrList (pfdia->_ncs_font);
                free (pfdia);
                break;
            }

        case MSG_COMMAND:
            {
                int id = LOWORD (wParam);
                int nc = HIWORD (wParam);
                switch (id)
                {
                    case IDC_NCSFD_ITALIC:
                    case IDC_NCSFD_STRIKEOUT:
                    case IDC_NCSFD_UNDERLINE:
                        if (nc == BN_CLICKED)
                        {
                            font = UpdateFont(hDlg, font, font_name);
                        }
                        break;
                    case IDC_NCSFD_FONT:
                        if (nc == CBN_SELCHANGE)
                        {
                            RefreshCharset (hDlg, NULL);
                            RefreshSize(hDlg, -1);
                            font = UpdateFont(hDlg, font, font_name);
                        }
                        break;
                    case IDC_NCSFD_CHARSET:
                    case IDC_NCSFD_STYLE:
                    case IDC_NCSFD_FLIP:
                    case IDC_NCSFD_SIZE:
                        if (nc == CBN_SELCHANGE)
                        {
                            font = UpdateFont(hDlg, font, font_name);
                        }
                        break;
                    case IDC_NCSFD_OK:
                        if (font){
                            SaveFontData (hDlg, font, font_name);
                            DestroyLogFont (font);
                        }
                        EndDialog (hDlg, IDOK);
                        break;
                    case IDC_NCSFD_CANCEL:
                        if (font)
                            DestroyLogFont (font);

						EndDialog (hDlg, IDCANCEL);
						break;
                    default:
                    	break;
                }
                break;
            }
    }

    return DefaultDialogProc (hDlg, message, wParam, lParam);
}

BOOL FontSelDlg  (PDLGTEMPLATE dlg_template, HWND hwnd, WNDPROC proc, PFONTDATA pfdd)
{
    PDLGTEMPLATE font_dlg;
    WNDPROC font_proc;

    if (dlg_template) {
        font_dlg = dlg_template;
    }
    else {
        font_dlg = &DefFontDlg;
    }

    if (proc) {
        font_proc = proc;
    }
    else {
        font_proc = FontSelProc;
    }

    return ShowCommonDialog (font_dlg, hwnd, font_proc, pfdd);
}


/////////////////////////////////////////////////////////////////
//font manage dialog
/////////////////////////////////////////////////////////////////

#define IDC_FMD_LISTVIEW 	161
//#define IDC_FMD_GLOBAL		162
#define IDC_FMD_IMPORT		163
#define IDC_FMD_OK			164
#define IDC_FMD_CANCEL		165
#define IDC_FMD_CAP_FEDIT	166
#define IDC_FMD_CAP_FBTN	167
#define IDC_FMD_CLT_FEDIT	168
#define IDC_FMD_CLT_FBTN	169

#if 0
static CTRLDATA FontMngCtrl [] =
{
    {
    	CTRL_LISTVIEW,
        WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | LVS_WITHGRID | LVS_NOTIFY,
        7, 10, 460, 200,
        IDC_FMD_LISTVIEW,
        "",
        0, WS_EX_TRANSPARENT | WS_EX_USEPARENTRDR
    },
    { CTRL_STATIC,
        WS_CHILD | WS_VISIBLE | SS_GROUPBOX,
        7, 230, 440, 120,
        0,
        "Default fonts for project:",
        0, WS_EX_USEPARENTRDR | WS_EX_TRANSPARENT
    },
    { CTRL_STATIC,
        WS_CHILD | WS_VISIBLE | SS_LEFT ,
        17, 270, 105, 25,
        0,
        "Caption Font:",
        0, WS_EX_USEPARENTRDR | WS_EX_TRANSPARENT
    },
    { CTRL_SLEDIT,
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_READONLY,
        125, 265, 250, 25,
        IDC_FMD_CAP_FEDIT,
        "",
        0, WS_EX_USEPARENTRDR
    },
    {
        CTRL_BUTTON,
        WS_CHILD | WS_VISIBLE,
        385, 265, 30, 25,
        IDC_FMD_CAP_FBTN,
        "...",
        0, WS_EX_TRANSPARENT
    },
    { CTRL_STATIC,
        WS_CHILD | WS_VISIBLE | SS_LEFT ,
        17, 310, 105, 25,
        0,
        "Client Font:",
        0, WS_EX_USEPARENTRDR | WS_EX_TRANSPARENT
    },
    { CTRL_SLEDIT,
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_READONLY,
        125, 305, 250, 25,
        IDC_FMD_CLT_FEDIT,
        "",
        0, WS_EX_USEPARENTRDR
    },
    {
        CTRL_BUTTON,
        WS_CHILD | WS_VISIBLE,
        385, 305, 30, 25,
        IDC_FMD_CLT_FBTN,
        "...",
        0, WS_EX_TRANSPARENT
    },
    {
        CTRL_BUTTON,
        WS_CHILD | WS_VISIBLE,
        147, 370, 100, 30,
        IDC_FMD_IMPORT,
        "Import",
        0, WS_EX_TRANSPARENT | WS_EX_USEPARENTRDR
    },
    {
        CTRL_BUTTON,
        WS_CHILD | WS_VISIBLE,
        367, 370, 100, 30,
        IDOK,
        "OK",
        0, WS_EX_TRANSPARENT | WS_EX_USEPARENTRDR
    },
    {
        CTRL_BUTTON,
        WS_CHILD | WS_VISIBLE,
        257, 370, 100, 30,
        IDCANCEL,
        "Cancel",
        0, WS_EX_TRANSPARENT | WS_EX_USEPARENTRDR
    }

};

static DLGTEMPLATE FontMngTmpl =
{
    WS_DLGFRAME | WS_BORDER | WS_CAPTION,
    WS_EX_USEPARENTRDR,
    0, 0, 480, 440,
    "Font Manage", 0, 0,
    TABLESIZE(FontMngCtrl),
    FontMngCtrl
};
#endif

FontMngDlg::FontMngDlg (HWND hParent, MngData *data)
{
	int x,y;
	RECT rp,rs;
	list<ExFont*>::iterator it;

	mngData = data;

	cap_font = *data->cap_font;
	clt_font = *data->clt_font;

	for (it = data->pExFontList->begin(); it != data->pExFontList->end(); ++it){
		exFontList.push_back(*it);
	}

#if 0
	Create(hParent, &FontMngTmpl);
#else
	Create(hParent, GetDlgTemplate(ID_FONTMANAGER));
#endif
	hListView = GetChild(IDC_FMD_LISTVIEW);
	SetWindowAdditionalData(hListView, (DWORD)this);
	GetWindowRect(&rs);
	::GetWindowRect(hParent, &rp);
	x = rp.left + (RECTW(rp) - RECTW(rs))/2;
	y = rp.top + (RECTH(rp) - RECTH(rs))/2;
	if(x<0) x = 0;
	if(y<0) y = 0;
	MoveWindow(x,y,RECTW(rs),RECTH(rs),TRUE);
}

FontMngDlg::~FontMngDlg ()
{
	list<ExFont*>::iterator it;

	for (it = fontList.begin(); it != fontList.end(); ++it){
		delete (*it);
	}
	fontList.clear();
	exFontList.clear();
}

int FontMngDlg::RefreshList (HWND hListView)
{
	unsigned int i;
	LVITEM lvItem = {0};
	LVSUBITEM subdata;
    const DEVFONT* dev_font = NULL;
	list<ExFont*>::iterator it;

    ::SendMessage(hListView, LVM_DELALLITEM, 0, 0);
	for (it = fontList.begin(); it != fontList.end(); ++it){
		delete (*it);
	}
	fontList.clear();

    while ((dev_font = GetNextDevFont (dev_font)))
    {
    	BOOL has = FALSE;
        for (it = fontList.begin(); it != fontList.end(); ++it)
         {
         	if ((*it)->font_name.compare(dev_font->name) == 0)
         		has = TRUE;
         }
        if (!has){
			ExFont *font = new ExFont;
			font->font_name = dev_font->name;
			fontList.push_back(font);
        }
    }

	for (i = 0, it = fontList.begin(); it != fontList.end(); ++it, ++i)
	{
		lvItem.nItem = i;
		lvItem.nItemHeight = 25;
	    GHANDLE item = ::SendMessage(hListView, LVM_ADDITEM, 0, (LPARAM)&lvItem);

	    subdata.flags = 0;
	    subdata.image = 0;
	    subdata.nItem = lvItem.nItem;
	    subdata.subItem = 0;
	    subdata.pszText = (char *)((*it)->font_name.c_str());
	    ::SendMessage (hListView, LVM_SETSUBITEM, item, (LPARAM) &subdata);
	}
/*
	i++;

	for (it = exFontList.begin(); it != exFontList.end(); ++it, ++i)
	{
		lvItem.nItem = i;
		lvItem.nItemHeight = 25;
	    GHANDLE item = ::SendMessage(hListView, LVM_ADDITEM, 0, (LPARAM)&lvItem);

	    subdata.flags = 0;
	    subdata.image = 0;
	    subdata.nItem = lvItem.nItem;
	    subdata.subItem = 0;
	    subdata.pszText = (char *)((*it)->font_name.c_str());
	    ::SendMessage (hListView, LVM_SETSUBITEM, item, (LPARAM) &subdata);
	}
*/
	return 0;
}

BOOL FontMngDlg::onInitDialog(HWND hDlg, LPARAM lParam)
{
	unsigned int i;
	HWND hListView;

	LVCOLUMN lvcol[] = {
		{0, 460, (char *)"Device Fonts", 50, 0L, NULL, 0},
	};

	// init list view header ...
	hListView = GetChild(IDC_FMD_LISTVIEW);
	for (i = 0; i < TABLESIZE(lvcol); i++){
		::SendMessage (hListView, LVM_ADDCOLUMN, 0, (LPARAM) &lvcol[i]);
	}

	RefreshList (hListView);

	::SetWindowText(GetChild(IDC_FMD_CAP_FEDIT), cap_font.c_str());
	::SetWindowText(GetChild(IDC_FMD_CLT_FEDIT), clt_font.c_str());

	return TRUE;
}

void FontMngDlg::onOK()
{
	list<ExFont *>::iterator it;

	mngData->cap_font->clear();
	mngData->cap_font->append(cap_font);
	mngData->clt_font->clear();
	mngData->clt_font->append(clt_font);

	mngData->pExFontList->clear();
	for (it = exFontList.begin(); it != exFontList.end(); it++)
	{
		mngData->pExFontList->push_back(*it);
	}

	EndDialog(IDOK);
}

void FontMngDlg::onCancel()
{
	EndDialog(IDCANCEL);
}

static char *getFontName(const char *file)
{
	const char* ext = strrchr(file, '.');
	if(ext == NULL)
		return NULL;

	if (strcasecmp(ext, ".ttf") == 0){
		char tmp [512];
		const char* fn = strrchr(file, '/') + 1;
		strcpy(tmp, "ttf-");
		strncat(tmp, fn, ext - fn);
		strcat(tmp, "-rrncnn-0-0-UTF-8");
		return strdup(tmp);
	} else if (strcasecmp(ext, ".upf") == 0){
		char * font_name;
#ifdef WIN32
		void *head =  win_mmap(file);
		font_name = strdup(((UPF_HEADER *)head)->font_name);
		win_munmap (head);
#else
		UPF_HEADER * head;
		FILE* fp = NULL;
		fp = fopen (file, "rb");
		head = (UPF_HEADER *) mmap( 0, sizeof(UPF_HEADER), PROT_READ, MAP_SHARED, fileno(fp), 0 );
		font_name = strdup(head->font_name);
		munmap (head, sizeof(UPF_HEADER));
		fclose(fp);
#endif
		return font_name;
	}

	return NULL;
}

void FontMngDlg::onImport()
{
	static const char *font_fliter =
#if (defined (_MGFONT_TTF) || defined (_MGFONT_FT2))
	#ifdef _MGFONT_UPF
		"TrueType(*.ttf)|UPF(*.upf)";
	#else
		"TrueType(*.ttf)";
	#endif
#else
	#ifdef _MGFONT_UPF
		"UPF(*.upf)";
	#else
		"AllFile(*.*)";
	#endif
#endif
	FILEDLGDATA pfdd = {0};
	strcpy (pfdd.filepath, "./");
	strcpy(pfdd.filter, font_fliter);
	pfdd.is_save = FALSE;

	if(FileOpenSaveDialog(GetDlgTemplate(ID_FONTSELECT), GetHandle(), NULL, &pfdd))
	{
		list<ExFont*>::iterator it;
		char *font_name = getFontName(pfdd.filefullname);

		for (it = fontList.begin(); it != fontList.end(); ++it){
			if ((*it)->font_name.compare(font_name) == 0){
				MessageBox(_("This font has been load !"), _("Font existed"), MB_OK);
			}
		}

		ExFont *exfont = new ExFont;
		exfont->file_name = pfdd.filefullname;
		exfont->font_name = font_name;
		exfont->ref = 0;
		exFontList.push_back(exfont);
		//RefreshList (hListView);
		LVITEM lvItem = {0};
		LVSUBITEM subdata;
		lvItem.nItem = ::SendMessage(hListView, LVM_GETITEMCOUNT, 0, 0);
		lvItem.nItemHeight = 25;
	    GHANDLE item = ::SendMessage(hListView, LVM_ADDITEM, 0, (LPARAM)&lvItem);

	    subdata.flags = 0;
	    subdata.image = 0;
	    subdata.nItem = lvItem.nItem;
	    subdata.subItem = 0;
	    subdata.pszText = font_name;
	    ::SendMessage (hListView, LVM_SETSUBITEM, item, (LPARAM) &subdata);

		free(font_name);
	}
}

void FontMngDlg::onCapFont()
{
	FONTDATA pfdd = {0};
	pfdd.min_size = 0;
	pfdd.max_size = 72;

	if (!cap_font.empty())
		strcpy(pfdd.font_name, cap_font.c_str());

	if (TRUE == FontSelDlg (NULL, GetHandle(), NULL, &pfdd))
	{
		cap_font = pfdd.font_name;
		::SetWindowText(GetChild(IDC_FMD_CAP_FEDIT), cap_font.c_str());
	}
	return;
}

void FontMngDlg::onCltFont()
{
	FONTDATA pfdd = {0};
	pfdd.min_size = 0;
	pfdd.max_size = 72;

	if (!clt_font.empty())
		strcpy(pfdd.font_name, clt_font.c_str());

	if (TRUE == FontSelDlg (NULL, GetHandle(), NULL, &pfdd))
	{
		clt_font = pfdd.font_name;
		::SetWindowText(GetChild(IDC_FMD_CLT_FEDIT), clt_font.c_str());
	}
	return;
}

BEGIN_MSG_MAP(FontMngDlg)
	MAP_INITDIALOG(onInitDialog)
	BEGIN_COMMAND_MAP
		MAP_COMMAND(IDOK, onOK)
		MAP_COMMAND(IDCANCEL, onCancel)
		MAP_COMMAND(IDC_FMD_IMPORT, onImport)
		MAP_COMMAND(IDC_FMD_CAP_FBTN, onCapFont)
		MAP_COMMAND(IDC_FMD_CLT_FBTN, onCltFont)
	END_COMMAND_MAP
	MAP_CLOSE(onCancel)
END_MSG_MAP

