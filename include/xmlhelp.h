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

#ifndef XML_HELP_H
#define XML_HELP_H

#define xhIsNode(node, nodename) \
	((node->type == XML_ELEMENT_NODE) && xmlStrcmp((node)->name, (xmlChar*)(nodename))==0)

#define xhGetNodeText(node) \
	xmlNodeListGetString((node)->doc, (node)->xmlChildrenNode, 1)

int xhGetNodeInt(xmlNodePtr node);

xmlChar* xhGetChildText(xmlNodePtr node, const char* node_name);

int xhGetIntProp(xmlNodePtr node, const char* attr, int def = -1);

int xhGetChildInt(xmlNodePtr node, const char* node_name, int defValue=0);

xmlNodePtr xhGetChild(xmlNodePtr node, const char* node_name);

int xhGetChildCount(xmlNodePtr node, const char* node_name);

unsigned int xhGetBoolProp(xmlNodePtr node, const char* attr_name, unsigned int true_value = 1, const char* str_true="true");

bool xhHasElementNode(xmlNodePtr node);

#endif
