/*
 * file: mgcomm.h the common header file of MGFC
 *
 * MGFC -MiniGUI Foundation Classes, The C++ Classes based on MiniGUI
 * CopyRight (c) 2007
 * Author: vecodo doon. (vecodo@sohu.com)
 * http://www.mgfc.org.cn  http://www.minigui.org
 * Any question e-mail: mgfc888@sohu.com, mgfc666@sina.com
 *
 * GPL Stander
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#ifndef MGCOMM_H
#define MGCOMM_H
/*
  This is the MGFC common include file
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <sys/types.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#define MAKE_MG_VER(major, micro, minor) \
	(((major)<<16)|((micro)<<8)|(minor))

#define MG_VER MAKE_MG_VER(MINIGUI_MAJOR_VERSION,MINIGUI_MICRO_VERSION,MINIGUI_MINOR_VERSION)

#define MG_VER_3_0_x MAKE_MG_VER(3,0,0)

#define MAX_TEXT_BUF_SIZE 512

#if MG_VER < MG_VER_3_0_x
#include <minigui/mywindows.h>
#include <minigui/mgext.h>
#endif

#ifndef min
#define min(a,b)    ((a)<(b)?(a):(b))
#endif
#ifndef max
#define max(a,b)    ((a)>(b)?(a):(b))
#endif

/*
 * undefine the function-like Marco, and rewite them as inline functions,
 * so that we can reload them in MGFC classes.
 */
#ifdef SetTimer //Minigui >= 2.0.4
#undef SetTimer
inline BOOL SetTimer(HWND hWnd,int id,unsigned int speed) { return SetTimerEx(hWnd,id,speed,NULL); }
#endif

#ifdef TextOut
#undef TextOut
inline int TextOut(HDC hdc,int x,int y,const char* strText) { return TextOutLen(hdc,x,y,strText,-1); }
#endif

#ifdef TabbedTextOut
#undef TabbedTextOut
inline int TabbedTextOut(HDC hdc, int x, int y, const char* spText){
	return TabbedTextOutLen(hdc,x,y,spText,-1);
}
#endif

#ifdef DrawText
#undef DrawText
inline int DrawText(HDC hdc, const char* pText, int nCount,RECT* pRect, UINT nFormat){
	return DrawTextEx2(hdc,pText,nCount,pRect,0,nFormat,NULL);
}
#endif


#ifdef SetPenWidth
#undef SetPenWidth
inline unsigned int SetPenWidth(HDC hdc,DWORD width){
	return (unsigned int) SetDCAttr (hdc, DC_ATTR_PEN_WIDTH, (DWORD) width);
}
#endif

/*
 * The packed class of POINT struct
 */
class MGPoint:public POINT
{
public:
	MGPoint(){}
	MGPoint(int x,int y){
		SetPoint(x,y);
	}
	MGPoint(const POINT &pt){
		SetPoint(pt);
	}
	MGPoint(const MGPoint &mgpt){
		SetPoint(mgpt);
	}
	~MGPoint(){}

	inline void SetPoint(int x,int y){
		this->x = x; this->y = y;
	}
	inline void SetPoint(const POINT &pt){
		this->x = pt.x; this->y = pt.y;
	}
	inline void SetPoint(const POINT* ppt){
		if(ppt)
			x = ppt->x; y = ppt->y;
	}
	inline void SetPoint(const MGPoint &mgpt){
		x = mgpt.x; y = mgpt.y;
	}
	inline void SetPoint(const MGPoint *pmgpt){
		if(pmgpt)
			x = pmgpt->x; y = pmgpt->y;
	}

	inline MGPoint& operator=(const POINT& pt){
		SetPoint(pt);
		return *this;
	}
	inline MGPoint& operator=(const MGPoint& mgpt){
		SetPoint(mgpt);
		return *this;
	}
};

