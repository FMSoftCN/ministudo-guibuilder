
#ifndef GRID_LAYOUT_H
#define GRID_LAYOUT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _layout_unit{
	unsigned int type:2;
	int logic_size:14;
	unsigned int current_size:16;
}LAYOUT_UNIT;
enum layout_unit_type{
	lutPixel = 0,
	lutPercent,
	lutSpliter
};


typedef struct _layout_bkground{
	int type;
	union{
		gal_pixel bkcolor;
		PBITMAP tilBmp;
		struct {
			void (*drawbk)(HDC,int,int,int,int,void *param);
			void *param;
		}callback;
	}data;
}LAYOUT_BKGROUND;
enum layout_bkground_type{
	lbtNull=0,
	lbtColor,
	lbtBmpBrush,
	lbtCallback
};
#define RGBR(rgb)  (((rgb)&0xFF0000)>>16)
#define RGBG(rgb)  (((rgb)&0xFF00)>>8)
#define RGBB(rgb)  ((rgb)&0xFF)

/*
typedef struct _user_grid_type_intf{
	void (*setRect)(int x, int y, int width, int height, void * data);
	int (*parentMsgHook)(HWND hParent, int message, WPARAM wParam, LPARAM lParam, void* data);
}USER_GRID_TYPE_INTF;
*/

typedef struct _user_grid_type_inf{
	//USER_GRID_TYPE_INTF* intf;
	void (*setRect)(void* user_data, int x, int y, int width, int height);
	void* (*user_to_gridlayout)(void* user_data);
	void (*show)(void* user_data, int show);
	void (*free)(void* user_data);
}GRID_USER_TYPE_INF;

typedef struct _user_grid_type {
	GRID_USER_TYPE_INF * inf;
	void *user;
}GRID_USER_TYPE;

typedef struct _grid_layout GRID_LAYOUT;

typedef enum _grid_cell_type{
	gct_unknown = 0,
	gct_hwnd,
	gct_grid,
	gct_user,
	gct_max
}GRID_CELL_TYPE;

typedef struct _grid_cell{
	unsigned int type:8;
	unsigned int revert:8;
	unsigned int flag:16;
	union{
		HWND hwnd;
		GRID_LAYOUT* grid;
		GRID_USER_TYPE* user;
		void* data;
	}data;
	#define hwnd_cell data.hwnd
	#define grid_cell data.grid
	#define user_cell data.user
}GRID_CELL;

#define NOTNULL_USER(cell,func)  ((cell) \
	&& (cell)->data.user \
	&& (cell)->data.user->inf \
	&& (cell)->data.user->inf->func)

#define USER_DATA(cell)  ((cell)->data.user->user)
#define USER_CALL(cell, func) (*((cell)->data.user->inf->func))

inline static void grid_cell_set_rect(GRID_CELL *cell, int x, int y, int width, int height)
{
	if(NOTNULL_USER(cell,setRect))
	{
		//printf("+++++x=%d, y=%d, width=%d, height=%d\n", x, y, width, height);
		USER_CALL(cell,setRect)(USER_DATA(cell),
			x, y, width, height);
	}
}

inline static void * grid_cell_user_to_gridlayout(GRID_CELL* cell)
{
	if(NOTNULL_USER(cell, user_to_gridlayout))
	{
		return USER_CALL(cell, user_to_gridlayout)(USER_DATA(cell));
	}
	return NULL;
}

inline  static void grid_cell_show(GRID_CELL* cell, int show)
{
	if(NOTNULL_USER(cell, show)){
		USER_CALL(cell,show)(USER_DATA(cell),show);
	}
}

struct _grid_layout{
	HWND hwndAttached;
	// the size of grid layout
	int x, y, width, height;

	int row_cnt; // the total rows of grid
	LAYOUT_UNIT * row_height; // if it is null, mean avarage every rows

	int col_cnt; // the total cols of grid
	LAYOUT_UNIT * col_width; // if it is null, mean avarage every cols

	GRID_CELL ** cells;

/*	LAYOUT_BKGROUND bkground;
*/

};

//create a new layout
GRID_LAYOUT* CreateGridLayout(int rows, int cols);

int InitGridLayout(GRID_LAYOUT* layout, int rows, int cols);

//set the width of col or height of row
void GridLayoutSetColWidth(GRID_LAYOUT* playout, int col, int type, int logic_size);

void GridLayoutSetRowHeight(GRID_LAYOUT *playout, int row, int type, int logic_size);

void * GridLayoutGetCell(GRID_LAYOUT *playout, int row, int col);

//set new obj into the cell, and return the old one
void* GridLayoutSetCell(GRID_LAYOUT* playout, int row, int col, int cell_type, void* cell_data);

GRID_CELL*  GridLayoutHit(GRID_LAYOUT* playout, int x, int y);

void CleanupGridLayout(GRID_LAYOUT *playout, void (*free_cell)(GRID_CELL*));

void DeleteGridLayout(GRID_LAYOUT *playout, void (*free_cell)(GRID_CELL*));

void DrawBkground(HDC hdc, int x, int y, int w, int h, LAYOUT_BKGROUND *pbkg);

void GridLayoutDrawBkground(GRID_LAYOUT* playout, HDC hdc, int x, int y, int w, int h);

#define GridLayoutUserData(playout)  ((GRID_LAYOUT*)(playout)->user_data)

// if deal_obj return FALSE, exit the function

void GridLayoutEnumAllCellObjs(GRID_LAYOUT* playout, BOOL (*deal_obj)(GRID_CELL*,GRID_LAYOUT* _this,int row, int col,void *),void * user_data);

BOOL GridLayoutGetCellRect(GRID_LAYOUT* layout, int row, int col, PRECT prt);

