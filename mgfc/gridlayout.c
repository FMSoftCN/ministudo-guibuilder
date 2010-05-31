
#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>

#define SPLITER_SIZE 2
#include "gridlayout.h"

HCURSOR hhorzCur= IDC_SIZENS;
HCURSOR hverCur = IDC_SIZEWE;
static void recalca_units(LAYOUT_UNIT* units, int n, int width)
{
	int i;
	int avgs = 0;
	int recent = width;
	int avg_size;

	if(!units || width <= 0) return;

	for(i=0;i<n;i++)
	{
		if(units[i].type != lutSpliter  && units[i].logic_size == 0)
		{
			avgs ++;
			continue;
		}

		if(units[i].type == lutPixel){
			units[i].current_size = units[i].logic_size;
		}
		else if(units[i].type == lutSpliter)
		{
			units[i].current_size = SPLITER_SIZE;
		}
		else{
			units[i].current_size = units[i].logic_size*width/100;
		}
		if(units[i].current_size > recent)
			units[i].current_size = recent;
		recent -= units[i].current_size;
	}

	if(recent <= 0)
		return;

	if(avgs <= 0)
		avgs = 1;

	avg_size = recent/avgs;
	for(i=0; i<n && avgs > 0; i++)
	{
		if(units[i].type != lutSpliter  && units[i].logic_size == 0)
		{
			units[i].current_size = avg_size;
			avgs --;
		}
	}
}

int InitGridLayout(GRID_LAYOUT* gl, int rows, int cols)
{
	int col;
	//CleanupGridLayout(gl, NULL);

	gl->row_cnt = rows;
	gl->col_cnt = cols;

	gl->cells = (GRID_CELL**)calloc(cols, sizeof(GRID_CELL*));

	for(col = 0; col<cols; col++)
	{
		gl->cells[col] = (GRID_CELL*)calloc(rows, sizeof(GRID_CELL));
	}
	return 1;
}

GRID_LAYOUT* CreateGridLayout(int rows, int cols)
{
	GRID_LAYOUT* gl;

	if(rows <= 0 || cols<= 0)
		return NULL;

	gl = (GRID_LAYOUT*)calloc(sizeof(GRID_LAYOUT),1);

	InitGridLayout(gl, rows, cols);

	return gl;
}

static LAYOUT_UNIT* set_unit_size(LAYOUT_UNIT* units, int idx, int n, int width, int type, int logic_size)
{
	if(idx < 0 || idx >= n)
		return units;

	if(units == NULL)
	{
		units = (LAYOUT_UNIT*)calloc(n, sizeof(LAYOUT_UNIT));
	}

	if(type<0 || type>3)
		type = 0;

	units[idx].type = type;
	units[idx].logic_size = logic_size;

	recalca_units(units, n, width);
	return units;
}

void GridLayoutSetRowHeight(GRID_LAYOUT* playout, int row, int type, int logic_size)
{
	if(playout==NULL || row>= playout->row_cnt)
		return;
	playout->row_height = set_unit_size(playout->row_height, row,
		playout->row_cnt,
		playout->height,
		type,
		logic_size);
}

void GridLayoutSetColWidth(GRID_LAYOUT* playout, int col, int type, int logic_size)
{
	if(playout==NULL || col>= playout->col_cnt )
		return;
	playout->col_width = set_unit_size(playout->col_width, col,
		playout->col_cnt,
		playout->width,
		type,
		logic_size);
}

void *GridLayoutGetCell(GRID_LAYOUT* playout, int row, int col)
{
	if(!playout || (col <0 && col>=playout->col_cnt)
		|| (row < 0 && row>=playout->row_cnt)
		|| (playout->col_width && playout->col_width[col].type == lutSpliter)
		|| (playout->row_height && playout->row_height[row].type == lutSpliter))
		return NULL;

	return playout->cells[col][row].data.data;
}

//set new obj into the cell, and return the old one
void* GridLayoutSetCell(GRID_LAYOUT* playout, int row, int col, int type , void * new_obj)
{
	void* old;

	if(!playout || (col <0 && col>=playout->col_cnt)
		|| (row < 0 && row>=playout->row_cnt)
		|| (playout->col_width && playout->col_width[col].type == lutSpliter)
		|| (playout->row_height && playout->row_height[row].type == lutSpliter))
		return NULL;

	//check type of cell
	if(type <= gct_unknown || type >=gct_max)
		return NULL;

	old = playout->cells[col][row].data.data;
	playout->cells[col][row].data.data = new_obj;
	playout->cells[col][row].type = type;
	return old;
}

