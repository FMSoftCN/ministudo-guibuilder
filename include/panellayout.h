
#ifndef PANNEL_LAYOUT_H
#define PANNEL_LAYOUT_H

#define MAKE_TAB_ID(row, col)  (((row)<<8) | (col))
#define ID_TABSHEET_BEGIN 1000
#define ID_TABSHEET_EDITOR  ID_TABSHEET_BEGIN + MAKE_TAB_ID(0,2)

class PanelCell : public GridLayoutUserCellData
{
protected:
	string m_name;

public:
	PanelCell(const char* name=NULL){
		if(name)
			m_name = name;
	}

	const char* getName(){ return m_name.c_str(); }
};

class PanelPageCell: public PanelCell
{
protected:

public:
	PanelPageCell(const char* name):PanelCell(name){
		m_tabsheet = HWND_INVALID;
	}


	HWND m_tabsheet;
	void setRect(int x, int y, int width, int height){
		//printf("m_tablesheet=%p, %d,%d,%d,%d\n",m_tabsheet,x, y, width, height);
		MoveWindow(m_tabsheet, x, y, width, height, TRUE);
	}

	void show(int show){
		ShowWindow(m_tabsheet, show?SW_SHOW:SW_HIDE);
	}

	void insertPanel(HWND hParent,Panel* panel);
};

class PanelProxyCell : public PanelCell
{
	PanelCell * m_realCell;
public:
	PanelProxyCell(const char* name){
		if(!name)
			throw("PanelProxyCell: must have a name");
		m_name = name;
		m_realCell = NULL;
	}
	~PanelProxyCell(){

	}

	void setCell(PanelCell* pcell) {
		m_realCell = pcell;
	}

	void setRect(int x, int y, int width, int height){
		if(m_realCell)
			m_realCell->setRect(x, y, width, height);
	}

	void show(int show){
		if(m_realCell)
			m_realCell->show(show);
	}
};


class PanelLayout;
class CellInfo{
private:
    int row;
    int col;
    PanelLayout *layout;

public:
    CellInfo (int row, int col, PanelLayout *layout)
    {
        this->row = row;
        this->col = col;
        this->layout = layout;
    }

    int getRow() { return row;}
    int getColumn() { return col;}
    PanelLayout* getPanelLayout() { return layout;}

    ~CellInfo (){ }
};

class PanelManager
{
public:
	virtual ~PanelManager()
    {
        CellInfo* info;
        map<string, CellInfo*>::iterator it; 

        for(it = namedCells.begin(); it != namedCells.end(); ++it){
            info = it->second;
            if(info)
                delete info;
        }
    }

	virtual Panel* createPanel(const char* name, const char* caption, const mapex<string, string> *params) = 0;
	BOOL getCellByName(const char* name, int &col, int &row, PanelLayout** layout);
	mapex<string, CellInfo*> namedCells;
};

class PanelLayout : public GridLayout, public PanelCell
{
public:

	PanelLayout(const char* name=NULL);
	~PanelLayout();
	PanelLayout(int row, int height, const char* name = NULL);


	bool load(const char* panelInfo, PanelManager* pPanelManager);
	bool load(xmlNodePtr node, PanelManager* pPanelManager);

	void* userToGridlayout(){ return (void*)(GridLayout*)this; }

	void setRect(int x, int y, int width, int height)
	{
		SetGridLayoutRect(x, y, width, height);
	}

	void show(int show);

	HWND getWindowOwner(int col, int row, const char* strCaption = NULL);

	void insertWindow(int col, int row, HWND hwnd);

protected:

	void setSubPanelLayout(int row, int col, PanelLayout * pl);

	GRID_CELL* findCellByName(const char* name);

	void loadCell(int row, int col, xmlNodePtr node, PanelManager* pPanelManager);
	Panel * loadPanel(int row, int col, xmlNodePtr node, PanelManager* pPanelManager);
};

#endif