inline BOOL operator==(const MGPoint &mgpt1, const MGPoint &mgpt2){
	return mgpt1.x==mgpt2.x && mgpt1.y==mgpt2.y;
}
inline BOOL operator==(const POINT& pt1,const POINT& pt2){
	return pt1.x==pt2.x && pt1.y==pt2.y;
}
inline BOOL operator==(const POINT& pt,const MGPoint& mgpt){
	return pt.x==mgpt.x && pt.y==mgpt.y;
}
inline BOOL operator==(const MGPoint& mgpt,const POINT& pt){
	return mgpt.x==pt.x && mgpt.y==pt.y;
}

/*
 * The packed class of RECT
 */
class MGRect: public RECT
{
public:
	MGRect(){}

	MGRect(const RECT &rt){
		SetRect(rt);
	}
	MGRect(const RECT *prt){
		SetRect(prt);
	}
	MGRect(const MGRect & mgrt){
		SetRect(mgrt);
	}
	MGRect(const MGRect * pmgrt){
		SetRect(pmgrt);
	}
	MGRect(int w,int h){
		left = top = 0;
		right = w; bottom = h;
	}
	MGRect(int left,int top,int right,int bottom){
		SetRect(left,top,right,bottom);
	}
	~MGRect(){}

	inline void SetRect(const MGRect &mgrt){
		left = mgrt.left;
		top = mgrt.top;
		right = mgrt.right;
		bottom = mgrt.bottom;
	}
	inline void SetRect(const MGRect *pmgrt){
		if(pmgrt){
			left = pmgrt->left; top = pmgrt->top;
			right = pmgrt->right; bottom = pmgrt->bottom;
		}
		else {
			left = top = right = bottom = 0;
		}
	}
	inline void SetRect(const RECT &rt){
		*((RECT*)this) = rt;
	}
	inline void SetRect(const RECT *prt){
		if(prt)
			*((RECT*)this) = *prt;
		else{
			left = top = right = bottom = 0;
		}
	}
	inline void SetRect(int width,int height){
		right = left + width;
		bottom = top + height;
	}
	inline void SetRect(int left,int top,int right,int bottom){
		this->left = left;
		this->top = top;
		this->right = right;
		this->bottom = bottom;
	}

	inline MGRect& operator=(const RECT &rt){
		SetRect(rt);
		return *this;
	}
	inline MGRect& operator=(const MGRect &mgrt){
		SetRect(mgrt);
		return *this;
	}

	inline int Width() const { return right-left; }
	inline int Height() const { return bottom-top; }

	inline void SetWidth(int width){ right = left + width; }
	inline void SetHeight(int height){ bottom = top + height; }

	inline void OffsetRect(int x,int y){ left+=x;top+=y;right+=x;bottom+=y; }

	inline void Empty(){ left = top = right = bottom = 0; }

	inline void InflateRect(int cx,int cy){
		::InflateRect(this,cx,cy);
	}

	inline void InflateRectToPt(int x,int y){
		::InflateRectToPt(this,x,y);
	}

	inline BOOL PtInRect(int x,int y){
		return ::PtInRect(this,x,y);
	}
	inline BOOL PtInRect(const POINT& pt){
		return ::PtInRect(this,pt.x,pt.y);
	}
	inline BOOL PtInRect(const MGPoint& mgpt){
		return ::PtInRect(this,mgpt.x,mgpt.y);
	}

	inline BOOL IsEmpty(){
		return ::IsRectEmpty(this);
	}
	inline BOOL operator!(){
		return IsEmpty();
	}
};

inline BOOL operator==(const RECT& rt1,const RECT& rt2){
	return EqualRect(&rt1,&rt2);
}
inline BOOL operator==(const MGRect& mgrt1,const MGRect& mgrt2){
	return EqualRect(&mgrt1,&mgrt2);
}
inline BOOL operator==(const MGRect& mgrt,const RECT& rt){
	return EqualRect(&mgrt,&rt);
}
inline BOOL operator==(const RECT& rt,const MGRect& mgrt){
	return EqualRect(&rt,&mgrt);
}

