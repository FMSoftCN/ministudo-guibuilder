/*
 * edituipanel.h
 *
 *  Created on: 2009-3-22
 *      Author: dongjunjie
 */

#ifndef EDITUIPANEL_H_
#define EDITUIPANEL_H_

extern const char *getRdrFont(const char* rdr, int idx);

class EditUIPanel: public  Panel,
					public TMGStaticSubclass<MGStatic>,
					public TMGScrollWnd<EditUIPanel>,
					PanelEventHandler,
					UndoRedoObject
{

protected:
	DECLARE_MSG_MAP;

	ComponentInstance * baseInstance;

	BITMAP bmpOutline; //64x64 bmp
	void updateOutline(BOOL bUpdatePrevWindow=FALSE, BOOL bsendEvent = TRUE);

	HDC updatePreviewWindow();

	string strXmlFile;

	void onLButtonDown(int x, int y, DWORD key_flag);
	void onLButtonUp(int x, int y, DWORD key_flag);
	void onMouseMove(int x, int y, DWORD key_flag);
	void onLButtonDblClk(int x, int y, DWORD key_flag);
	void onRButtonUp(int x, int y, DWORD key_flag);
	void onPaint(HDC hdc);
	void onHScroll(int hs_nc,int pos){ onScroll(hs_nc, pos, SB_HORZ); }
	void onVScroll(int hs_nc,int pos){ onScroll(hs_nc, pos, SB_VERT); }
	void onCSizeChanged(int cx, int cy){
		updateScrollbar(FALSE);
	}
	BOOL onKeyDown(int scancode, DWORD key_status);
	BOOL onKeyUp(int scancode, DWORD key_status);

	BOOL onEraseBkgnd(HDC hdc,PRECT clip);

	HWND hwndTopMost;
	HDC hdcPreviewWnd;

	//scroll process
	int xpos, ypos;
	void onScroll(int hs_nc, int pos, int sb);

	//operation
	//copy
	//cut
	//paste
	//delete
	void deleteSelectedInstance();

public:
	EditUIPanel(PanelEventHandler* handler);
	virtual ~EditUIPanel();

	HWND createPanel(HWND hParent);

	HWND getHandler(){
		return GetHandle();
	}

	BOOL open(const char* xmlFile);
	void close(BOOL bSaveInpdentXMLFile=FALSE);

	BOOL saveXML(BOOL bForce);
	string saveBin(BinStream *bin);
	BOOL saveSource();

	PBITMAP getOutlineBmp();

	enum EditUIPanelFlag{
		CreateControl = 0x01,
		CreateControlAlways = 0x02,
		UpdatePreviewWindow = 0x04,
		RequestMouseMsg = 0x08,
		AutoSnapeGrid = 0x10,
		StartWnd = 0x20,
		PreviewMode = 0x40,
		SourceChanged = 0x80,
		ProperyChanged = 0x100,
		Hidden = 0x200
	};

	//////
	HWND setPreview(BOOL bPreview=TRUE);

	BOOL isPreview(){
		return flags&PreviewMode;
	}

	////
	void setStartWnd(BOOL bStartWnd){
		if((bStartWnd && isStartWnd()) || (!bStartWnd && !isStartWnd()))
			return ;
		setSourceChanged();
		if(bStartWnd)
			flags |= StartWnd;
		else
			flags &= ~StartWnd;
	}

	BOOL isStartWnd(){ return flags & StartWnd; }
	////

	///////  hide flags
	void hide(BOOL bhide=TRUE);
	BOOL isHidden(){ return flags & Hidden; }

	void updateScrollbar(BOOL bUpdate = TRUE);
private:
	void hideInstances();
	///////

public:

#ifdef WIN32
	WINDOW_ELEMENT_RENDERER *editor_win_rdr;
#else
	WINDOW_ELEMENT_RENDERER editor_win_rdr;
#endif
	///////////////
	BOOL isSourceChanged(){
		return flags & SourceChanged;
	}
	void setSourceChanged(BOOL bsendEvent=TRUE){
		if(flags & SourceChanged)
			return;

		flags |= SourceChanged;
		if(bsendEvent)
			sendEvent(EDITUIPANEL_MODIFIED, (DWORD)this, (DWORD)TRUE);
	}
	void clearSourceChanged(){
		if(!(flags&SourceChanged))
			return;

		flags &= ~SourceChanged;
		sendEvent(EDITUIPANEL_MODIFIED, (DWORD)this, (DWORD)FALSE);
	}
	///////

	//////
	BOOL isPropertyChanged(){
		return flags & ProperyChanged;
	}
	void setPropertyChanged(BOOL bSendEvent=TRUE){
		if(flags&ProperyChanged)
			return;
		flags |= ProperyChanged;
		if(bSendEvent)
			sendEvent(EDITUIPANEL_MODIFIED, (DWORD)this, (DWORD)TRUE);
	}
	void clearPropertyChanged(){
		if(!(flags&ProperyChanged))
			return;
		flags &= ~ProperyChanged;
		sendEvent(EDITUIPANEL_MODIFIED, (DWORD)this, (DWORD)FALSE);
	}
	/////

	void setCreateInfo(const char* strName, BOOL bAlways){
		if(strName){
			curClassName = strName;
			flags &=~CreateControlAlways;
			if(bAlways)
				flags |= CreateControlAlways;
			flags |= CreateControl;
		}
		else
		{
			curClassName.clear();
			flags &= ~(CreateControlAlways|CreateControl);
		}
	}

	void autoSetSnapeGrid(){
		if(flags & AutoSnapeGrid)
			flags &= ~AutoSnapeGrid;
		else
			flags |= AutoSnapeGrid;

		if(baseInstance)
			baseInstance->setSnapGrid(flags & AutoSnapeGrid);
		//update
		update();
	}

	void setSnapeGrid(BOOL bset = TRUE){
		if( (isSnapeGrid() && bset)
			|| (!isSnapeGrid() && !bset))
			return ;
		if(bset)
			flags |= AutoSnapeGrid;
		else
			flags &= ~AutoSnapeGrid;
		if(baseInstance)
			baseInstance->setSnapGrid(flags & AutoSnapeGrid);
		//update
		update();
	}

	BOOL isSnapeGrid(){
		return flags & AutoSnapeGrid;
	}

	int updateInstanceField(Instance *instance, int id);

	void recreatePreviewWnd(Instance* instance);

	ComponentInstance * getBaseInstance() { return baseInstance; }

	ComponentInstance * getCurrentInstance(){
		if(::IsWindow(curWnd))
			return ComponentInstance::FromHandler(curWnd);
		return baseInstance;
	}

	void selectInstance(ComponentInstance * instance);

	void active();

	const char* getFileName()
	{
		const char* strFile = strXmlFile.c_str();
		const char* strFileName = strrchr(strFile, '/');
		if(strFileName == NULL)
			strFileName = strrchr(strFile, '\\');
		if(strFileName == NULL)
			strFileName = strFile;
		else
			strFileName ++;
		return strFileName;
	}
	const char* getXMLFile(){ return strXmlFile.c_str(); }

	DWORD processEvent(Panel* sender, int event_id, DWORD param1 = 0, DWORD param2 = 0 );

	void setRendererEditor(FieldPanel* panelRdr);

	void updateSpecialField(int field_id, DWORD param){
		if(baseInstance && baseInstance->updateSpecialField(field_id,param))
		{
			flags |= UpdatePreviewWindow;
			InvalidateRect();
		}
	}

	void setDefRenderer(const char* defRdrName);

	void updateRefResValue(int res_id);
	BOOL updateRefResId(int res_old_id, int res_new_id);
	void updateRefResIdName(int res_id);
	void updateTexts();
    void deleteRefRes(int id);

	void gotoCode(Instance* inst, int event_id);

	void saveTemplates(const char* strFile);

private:
	//selection manager

	//state
	enum state{
		Normal = 0,
		PreSizing,
		Selecting,
		Moving,
		Sizing,
		Creating,
		//AutoMouseMsg, //auto process mouse message
	};

	vector<HWND> selectedWnd;
	HWND curWnd;
	string curClassName;

	ComponentInstance * curContainer;

	Uint16 curState;
	Uint16 flags;

	ComponentInstance* InstanceFromHandle(HWND hwnd){
		if(!::IsWindow(hwnd))
			return baseInstance;
		return ComponentInstance::FromHandler(hwnd);
	}

	void drawSelections(HDC hdc);

	void drawSelection(HDC hdc, HWND hctrl);

	void cancelSelectAll(){
		selectedWnd.clear();
		curWnd = baseInstance->getPreviewHandler();
	}
	//true: have insert, false, had inserted, don't insert too
	BOOL insertSelect(HWND hctrl, BOOL bcur=FALSE);

	BOOL isInSelectedList(HWND hctrl){
		for(vector<HWND>::iterator it=  selectedWnd.begin();
		it != selectedWnd.end(); ++it)
		if(*it == hctrl)
			return TRUE;
		return FALSE;
	}

	//true have moved, false not removed
	BOOL removeSelect(HWND hctrl);

	HWND getWndAt(int x, int y, int& hitcode);

	ComponentInstance* testContainer(int x, int y, ComponentInstance** pcinst);

	struct SizeEditInfo {
		HWND hwnd;
		DWORD anchor;
		HCURSOR hcur;
	};

	enum AnchorType{
		AnchorLeft = 0x01,
		AnchorRight = 0x02,
		AnchorTop = 0x04,
		AnchorBottom = 0x08
	};

	static SizeEditInfo _sei;

	void clientToMainWnd(HWND hwnd,int &x, int &y);
	void clientToMainWnd(HWND hwnd,RECT &rt);
	void mainWndToClient(HWND hwnd, int &x, int &y);

	SizeEditInfo * getEditInfo(int x, int y);
	static BOOL inAnchor(int x, int y, int xanchor, int yanchor);
	static HCURSOR getCursorByAnchor(DWORD anchor);

	void onMovingUp(int x, int y);
	void onMovingMove(int x, int y);

	void onSelectingUp(int x, int y);
	void onSelectingMove(int x, int y);

	static BOOL setSizedRect(RECT *prt, int x, int y, DWORD anchor);
	void onSizingUp(int x, int y);
	void onSizingMove(int x, int y);

	void onCreatingUp(int x, int y);
	#define onCreatingMove onSelectingMove
	//void onCreatingMove(int x, int y);

	//void onMutilSelect(int x, int y);

	//avoid main preview window flash

	void dumpWindowRenderer(WINDOW_ELEMENT_RENDERER *win_rdr);
	static void draw_caption (HWND hWnd, HDC hdc, BOOL is_active);
	static void draw_border(HWND hWnd, HDC hdc, BOOL is_active);
	static void draw_caption_button(HWND hWnd, HDC hdc, int ht_code, int status);

	void drawAcceptContainer(HDC hdc, HWND hwnd);

	void notifySelChanged();

	BOOL isInstanceIn(ComponentInstance *inst){
		if(inst == NULL || baseInstance == NULL)
			return FALSE;
		while(inst && inst != baseInstance)
			inst = inst->getParent();
		return inst != NULL;
	}

	void notifyBoundChanged(BOOL bLocation = TRUE, BOOL bSize=TRUE, HWND hwnd = HWND_INVALID);

	//BOOL PreProcMessage(int message, WPARAM wParam, LPARAM lParam, int *prt);
	BOOL instanceProcessMouseMessage(int message, int x, int y, DWORD key_data, int *pret=NULL);

	//process command
public:
	void onPopMenuCmd(int id);
protected:

	void onPopMenuUser(int id);

	void addResource(ComponentInstance *instance, const char* source = NULL);

	void copy(BOOL bremove = FALSE);
	void paste();

	void snapeGrid(int &x, int &y);

	enum {
		ALIGN_LEFT = 0,
		ALIGN_RIGHT,
		ALIGN_CENTER,
		ALIGN_TOP,
		ALIGN_BOTTOM,
		ALIGN_MIDDLE,
		ALIGN_MAX
	};
	void align(int at);
	void spreedOut(BOOL bHorz=TRUE);
	void sameSize(BOOL bWidth=TRUE);

	void syncSelectedWndFromInstance(ComponentInstance* cinst, HWND holdWnd);

	char* getSourceFileName(char* szFileName)
	{
		const char* strFile = strrchr(strXmlFile.c_str(), '/');
		if(strFile == NULL)
			strFile = strrchr(strXmlFile.c_str(), '\\');
		if(strFile == NULL)
			strFile = strXmlFile.c_str();
		else
			strFile ++;
		strcpy(szFileName, strFile);
		strFile = strrchr(szFileName,'.');

		const char* strSrcExt = g_env->getSysConfig("uieditor/trans/extend",".c");

		if(strFile)
			strcpy((char*)strFile, strSrcExt);
		else
			strcat(szFileName,strSrcExt);
		return szFileName;
	}

	void moveSelections(int xoffset, int yoffset);
	void tabToNextSelection();

	static BOOL select_key_state;

protected:
	///undo redo support
	DefaultUndoRedoObserver undoRedoObserver;

	void pushMoveUndoRedoCommand(ComponentInstance *old_container);
	void pushSizeUndoRedoCommand(ComponentInstance * special=NULL);
	void pushCreateDeleteUndoRedoCommand(BOOL bCreate=TRUE);

	void undoRedoUpdate(const UndoRedoCommand *cmd);

public:
	void undo();
	void redo();

	void connectEvents();

	void exportTemplate(void);

private:
	void merge(const char *str_src, const char* str_dest);

public:
	void updateRdrElement(Instance *inst, int ele_id);
private:
	BOOL updateInstanceRdr(ComponentInstance *cinst,int id, DWORD params[2]);

	BOOL deletedFieldFromInst(ComponentInstance *cinst, int id);

	void removeInstanceId(ResManager *resMgr,ComponentInstance *cinst);

	void disableSource();

	void drawInvisibleAndContainer(HDC hdc, ComponentInstance * inst);

public:
	void update(){
		flags |= UpdatePreviewWindow;
		InvalidateRect();
	}
};

#endif /* EDITUIPANEL_H_ */
