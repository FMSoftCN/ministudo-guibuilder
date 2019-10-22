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

#include "xmlheads.h"

xmlChar* xhGetChildText(xmlNodePtr node, const char* node_name)
{
	xmlNodePtr child = xhGetChild(node, node_name);
	if(child)
		return xhGetNodeText(child);
	return NULL;
}

int xhGetChildInt(xmlNodePtr node, const char* node_name, int defValue /*=0*/){
	xmlChar* xtxt = xhGetChildText(node, node_name);
	if(xtxt){
		int intVal = strtol((const char*)xtxt,NULL,0);
		xmlFree(xtxt);
		return intVal;
	}

	return defValue;
}

int xhGetNodeInt(xmlNodePtr node)
{
	xmlChar * str = xhGetNodeText(node);
	if(str){
		int v = strtol((const char*)str,NULL,0);
		xmlFree(str);
		return v;
	}

	return -1;
}

int xhGetIntProp(xmlNodePtr node, const char* attr, int def)
{
	int v = def;
	if(node && attr)
	{
		xmlChar* xstr = xmlGetProp(node, (const xmlChar*)attr);
		if(xstr){
			v = strtol((const char*)xstr,NULL,0);
			xmlFree(xstr);
		}
	}
	return v;
}

xmlNodePtr xhGetChild(xmlNodePtr node, const char* node_name)
{
	if(node == NULL || node_name == NULL || node->type != XML_ELEMENT_NODE)
		return NULL;

	node = node->xmlChildrenNode;
	while(node){
		if(node->type == XML_ELEMENT_NODE && xmlStrcmp(node->name, (const xmlChar*)node_name) == 0){
			return node;
		}
		node = node->next;
	}
	return NULL;
}

int xhGetChildCount(xmlNodePtr node, const char* node_name)
{
	int count = 0;
	if(node == NULL || node_name == NULL || node->type != XML_ELEMENT_NODE)
		return count;

	node = node->xmlChildrenNode;
	while(node){
		if(node->type == XML_ELEMENT_NODE && xmlStrcmp(node->name, (const xmlChar*)node_name) == 0){
			count ++;
		}
		node = node->next;
	}
	return count;
}

unsigned int xhGetBoolProp(xmlNodePtr node, const char* attr_name, unsigned int true_value/* = 1*/, const char* str_true/*="true"*/)
{
	if(node == NULL || attr_name == NULL)
		return 0;

	if(str_true == NULL)
		str_true = "true";

	xmlChar* xstr = xmlGetProp(node, (const xmlChar*)attr_name);
	if(xstr == NULL)
		return 0;

	if(xmlStrcmp(xstr, (const xmlChar*)str_true) == 0){
		xmlFree(xstr);
		return true_value;
	}

	xmlFree(xstr);

	return 0;
}

bool xhHasElementNode(xmlNodePtr node)
{
	if(node == 0)
		return false;

	for(node=node->xmlChildrenNode; node; node=node->next)
		if(node->type == XML_ELEMENT_NODE)
			return true;
	return false;
}
