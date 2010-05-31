/*
 * defundo-redo-observer.h
 *
 *  Created on: 2009-6-25
 *      Author: dongjunjie
 */

#ifndef DEFUNDOREDOOBSERVER_H_
#define DEFUNDOREDOOBSERVER_H_

#define DEF_UNDOREDO_MAX  32
#define DEF_UNDOREDO_MIN  8
class DefaultUndoRedoObserver : public UndoRedoObserver
{
public:
	DefaultUndoRedoObserver(int maxcount=DEF_UNDOREDO_MAX);
	~DefaultUndoRedoObserver();

	void pushUndoRedoCommand(UndoRedoCommand* cmd);
	const UndoRedoCommand* undo();
	const UndoRedoCommand* redo();
	bool canUndo(){ return undo_tail != undo_head; }
	bool canRedo(){ return redo_tail != undo_tail; }

	void emptyRedo();

protected:
	UndoRedoCommand ** commands;
	int undo_tail;
	int undo_head;
	int redo_tail;
	int maxcount;
};

#endif /* DEFUNDOREDOOBSERVER_H_ */
