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

#ifndef WIN32
#include <unistd.h>
#endif
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <list>
#include <map>
#include "mapex.h"
#include <vector>

#include "mgheads.h"
#include "mgfcheads.h"
#include "xmlheads.h"
#include "xmlhelp.h"

using namespace std;

#include "undo-redo.h"
#include "log.h"
#include "stream.h"
#include "resenv.h"

#include "valuetype.h"
#include "class-instance.h"

#include "panel.h"
#include "panellayout.h"

#include "reseditor.h"
#include "editable-listview.h"
#include "texteditor.h"
#include "textlistpanel.h"
#include "text-dlg.h"

#include "translater.h"
#include "GTranslater.h"


BOOL TextEditor::readXmlConfig(const char *xmlFile)
{
	xmlDocPtr doc;
	xmlNodePtr node;
	TeNode* langNode;

	xmlChar* xcharSet = NULL;
	xmlChar* xlangType = NULL;
	xmlChar* xstatus = NULL;
	xmlChar* xcountry = NULL;

	if(xmlFile == NULL)
		return FALSE;

	doc = xmlParseFile(xmlFile);
	if(doc == NULL)
	{
		//file is not exist, create it
		FileStreamStorage fss(xmlFile);
		TextStream stream(&fss);
		stream.println("<text-configuration>");

		stream.indent();
		//ADD english (en_US)
		stream.println("<text status=\"default\">");
		stream.indent();
		stream.println("<lang>en</lang>");
		stream.println("<country>US</country>");
		stream.println("<charset>utf-8</charset>");
		stream.unindent();
		stream.println("</text>");
		//TODO add local info
		stream.unindent();

		stream.println("</text-configuration>");
		fss.close();
		//reopen it
		doc = xmlParseFile(xmlFile);
		if(!doc)
			return FALSE;
	}

	node = xmlDocGetRootElement(doc);
	if(node == NULL || !xhIsNode(node,"text-configuration"))
	{
		xmlFreeDoc(doc);
		return FALSE;
	}

	for(node = node->xmlChildrenNode; node; node = node->next)
	{
		if(!xhIsNode(node, "text"))
			continue;
		if(xmlStrcmp(node->name, (const xmlChar*)"text") == 0)
		{
			xstatus = xmlGetProp(node, (const xmlChar*)"status");

			for(xmlNodePtr child = node->xmlChildrenNode; child; child=child->next)
			{
				if(child->type != XML_ELEMENT_NODE)
					continue;

				if(xmlStrcmp(child->name, (const xmlChar*)"lang") == 0){
					xlangType = xhGetNodeText(child);
					if(xlangType == NULL)
						break;

				}
				else if(xmlStrcmp(child->name, (const xmlChar*)"country") == 0){
					xcountry = xhGetNodeText(child);
				}
				else if(xmlStrcmp(child->name, (const xmlChar*)"charset") == 0){
					xcharSet = xhGetNodeText(child);
				}

			}
		}
		if(xlangType != NULL && xcharSet != NULL)
		{
			langNode = new TeNode;
			langNode->status = 0;
			if(strstr((const char*)xstatus, "default") != 0)
			{
				langNode->status |= TeNode::DEF_LANG;
				pDefTeNode = langNode;
			} else if(strcmp((char *)xstatus, "current") == 0)
			{
				langNode->status |= TeNode::CUR_LANG;
				pCurTeNode = langNode;
			}
			strcpy(langNode->langType, (char *)xlangType);
			strcpy(langNode->country, (char *)xcountry);
			strcpy(langNode->charSet, (char *)xcharSet);

			configList.push_back(langNode);
		}

        if (xstatus)
            xmlFree(xstatus);
		if(xlangType)
			xmlFree(xlangType);
		if(xcountry)
			xmlFree(xcountry);
		if(xcharSet)
			xmlFree(xcharSet);
	}

	xmlFreeDoc(doc);
	return TRUE;
}