/*
 * Clip Region
 * if you want extend this class, please don't add new member,
 * because this class only created by the minigui API
 */
extern BLOCKHEAP __mg_FreeClipRectList;
class MGClipRgn:public CLIPRGN
{
public:
	MGClipRgn(PBLOCKHEAP pblock=NULL){
		::InitClipRgn(this,(pblock==NULL)?&__mg_FreeClipRectList:pblock);
	}
	MGClipRgn(const RECT& rt,PBLOCKHEAP pblock=NULL){
		::InitClipRgn(this,pblock==NULL?&__mg_FreeClipRectList:pblock);
		::SetClipRgn(this,&rt);
	}
	MGClipRgn(const MGRect& mgrt,PBLOCKHEAP pblock=NULL){
		::InitClipRgn(this,pblock==NULL?&__mg_FreeClipRectList:pblock);
		::SetClipRgn(this,&mgrt);
	}
	MGClipRgn(const MGClipRgn& mgrgn){
		::ClipRgnCopy(this,&mgrgn);
	}
	MGClipRgn(const CLIPRGN &rgn){
		::ClipRgnCopy(this,&rgn);
	}
	MGClipRgn(const PCLIPRGN pClipRgn){
		::ClipRgnCopy(this,pClipRgn);
	}
	MGClipRgn(int sx,int sy,int r,PBLOCKHEAP pblock=NULL){
		::InitClipRgn(this,pblock==NULL?&__mg_FreeClipRectList:pblock);
		::InitCircleRegion(this,sx,sy,r);
	}
	MGClipRgn(int x,int y,int rx,int ry,PBLOCKHEAP pblock=NULL){
		::InitClipRgn(this,pblock==NULL?&__mg_FreeClipRectList:pblock);
		::InitEllipseRegion(this,x,y,rx,ry);
	}
	MGClipRgn(const POINT *pts,int vertics,PBLOCKHEAP pblock=NULL){
		::InitClipRgn(this,pblock==NULL?&__mg_FreeClipRectList:pblock);
		::InitPolygonRegion(this,pts,vertics);
	}

	~MGClipRgn(){
		::EmptyClipRgn(this);
	}

	inline MGClipRgn & operator=(const RECT& rt){
		SetRect(rt);
		return *this;
	}
	inline MGClipRgn & operator=(const MGRect& mgrt){
		SetRect(mgrt);
		return *this;
	}
	inline MGClipRgn & operator=(const MGClipRgn &mgrgn){
		SetRgn((const PCLIPRGN)&mgrgn);
		return *this;
	}
	inline MGClipRgn & operator=(const CLIPRGN &rgn){
		SetRgn(&rgn);
		return *this;
	}

	inline void Empty(){
		::EmptyClipRgn(this);
	}
	inline BOOL Intersect(const MGClipRgn& rgn1,const MGClipRgn& rgn2){
		return ::ClipRgnIntersect(this,&rgn1,&rgn2);
	}
	inline BOOL Intersect(const CLIPRGN* pRgn1, const CLIPRGN* pRgn2){
		return ::ClipRgnIntersect(this,pRgn1,pRgn2);
	}
	inline void GetBoundRect(PRECT prect){
		::GetClipRgnBoundRect(this,prect);
	}

	inline BOOL SetRect(const RECT& rt){
		return ::SetClipRgn(this,&rt);
	}
	inline BOOL SetRect(const MGRect& mgrt){
		return ::SetClipRgn(this,&mgrt);
	}
	inline BOOL SetRect(const PRECT prt){
		return ::SetClipRgn(this,prt);
	}

	inline BOOL SetRgn(const CLIPRGN* prgn){
		return ::ClipRgnCopy(this,prgn);
	}

	inline BOOL IsEmpty(){
		return ::IsEmptyClipRgn(this);
	}
	inline BOOL operator !(){
		return IsEmpty();
	}

