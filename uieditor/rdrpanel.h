/*
 * rdrpanel.h
 *
 *  Created on: 2009-3-22
 *      Author: dongjunjie
 */

#ifndef RDRPANEL_H_
#define RDRPANEL_H_

class RendererPanel: public FieldPanel {
protected:
	static IDValueType _idValueType;
	ValueType * getIDValueType() {
		return &_idValueType;
	}
	virtual PFNLVCOMPARE getSortFunc(){ return _rdr_cmp; }
private:
	static int _rdr_cmp(HLVITEM nItem1, HLVITEM nItem2, PLVSORTDATA sortData);

public:
	RendererPanel(PanelEventHandler* handler);
	virtual ~RendererPanel();
};

#endif /* RDRPANEL_H_ */
