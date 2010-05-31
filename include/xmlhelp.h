
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
