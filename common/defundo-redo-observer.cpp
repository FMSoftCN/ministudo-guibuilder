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
