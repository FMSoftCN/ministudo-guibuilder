/*
 * defundo-redo-observer.cpp
 *
 *  Created on: 2009-6-25
 *      Author: dongjunjie
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "undo-redo.h"
#include "defundo-redo-observer.h"

DefaultUndoRedoObserver::DefaultUndoRedoObserver(int maxcount)
{
	if(DEF_UNDOREDO_MIN > maxcount)
		maxcount = DEF_UNDOREDO_MIN;

	this->maxcount = maxcount;
	undo_head = 0;
	undo_tail = 0;
	redo_tail = 0;

	//create
	commands = new UndoRedoCommand*[maxcount];
	memset(commands, 0, maxcount*sizeof(UndoRedoCommand*));
}

DefaultUndoRedoObserver::~DefaultUndoRedoObserver()
{
	for(int i=undo_head; i!=redo_tail; i++){
		if(i==maxcount)
			i = 0;
		if(commands[i])
			delete commands[i];
	}
	delete[] commands;
}

void DefaultUndoRedoObserver::pushUndoRedoCommand(UndoRedoCommand *cmd)
{
	int next;
	if(!cmd)
		return;

	//delete all the redo
	emptyRedo();

	next = undo_tail + 1;
	if(next >= maxcount)
		next = 0;
	if(next == undo_head) //stack if full, drap out the head
	{
		if(commands[undo_head])
			delete commands[undo_head];
		undo_head ++;
		if(undo_head==maxcount)
			undo_head = 0;
	}

	commands[undo_tail] = cmd;
	undo_tail = next;

	redo_tail = undo_tail;
}

const UndoRedoCommand* DefaultUndoRedoObserver::undo()
{
	if(!canUndo())
		return NULL;

	undo_tail --;
	if(undo_tail < 0)
		undo_tail = maxcount - 1;

	UndoRedoCommand * cmd = commands[undo_tail];
	if(cmd)
		cmd->execute();//auto reverse
	return cmd;
}

const UndoRedoCommand* DefaultUndoRedoObserver::redo()
{
	if(!canRedo())
		return NULL;

	UndoRedoCommand *cmd = commands[undo_tail];
	if(cmd)
		cmd->execute();//auto revers

	undo_tail ++;
	if(undo_tail == maxcount)
		undo_tail = 0;
	return cmd;
}


void DefaultUndoRedoObserver::emptyRedo()
{
	for(int i=undo_tail; i!=redo_tail; i++)
	{
		if(i==maxcount)
			i = 0;
		if(commands[i])
			delete commands[i];
	}
}
