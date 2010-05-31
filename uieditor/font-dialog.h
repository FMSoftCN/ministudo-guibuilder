#ifndef _NCS_FONT_H
#define _NCS_FONT_H

#ifdef  __cplusplus
extern "C" {
#endif

typedef struct _FONTDATA {
	/* The font minimize size. */
	int     min_size;
	/* The font maximize size. */
	int     max_size;
	char 	font_name[256];
} FONTDATA, *PFONTDATA;

static const char* style_str [] =
{
    "Regular",
    "Black",
    "Bold",
    "Book",
    "Demibold",
    "Light",
    "Subpixel",
};

static const char style_arg [] =
{
    FONT_WEIGHT_REGULAR,
    FONT_WEIGHT_BLACK,
    FONT_WEIGHT_BOLD,
    FONT_WEIGHT_BOOK,
    FONT_WEIGHT_DEMIBOLD,
    FONT_WEIGHT_LIGHT,
    FONT_WEIGHT_SUBPIXEL
};

static const DWORD style_value[] =
{
	FS_WEIGHT_REGULAR,
	FS_WEIGHT_BLACK,
	FS_WEIGHT_BOLD,
	FS_WEIGHT_BOOK,
	FS_WEIGHT_DEMIBOLD,
	FS_WEIGHT_LIGHT,
	FS_WEIGHT_SUBPIXEL
};

static const char* flip_str [] =
{
    "None    ",
    "Horz    ",
    "Vert    ",
    "HorzVert",
};

static const char flip_arg [] =
{
    'n',//FONT_FLIP_NIL,
    FONT_FLIP_HORZ,
    FONT_FLIP_VERT,
    FONT_FLIP_HORZVERT,
};

static const DWORD flip_value[] =
{
	FS_SLANT_MASK,
	FS_FLIP_HORZ,
	FS_FLIP_VERT,
	FS_FLIP_HORZVERT,
};

/* Data Structure*/

typedef struct _NCS_CHARSET
{
    char                name [LEN_FONT_NAME + 1];
    struct _NCS_CHARSET *next;
} NCS_CHARSET, *NCS_PCHARSET;

typedef struct _NCS_FONT
{
    char        name [LEN_FONT_NAME + 1];
    NCS_CHARSET _ncs_chset;
	int   is_ttf;//is truetype font?
	int   sizeCount;
	int * supportSize;
    struct _NCS_FONT *next;
} NCS_FONT, *NCS_PFONT;

typedef struct _NCS_FONTDIA
{
    NCS_PFONT  _ncs_font;
    PFONTDATA  pfdd;
} NCS_FONTDIA, *NCS_PFONTDIA;

BOOL FontSelDlg  (PDLGTEMPLATE dlg_template,HWND hwnd, WNDPROC proc, PFONTDATA pfdd);


///////////////////////////////////////////////////

#define LEN_VERSION_MAX       10
#define LEN_VENDER_NAME_MAX   12
#define LEN_DEVFONT_NAME_MAX  127

typedef struct _UPF_HEADER{
    char     ver_info [LEN_VERSION_MAX];
    char     vender_name [LEN_VENDER_NAME_MAX];
    Uint16   endian;
    char     font_name [LEN_DEVFONT_NAME_MAX + 1];
}UPF_HEADER;

class ExFont
{
public:
    string font_name;
    string file_name;
    int ref;
};

class MngData
{
public:
	string *cap_font;
	string *clt_font;

	list<ExFont*> *pExFontList;
};

class FontMngDlg: public MGMainWnd
{
protected:
	HWND hListView;

	string cap_font;
	string clt_font;

	list<ExFont*> exFontList;
	list<ExFont*> fontList;

	MngData *mngData;

	int RefreshList (HWND hlist);
	void onOK();
	void onCancel();
	void onImport();
	void onCapFont();
	void onCltFont();
	void onCtrlFont();
	BOOL onInitDialog(HWND hDlg, LPARAM lParam);

	DECLARE_MSG_MAP;

public:
	FontMngDlg (HWND hParent, MngData *data);
	~FontMngDlg();
};


#ifdef  __cplusplus
}
#endif

#endif /* _NCS_FONT_H */