static int get_idx_by_pos(LAYOUT_UNIT* units, int pos ,int n)
{
	int i;
	int w = 0;
	for(i=0;i<n && pos>w;i++) {
		w += units[i].current_size;
//		printf("pos=%d,w=%d,i=%d\n",pos,w,i);
	}
	return pos>w?-1:(i-1);
}

BOOL GetRowColByPos(GRID_LAYOUT* playout, int x, int y, int *row, int *col)
{
	if(playout->row_height)
		*row = get_idx_by_pos(playout->row_height, y - playout->y, playout->row_cnt);
	else
		*row = (y-playout->y)/(playout->height/playout->row_cnt);

	if(playout->col_width)
		*col = get_idx_by_pos(playout->col_width, x - playout->x, playout->col_cnt);
	else
		*col = (x-playout->x)/(playout->width/playout->col_cnt);
	return (*row>=0 && (*row)<playout->row_cnt
			&& *col>=0 && (*col)<playout->col_cnt);
}


GRID_CELL* GridLayoutHit(GRID_LAYOUT* playout, int x, int y)
{
	int col,row;

	GetRowColByPos(playout, x, y, &row, &col);

	if(col>=0 && row>=0 && col<playout->col_cnt && row<playout->row_cnt)
		return & playout->cells[col][row];
	return NULL;
}

void* GridLayoutRemove(GRID_LAYOUT* playout, int row, int col)
{
	GRID_CELL* old;
	if(!playout || (col <0 && col>=playout->col_cnt)
		|| (row < 0 && row>=playout->row_cnt))
		return NULL;

	old = playout->cells[col][row].data.data;
	playout->cells[col][row].data.data = NULL;
	return old;
}

void* GridLayoutRemoveHit(GRID_LAYOUT* playout, int x, int y)
{
	int col,row;

	GetRowColByPos(playout, x, y, &row, &col);
	return GridLayoutRemove(playout, row, col);
}

static void free_cell_default(GRID_CELL *cell)
{
	if(cell->type == gct_grid)
		DeleteGridLayout(cell->data.grid, NULL);
	else if(cell->type == gct_user)
	{
		if(cell->data.user->inf && cell->data.user->inf->free)
			cell->data.user->inf->free(cell->data.user->user);
	}
}

void CleanupGridLayout(GRID_LAYOUT *playout, void (*free_cell)(GRID_CELL*))
{
	int col, row;;

	if(!playout)
		return;

	if(free_cell == NULL)
		free_cell = free_cell_default;

	for(col=0;col<playout->col_cnt;col++)
	{
		if(free_cell){
			for(row=0;row<playout->row_cnt;row++)
			{
				(*free_cell)(&(playout->cells[col][row]));
			}
		}
		free(playout->cells[col]);
	}
	free(playout->cells);

	if(playout->row_height)
		free(playout->row_height);

	if(playout->col_width)
		free(playout->col_width);
/*
	if(playout->bkground.type == lbtBmpBrush){
		UnloadBitmap(playout->bkground.data.tilBmp);
		free(playout->bkground.data.tilBmp);
	}*/
}

void DeleteGridLayout(GRID_LAYOUT *playout, void (*free_cell)(GRID_CELL*))
{
	if(playout){
		CleanupGridLayout(playout, free_cell);
		free(playout);
	}
}


void DrawBkground(HDC hdc, int x, int y, int w, int h, LAYOUT_BKGROUND *pbkg)
{
	register int rgb;

	switch(pbkg->type)
	{
	case lbtColor:
		{
			SetBrushType(hdc, BT_SOLID);
			rgb = pbkg->data.bkcolor;
			SetBrushColor(hdc, RGB2Pixel(hdc,RGBR(rgb),RGBG(rgb),RGBB(rgb)));
			FillBox(hdc,x,y,w,h);
		}
		break;

	case lbtBmpBrush:
		{
			SetBrushType(hdc, BT_TILED);
			SetBrushInfo(hdc,pbkg->data.tilBmp, NULL);
			FillBox(hdc,x,y,w,h);
			break;
		}
	case lbtCallback:
		{
			(*pbkg->data.callback.drawbk)(hdc,x,y,w,h,
				pbkg->data.callback.param);
		}
		break;
	}
}

