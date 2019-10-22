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

#include <stdio.h>
#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include "gridlayout.h"
#include "fixedlayout.h"

inline void _fixed_layout_calca(int begin, int end, LAYOUT_UNIT *puint)
{
	if(puint->type == lutPixel)
	{
		if(puint->logic_size >= 0)
			puint->current_size = begin + puint->logic_size;
		else
			puint->current_size = end + puint->logic_size ;
	}
	else if(puint->type == lutPercent){
		if(puint->logic_size >= 0)
			puint->current_size = (end-begin)*puint->logic_size/100 + begin;
		else
			puint->current_size = (end-begin)*puint->logic_size/100 + end ;
	}
}

static void _fixed_layout_calcsize(const RECT *prt, FIXED_LAYOUT_CELL * pcell)
{
	if(pcell == NULL || prt == NULL)
		return;

	_fixed_layout_calca(prt->left, prt->right, &pcell->rect.x);
	_fixed_layout_calca(0, prt->right-prt->left , &pcell->rect.w);
	_fixed_layout_calca(prt->top, prt->bottom, &pcell->rect.y);
	_fixed_layout_calca(0, prt->bottom - prt->top, &pcell->rect.h);
}

inline static void _fixed_layout_move_cell(FIXED_LAYOUT_CELL *pcell)
{
	switch(pcell->cell_type ){
	case gct_hwnd:
		MoveWindow(pcell->data.hwnd,
				pcell->rect.x.current_size ,
				pcell->rect.y.current_size ,
				pcell->rect.w.current_size,
				pcell->rect.h.current_size,
				TRUE);
		break;
	case gct_grid:
		SetGridLayoutRect(pcell->data.grid,
			pcell->rect.x.current_size ,
			pcell->rect.y.current_size ,
			pcell->rect.w.current_size,
			pcell->rect.h.current_size);
		break;
	case gct_user:
		if(pcell->data.user && pcell->data.user->inf && pcell->data.user->inf->setRect)
			(*pcell->data.user->inf->setRect)(pcell->data.user->user,
					pcell->rect.x.current_size ,
					pcell->rect.y.current_size ,
					pcell->rect.w.current_size,
					pcell->rect.h.current_size);
		break;
	}
}

static void _fixedlayout_setRect(void * user_data, int x, int y, int width, int height){
	FIXED_LAYOUT* pfl = (FIXED_LAYOUT*) user_data;
	FIXED_LAYOUT_CELL * pcell = pfl->head;
	RECT rt = {x, y, x+width, y+height };
	//realloc the size of every

	pfl->x = x;
	pfl->y = y;
	pfl->w = width;
	pfl->h = height;

	while(pcell)
	{
		//realloc size
		_fixed_layout_calcsize(&rt, pcell);
		_fixed_layout_move_cell( pcell);
		pcell = pcell->next;
	}
}

void  _fixedlayout_show(void* user_data, int show)
{
	FIXED_LAYOUT* pfl = (FIXED_LAYOUT*) user_data;
	FIXED_LAYOUT_CELL *pcell;

	if(pfl== NULL)
		return;

	pcell = pfl->head;
	while(pcell)
	{
		switch(pcell->cell_type){
		case gct_hwnd:
			ShowWindow(pcell->data.hwnd, show?SW_SHOW:SW_HIDE);
			break;
		case gct_grid:
			GridLayoutShow(pcell->data.grid, show);
			break;
		case gct_user:
			if(pcell->data.user && pcell->data.user->inf && pcell->data.user->inf->show)
				(*pcell->data.user->inf->show)(pcell->data.user->user, show);
			break;
		default:
			break;
		}
	}


}

static GRID_USER_TYPE_INF _fixed_out_type = {
		/*.setRect = */_fixedlayout_setRect,
		/*.user_to_gridlayout = */NULL,
		/*.show = */_fixedlayout_show,
		/*.free = */NULL
};

BOOL FixedLayoutInit(FIXED_LAYOUT * pfl)
{
	if(pfl == NULL)
		return FALSE;
	pfl->head = NULL;
	pfl->user_data = pfl;
	pfl->inf = &_fixed_out_type;
	pfl->x = pfl->y = pfl->w = pfl->h = 0;
	return TRUE;
}

BOOL FixedLayoutInsert(FIXED_LAYOUT* pfl, int type, FIXED_LAYOUT_CELL_RECT* rect,void *celldata)
{
	FIXED_LAYOUT_CELL * pcell;
	//do not check
	if(pfl == NULL || celldata == NULL || type <= 0 || type >=gct_max || rect == NULL)
		return FALSE;

	 pcell = (FIXED_LAYOUT_CELL*)malloc(sizeof(FIXED_LAYOUT_CELL));
	pcell->rect = *rect;
	pcell->data.data = celldata;
	pcell->cell_type = type;
	pcell->next = pfl->head;
	pfl->head = pcell;

	return TRUE;
}

void FixedLayoutClear(FIXED_LAYOUT* pfl)
{
	FIXED_LAYOUT_CELL * pcell;
	if(pfl == NULL)
		return;

	pcell = pfl->head;
	while(pcell)
	{
		FIXED_LAYOUT_CELL * pt = pcell;
		free(pt);
		pcell = pcell->next;
	}
	pfl->head = NULL;
}
