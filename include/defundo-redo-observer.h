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