static void _grid_layout_deal_cell_with_position(GRID_LAYOUT* playout,
	void (*deal_cell)(GRID_CELL* cell, PRECT prc, void *param),
	void *param)
{
	int col=-1,row=-1;
	int avgcol=0, avgrow=0;
	RECT rect;

	if(!playout || !deal_cell) return;

	rect.left = playout->x;
	rect.top = playout->y;

	if(!playout->col_width) avgcol = playout->width/playout->col_cnt;
	if(!playout->row_height) avgrow = playout->height/playout->row_cnt;

	for(col=0;col<playout->col_cnt;col++)
	{
		rect.right = rect.left+ (playout->col_width?playout->col_width[col].current_size:avgcol);
		rect.top = playout->y;
		if(playout->col_width == NULL || playout->col_width[col].type != lutSpliter)
		{
			for(row=0;row<playout->row_cnt;row++)
			{
				rect.bottom = rect.top + (playout->row_height?playout->row_height[row].current_size:avgrow);

				if(playout->row_height == NULL || playout->row_height[row].type != lutSpliter)
				{
					GRID_CELL* cell = &playout->cells[col][row];
					deal_cell(cell,&rect,param);
				}
				rect.top = rect.bottom;
			}
		}
		rect.left = rect.right;
	}
}

void GridLayoutEnumAllCellObjs(GRID_LAYOUT* playout, BOOL (*deal_obj)(GRID_CELL*,GRID_LAYOUT* this,int row, int col,void *),void * user_data)
{
	int row,col;
	
	if(!playout || !deal_obj)
		return;

	for(row=0;row<playout->row_cnt;row++)
	{
		for(col=0;col<playout->col_cnt;col++)
		{
			if(playout->cells[col][row].type!=0)
				if(!(*deal_obj)(&playout->cells[col][row],playout,row, col, user_data))
					return;
		}
	}
}

BOOL inline is_in_gridlayout (int x, int y, GRID_LAYOUT* layout)
{
    int in_x, in_y;

	in_x= x - layout->x;
    in_y = y - layout->y;

    return (in_x >= 0 && in_x < layout->width
            && in_y >= 0 && in_y < layout->height);
}


static int getPosFromUnit(LAYOUT_UNIT *unit, int idx, int n)
{
	int i;
	int pos = 0;
	for(i=0; i<idx;i++)
	{
		pos += unit[i].current_size;
	}
	return pos;
}

BOOL GridLayoutGetCellRect(GRID_LAYOUT* playout, int row, int col, PRECT prt)
{
	if((row<0 || row>=playout->row_cnt)
		|| (col<0 || col>=playout->col_cnt))
		return FALSE;

	if(playout->col_width)
	{
		prt->left = playout->x + getPosFromUnit(playout->col_width, col, playout->col_cnt);
		prt->right = prt->left + playout->col_width[col].current_size;
	}
	else
	{
		int avg = playout->width / playout->col_cnt;
		prt->left = playout->x + avg* col;
		prt->right = prt->left + avg;
	}

	if(playout->row_height)
	{
		prt->top = playout->y + getPosFromUnit(playout->row_height,row, playout->row_cnt);
		prt->bottom = prt->top + playout->row_height[row].current_size;
	}
	else
	{
		int avg = playout->height / playout->row_cnt;
		prt->top = playout->y + avg * row;
		prt->bottom = prt->top + avg;
	}
	return TRUE;
}

BOOL GetHittedGridCenterPos (GRID_LAYOUT* layout, int x, int y, int* grid_x, int* grid_y)
{
    int row;
    int col;
    RECT rc;

    if (!is_in_gridlayout (x, y, layout))
        return FALSE;

    GetRowColByPos (layout, x, y, &row, &col);
    GridLayoutGetCellRect(layout, row, col, &rc);
    *grid_x = (rc.left + rc.right) >> 1;
    *grid_y = (rc.top + rc.bottom) >> 1;
    return TRUE;
}

static void _grid_layout_resize(GRID_CELL* cell, PRECT prt,void* param)
{
	switch(cell->type){
	case gct_hwnd:
		if(IsWindow(cell->data.hwnd)){
			//printf("size changed: hwnd:%p, %d,%d,%d,%d\n",cell->data.hwnd,prt->left, prt->top, prt->right, prt->bottom);
			MoveWindow(cell->data.hwnd, prt->left, prt->top, RECTWP(prt), RECTHP(prt), TRUE);
		}
		break;
	case gct_grid:
		if(cell->data.grid){
			SetGridLayoutRect(cell->data.grid, prt->left,prt->top, RECTWP(prt), RECTHP(prt) );
		}
		break;
	case gct_user:
		grid_cell_set_rect(cell,
			prt->left,prt->top, RECTWP(prt), RECTHP(prt));
		break;
	default:
		return;
	}
}

