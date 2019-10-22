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

#ifndef RDRPREVIEWPANEL_H_
#define RDRPREVIEWPANEL_H_

class RendererPreviewPanel: public Panel, public TMGStaticSubclass<MGStatic> {

protected:
	DECLARE_MSG_MAP;
	RendererInstance *inst;

	void onPaint(HDC hdc);

public:
	RendererPreviewPanel(PanelEventHandler* handler);
	virtual ~RendererPreviewPanel();

	HWND createPanel(HWND hParent);

	HWND getHandler(){
		return GetHandle();
	}

	HWND createPreviewWindow(RendererInstance *instance);
	void destroyPreviewWindow();
	RendererInstance* getPreviewInstance() {return inst; }
	void updateInstanceField(RendererInstance *instance, int* ids);
};

#endif /* RDRPREVIEWPANEL_H_ */