	inline BOOL AddRect(const RECT &rt){
		return ::AddClipRect(this,&rt);
	}
	inline BOOL AddRect(const MGRect &mgrt){
		return ::AddClipRect(this,&mgrt);
	}
	inline BOOL AddRect(const PRECT prt){
		return ::AddClipRect(this,prt);
	}

	inline BOOL Intersect(const RECT &rt){
		return ::IntersectClipRect(this,&rt);
	}
	inline BOOL Intersect(const MGRect &mgrt){
		return ::IntersectClipRect(this,&mgrt);
	}
	inline BOOL Intersect(const PRECT prt){
		return ::IntersectClipRect(this,prt);
	}

	inline BOOL Subtract(const RECT &rt){
		return ::SubtractClipRect(this,&rt);
	}
	inline BOOL Subtract(const MGRect &mgrt){
		return ::SubtractClipRect(this,&mgrt);
	}
	inline BOOL Subtract(const PRECT prt){
		return ::SubtractClipRect(this,prt);
	}

	inline BOOL PtIn(int x,int y){
		return ::PtInRegion(this,x,y);
	}
	inline BOOL PtIn(const POINT pt){
		return ::PtInRegion(this,pt.x,pt.y);
	}
	inline BOOL PtIn(const MGPoint mgpt){
		return ::PtInRegion(this,mgpt.x,mgpt.y);
	}

	inline BOOL RectIn(const RECT& rt){
		return ::RectInRegion(this,&rt);
	}
	inline BOOL RectIn(const MGRect& mgrt){
		return ::RectInRegion(this,&mgrt);
	}
	inline BOOL RectIn(const PRECT prt){
		return ::RectInRegion(this,prt);
	}

	inline void Offset(int x,int y){
		::OffsetRegion(this,x,y);
	}
	inline void Offset(const POINT pt){
		::OffsetRegion(this,pt.x,pt.y);
	}
	inline void Offset(const MGPoint mgpt){
		::OffsetRegion(this,mgpt.x,mgpt.y);
	}

	inline BOOL Union(const CLIPRGN* src1, const CLIPRGN* src2){
		return ::UnionRegion(this,src1,src2);
	}
	inline BOOL Union(const CLIPRGN* src){
		return ::UnionRegion(this,this,src);
	}

	inline BOOL Subtract(const CLIPRGN* rgnM, const CLIPRGN* rgnS){
		return ::SubtractRegion(this,rgnM,rgnS);
	}
	inline BOOL Subtract(const CLIPRGN* rgnS){
		return ::SubtractRegion(this,this,rgnS);
	}

	inline BOOL Xor(const CLIPRGN *src1, const CLIPRGN *src2){
		return ::XorRegion(this,src1,src2);
	}
	inline BOOL Xor(const CLIPRGN *src){
		return ::XorRegion(this,this,src);
	}

	inline void operator += (const RECT &rt){
		AddRect(rt);
	}
	inline void operator += (const MGRect &mgrt){
		AddRect(mgrt);
	}
	inline void operator += (const MGClipRgn &mgrgn){
		Union(&mgrgn);
	}
	inline void operator += (const CLIPRGN &rgn){
		Union(&rgn);
	}

	inline void operator -= (const RECT &rt){
		Subtract(rt);
	}
	inline void operator -= (const MGRect &mgrt){
		Subtract(mgrt);
	}
	inline void operator -= (const MGClipRgn &mgrgn){
		Subtract(&mgrgn);
	}
	inline void operator -= (const CLIPRGN &rgn){
		Subtract(&rgn);
	}

	inline void operator ^= (const MGClipRgn &mgrgn){
		Xor(&mgrgn);
	}
	inline void operator ^= (const CLIPRGN &rgn){
		Xor(&rgn);
	}
};

typedef void* (*RUNTIME_OBJ)(void);

#define DECLEAR_RUNTIME(class_name) public: static void* GetRunTimeClass(){ return new class_name; }
#define RUNTIME_CLASS(class_name)  class_name::GetRunTimeClass

#endif