void SetGridLayoutRect(GRID_LAYOUT* playout, int x, int y, int width, int height)
{
	if(playout == NULL)
		return ;

	if(width<=0 || height<=0 )
		return;

	playout->x = x;
	playout->y = y;

	if(width == playout->width && height == playout->height){
		_grid_layout_deal_cell_with_position(playout,_grid_layout_resize,NULL);
		return;
	}

	playout->width = width;
	playout->height = height;

	if(playout->col_width)
		recalca_units(playout->col_width, playout->col_cnt, playout->width);
	if(playout->row_height)
		recalca_units(playout->row_height, playout->row_cnt,playout->height);

	_grid_layout_deal_cell_with_position(playout,_grid_layout_resize,NULL);

}

static BOOL getSpliterInfo(GRID_LAYOUT* * pgl, int * is_horz, int * spliter_idx, int x, int y)
{
	int col, row;
	GRID_LAYOUT* gl = *pgl;
	while(GetRowColByPos(gl, x, y, &row, &col))
	{
		//printf("col=%d,row=%d\n", col, row);
		if(gl->col_width && gl->col_width[col].type == lutSpliter){
			//printf("find spilter -- col=%d\n", col);
			*spliter_idx = col;
			*is_horz = 0;
			*pgl = gl;
			return TRUE;
		}
		else if(gl->row_height && gl->row_height[row].type == lutSpliter)
		{
			//printf("find spilter -- row=%d\n", row);
			*spliter_idx = row;
			*is_horz = 1;
			*pgl = gl;
			return TRUE;
		}
		else  if(gl->cells){
			GRID_CELL *cell = &gl->cells[col][row];
			if(cell->data.data && cell->type == gct_grid)
				gl = cell->data.grid;
			else if(cell->data.data && cell->type == gct_user && (gl=grid_cell_user_to_gridlayout(cell))!=NULL){
				// do nothing
			}
			else
				break;
		}
		else{
			break;
		}
	}
	return FALSE;
}

static void drawSpliterBar(HDC hdc, GRID_LAYOUT* gl, int is_horz, int x, int y)
{
	int bx, by, bw, bh;
	if(is_horz){
		bx = gl->x;
		by = y -SPLITER_SIZE/2;
		bw = gl->width;
		bh =  SPLITER_SIZE;
	}
	else
	{
		by = gl->y;
		bh = gl->height;
		bx = x - SPLITER_SIZE/2;
		bw=  SPLITER_SIZE;
	}
	FillBox(hdc, bx, by , bw, bh);
}


static void updateSpliterSize(GRID_LAYOUT* gl, int is_horz, int idx, int x, int y)
{
	int idxbefore, idxafter;
	LAYOUT_UNIT* units = is_horz? gl->row_height:gl->col_width;
	int count = is_horz? gl->row_cnt:gl->col_cnt;
	int width = is_horz?gl->height:gl->width;
	int start = is_horz?gl->y:gl->x;
	int xy = is_horz?y:x;

	for(idxbefore = idx-1; idxbefore >= 0 && units[idxbefore].type == lutSpliter ; idxbefore --);
	for(idxafter = idx+1; idxafter < count && units[idxbefore].type == lutSpliter; idxafter ++);

	if(idxbefore  >=  0) {
		int nstart = start + getPosFromUnit(units, idxbefore, count);
		if(units[idxbefore].type == lutPixel){
			units[idxbefore].logic_size =  (xy<=nstart? 1:(xy-nstart));
		}
		else {
			if(width>0)
				units[idxbefore].logic_size = ((xy<=nstart)?1:((xy-nstart)*100/width));
		}
//			printf("log size=%d\n", units[idx].logic_size);
//			recalca_units(units, gl->row_cnt, gl->height);
	}

	if(idxafter < count)
	{
		int nend = start + getPosFromUnit(units, idxafter, count) + units[idxafter].current_size;
		if(units[idxafter].type == lutPixel){
			units[idxafter].logic_size  = (xy >=nend)?1:( nend - xy);
		}
		else {
			if(width > 0)
				units[idxafter].logic_size  = ((xy>=nend)?1:(nend-xy)*100/width);
		}
	}

	recalca_units(units, count, width);

	_grid_layout_deal_cell_with_position(gl,_grid_layout_resize,NULL);

}