void TextEditor::saveXmlConfig(const char *xmlFile)
{
	TeNode *node;
	list<TeNode*>::iterator iter;

	if (xmlFile == NULL || 0 >= configList.size()){
		return;
	}

	FileStreamStorage fss(xmlFile);
	TextStream stream(&fss);
	stream.println("<text-configuration>");

	stream.indent();

	for(iter = configList.begin(); iter != configList.end(); ++iter)
	{
		node = *iter;

		if (node->status & TeNode::DEF_LANG) {
			stream.println("<text status=\"default\">");
		} else if (node->status & TeNode::CUR_LANG) {
			stream.println("<text status=\"current\">");
		} else {
			stream.println("<text status=\"0\">");
		}

		stream.indent();
		stream.println("<lang>%s</lang>", _ERT(node->langType));
		stream.println("<country>%s</country>", _ERT(node->country));
		stream.println("<charset>utf-8</charset>");
		stream.unindent();
		stream.println("</text>");
	}

	stream.unindent();

	stream.println("</text-configuration>");
	fss.close();
}

BOOL TextEditor::open(const char* xmlFile)
{
	map<int, string>::iterator mapIter;
	char szFileName[1024];
	loadXMLIDs(xmlFile);
	list<TeNode*>::iterator listIter;

	for(listIter = configList.begin(); listIter != configList.end(); ++listIter)
	{
		TeNode & txtNode = **listIter;
		sprintf(szFileName,"%s/text/%s_%s.txt",g_env->getResourcePath(), txtNode.langType, txtNode.country);
		txtNode.readText(szFileName);
	}

	listPanel->setContents(reses, configList);
	listPanel->updateHead (pDefTeNode, pCurTeNode);

	return TRUE;
}

string TextEditor::save(BinStream* bin)
{
	char fileTemp[1024];
	list<TeNode*>::iterator iter;

	if (!configList.size()) {
		LOG_WARNING("no list\n");
		return "";
	}

	sprintf(fileTemp,"%s/text/text-config.xml", g_env->getResourcePath());
	saveXmlConfig(fileTemp);

	sprintf(fileTemp,"%s/text/id.xml", g_env->getResourcePath());
	saveXMLIDs(fileTemp);

	int begin = bin->tell();

	//save sect-header
	bin->save32(0);
	bin->save32(configList.size());

 	//save NCSRM_DEFLOCALE_INFO sect
	if (pCurTeNode){
		bin->save8Arr((uint8_t*)pCurTeNode->langType,4);
		bin->save8Arr((uint8_t*)pCurTeNode->country,4);
	} else {
		bin->save8Arr((uint8_t*)pDefTeNode->langType,4);
		bin->save8Arr((uint8_t*)pDefTeNode->country,4);
	}

	int pos, offset = 0;
	unsigned int i = 0;
    NCSRM_LOCALEITEM item;
    memset (&item, 0, sizeof(NCSRM_LOCALEITEM));
	for (i = 0; i < configList.size(); i++){
        bin->save8Arr((const uint8_t*)&item, sizeof(NCSRM_LOCALEITEM));
	}

	//save NCSRM_LOCALEITEM
	i = 0;
	for(iter = configList.begin(); iter != configList.end(); ++iter)
	{
		TeNode *ptxtNode = *iter;
		ptxtNode->saveText();

		//save local item information
		offset = bin->tell() - begin;
		pos = begin + sizeof(NCSRM_SECTHEADER) + sizeof(NCSRM_DEFLOCALE_INFO)
            + i*sizeof(NCSRM_LOCALEITEM);

		bin->seek(pos, StreamStorage::seek_begin);
		bin->save8Arr((uint8_t*)ptxtNode->langType, 4); // language
		bin->save8Arr((uint8_t*)ptxtNode->country, 4);  // country
		bin->save32(0); //filename_id
		bin->save32(offset);	// offset ---incore used
		bin->seek(0, StreamStorage::seek_end);

		ptxtNode->saveTextBin(bin);

		i++;
	}

	//change section size
	int sect_size = bin->tell() - begin;
	bin->seek(begin, StreamStorage::seek_begin);
	bin->save32(sect_size);
	bin->seek(0, StreamStorage::seek_end);

	return string(LOCALE_SAVEFILE);
}

