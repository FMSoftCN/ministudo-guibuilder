/*
 * window-instance.h
 *
 *  Created on: 2009-3-27
 *      Author: dongjunjie
 */

#ifndef WINDOWINSTANCE_H_
#define WINDOWINSTANCE_H_

class WindowInstance: public ComponentInstance {
public:
	WindowInstance(Class *cls);
	WindowInstance(const WindowInstance &winst);

	virtual ~WindowInstance();

	BOOL loadFromXML(xmlNodePtr node);

	virtual void saveXMLToStream(TextStream *stream){
		saveXMLToStream(stream, WI_SAVE_NORMAL);
	}

	BOOL saveIndependXML(TextStream *textStream, BOOL saveTmplXML = FALSE){
		if (saveTmplXML)
			saveXMLToStream(textStream, WI_SAVE_INDEPEND | WI_SAVE_TMPL);
		else
			saveXMLToStream(textStream, WI_SAVE_INDEPEND);
		return TRUE;
	}

	enum wiSaveFlag{
		WI_SAVE_NORMAL=0x00,
		WI_SAVE_INDEPEND=0x01,
		WI_SAVE_TMPL=0x10
	};

	void saveXMLToStream(TextStream *stream, int flag = WI_SAVE_NORMAL);

	//return size of total window
	virtual int saveBinToStream(BinStream *stream);

	HWND createPreviewWindow(HWND hParent = HWND_INVALID);
	void destroyPreviewWindow();

	virtual void setPreviewWindowHandler(HWND hwnd);
	virtual void resetNCSProps();

	virtual void setCaption(const char* strCaption);

	virtual Instance* getRendererInstance();

	virtual const char* getClassType() { return "window"; }

	Instance * clone();

	HWND recreatePreviewWindow(HWND hParent = HWND_INVALID);

	int syncPreviewWindow(int id);

	BOOL updateSpecialField(int field_id, DWORD param);

	BOOL getDlgTemplate(DLGTEMPLATE* pteml);

	BOOL getCtrlData(CTRLDATA *pctrl);

	// rdr to avoid draw unactive main window
	WINDOW_ELEMENT_RENDERER *old_win_rdr;

	void toLuaTable(void *luaState, int table_idx);
	BOOL setDefRenderer(const char* defRdrName);

	const char* getCaption();
	const char *getFont();
protected:

	HWND createNCSWindow(HWND hParent);

	HWND createOldWindow(HWND hParent);

	//get window info
	DWORD getCommStyle(int id_begin, int id_end);
	DWORD getStyle(){ return getCommStyle(ComponentInstance::PropStyleBegin,ComponentInstance::PropStyleEnd); }
	DWORD getExStyle(){ return getCommStyle(ComponentInstance::PropExStyleBegin,ComponentInstance::PropExStyleEnd); }

	const char* getControlClass();
	NCS_RDR_INFO* getRendererInfo(NCS_RDR_INFO *rdr_info);
	DWORD getBkColor(){ return getField(PropBkColor); }
	inline gal_pixel color2Pixel(DWORD clr, HDC hdc=HDC_SCREEN){
		return RGBA2Pixel(hdc, GetRValue(clr), GetGValue(clr), GetBValue(clr), GetAValue(clr));
	}

	void resetRenderer();

	static LRESULT _main_window_proc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	WNDPROC old_proc;

	BOOL useNewControlSet(){
		const char* controlset = g_env->getSysConfig("control-set","new-control-set");
		return (controlset == NULL || strcmp(controlset,"new-control-set") == 0);
	}

	void previewWindow(BOOL bPreview = TRUE);

};

#endif /* WINDOWINSTANCE_H_ */
