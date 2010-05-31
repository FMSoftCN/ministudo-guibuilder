#include <stdio.h>
#include <stdlib.h>

#include <libxml/xmlmemory.h>
#include <libxml/xmlstring.h>
#include <libxml/debugXML.h>
#include <libxml/HTMLtree.h>
#include <libxml/xmlIO.h>
#include <libxml/xinclude.h>
#include <libxml/catalog.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>

int 
main(int argc, char *argv[])
{
	int i;
	xmlChar * keyword;
//	xmlChar * xpath = "//@caption";

	if (argc != 3)
	{
		fprintf(stderr, "Usage: %s filename keyword\n", argv[0]);
		return 1;
	}

	xmlDocPtr doc = xmlParseFile(argv[1]);
	xmlXPathContextPtr context = xmlXPathNewContext(doc);
	xmlXPathObjectPtr result = xmlXPathEvalExpression(argv[2], context);

	if (xmlXPathNodeSetIsEmpty(result->nodesetval))
	{
		printf("No result\n");
		return 1;
	}

	xmlNodeSetPtr nodeset = result->nodesetval;
	
	for (i = 0; i < nodeset->nodeNr; i++)
	{
		keyword = xmlNodeListGetString(doc, nodeset->nodeTab[i]->xmlChildrenNode, 1);
		printf("_\(\"%s\")\n", keyword);
		xmlFree(keyword);
	}

	if (doc)
		xmlFreeDoc(doc);	
	
	xmlXPathFreeContext(context);
	xmlXPathFreeObject(result);
		
	return 0;
}