void TextEditor::updateRes()
{
	//FIXME
	char xmlName[1024];

	enableMenuItem(TXT_MENUCMD_DEL, FALSE);

	sprintf(xmlName, "%s/text/id.xml", g_env->getResourcePath());
	open(xmlName);
}

void TextEditor::executeCommand(int cmd_id, int status, DWORD param)
{
	list<TeNode*>::iterator listIter;
	int curExist = 0;
	switch(cmd_id)
	{
		case TXT_MENUCMD_PROFILE:
		{
			AddTextDialog textDialog (GetHandle(), configList);

			if (IDOK == textDialog.DoMode())
			{
				for (listIter = configList.begin(); listIter != configList.end(); ++listIter) {
					TeNode *insideNode = *listIter;
					if (insideNode->status == TeNode::CUR_LANG)
					{

					//	listPanel->setSubText(pCurTeNode);
						curExist = 1;
						if (insideNode != pCurTeNode)
							pCurTeNode = insideNode;
					}
					if (insideNode->status == TeNode::DEF_LANG
							&& insideNode != pDefTeNode)
					{
						pDefTeNode = insideNode;
				//		listPanel->setSubText(pDefTeNode);
					}
				}
				if(curExist == 0)
				{
					pCurTeNode = NULL;
				}
				listPanel->updateHead(pDefTeNode, pCurTeNode);
				listPanel->updateList(configList, reses);
				listPanel->updateEditors();
				updateCombobox();
				g_env->updateAllRes(NCSRT_TEXT);
				enableMenuItem(GBC_SAVE);
			}
			break;
		}
		case TXT_MENUCMD_NEW:
		{
			onNewText();
			break;
		}
		case TXT_MENUCMD_DEL:
		{
			onDeleteText();
			break;
		}
		case TXT_MENUCMD_TRANS:
		{
			onTrans();
			break;
		}
		case TXT_MENUCMD_TRANSALL:
		{
			onTransAll();
			break;
		}
		default:
			break;
	}
}

void TextEditor::onNewText(void)
{
	char szTextName[100];

	newTextName(szTextName);
	int txt_id = createRes(NCSRT_TEXT, szTextName, -1, NULL, 0);
    if (txt_id != -1)
	{
        listPanel->appendText(reses.at(txt_id));
		enableMenuItem(GBC_SAVE);
	}
}

void TextEditor::onDeleteText()
{
	//listPanel->deleteText();
	int id = listPanel->getSelection() | (NCSRT_TEXT<<16);
	//printf("delete :%d\n", id);
	if(removeRes(id))
		enableMenuItem(GBC_SAVE);
}

void TextEditor::onTrans(void)
{
	if (!pDefTeNode || !pCurTeNode)
		return;
	ts = GTranslater::getInstance();
	progDlg = new ProgressDialog (GetHandle(), "", ts);

	pthread_create(&pt_trans, NULL, _trans, this);

	if (IDCANCEL == progDlg->DoMode())
	{
		pthread_cancel(pt_trans);
	}

	delete progDlg;
	g_env->updateAllRes(NCSRT_TEXT);
}