static BOOL processSpliter(HWND hwnd, GRID_LAYOUT* gl, int message, WPARAM wParam, LPARAM lParam)
{
	static int _old_x = 0;
	static int _old_y = 0;
	static int _is_horz = 0;
	static int _spliter_idx = -1;
	static HDC _hdc = 0;
	static GRID_LAYOUT* _spliter_layout = NULL;

	int x = LOSWORD(lParam);
	int y = HISWORD(lParam);

	if(_spliter_layout == NULL){
		if(message == MSG_MOUSEMOVE || message == MSG_LBUTTONDOWN){
			if(!getSpliterInfo(&gl,  &_is_horz,&_spliter_idx, x, y))
				return FALSE;
			//find  a invalidate spliter center
			if(_spliter_idx > 0 && !((_is_horz && gl->row_cnt-1<=_spliter_idx) || (!_is_horz && gl->col_cnt -1<= _spliter_idx) ))
			{
				//
				SetCursor(GetSystemCursor(_is_horz?hhorzCur:hverCur));
				if(MSG_LBUTTONDOWN == message){
					_spliter_layout = gl;
					_old_x = x;
					_old_y = y;
					//printf("--hwnd=%p, old=(%d,%d) --- (%d,%d)\n",hwnd, _old_x, _old_y, x, y);
					_hdc = GetClientDC(hwnd);
					SetRasterOperation(_hdc, ROP_XOR);
					drawSpliterBar(_hdc, gl, _is_horz, x, y);
					SetCapture(hwnd);
					return TRUE;
				}
			}
		}

		return FALSE;
	}
	switch(message)
	{
	case MSG_MOUSEMOVE:
	{
		SetCursor(GetSystemCursor(_is_horz?hhorzCur:hverCur));
		//FIXME: when Call GetCapture, MiniGUI send x, y to window as screen position
		ScreenToClient(hwnd, &x, &y);
		drawSpliterBar(_hdc, _spliter_layout, _is_horz, _old_x, _old_y);
		drawSpliterBar(_hdc, _spliter_layout, _is_horz, x, y);
		//printf("hwnd=%p, old=(%d,%d) --- (%d,%d)\n",hwnd,_old_x, _old_y, x, y);
		_old_x = x;
		_old_y = y;
	}
		return TRUE;
	case MSG_LBUTTONUP:
		drawSpliterBar(_hdc, _spliter_layout, _is_horz, _old_x, _old_y);
		ReleaseDC(_hdc);
		_hdc = 0;
		SetCursor(GetDefaultCursor());
		ReleaseCapture();
		//
		ScreenToClient(hwnd, &x, &y);
		//printf("spliter_idx = %d\n",_spliter_idx);
		updateSpliterSize(_spliter_layout,_is_horz, _spliter_idx, x, y);
		_spliter_layout  = NULL;
		return TRUE;
	}
	return FALSE;
}

int GridLayoutHookProc(HWND hwnd, GRID_LAYOUT* gl, int message, WPARAM wParam, LPARAM lParam )
{
	if(gl){
		if(message == MSG_CSIZECHANGED)
		{
			SetGridLayoutRect(gl, 0, 0, wParam, lParam);
		}
		else {
			return processSpliter(hwnd, gl, message, wParam, lParam);
		}
	}
	return 0;
}

static BOOL _gridlayout_show(GRID_CELL* cell,GRID_LAYOUT* _this,int row, int col,void *data)
{
	switch(cell->type)
	{
	case gct_hwnd:
		ShowWindow(cell->data.hwnd, data?SW_SHOW:SW_HIDE);
		break;
	case gct_grid:
		GridLayoutShow(cell->data.grid, (int)data);
		break;
	case gct_user:
		grid_cell_show(cell,(int)data);
		break;
	}
	return TRUE;
}

void GridLayoutShow(GRID_LAYOUT* gl, int show)
{
	GridLayoutEnumAllCellObjs(gl,_gridlayout_show,(void *)show);
}

/*

BOOL ApplyGridLayout(HWND hwnd, GRID_LAYOUT* playout)
{
	if(playout == NULL || !IsWindow(hwnd))
		return FALSE;

	SetWindowAdditionalData(hwnd, (DWORD)playout);
	playout->hwndAttached = hwnd;
	playout->old_proc = SetWindowCallbackProc(hwnd, (WNDPROC)gridLayoutHookProc);

	return TRUE;
}
*/