BOOL GetRowColByPos(GRID_LAYOUT* playout, int x, int y, int *row, int *col);

void* GridLayoutRemove(GRID_LAYOUT* playout, int row, int col);

void* GridLayoutRemoveHit(GRID_LAYOUT* playout, int x, int y);

BOOL GetHittedGridCenterPos (GRID_LAYOUT* layout, int x, int y, int* grid_x, int* grid_y);

//
void SetGridLayoutRect(GRID_LAYOUT* playout, int x, int y, int width, int height);

int GridLayoutHookProc(HWND hwnd,  GRID_LAYOUT* gl, int message, WPARAM wParam, LPARAM lParam );

/*BOOL  ApplyGridLayout(HWND hwnd, GRID_LAYOUT* playout);*/

void GridLayoutShow(GRID_LAYOUT* gl, int show);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

class GridLayoutUserCellData
{
protected:
	GRID_USER_TYPE m_type;
public:
	GridLayoutUserCellData(){
		static GRID_USER_TYPE_INF inf = {
				GridLayoutUserCellData::setRect,
				GridLayoutUserCellData::user_to_gridlayout,
				GridLayoutUserCellData::show,
				GridLayoutUserCellData::free
		};

		m_type.user = (void*) this;
		m_type.inf = &inf;
	}
	virtual ~GridLayoutUserCellData(){}

	virtual void setRect(int x, int y, int width, int height) = 0;
	virtual void* userToGridlayout(){ return NULL; }
	virtual void show(int show) = 0;

	GRID_USER_TYPE* GetUserType(){
		return &m_type;
	}
private:
	static void setRect(void * user_data,int x, int y, int width, int height){
		GridLayoutUserCellData* pdata = (GridLayoutUserCellData*)user_data;
		if(pdata)
			pdata->setRect(x, y, width, height);
	}
	static void*  user_to_gridlayout(void* user_data){
		GridLayoutUserCellData* pdata = (GridLayoutUserCellData*)user_data;
		if(pdata)
			return pdata->userToGridlayout();
		return NULL;
	}
	static void show(void* user_data, int show){
		GridLayoutUserCellData* pdata = (GridLayoutUserCellData*)user_data;
		if(pdata)
			pdata->show(show);
	}
	static void free(void* user_data) {
		if(user_data)
			delete (GridLayoutUserCellData*)(user_data);
	}
};

class GridLayout : protected GRID_LAYOUT
{
	void zeroGridLayout()
	{
		x = y = width = height = 0;
		col_cnt = row_cnt = 0;
		cells = NULL;
		col_width = row_height = NULL;
		hwndAttached = HWND_INVALID;
	}
public:
	GridLayout(){zeroGridLayout(); }
	GridLayout(int row, int height){zeroGridLayout(); InitGridLayout((GRID_LAYOUT*)this, row, height); }

	bool Init(int row, int height){return  InitGridLayout((GRID_LAYOUT*)this, row, height); }

	~GridLayout(){ Clean(NULL); }

	void SetColWidth(int col, int type, int logic_size){
		GridLayoutSetColWidth((GRID_LAYOUT*)this, col, type, logic_size);
	}

	void SetRowHeight(int row, int type, int logic_size){
		GridLayoutSetRowHeight((GRID_LAYOUT*)this, row, type, logic_size);
	}

	void * GetCell(int row, int col){
		return GridLayoutGetCell((GRID_LAYOUT*)this, row, col);
	}

	//set new obj into the cell, and return the old one
	void*  SetCell(int row, int col, int cell_type, void* cell_data){
		return GridLayoutSetCell((GRID_LAYOUT*)this, row, col, cell_type, cell_data);
	}

	GRID_CELL*  Hit(int x, int y){
		return GridLayoutHit((GRID_LAYOUT*)this, x, y);
	}

	void Clean( void (*free_cell)(GRID_CELL*)){
		CleanupGridLayout((GRID_LAYOUT*)this, free_cell);
	}

	// if deal_obj return FALSE, exit the function
	//
	void EnumAllCellObjs( BOOL (*deal_obj)(GRID_CELL*,GRID_LAYOUT* _this,int row, int col,void *),void * user_data){
		GridLayoutEnumAllCellObjs((GRID_LAYOUT*)this, deal_obj, user_data);
	}

	BOOL GetCellRect(int row, int col, PRECT prt){
		return GridLayoutGetCellRect((GRID_LAYOUT*)this, row, col, prt);
	}

	BOOL GetRowColByPos( int x, int y, int *row, int *col){
		return ::GetRowColByPos((GRID_LAYOUT*)this, x, y, row, col);
	}

	void* Remove( int row, int col){
		return GridLayoutRemove((GRID_LAYOUT*)this, row, col);
	}

	void* RemoveHit( int x, int y){
		return GridLayoutRemoveHit((GRID_LAYOUT*)this, x, y);
	}

	BOOL GetHittedGridCenterPos ( int x, int y, int* grid_x, int* grid_y){
		return ::GetHittedGridCenterPos((GRID_LAYOUT*)this, x, y, grid_x, grid_y);
	}

	void SetGridLayoutRect(int x, int y, int width, int height){
		::SetGridLayoutRect((GRID_LAYOUT*)this, x, y, width, height);
	}

	int GridLayoutHookProc(HWND hwnd, int message, WPARAM wParam, LPARAM lParam){
		return ::GridLayoutHookProc(hwnd, (GRID_LAYOUT*)this, message, wParam, lParam);
	}

	void SetAttachedWnd(HWND hwnd){
		hwndAttached = hwnd;
	}
	/*
	BOOL  Apply(HWND hwnd){
		return ApplyGridLayout(hwnd, (GRID_LAYOUT*)this);
	}*/
};

#endif

#endif