void *TextEditor::_trans(void *arg)
{
	int id = 0;
	char buff [8];
	const char *src;

	TextEditor *_ths = (TextEditor *)arg;
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	HLVITEM sel =_ths->listPanel->GetSelectedItem();
	_ths->listPanel->GetSubitemText(sel, 1, buff, 7);
	buff[7] = '\0';
	id = strtol(buff, NULL, 0) | (NCSRT_TEXT << 16);
	src = _ths->pDefTeNode->getText(id);

	_ths->progDlg->setTitle(src);
	_ths->progDlg->setTotal(100);
	_ths->progDlg->setProg(10);
	_ths->ts->setSourceLangue(_ths->pDefTeNode->langType, _ths->pDefTeNode->country);
	_ths->ts->setTargetLangue(_ths->pCurTeNode->langType, _ths->pCurTeNode->country);
	_ths->ts->setSourceString(src);
	_ths->progDlg->setProg(40);
	_ths->ts->translate();
	_ths->progDlg->setProg(80);
	_ths->pCurTeNode->setText(id, _ths->ts->getResult());
	_ths->listPanel->setSubText(_ths->pCurTeNode);
	_ths->listPanel->updateEditors();
	_ths->progDlg->setProg(100);

	return NULL;
}

void TextEditor::onTransAll(void)
{
	void *tred;
	if (!pDefTeNode || !pCurTeNode)
		return;
	ts = GTranslater::getInstance();
	progDlg = new ProgressDialog (GetHandle(), "", ts);

	pthread_create(&pt_trans, NULL, _transAll, this);
	
	if (IDCANCEL == progDlg->DoMode())
	{
		//pthread_detach(pt_trans);
		pthread_cancel(pt_trans);
		
		//pthread_join(pt_trans, &tred);
	}
	delete progDlg;
	g_env->updateAllRes(NCSRT_TEXT);
}

void *TextEditor::_transAll(void *arg)
{
	int id, idx;
	const char *src;
	map<int, ResEditor::Resource*>::iterator it;

	TextEditor *_ths = (TextEditor *)arg;
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	_ths->progDlg->setTotal(100 * _ths->reses.size());

	for(it = _ths->reses.begin(), idx = 1; it != _ths->reses.end(); ++it, idx++)
	{
		_ths->ts->reset();
		id = it->first;
		src = _ths->pDefTeNode->getText(id);
		_ths->progDlg->setTitle(src);
		_ths->ts->setSourceLangue(_ths->pDefTeNode->langType, _ths->pDefTeNode->country);
		_ths->ts->setTargetLangue(_ths->pCurTeNode->langType, _ths->pCurTeNode->country);
		_ths->ts->setSourceString(src);
		_ths->ts->translate();
		_ths->pCurTeNode->setText(id, _ths->ts->getResult());
		_ths->progDlg->setProg(99 * idx);
	}
	_ths->listPanel->setSubText(_ths->pCurTeNode);
	_ths->listPanel->updateEditors();
	_ths->progDlg->setProg(100 * _ths->reses.size());
	return NULL;
}

const char* TextEditor::newTextName(char* strTextName)
{
	static unsigned int _id_name_count = 1;
	if(strTextName == NULL)
		return NULL;

	do{
		sprintf(strTextName, "ID_TEXT%u", _id_name_count ++);
		if(namedRes.at(strTextName) == NULL)
			return strTextName;
	}while(_id_name_count < 0xFFFFFFFF);

	return NULL;
}

void TextEditor::onResUseChanged(int id, BOOL bAdd/*=TRUE*/)
{
	if(!listPanel)
		return;

	if(bAdd)
		listPanel->onNewTextInserted(id);
	else
		listPanel->deleteItemById(id);
}

void TextEditor::createTextFile(const char *szFileName)
{
	map<int, ResEditor::Resource*>::iterator it;
	char sId[20] = "";

	FILE *fp = fopen(szFileName, "w");
	if(fp == NULL)
		return;

	for(it = reses.begin(); it != reses.end(); it++)
	{
		memset(sId, 0, 20);
		sprintf(sId, "0x%08x\r\n", it->first);
		fwrite(sId, strlen(sId), 1, fp);
	}

	fclose(fp);
}

