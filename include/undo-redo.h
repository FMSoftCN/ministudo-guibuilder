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

#ifndef UNDOREDO_H_
#define UNDOREDO_H_

#define instanceof(objptr, clss)  ((dynamic_cast<const clss*>(objptr))!=NULL)

class UndoRedoCommand{
public:
	virtual ~UndoRedoCommand(){}
	virtual void execute() = 0;
};

class UndoRedoObserver
{
public:
	virtual ~UndoRedoObserver(){}
	virtual void pushUndoRedoCommand(UndoRedoCommand* cmd) = 0;
	virtual bool canUndo() = 0;
	virtual bool canRedo() = 0;
	virtual const UndoRedoCommand* undo() = 0;
	virtual const UndoRedoCommand* redo() = 0;

	virtual void  emptyRedo() = 0;
};

class UndoRedoObject
{
protected:
	UndoRedoObserver * observer;

	inline void pushUndoRedoCommand(UndoRedoCommand * cmmd) {
		if(observer)
		observer->pushUndoRedoCommand(cmmd);
	}

	inline bool haveUndoRedoObserver(){
		return observer!=NULL;
	}

	inline bool canUndo(){
		return observer?observer->canUndo():false;
	}

	inline bool canRedo(){
		return observer?observer->canRedo():false;
	}

	inline void emptyRedo(){
		if(observer)
			observer->emptyRedo();
	}
public:
	UndoRedoObject(UndoRedoObserver* observer=NULL):observer(observer){}
	virtual ~UndoRedoObject(){}
	virtual void setUndoRedoObserver(UndoRedoObserver * undo_redo_observer) {
		observer = undo_redo_observer;
	}
};

#endif /* UNDOREDO_H_ */
