/*
 * imgeditor.h
 *
 *  Created on: 2009-3-21
 *      Author: dongjunjie
 */

#ifndef IMGEDITOR_H_
#define IMGEDITOR_H_


class ImageEditor: public ResEditor
{
	DirImageView 		*dir_img_view;
	DirTreePanel 		*dir_tree_panel;
	ImageResListPanel	*img_res_list;
	ResImageView 		*res_img_view;
	ResDescPanel 		*res_desc_panel;

	IDRangeManager idrm;

protected:
	IDRangeManager* getIDRangeManager(int type, const char* name = NULL){
		if(type == NCSRT_IMAGE)
			return &idrm;
		return NULL;
	}
public:
	ImageEditor();
	virtual ~ImageEditor();

	DWORD processEvent(Panel* sender, int event_id, DWORD param1 = 0, DWORD param2 = 0 );
	Panel* createPanel(const char* name, const char* caption, const mapex<string, string>*params);
	void executeCommand(int cmd_id, int status, DWORD param);

	virtual void updateRes();
	virtual BOOL open(const char* xmlFile);
	virtual string save(BinStream* bin);

	int getAllIDRangeManagers(vector<IDRangeManager*> & mngrlist){
		mngrlist.push_back(&idrm);
		return 1;
	}

	int getTypeMask()
	{
		return NCSRT_IMAGE;
	}

	const char* getTypeName(int type_mask)
	{
		return "Image";
	}
	int getSelectedResID() {
		if(img_res_list){
			return img_res_list->GetSelect();
		}
		return 0;
	};

//added by ly
	BOOL setResId(int oldId, int newId, DWORD res = 0);
	int  setResName(int id, const char* name, DWORD res = 0);

	virtual void onResNameChanged(int id, const char* newName);
	virtual void onResIdChanged(int oldId, int newId);

    BOOL isAvailableIDName(const char* idName);
protected:
	void importAll();
	void import();

	void activeImageView(BOOL bRes=TRUE){
		HWND hProp = ::GetParent(::GetParent(dir_img_view->getHandler()));
		::SendMessage(hProp, PSM_SETACTIVEINDEX, bRes?1:0, 0);
	}

	virtual void active(bool bactive, int reason){
		ResEditor::active(bactive, reason);
		if (bactive && reason == REF_ACTIVE)
		{
			activeImageView(TRUE);
			/*
			if (img_res_list){
				img_res_list->SelectRes(-1);
			}
			need icon view support unselect ....
			*/
		}
	}


	const char* getImageResFile(int id){
		ResEditor::Resource* res = reses.at(id);
		return res?g_env->getString(res->source_id):"";
	}

	int insertImageRes(const char* source_file, const char* file_name=NULL,char* id_name=NULL);

	BOOL loadConfig(xmlNodePtr root_node);

	BOOL saveConfig(TextStream* stream);

protected:
	BOOL WndProc(UINT iMsg,WPARAM wParam,LPARAM lParam,int *pret);
};

#endif /* IMGEDITOR_H_ */