void TextEditor::updateCombobox()
{
	list<TeNode*>::iterator listIter;
	int cur_sel = -1;
	char sItemName[TEXT_LEN];
	char szFileName[1024];

	::SendMessage(hCombobox, CB_RESETCONTENT, 0, 0);

	for(listIter = configList.begin(); listIter != configList.end(); ++listIter)
	{
		TeNode * txtNode = *listIter;

		if (txtNode->status == TeNode::CUR_LANG) {
			pCurTeNode = txtNode;
		}
		if (txtNode->status == TeNode::DEF_LANG) {
			pDefTeNode = txtNode;
			continue;
		}

		sprintf(sItemName, "%s_%s", txtNode->langType, txtNode->country);
		LRESULT idx = ::SendMessage(hCombobox, CB_ADDSTRING, 0, (LPARAM)sItemName);

		sprintf(szFileName,"%s/text/%s_%s.txt",g_env->getResourcePath(), txtNode->langType, txtNode->country);
		FILE *fp = fopen(szFileName, "rt");
		if(fp == NULL)
			createTextFile(szFileName);
		else
			fclose(fp);

		txtNode->idx = idx;
		if(txtNode->status & TeNode::CUR_LANG)
			cur_sel = idx;
		::SendMessage(hCombobox, CB_SETITEMADDDATA, (WPARAM)idx, (LPARAM)(*listIter));
	}

	if(cur_sel >= 0)
	{
		::SendMessage(hCombobox, CB_SETCURSEL, cur_sel, 0);
	}
}

TextEditor::TextEditor()
:idrmUser(this, NCSRT_TEXT, "user", 0, 10000),
 idrmAnonymous(this, NCSRT_TEXT, "anonymous",10000)
{
	pDefTeNode = NULL;
	pCurTeNode = NULL;
	srand(time(NULL));
}

TextEditor::~TextEditor()
{
	list<TeNode*>::iterator listIter;
	for(listIter = configList.begin(); listIter != configList.end(); ++listIter)
	{
		TeNode * txtNode = *listIter;
		delete txtNode;
	}
    delete listPanel;
}

DWORD TextEditor::processEvent(Panel* sender, int event_id, DWORD param1, DWORD param2)
{
	//TODO
	switch(event_id)
	{
	case EVENT_UPDATE_VALUE:
		enableMenuItem(GBC_SAVE);
		break;
	case EVENT_SEL_CHANGED:
		enableMenuItem(TXT_MENUCMD_DEL, param1);
		break;
	}
	return 0;
}

int TextEditor::setResName(int id, const char* name, DWORD res/*=0*/)
{
	int newid = ResEditor::setResName(id, name);
	if(newid != -1){
		if(newid != id){
			for(list<TeNode*>::iterator it = configList.begin();
					it != configList.end(); ++it)
			{
				(*it)->changeId(id, newid);
			}
			listPanel->updateId(id, newid);
		}
		listPanel->updateIdName(id);
		return newid;
	}
	return -1;
}

BOOL TextEditor::setResId(int oldId, int newId, DWORD res/*=0*/)
{
	if (ResEditor::setResId(oldId, newId)) {
		for(list<TeNode*>::iterator it = configList.begin();
				it != configList.end(); ++it)
		{
			(*it)->changeId(oldId, newId);
		}
        listPanel->updateId(oldId, newId);
		return TRUE;
	}
	return FALSE;
}

BOOL TextEditor::setRes(int id, DWORD dwres){
	if(setRes(id, (const char*)dwres, TRUE))
	{
		if(listPanel)
					listPanel->onTextUpdated(id);
		return TRUE;
	}
	return FALSE;
}

Panel* TextEditor::createPanel(const char* name, const char* caption, const mapex<string,string>* params)
{
	if(strcmp(name, "TextListPanel") == 0)
	{
		listPanel = new TextListPanel(this);
		return listPanel;
	}

	return NULL;
}

