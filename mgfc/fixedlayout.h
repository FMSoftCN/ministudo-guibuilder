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

#ifndef FIXED_LAYOUT_H
#define FIXED_LAYOUT_H

/*
 *  Fixed Layout: every information in this layout is  in a fixed size
 *
 */

#ifdef __cplusplus
extern "C"{
#endif

typedef struct _fixed_layout_cell_rect{
	LAYOUT_UNIT x, y, w, h;
}FIXED_LAYOUT_CELL_RECT;

typedef struct _fixed_layout_cell_info{
	FIXED_LAYOUT_CELL_RECT	rect;
	int cell_type; //GRID_CELL_TYPE
	union{
		HWND hwnd;
		GRID_LAYOUT* grid;
		GRID_USER_TYPE* user;
		void* data;
	}data;
	struct _fixed_layout_cell_info * next;
}FIXED_LAYOUT_CELL;

typedef struct _fixed_layout_t{
	GRID_USER_TYPE_INF * inf;
	void * user_data;
	FIXED_LAYOUT_CELL * head;
	int x, y, w, h;
}FIXED_LAYOUT;

BOOL FixedLayoutInit(FIXED_LAYOUT * pfl);

BOOL FixedLayoutInsert(FIXED_LAYOUT* pfl, int type, FIXED_LAYOUT_CELL_RECT* rect, void *celldata);

void FixedLayoutClear(FIXED_LAYOUT* pfl);


#ifdef __cplusplus
}
#endif


#ifdef __cplusplus
class FixedLayout : protected  FIXED_LAYOUT
{
public:
	FixedLayout(){
		FixedLayoutInit(this);
	}

	~FixedLayout(){
		FixedLayoutClear(this);
	}

	BOOL Insert(int type, FIXED_LAYOUT_CELL_RECT* prt, void * celldata){
		return FixedLayoutInsert(this,type, prt, celldata);
	}

};


#endif

#endif
