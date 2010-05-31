
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