void TextEditor::_textCombobox_notifi(HWND hwnd, LINT id, int nc, DWORD add_data)
{
	TextEditor* _this = (TextEditor*)GetWindowAdditionalData(hwnd);

	if(_this)
		_this->onTextComboboxNotification(hwnd, id, nc, add_data);
}

void TextEditor::onTextComboboxNotification(HWND hwnd, LINT id, int nc, DWORD add_data)
{
    TeNode *pNode = NULL;
    list<TeNode*>::iterator listIter;

    if(nc == CBN_SELCHANGE){
        int cur_sel;
        if (0 <= (cur_sel = ::SendMessage (hwnd, CB_GETCURSEL, 0, 0))) {
            pNode = (TeNode *)(::SendMessage(hwnd, CB_GETITEMADDDATA, cur_sel, 0));
            if (pNode != pCurTeNode) {
                listPanel->setSubText(pNode);
                if (pCurTeNode)
                    pCurTeNode->status = 0;
                pNode->status |= TeNode::CUR_LANG;
                pCurTeNode = pNode;
                listPanel->updateHead(pDefTeNode, pCurTeNode);
                listPanel->updateEditors();
                g_env->updateAllRes(NCSRT_TEXT);
            }
        }
    }
}

BOOL TextEditor::initEditor()
{
	char szFileName[1024];

	sprintf(szFileName,"%s/text/text-config.xml",g_env->getResourcePath());
	readXmlConfig(szFileName);

	hCombobox = GetChild(102);
	if(hCombobox != 0 && ::IsWindow(hCombobox))
	{
		::SetWindowAdditionalData(hCombobox, (DWORD)this);
		::SetNotificationCallback(hCombobox, _textCombobox_notifi);

		updateCombobox();
		listPanel->updateHead(pDefTeNode, pCurTeNode);
	}
	return TRUE;
}

//get res
DWORD TextEditor::getRes(int id)
{
	if(id <= 0 || ID2TYPE(id) != NCSRT_TEXT)
		return 0;

	if(pCurTeNode){
		const char* str = pCurTeNode->getText(id);
		if(str)
			return (DWORD)str;
	}

	if(pDefTeNode){
		const char* str = pDefTeNode->getText(id);
		if(str)
			return (DWORD)str;
	}

	return 0;
}

//set res
BOOL TextEditor::setRes(int id, const char* str, BOOL check_id_exist)
{
	if(id <= 0 || ID2TYPE(id) != NCSRT_TEXT)
			return FALSE;

	if(str == NULL)
		return FALSE;

	//create firstly
	if(check_id_exist && reses.at(id) == NULL)
		return FALSE;

	if(pCurTeNode && pCurTeNode->setText(id,str))
		return TRUE;

	if(pDefTeNode && pDefTeNode->setText(id,str))
		return TRUE;

	return FALSE;
}

int TextEditor::createRes(int type, const char* name, int id, const char* source,DWORD init_res)
{
	list<TeNode*>::iterator it;
	id = ResEditor::createRes(type, name, id, NULL, init_res);
	if (id == -1)
		return -1;

	for(it = configList.begin();it != configList.end(); ++it)
    {
		(*it)->setText(id, "");
    }

	/*if(*/setRes(id,source, FALSE);/*)*/
	//{
	//	if(listPanel )
	//		listPanel->onNewTextInserted(id);
	//}

	return id;
}

BOOL TextEditor::removeRes(int id, DWORD res/*=0*/)
{
	if(id < 0 || ID2TYPE(id) != NCSRT_TEXT)
		return FALSE;

	if(!ResEditor::removeRes(id))
		return FALSE;

	//remove from listview
	if(listPanel)
		listPanel->deleteItemById(id);

	for(list<TeNode*>::iterator it = configList.begin();
		it != configList.end(); ++it)
	{
		TeNode * tn = *it;
		if(!tn)
			continue;
		tn->langMap.erase(id);
	}

	return TRUE;
}

