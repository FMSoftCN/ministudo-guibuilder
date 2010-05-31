/*
 * undo-redo.h
 *
 *  Created on: 2009-3-25
 *      Author: dongjunjie
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
