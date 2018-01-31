
#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include "msd_intl.h"

#undef _
#define _(x) x

static CTRLDATA _FontManager_controls[] = {
		{
			"button", /* class_name */
			0x08010000, /* style */
			304, /* x */
			369, /* y */
			80, /* w */
			30, /* h */
			1, /* id */
			_("OK"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"button", /* class_name */
			0x08010000, /* style */
			403, /* x */
			369, /* y */
			80, /* w */
			30, /* h */
			2, /* id */
			_("Cancel"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"button", /* class_name */
			0x08010000, /* style */
			206, /* x */
			369, /* y */
			80, /* w */
			30, /* h */
			163, /* id */
			_("Import"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"listview", /* class_name */
			0x084D0001, /* style */
			20, /* x */
			32, /* y */
			464, /* w */
			195, /* h */
			161, /* id */
			_(""), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"static", /* class_name */
			0x08010007, /* style */
			20, /* x */
			237, /* y */
			464, /* w */
			114, /* h */
			11, /* id */
			_("Default font for project"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"static", /* class_name */
			0x08010010, /* style */
			21, /* x */
			8, /* y */
			381, /* w */
			23, /* h */
			12, /* id */
			_("System device font name:"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"static", /* class_name */
			0x08010030, /* style */
			28, /* x */
			260, /* y */
			106, /* w */
			30, /* h */
			13, /* id */
			_("Caption font:"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"static", /* class_name */
			0x08010030, /* style */
			26, /* x */
			302, /* y */
			107, /* w */
			30, /* h */
			14, /* id */
			_("Client font:"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"sledit", /* class_name */
			0x08410000, /* style */
			138, /* x */
			261, /* y */
			264, /* w */
			30, /* h */
			166, /* id */
			_(""), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"sledit", /* class_name */
			0x08410000, /* style */
			138, /* x */
			303, /* y */
			264, /* w */
			30, /* h */
			168, /* id */
			_(""), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"button", /* class_name */
			0x08010000, /* style */
			415, /* x */
			261, /* y */
			56, /* w */
			30, /* h */
			167, /* id */
			_("..."), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"button", /* class_name */
			0x08010000, /* style */
			415, /* x */
			303, /* y */
			56, /* w */
			30, /* h */
			169, /* id */
			_("..."), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
};
static DLGTEMPLATE _FontManager_templ = {
	0x30C00000,/* style */
	0, /* exstyle */
	0, /* x */
	0, /* y */
	506, /* w */
	440, /* h */
	_("Font Management"), /* caption */
	0, /* hIcon */
	0, /* hMenu */
	sizeof(_FontManager_controls)/sizeof(CTRLDATA), /* controlnr */
	_FontManager_controls, /* controls */
	0 /* dwAddData */
};

static CTRLDATA _ImportImage_controls[] = {
		{
			"button", /* class_name */
			0x08010000, /* style */
			44, /* x */
			128, /* y */
			100, /* w */
			32, /* h */
			202, /* id */
			_("Import"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"button", /* class_name */
			0x08010000, /* style */
			176, /* x */
			128, /* y */
			100, /* w */
			32, /* h */
			2, /* id */
			_("Cancel"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"sledit", /* class_name */
			0x08410000, /* style */
			34, /* x */
			62, /* y */
			252, /* w */
			30, /* h */
			201, /* id */
			_("IDB_IMG_FILENAME"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"static", /* class_name */
			0x08010010, /* style */
			36, /* x */
			28, /* y */
			203, /* w */
			30, /* h */
			12, /* id */
			_("Input image ID name:"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
};
static DLGTEMPLATE _ImportImage_templ = {
	0x38C00000,/* style */
	0, /* exstyle */
	0, /* x */
	0, /* y */
	325, /* w */
	214, /* h */
	_("Import Image Resource"), /* caption */
	0, /* hIcon */
	0, /* hMenu */
	sizeof(_ImportImage_controls)/sizeof(CTRLDATA), /* controlnr */
	_ImportImage_controls, /* controls */
	0 /* dwAddData */
};

static CTRLDATA _NewFile_controls[] = {
		{
			"propsheet", /* class_name */
			0x08010000, /* style */
			24, /* x */
			40, /* y */
			476, /* w */
			220, /* h */
			100, /* id */
			_(""), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"sledit", /* class_name */
			0x08410000, /* style */
			173, /* x */
			280, /* y */
			327, /* w */
			29, /* h */
			101, /* id */
			_(""), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"static", /* class_name */
			0x08010010, /* style */
			24, /* x */
			287, /* y */
			144, /* w */
			14, /* h */
			11, /* id */
			_("Input file name:"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"button", /* class_name */
			0x08010000, /* style */
			264, /* x */
			320, /* y */
			100, /* w */
			34, /* h */
			1, /* id */
			_("OK"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"button", /* class_name */
			0x08010000, /* style */
			400, /* x */
			320, /* y */
			100, /* w */
			34, /* h */
			2, /* id */
			_("Cancel"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"button", /* class_name */
			0x08010003, /* style */
			20, /* x */
			320, /* y */
			200, /* w */
			28, /* h */
			102, /* id */
			_("Overwrite exist file"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"static", /* class_name */
			0x08010010, /* style */
			24, /* x */
			16, /* y */
			344, /* w */
			21, /* h */
			15, /* id */
			_("Please select a window template:"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
};
static DLGTEMPLATE _NewFile_templ = {
	0x30C00000,/* style */
	0, /* exstyle */
	0, /* x */
	0, /* y */
	526, /* w */
	404, /* h */
	_("New file ..."), /* caption */
	0, /* hIcon */
	0, /* hMenu */
	sizeof(_NewFile_controls)/sizeof(CTRLDATA), /* controlnr */
	_NewFile_controls, /* controls */
	0 /* dwAddData */
};

static CTRLDATA _NewRenderer_controls[] = {
		{
			"button", /* class_name */
			0x08010000, /* style */
			84, /* x */
			228, /* y */
			100, /* w */
			32, /* h */
			1, /* id */
			_("OK"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"button", /* class_name */
			0x08010000, /* style */
			228, /* x */
			228, /* y */
			100, /* w */
			32, /* h */
			2, /* id */
			_("Cancel"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"static", /* class_name */
			0x08010007, /* style */
			28, /* x */
			20, /* y */
			344, /* w */
			184, /* h */
			15, /* id */
			_("New LF renderer"), /* caption */
			0, /* dwAddData */
			0x00000000, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"static", /* class_name */
			0x08010030, /* style */
			37, /* x */
			65, /* y */
			124, /* w */
			32, /* h */
			16, /* id */
			_("Renderer type:"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"static", /* class_name */
			0x08010030, /* style */
			37, /* x */
			113, /* y */
			124, /* w */
			32, /* h */
			17, /* id */
			_("Control type:"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"static", /* class_name */
			0x08010030, /* style */
			37, /* x */
			157, /* y */
			124, /* w */
			32, /* h */
			18, /* id */
			_("ID name:"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"sledit", /* class_name */
			0x08410000, /* style */
			168, /* x */
			152, /* y */
			176, /* w */
			28, /* h */
			120, /* id */
			_("IDR_"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"combobox", /* class_name */
			0x0801000B, /* style */
			168, /* x */
			60, /* y */
			180, /* w */
			28, /* h */
			100, /* id */
			_(""), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"combobox", /* class_name */
			0x0801000B, /* style */
			168, /* x */
			108, /* y */
			180, /* w */
			28, /* h */
			110, /* id */
			_(""), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
};
static DLGTEMPLATE _NewRenderer_templ = {
	0x30C00000,/* style */
	0, /* exstyle */
	0, /* x */
	0, /* y */
	401, /* w */
	310, /* h */
	_("Create New Renderer"), /* caption */
	0, /* hIcon */
	0, /* hMenu */
	sizeof(_NewRenderer_controls)/sizeof(CTRLDATA), /* controlnr */
	_NewRenderer_controls, /* controls */
	0 /* dwAddData */
};

static CTRLDATA _NewRendererSet_controls[] = {
		{
			"static", /* class_name */
			0x08010007, /* style */
			28, /* x */
			20, /* y */
			344, /* w */
			169, /* h */
			9, /* id */
			_("New renderer set"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"button", /* class_name */
			0x08010000, /* style */
			84, /* x */
			224, /* y */
			100, /* w */
			32, /* h */
			1, /* id */
			_("OK"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"button", /* class_name */
			0x08010000, /* style */
			224, /* x */
			224, /* y */
			100, /* w */
			32, /* h */
			2, /* id */
			_("Cancel"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"static", /* class_name */
			0x08010030, /* style */
			34, /* x */
			73, /* y */
			125, /* w */
			30, /* h */
			12, /* id */
			_("Renderer type:"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"static", /* class_name */
			0x08010030, /* style */
			34, /* x */
			122, /* y */
			125, /* w */
			30, /* h */
			13, /* id */
			_("Set ID name:"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"sledit", /* class_name */
			0x08410000, /* style */
			168, /* x */
			117, /* y */
			176, /* w */
			28, /* h */
			120, /* id */
			_("IDRS_"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"combobox", /* class_name */
			0x0801000B, /* style */
			168, /* x */
			68, /* y */
			176, /* w */
			28, /* h */
			100, /* id */
			_(""), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
};
static DLGTEMPLATE _NewRendererSet_templ = {
	0x30C00000,/* style */
	0, /* exstyle */
	0, /* x */
	0, /* y */
	401, /* w */
	310, /* h */
	_("Create New Renderer Set"), /* caption */
	0, /* hIcon */
	0, /* hMenu */
	sizeof(_NewRendererSet_controls)/sizeof(CTRLDATA), /* controlnr */
	_NewRendererSet_controls, /* controls */
	0 /* dwAddData */
};

static CTRLDATA _AddRenderer_controls[] = {
		{
			"button", /* class_name */
			0x08010000, /* style */
			50, /* x */
			230, /* y */
			80, /* w */
			30, /* h */
			1, /* id */
			_("OK"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"button", /* class_name */
			0x08010000, /* style */
			270, /* x */
			230, /* y */
			80, /* w */
			30, /* h */
			2, /* id */
			_("Cancel"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"listbox", /* class_name */
			0x084D0009, /* style */
			17, /* x */
			19, /* y */
			355, /* w */
			190, /* h */
			100, /* id */
			_(""), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
};
static DLGTEMPLATE _AddRenderer_templ = {
	0x30C00000,/* style */
	0, /* exstyle */
	0, /* x */
	0, /* y */
	400, /* w */
	310, /* h */
	_("Add Renderer to RdrSet"), /* caption */
	0, /* hIcon */
	0, /* hMenu */
	sizeof(_AddRenderer_controls)/sizeof(CTRLDATA), /* controlnr */
	_AddRenderer_controls, /* controls */
	0 /* dwAddData */
};

static CTRLDATA _SetDefaultRenderer_controls[] = {
		{
			"listbox", /* class_name */
			0x08410001, /* style */
			24, /* x */
			44, /* y */
			172, /* w */
			152, /* h */
			100, /* id */
			_(""), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"static", /* class_name */
			0x08010010, /* style */
			24, /* x */
			16, /* y */
			172, /* w */
			24, /* h */
			10, /* id */
			_("Select renderer:"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"button", /* class_name */
			0x08010000, /* style */
			224, /* x */
			46, /* y */
			100, /* w */
			32, /* h */
			1, /* id */
			_("OK"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"button", /* class_name */
			0x08010000, /* style */
			224, /* x */
			94, /* y */
			100, /* w */
			32, /* h */
			2, /* id */
			_("Cancel"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
};
static DLGTEMPLATE _SetDefaultRenderer_templ = {
	0x38C00000,/* style */
	0, /* exstyle */
	0, /* x */
	0, /* y */
	348, /* w */
	244, /* h */
	_("Set Default Renderer"), /* caption */
	0, /* hIcon */
	0, /* hMenu */
	sizeof(_SetDefaultRenderer_controls)/sizeof(CTRLDATA), /* controlnr */
	_SetDefaultRenderer_controls, /* controls */
	0 /* dwAddData */
};

static CTRLDATA _SetScreenSize_controls[] = {
		{
			"button", /* class_name */
			0x08010000, /* style */
			44, /* x */
			174, /* y */
			90, /* w */
			32, /* h */
			1, /* id */
			_("OK"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"button", /* class_name */
			0x08010000, /* style */
			164, /* x */
			174, /* y */
			90, /* w */
			32, /* h */
			2, /* id */
			_("Cancel"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"sledit", /* class_name */
			0x08410000, /* style */
			132, /* x */
			37, /* y */
			122, /* w */
			30, /* h */
			100, /* id */
			_(""), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"sledit", /* class_name */
			0x08410000, /* style */
			132, /* x */
			82, /* y */
			122, /* w */
			30, /* h */
			101, /* id */
			_(""), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"static", /* class_name */
			0x08010030, /* style */
			27, /* x */
			33, /* y */
			80, /* w */
			34, /* h */
			12, /* id */
			_("Width:"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"static", /* class_name */
			0x08010030, /* style */
			27, /* x */
			122, /* y */
			80, /* w */
			34, /* h */
			11, /* id */
			_("Depth:"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"static", /* class_name */
			0x08010030, /* style */
			27, /* x */
			80, /* y */
			80, /* w */
			34, /* h */
			13, /* id */
			_("Height:"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"combobox", /* class_name */
			0x0801100B, /* style */
			132, /* x */
			127, /* y */
			122, /* w */
			25, /* h */
			15, /* id */
			_(""), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
};
static DLGTEMPLATE _SetScreenSize_templ = {
	0x38C00000,/* style */
	0, /* exstyle */
	0, /* x */
	0, /* y */
	300, /* w */
	250, /* h */
	_("Set Screen Size"), /* caption */
	0, /* hIcon */
	0, /* hMenu */
	sizeof(_SetScreenSize_controls)/sizeof(CTRLDATA), /* controlnr */
	_SetScreenSize_controls, /* controls */
	0 /* dwAddData */
};

static CTRLDATA _AddLang_controls[] = {
		{
			"combobox", /* class_name */
			0x0801000B, /* style */
			29, /* x */
			46, /* y */
			215, /* w */
			28, /* h */
			2100, /* id */
			_(""), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"button", /* class_name */
			0x08010000, /* style */
			36, /* x */
			96, /* y */
			90, /* w */
			32, /* h */
			1, /* id */
			_("OK"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"button", /* class_name */
			0x08010000, /* style */
			143, /* x */
			96, /* y */
			90, /* w */
			32, /* h */
			2, /* id */
			_("Cancel"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"static", /* class_name */
			0x08010000, /* style */
			25, /* x */
			16, /* y */
			146, /* w */
			28, /* h */
			17, /* id */
			_("Select language:"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
};
static DLGTEMPLATE _AddLang_templ = {
	0x30C00000,/* style */
	0, /* exstyle */
	0, /* x */
	0, /* y */
	276, /* w */
	168, /* h */
	_("Add Language"), /* caption */
	0, /* hIcon */
	0, /* hMenu */
	sizeof(_AddLang_controls)/sizeof(CTRLDATA), /* controlnr */
	_AddLang_controls, /* controls */
	0 /* dwAddData */
};
static CTRLDATA _TextProfile_controls[] = {
		{
			"button", /* class_name */
			0x08010000, /* style */
			316, /* x */
			29, /* y */
			98, /* w */
			30, /* h */
			300, /* id */
			_("Add"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"button", /* class_name */
			0x08010000, /* style */
			317, /* x */
			71, /* y */
			98, /* w */
			30, /* h */
			400, /* id */
			_("Delete"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"button", /* class_name */
			0x08010000, /* style */
			317, /* x */
			111, /* y */
			98, /* w */
			30, /* h */
			500, /* id */
			_("Set Default"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"button", /* class_name */
			0x08010000, /* style */
			317, /* x */
			155, /* y */
			98, /* w */
			30, /* h */
			600, /* id */
			_("Set Current"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"button", /* class_name */
			0x08010000, /* style */
			192, /* x */
			232, /* y */
			100, /* w */
			32, /* h */
			1, /* id */
			_("OK"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"button", /* class_name */
			0x08010000, /* style */
			315, /* x */
			232, /* y */
			100, /* w */
			32, /* h */
			2, /* id */
			_("Cancel"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"listview", /* class_name */
			0x084D0001, /* style */
			24, /* x */
			28, /* y */
			269, /* w */
			168, /* h */
			100, /* id */
			_(""), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
};
static DLGTEMPLATE _TextProfile_templ = {
	0x30C00000,/* style */
	0, /* exstyle */
	0, /* x */
	0, /* y */
	443, /* w */
	303, /* h */
	_("Profile ..."), /* caption */
	0, /* hIcon */
	0, /* hMenu */
	sizeof(_TextProfile_controls)/sizeof(CTRLDATA), /* controlnr */
	_TextProfile_controls, /* controls */
	0 /* dwAddData */
};

static CTRLDATA _ConnectEvent_controls[] = {
		{
			"button", /* class_name */
			0x08010000, /* style */
			303, /* x */
			231, /* y */
			80, /* w */
			30, /* h */
			2, /* id */
			_("Cancel"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"button", /* class_name */
			0x08010000, /* style */
			303, /* x */
			33, /* y */
			80, /* w */
			30, /* h */
			107, /* id */
			_("Add"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"button", /* class_name */
			0x08010000, /* style */
			303, /* x */
			72, /* y */
			80, /* w */
			30, /* h */
			106, /* id */
			_("Delete"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"button", /* class_name */
			0x08010000, /* style */
			303, /* x */
			111, /* y */
			80, /* w */
			30, /* h */
			116, /* id */
			_("Modify"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"button", /* class_name */
			0x08010000, /* style */
			303, /* x */
			150, /* y */
			80, /* w */
			30, /* h */
			117, /* id */
			_("GO Source"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"static", /* class_name */
			0x08010010, /* style */
			15, /* x */
			5, /* y */
			250, /* w */
			23, /* h */
			14, /* id */
			_("Event Listener:"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"listview", /* class_name */
			0x084D0001, /* style */
			15, /* x */
			63, /* y */
			273, /* w */
			198, /* h */
			115, /* id */
			_(""), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"button", /* class_name */
			0x08010000, /* style */
			303, /* x */
			189, /* y */
			80, /* w */
			30, /* h */
			1, /* id */
			_("OK"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"sledit", /* class_name */
			0x08410800, /* style */
			15, /* x */
			28, /* y */
			230, /* w */
			30, /* h */
			103, /* id */
			_(""), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"button", /* class_name */
			0x08010000, /* style */
			248, /* x */
			28, /* y */
			40, /* w */
			30, /* h */
			110, /* id */
			_("..."), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
};
static DLGTEMPLATE _ConnectEvent_templ = {
	0x30C00000,/* style */
	0, /* exstyle */
	0, /* x */
	0, /* y */
	407, /* w */
	320, /* h */
	_("Connect Events"), /* caption */
	0, /* hIcon */
	0, /* hMenu */
	sizeof(_ConnectEvent_controls)/sizeof(CTRLDATA), /* controlnr */
	_ConnectEvent_controls, /* controls */
	0 /* dwAddData */
};
static CTRLDATA _SelectEvent_controls[] = {
		{
			"button", /* class_name */
			0x08010000, /* style */
			137, /* x */
			323, /* y */
			80, /* w */
			30, /* h */
			1, /* id */
			_("OK"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"button", /* class_name */
			0x08010000, /* style */
			248, /* x */
			324, /* y */
			80, /* w */
			30, /* h */
			2, /* id */
			_("Cancel"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"static", /* class_name */
			0x08010010, /* style */
			22, /* x */
			13, /* y */
			323, /* w */
			22, /* h */
			17, /* id */
			_("Step 1: Select an event:"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"listbox", /* class_name */
			0x084D0001, /* style */
			22, /* x */
			70, /* y */
			323, /* w */
			127, /* h */
			105, /* id */
			_(""), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"static", /* class_name */
			0x08010010, /* style */
			22, /* x */
			200, /* y */
			323, /* w */
			22, /* h */
			20, /* id */
			_("Step 2: Input event name:"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"sledit", /* class_name */
			0x08410000, /* style */
			22, /* x */
			225, /* y */
			323, /* w */
			30, /* h */
			108, /* id */
			_(""), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"static", /* class_name */
			0x08010010, /* style */
			22, /* x */
			262, /* y */
			323, /* w */
			60, /* h */
			22, /* id */
			_("Note: The event name must be a validate C language function name."), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"sledit", /* class_name */
			0x08410800, /* style */
			22, /* x */
			35, /* y */
			280, /* w */
			30, /* h */
			102, /* id */
			_(""), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"button", /* class_name */
			0x08010000, /* style */
			305, /* x */
			35, /* y */
			40, /* w */
			30, /* h */
			109, /* id */
			_("..."), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
};
static DLGTEMPLATE _SelectEvent_templ = {
	0x30C30000,/* style */
	0, /* exstyle */
	0, /* x */
	0, /* y */
	396, /* w */
	403, /* h */
	_("Select Event"), /* caption */
	0, /* hIcon */
	0, /* hMenu */
	sizeof(_SelectEvent_controls)/sizeof(CTRLDATA), /* controlnr */
	_SelectEvent_controls, /* controls */
	0 /* dwAddData */
};
static CTRLDATA _InputEventName_controls[] = {
		{
			"button", /* class_name */
			0x08010000, /* style */
			105, /* x */
			163, /* y */
			80, /* w */
			30, /* h */
			1, /* id */
			_("OK"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"button", /* class_name */
			0x08010000, /* style */
			210, /* x */
			163, /* y */
			80, /* w */
			30, /* h */
			2, /* id */
			_("Cancel"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"static", /* class_name */
			0x08010010, /* style */
			13, /* x */
			6, /* y */
			271, /* w */
			22, /* h */
			23, /* id */
			_("Input event name:"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"sledit", /* class_name */
			0x08410000, /* style */
			13, /* x */
			37, /* y */
			271, /* w */
			30, /* h */
			120, /* id */
			_("SlEdit2"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"static", /* class_name */
			0x08010010, /* style */
			13, /* x */
			82, /* y */
			271, /* w */
			58, /* h */
			25, /* id */
			_("Note: The event name must be a validate C language function name."), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
};
static DLGTEMPLATE _InputEventName_templ = {
	0x30C00000,/* style */
	0, /* exstyle */
	0, /* x */
	0, /* y */
	333, /* w */
	245, /* h */
	_("Input event name"), /* caption */
	0, /* hIcon */
	0, /* hMenu */
	sizeof(_InputEventName_controls)/sizeof(CTRLDATA), /* controlnr */
	_InputEventName_controls, /* controls */
	0 /* dwAddData */
};

static CTRLDATA _FontSelect_controls[] = {
		{
			"button", /* class_name */
			0x08010000, /* style */
			242, /* x */
			344, /* y */
			80, /* w */
			30, /* h */
			530, /* id */
			_("OK"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"button", /* class_name */
			0x08010000, /* style */
			344, /* x */
			344, /* y */
			80, /* w */
			30, /* h */
			531, /* id */
			_("Cancel"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"static", /* class_name */
			0x08010010, /* style */
			12, /* x */
			8, /* y */
			100, /* w */
			24, /* h */
			521, /* id */
			_("File path:"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"combobox", /* class_name */
			0x0801000B, /* style */
			107, /* x */
			8, /* y */
			269, /* w */
			25, /* h */
			522, /* id */
			_(""), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"button", /* class_name */
			0x08010000, /* style */
			383, /* x */
			8, /* y */
			46, /* w */
			25, /* h */
			523, /* id */
			_("UP"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"listview", /* class_name */
			0x084D0001, /* style */
			12, /* x */
			41, /* y */
			417, /* w */
			216, /* h */
			524, /* id */
			_(""), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"static", /* class_name */
			0x08010010, /* style */
			12, /* x */
			274, /* y */
			100, /* w */
			24, /* h */
			525, /* id */
			_("File name:"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"static", /* class_name */
			0x08010010, /* style */
			12, /* x */
			304, /* y */
			100, /* w */
			24, /* h */
			527, /* id */
			_("File type:"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"sledit", /* class_name */
			0x08410000, /* style */
			112, /* x */
			269, /* y */
			317, /* w */
			25, /* h */
			526, /* id */
			_(""), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"combobox", /* class_name */
			0x0801000B, /* style */
			112, /* x */
			305, /* y */
			317, /* w */
			25, /* h */
			528, /* id */
			_(""), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"button", /* class_name */
			0x08010003, /* style */
			8, /* x */
			344, /* y */
			120, /* w */
			30, /* h */
			529, /* id */
			_("Hide file"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
};
static DLGTEMPLATE _FontSelect_templ = {
	0x38C00000,/* style */
	0, /* exstyle */
	0, /* x */
	0, /* y */
	458, /* w */
	417, /* h */
	_("Open Or Save File"), /* caption */
	0, /* hIcon */
	0, /* hMenu */
	sizeof(_FontSelect_controls)/sizeof(CTRLDATA), /* controlnr */
	_FontSelect_controls, /* controls */
	0 /* dwAddData */
};

static CTRLDATA _IDRangeEditor_controls[] = {
		{
			"static", /* class_name */
			0x08010010, /* style */
			6, /* x */
			7, /* y */
			75, /* w */
			23, /* h */
			9, /* id */
			_("Res type:"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"combobox", /* class_name */
			0x0801000B, /* style */
			88, /* x */
			8, /* y */
			310, /* w */
			24, /* h */
			10, /* id */
			_(""), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"listview", /* class_name */
			0x084D0001, /* style */
			8, /* x */
			48, /* y */
			391, /* w */
			108, /* h */
			11, /* id */
			_(""), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"button", /* class_name */
			0x08010000, /* style */
			7, /* x */
			173, /* y */
			123, /* w */
			30, /* h */
			12, /* id */
			_("New range"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"button", /* class_name */
			0x08010000, /* style */
			318, /* x */
			173, /* y */
			80, /* w */
			30, /* h */
			13, /* id */
			_("Exit"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"button", /* class_name */
			0x08010000, /* style */
			140, /* x */
			173, /* y */
			123, /* w */
			30, /* h */
			14, /* id */
			_("Extend range"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
};
static DLGTEMPLATE _IDRangeEditor_templ = {
	0x38C00000,/* style */
	0, /* exstyle */
	0, /* x */
	0, /* y */
	414, /* w */
	252, /* h */
	_("ID range editor"), /* caption */
	0, /* hIcon */
	0, /* hMenu */
	sizeof(_IDRangeEditor_controls)/sizeof(CTRLDATA), /* controlnr */
	_IDRangeEditor_controls, /* controls */
	0 /* dwAddData */
};
static CTRLDATA _NewIDRange_controls[] = {
		{
			"listview", /* class_name */
			0x084D0001, /* style */
			8, /* x */
			67, /* y */
			344, /* w */
			96, /* h */
			24, /* id */
			_(""), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"static", /* class_name */
			0x08010010, /* style */
			8, /* x */
			40, /* y */
			339, /* w */
			25, /* h */
			25, /* id */
			_("Usable range:"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"button", /* class_name */
			0x08010000, /* style */
			274, /* x */
			301, /* y */
			82, /* w */
			31, /* h */
			1, /* id */
			_("OK"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"sledit", /* class_name */
			0x08410000, /* style */
			92, /* x */
			204, /* y */
			115, /* w */
			24, /* h */
			38, /* id */
			_(""), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"sledit", /* class_name */
			0x08410000, /* style */
			236, /* x */
			204, /* y */
			115, /* w */
			24, /* h */
			39, /* id */
			_(""), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"static", /* class_name */
			0x08010000, /* style */
			212, /* x */
			212, /* y */
			23, /* w */
			10, /* h */
			31, /* id */
			_("~"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"combobox", /* class_name */
			0x0801000B, /* style */
			92, /* x */
			267, /* y */
			263, /* w */
			26, /* h */
			33, /* id */
			_(""), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"static", /* class_name */
			0x08010010, /* style */
			8, /* x */
			207, /* y */
			76, /* w */
			25, /* h */
			34, /* id */
			_("Range:"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"static", /* class_name */
			0x08010010, /* style */
			8, /* x */
			267, /* y */
			76, /* w */
			25, /* h */
			35, /* id */
			_("User:"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"button", /* class_name */
			0x08010000, /* style */
			186, /* x */
			301, /* y */
			82, /* w */
			31, /* h */
			2, /* id */
			_("Cancel"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"static", /* class_name */
			0x0C010010, /* style */
			9, /* x */
			6, /* y */
			346, /* w */
			34, /* h */
			26, /* id */
			_("Step 1: Select a usable range as you new range."), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"static", /* class_name */
			0x0C010010, /* style */
			7, /* x */
			169, /* y */
			346, /* w */
			34, /* h */
			29, /* id */
			_("Step 2: Input a new range. You should avoid conflict with other developers"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"static", /* class_name */
			0x0C010010, /* style */
			6, /* x */
			233, /* y */
			346, /* w */
			34, /* h */
			30, /* id */
			_("Step 3: Select or input a developer name Who Would Use The Range"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"button", /* class_name */
			0x08010000, /* style */
			7, /* x */
			301, /* y */
			163, /* w */
			31, /* h */
			36, /* id */
			_("Learn more ..."), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
};
static DLGTEMPLATE _NewIDRange_templ = {
	0x28C00000,/* style */
	0, /* exstyle */
	0, /* x */
	0, /* y */
	376, /* w */
	375, /* h */
	_("New range"), /* caption */
	0, /* hIcon */
	0, /* hMenu */
	sizeof(_NewIDRange_controls)/sizeof(CTRLDATA), /* controlnr */
	_NewIDRange_controls, /* controls */
	0 /* dwAddData */
};
static CTRLDATA _ExtendIDRange_controls[] = {
		{
			"sledit", /* class_name */
			0x08410000, /* style */
			88, /* x */
			9, /* y */
			83, /* w */
			24, /* h */
			38, /* id */
			_(""), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"sledit", /* class_name */
			0x08410000, /* style */
			204, /* x */
			9, /* y */
			81, /* w */
			24, /* h */
			39, /* id */
			_(""), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"static", /* class_name */
			0x08010000, /* style */
			176, /* x */
			16, /* y */
			23, /* w */
			10, /* h */
			40, /* id */
			_("~"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"button", /* class_name */
			0x08010000, /* style */
			86, /* x */
			60, /* y */
			82, /* w */
			31, /* h */
			1, /* id */
			_("OK"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"button", /* class_name */
			0x08010000, /* style */
			206, /* x */
			59, /* y */
			82, /* w */
			31, /* h */
			2, /* id */
			_("Cancel"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"static", /* class_name */
			0x08010030, /* style */
			3, /* x */
			9, /* y */
			80, /* w */
			25, /* h */
			45, /* id */
			_("100<"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"static", /* class_name */
			0x08010010, /* style */
			290, /* x */
			9, /* y */
			89, /* w */
			25, /* h */
			46, /* id */
			_(">=1000"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
};
static DLGTEMPLATE _ExtendIDRange_templ = {
	0x28C00000,/* style */
	0, /* exstyle */
	0, /* x */
	0, /* y */
	394, /* w */
	141, /* h */
	_("Extend Range"), /* caption */
	0, /* hIcon */
	0, /* hMenu */
	sizeof(_ExtendIDRange_controls)/sizeof(CTRLDATA), /* controlnr */
	_ExtendIDRange_controls, /* controls */
	0 /* dwAddData */
};

static CTRLDATA _About_controls[] = {
		{
			"button", /* class_name */
			0x08010000, /* style */
			180, /* x */
			340, /* y */
			92, /* w */
			32, /* h */
			1, /* id */
			_("OK"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"static", /* class_name */
			0x08010009, /* style */
			24, /* x */
			12, /* y */
			416, /* w */
			52, /* h */
			104, /* id */
			_(""), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"static", /* class_name */
			0x08010020, /* style */
			28, /* x */
			280, /* y */
			412, /* w */
			40, /* h */
			10, /* id */
			_("Copyright (c) 2010 Beijing FMSoft Technologies CO., LTD."), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"static", /* class_name */
			0x08010030, /* style */
			24, /* x */
			112, /* y */
			72, /* w */
			40, /* h */
			11, /* id */
			_("Version:"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"static", /* class_name */
			0x08010030, /* style */
			24, /* x */
			168, /* y */
			72, /* w */
			40, /* h */
			12, /* id */
			_("Build:"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"static", /* class_name */
			0x08010010, /* style */
			112, /* x */
			112, /* y */
			344, /* w */
			40, /* h */
			100, /* id */
			_(""), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"static", /* class_name */
			0x08010010, /* style */
			112, /* x */
			168, /* y */
			344, /* w */
			40, /* h */
			101, /* id */
			_(""), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"static", /* class_name */
			0x08010020, /* style */
			20, /* x */
			76, /* y */
			420, /* w */
			24, /* h */
			13, /* id */
			_("A GUI Builder for MiniGUI (with mGNCS)."), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"static", /* class_name */
			0x08010030, /* style */
			24, /* x */
			220, /* y */
			72, /* w */
			40, /* h */
			105, /* id */
			_("SN:"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"static", /* class_name */
			0x08010010, /* style */
			112, /* x */
			220, /* y */
			344, /* w */
			40, /* h */
			106, /* id */
			_(""), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
};
static DLGTEMPLATE _About_templ = {
	0x38C00000,/* style */
	0, /* exstyle */
	0, /* x */
	0, /* y */
	476, /* w */
	424, /* h */
	_("About GUI Builder"), /* caption */
	0, /* hIcon */
	0, /* hMenu */
	sizeof(_About_controls)/sizeof(CTRLDATA), /* controlnr */
	_About_controls, /* controls */
	0 /* dwAddData */
};

static CTRLDATA _SetStartWindow_controls[] = {
		{
			"button", /* class_name */
			0x08010000, /* style */
			223, /* x */
			39, /* y */
			80, /* w */
			30, /* h */
			1, /* id */
			_("OK"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"button", /* class_name */
			0x08010000, /* style */
			223, /* x */
			75, /* y */
			80, /* w */
			30, /* h */
			2, /* id */
			_("Cancel"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"static", /* class_name */
			0x08010010, /* style */
			9, /* x */
			9, /* y */
			294, /* w */
			25, /* h */
			10, /* id */
			_("Select start window"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"listbox", /* class_name */
			0x08490001, /* style */
			10, /* x */
			37, /* y */
			196, /* w */
			162, /* h */
			100, /* id */
			_(""), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
};
static DLGTEMPLATE _SetStartWindow_templ = {
	0x38C00000,/* style */
	0, /* exstyle */
	0, /* x */
	0, /* y */
	327, /* w */
	250, /* h */
	_("Select start window ..."), /* caption */
	0, /* hIcon */
	0, /* hMenu */
	sizeof(_SetStartWindow_controls)/sizeof(CTRLDATA), /* controlnr */
	_SetStartWindow_controls, /* controls */
	0 /* dwAddData */
};

static CTRLDATA _TranslateProgress_controls[] = {
		{
			"button", /* class_name */
			0x08010000, /* style */
			148, /* x */
			102, /* y */
			80, /* w */
			30, /* h */
			2, /* id */
			_("Cancel"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"static", /* class_name */
			0x08010010, /* style */
			7, /* x */
			8, /* y */
			363, /* w */
			46, /* h */
			112, /* id */
			_(""), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"progressbar", /* class_name */
			0x08010001, /* style */
			7, /* x */
			66, /* y */
			363, /* w */
			25, /* h */
			111, /* id */
			_(""), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
};
static DLGTEMPLATE _TranslateProgress_templ = {
	0x38C00000,/* style */
	0, /* exstyle */
	0, /* x */
	0, /* y */
	383, /* w */
	173, /* h */
	_("Translate"), /* caption */
	0, /* hIcon */
	0, /* hMenu */
	sizeof(_TranslateProgress_controls)/sizeof(CTRLDATA), /* controlnr */
	_TranslateProgress_controls, /* controls */
	0 /* dwAddData */
};

static CTRLDATA _SelectProjectType_controls[] = {
		{
			"button", /* class_name */
			0x08010000, /* style */
			342, /* x */
			229, /* y */
			80, /* w */
			30, /* h */
			1, /* id */
			_("OK"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"button", /* class_name */
			0x08010000, /* style */
			237, /* x */
			229, /* y */
			80, /* w */
			30, /* h */
			2, /* id */
			_("Cancel"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"static", /* class_name */
			0x08010010, /* style */
			21, /* x */
			9, /* y */
			248, /* w */
			25, /* h */
			10, /* id */
			_("Select project type"), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
		{
			"listbox", /* class_name */
			0x084D0001, /* style */
			21, /* x */
			36, /* y */
			401, /* w */
			169, /* h */
			100, /* id */
			_(""), /* caption */
			0, /* dwAddData */
			0, /* exstye */
			NULL, /* werdr_name */
			NULL /* we_attrs */
		},
};
static DLGTEMPLATE _SelectProjectType_templ = {
	0x38C00000,/* style */
	0, /* exstyle */
	0, /* x */
	0, /* y */
	455, /* w */
	316, /* h */
	_("Select project type"), /* caption */
	0, /* hIcon */
	0, /* hMenu */
	sizeof(_SelectProjectType_controls)/sizeof(CTRLDATA), /* controlnr */
	_SelectProjectType_controls, /* controls */
	0 /* dwAddData */
};


static DLGTEMPLATE* templs[] = {
	&_FontManager_templ,
	&_ImportImage_templ,
	&_NewFile_templ,
	&_NewRenderer_templ,
	&_NewRendererSet_templ,
	&_AddRenderer_templ,
	&_SetDefaultRenderer_templ,
	&_SetScreenSize_templ,
	&_AddLang_templ,
	&_TextProfile_templ,
	&_ConnectEvent_templ,
	&_SelectEvent_templ,
	&_InputEventName_templ,
	&_FontSelect_templ,
	&_IDRangeEditor_templ,
	&_NewIDRange_templ,
	&_ExtendIDRange_templ,
	&_About_templ,
	&_SetStartWindow_templ,
	&_TranslateProgress_templ,
	&_SelectProjectType_templ,
};

#ifdef _MSTUDIO_LOCALE
static void init_international_text()
{
    static int _inited = 0;
    int i;
    if(_inited)
        return;
    _inited = 1;
    for(i = 0; i < sizeof(templs) / sizeof(DLGTEMPLATE*); i ++) {
        int j;
        PCTRLDATA pctrls;
        if(!templs[i])
            continue;
        templs[i]->caption = msd_gettext(templs[i]->caption);
        pctrls = templs[i]->controls;
        for(j = 0; j < templs[i]->controlnr; j ++)
        {
            pctrls[j].caption = msd_gettext(pctrls[j].caption);
        }
    }
}


#endif


DLGTEMPLATE * GetDlgTemplate(int id)
{
	int count = sizeof(templs) / sizeof(DLGTEMPLATE*);
	id -= 10000;
#ifdef _MSTUDIO_LOCALE
    init_international_text();
#endif
	if(id < 0 || id >= count)
		return NULL;

	if(templs[id])
		templs[id]->dwStyle &= ~WS_VISIBLE;

	return templs[id];
}

LRESULT AutoCenterDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = DefaultDialogProc(hwnd, message, wParam, lParam);

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