//////////////////////////////
static inline bool isNumber(int ch)
{
	return ((ch>='0' && ch<='9')
				|| ch=='X'
				|| ch=='x'
				|| (ch>='A' && ch<='F')
				|| (ch>='a' && ch<='f'));
}

static int getId(FILE* fp)
{
	int i = 0;
	int ch;
	char szNumber[100];

	while(!feof(fp) && isNumber((ch=fgetc(fp))))
	{
		szNumber[i++] = ch;
	}
	szNumber[i] = 0;

	return strtol(szNumber,NULL, 0);
}

static inline bool isSpace(int ch)
{
	return (ch==' ' || ch=='\r' || ch=='\n' || ch=='\0');
}

static inline int getOct(FILE *fp)
{
	char szNumber[100];
	int i = 0;
	int ch;
	while(!feof(fp))
	{
		ch = fgetc(fp);
		if(!(ch>='0' && ch<='7'))
			break;
		szNumber[i++] = ch;
	}
	szNumber[i] = 0;
	ungetc(ch,fp);
	return strtol(szNumber,NULL, 8);
}

static inline int getHex(FILE* fp)
{
	char szNumber[100];
	int i = 0;
	int ch;
	while(!feof(fp) && isNumber((ch=fgetc(fp))))
	{
		szNumber[i++] = ch;
	}
	szNumber[i] = 0;
	ungetc(ch,fp);
	return strtol(szNumber,NULL, 16);
}

static string getString(FILE* fp)
{
	//skip space
	char szBuff[1024];
	int idx = 0;
	int ch = 0;
	int chend = 0;
	string str;
	while(!feof(fp) && isSpace((ch=fgetc(fp))));

	if(ch == '\"') //end by "
	{
		chend = ch;
		ch = fgetc(fp);
	}

	do{
		if(ch == '\\') //
		{
			ch = fgetc(fp);
			if(ch == '\r' || ch == '\n') //Mutli line changed
			{
				ch = fgetc(fp);
				if(chend && ch == chend)
					break;
			}
			else if(ch == 'n')
				ch = 10;
			else if(ch == 'r')
				ch = 13;
			else if(ch == 't')
				ch = 9;//tab
			else if(ch == 'b') //back
			{
				idx --;
				ch = fgetc(fp);
				continue;
			}
			else if(ch=='0') //translate number
			{
				ch = fgetc(fp);
				if(ch == 'X' || ch == 'x') //hex
				{
					ch = getHex(fp);
				}
				else
				{
					ungetc(ch,fp);
					ch = getOct(fp);
				}
			}
		}
		else
		{
			if((chend && ch == chend) || ( !chend && (ch == '\r' || ch == '\n'))) //end
				break;
		}

		szBuff[idx ++] = ch;

		if(idx >= (int)sizeof(szBuff)-32)
		{
			szBuff[idx] = '\0';
			str += szBuff;
			idx = 0;
		}

		if(feof(fp))
			break;

		ch = fgetc(fp);

	}while(1);

	if(chend)
	{
		//To Line end
		while(!feof(fp))
		{
			ch = fgetc(fp);
			if(ch == '\r' || ch == '\n')
				break;
		}
	}

	szBuff[idx] = '\0';
	str += szBuff;
	return str;
}

static void nextLine(FILE *fp)
{
	int ch;
	if(fp == NULL)
		return;

	do{
		ch = fgetc(fp);
	}while(!feof(fp) && ch!='\r' && ch!='\n');
}

BOOL TeNode::readText(const char* strFile)
{
	int id = 0;

	if(strFile == NULL)
		return FALSE;

	FILE *fp = fopen(strFile, "rt");
	if(fp == NULL)
		return FALSE;

	while(!feof(fp))
	{
		int ch = fgetc(fp);
		if(ch == '#') //comment
		{
			nextLine(fp);
			continue;
		}
		ungetc(ch,fp);

		id = getId(fp);
		if(id <= 0 || ID2TYPE(id) != NCSRT_TEXT)
		{
			nextLine(fp);
			continue;
		}

		langMap[id] = getString(fp);
	}

	return TRUE;
}
/*
string& replace_all(string& str,const string& old_value,const string& new_value) 
{ 
	while(true) { 
		string::size_type pos(0); 
		if( (pos=str.find(old_value))!=string::npos ) 
			str.replace(pos,old_value.length(),new_value); 
		else break; 
	} 
	return str; 
}

string& special_character(string& str)
{
	const string sep_windows = "\\";
	const string star_char = "\*";
}
*/
static inline void save_fromated_text(TextStream& stream, string &str)
{
	char szBuff[100];
	int idx=0;
	const char* strText = str.c_str();

	if(!strText){
		stream.println("");
		return ;
	}

	szBuff[idx++] = '\"';

	while(*strText)
	{
		switch(*strText)
		{
		case '\"':
			szBuff[idx++] = '\\';
			szBuff[idx++] = '\"';
			break;
		case 9:
			szBuff[idx++] = '\\';
			szBuff[idx++] = 't';
			break;
		case 10:
			szBuff[idx++] = '\\';
			szBuff[idx++] = 'n';
			break;
#ifdef WIN32
		case '\\':
			szBuff[idx++] = '\\';
			szBuff[idx++] = '\\';
			 break;
#endif
		default:
			szBuff[idx++] = *strText;
			break;
		}

		if(idx >= 80) //new line
		{
			szBuff[idx++] = '\\';
			szBuff[idx++] = '\0';
			stream.println("%s", szBuff);
			idx = 0;
		}

		strText ++;

	}

	if(idx > 0){
		szBuff[idx] = '\0';
		stream.println("%s\"",szBuff);
	}
}

BOOL TeNode::saveText()
{
	char szPath[1024];
	sprintf(szPath,"%s/text/%s_%s.txt",g_env->getResourcePath(),langType, country);

	FileStreamStorage fss(szPath);
	TextStream stream(&fss);

	stream.println("#The Text of %s_%s",langType, country);

	for(map<int,string>::iterator it = langMap.begin();
		it != langMap.end(); ++it)
	{
		string temp = it->second;
		LOG_WARNING("*****int 0x%08X, string =%s\n", it->first,temp.c_str() );
		stream.printf("0x%08X\t",it->first);
		save_fromated_text(stream, it->second);
	}

	return TRUE;
}

int TeNode::saveTextBin(BinStream* bin)
{
	string strPool;
	map<int, string>::iterator mapIter;
	int sect_size, size_maps, pos;
	const char *end_chars = "\0\0\0\0\0\0\0\0";

	size_maps = langMap.size();
	sect_size = sizeof(NCSRM_SECTHEADER) + size_maps * sizeof(NCSRM_IDITEM);
	
	bin->save32(sect_size);
	bin->save32(size_maps);
	
	pos = sect_size;

	//save iditem
	for(mapIter = langMap.begin(); mapIter != langMap.end(); ++mapIter)
	{
		int cnt = 0;
		int id = mapIter->first;
		string cont = mapIter->second;
		int _len = cont.length();
		
		cnt = _len + 3;

	    //4 bytes-align
		if (cnt % 4)
			cnt = ((cnt>>2) + 1)<<2;

		bin->save32(id);	//----- NCSRM_IDITEM.idItem
		bin->save32(0);		//----- NCSRM_IDITEM.filename_id
		bin->save32(pos);	//----- NCSRM_IDITEM.offset

		strPool.append(cont.c_str());
		strPool.append(end_chars, cnt - _len);
		pos += cnt;
	}

	bin->save8Arr((uint8_t*)strPool.c_str(), pos - sect_size);

    bin->seek(0, StreamStorage::seek_end);

	return size_maps;
}

///////////////////////
DECLARE_RESEDITOR(TextEditor)



