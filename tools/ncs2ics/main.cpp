

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xmlheads.h"
#include "xmlhelp.h"

#include <string>
#include <map>
#include <list>

using namespace std;


#include "trans.h"


//save as gui-config
static void save_one_obj(TransedObject* tobj, FILE *fp, char *prefix)
{
	int len = strlen(prefix);
	if(strcmp(tobj->getName().c_str(),"window::MainWnd") == 0)
	{
		fprintf(fp, "%s:!mainwnd{\n",prefix);
	}
	else 
	{
		fprintf(fp, "%s:!control{\n",prefix);
	}

	prefix[len++] = '\t';
	prefix[len] = 0;

	map<string,Variable*> &props = tobj->getProps();
	for(map<string, Variable*>::iterator it = props.begin();
		it != props.end(); ++it)
	{
		Variable *v = it->second;
		fprintf(fp,"%s%s:%s\n", prefix, it->first.c_str(), v->toString().c_str());
	}

	//printf children
	TransedObject* child = tobj->getChildren();
	if(child)
	{
		fprintf(fp,"%sconstrols:!controls{\n",prefix);
		prefix[len++] = '\t';
		prefix[len] = 0;
		for(;child; child = child->getNext())
		{
			save_one_obj(child, fp, prefix);
		}
		prefix[--len] = 0;
		fprintf(fp,"%s}\n", prefix);
	}

	
	if(len > 0)
		prefix[--len] = 0;
	fprintf(fp,"%s}\n\n",prefix);
}

static void save_gui_config(TransedObject *tobj, const char* source_file, const char* dest_file)
{
	if(!tobj || !dest_file)
		return ;
	
	FILE *fp = fopen(dest_file, "a+");
	if(!fp)
	{
		fprintf(stderr, "cannot open file \"%s\"\n", dest_file);
		return ;
	}
	
	char szPrefix[100] = "\0";
	//fp = stdout;
	save_one_obj(tobj, fp, szPrefix);

	//printf("-- fp=%p\n",fp);

	fclose(fp);

}

///////////////////////////////////////

#define TO_ESC_CHARS(ch)  do{cbuf[j++] = '\\'; cbuf[j++] = ch; } while(0)
#define ESC_TRNAS(esc_ch, tansed_ch)  if(str[i] == esc_ch) { TO_ESC_CHARS(tansed_ch); i++; continue; }
#define MAX_LINE 80
static const char* str_to_c_string(const char* str, char* cbuf)
{
	int i = 0;
	int j = 0;

	if(!str || !cbuf)
		return NULL;

	while(str[i])
	{
		ESC_TRNAS('\n','n');
		ESC_TRNAS('\r','r');
		ESC_TRNAS('\t','t');
		ESC_TRNAS('\\','\\');
		ESC_TRNAS('\'','\'');
		ESC_TRNAS('\"','\"');
		ESC_TRNAS('\v','v');
		ESC_TRNAS('\f','f');
		ESC_TRNAS('\a','a');
		ESC_TRNAS('\b','b');
		
		cbuf[j++] = str[i++];
		if(j % MAX_LINE == 0)
		{
			cbuf[j++] = '\\';
			cbuf[j++] = '\n';
		}
	}
	cbuf[j++] = 0;
	return cbuf;
}
#undef MAX_LINE
#undef ESC_TRNAS
#undef TO_ESC_CHARS

static void save_one_source(TransedObject* tobj, FILE *fp, char *prefix, const char* name)
{
	int len = strlen(prefix);
	char szbuf[1024*4];


	map<string,Variable*> &props = tobj->getProps();
	Variable* v;
	if(strcmp(tobj->getName().c_str(),"window::MainWnd") == 0)
	{
		prefix[len++] = '\t';
		prefix[len] = 0;


		//printf children
		TransedObject* child = tobj->getChildren();
		if(child)
		{
			fprintf(fp,"static CTRLDATA _%s_controls[] = {\n", name?name:"dlg");
			prefix[len++] = '\t';
			prefix[len] = 0;
			for(;child; child = child->getNext())
			{
				save_one_source(child, fp, prefix, name);
			}
			prefix[--len] = 0;
			fprintf(fp,"};\n", prefix);
		}

		
		if(len > 0)
			prefix[--len] = 0;


		fprintf(fp, "DLGTEMPLATE _%s_templ = {\n", name?name:"dlg");
		//style
		v = props["style"];
		fprintf(fp, "\t%s,/* style */\n", v?v->toString().c_str():"0");
		//exstyle
		v = props["exstyle"];
		fprintf(fp, "\t%s, /* exstyle */\n",v?v->toString().c_str():"0");
		//x, y, w, h
		v = props["x"];
		fprintf(fp,"\t%d, /* x */\n",v?v->getValue():0);
		v = props["y"];
		fprintf(fp,"\t%d, /* y */\n",v?v->getValue():0);
		v = props["width"];
		fprintf(fp,"\t%d, /* w */\n",v?v->getValue():0);
		v = props["height"];
		fprintf(fp,"\t%d, /* h */\n",v?v->getValue():0);
		//caption
		v = props["caption"];
		fprintf(fp,"\t_(\"%s\"), /* caption */\n",str_to_c_string(v?v->toString().c_str():"", szbuf));
		//hIcon
		fprintf(fp,"\t0, /* hIcon */\n");
		//hMenu
		fprintf(fp,"\t0, /* hMenu */\n");
		//controlnr
		fprintf(fp,"\tsizeof(_%s_controls)/sizeof(CTRLDATA), /* controlnr */\n",name?name:"dlg");
		//controls
		fprintf(fp,"\t_%s_controls, /* controls */\n", name?name:"dlg");
		//dwAddata
		fprintf(fp,"\t0 /* dwAddData */\n");

		fprintf(fp, "};\n");
	}
	else 
	{
		fprintf(fp, "%s{\n",prefix);
		//class_name
		v = props["class"];
		fprintf(fp,"%s\t\"%s\", /* class_name */\n", prefix,v?v->toString().c_str():"");
		//style
		v = props["style"];
		fprintf(fp,"%s\t%s, /* style */\n",prefix, v?v->toString().c_str():"");
		//x, y, w, h
		v = props["x"];
		fprintf(fp,"%s\t%d, /* x */\n", prefix ,v?v->getValue():0);
		v = props["y"];
		fprintf(fp,"%s\t%d, /* y */\n", prefix ,v?v->getValue():0);
		v = props["width"];
		fprintf(fp,"%s\t%d, /* w */\n", prefix ,v?v->getValue():0);
		v = props["height"];
		fprintf(fp,"%s\t%d, /* h */\n", prefix ,v?v->getValue():0);
		//id
		v = props["id"];
		fprintf(fp, "%s\t%d, /* id */\n", prefix, v?(v->getValue()&0xFFFF):0);
		//caption
		v = props["caption"];
		fprintf(fp,"%s\t_(\"%s\"), /* caption */\n",prefix, str_to_c_string(v?v->toString().c_str():"",szbuf));
		//dwAddData
		fprintf(fp,"%s\t0, /* dwAddData */\n", prefix);
		//exstyle
		v = props["exstyle"];
		fprintf(fp, "%s\t%s, /* exstye */\n", prefix, v?v->toString().c_str():"0");
		//werdr_name
		fprintf(fp, "%s\tNULL, /* werdr_name */\n",prefix);
		//we_attrs
		fprintf(fp, "%s\tNULL /* we_attrs */\n", prefix);

		fprintf(fp,"%s},\n",prefix);
	}


}

typedef struct _SAVE_SOURCE_INFO {
	const char* dest_file;
	const char** names;
	int          name_count;
	int          current;
}SAVE_SOURCE_INFO;

static void save_gui_source(TransedObject* tobj, const char* source_file, SAVE_SOURCE_INFO * pssi)
{
	if(!tobj || !pssi || !pssi->dest_file)
		return ;
	
	FILE *fp = fopen(pssi->dest_file, "a+");
	if(!fp)
	{
		fprintf(stderr, "cannot open file \"%s\"\n", pssi->dest_file);
		return ;
	}
	
	char szPrefix[100] = "\0";
	//fp = stdout;
	save_one_source(tobj, fp, szPrefix, pssi->current >=  pssi-> name_count ?NULL : pssi->names[pssi->current ++]);

	//printf("-- fp=%p\n",fp);

	fclose(fp);


}


int main(int argc, const char* argv[])
{
	SAVE_SOURCE_INFO ssi;

	if(argc < 2)
	{
		printf("Usage: %s <mStudio-Project-Path> [<output-file> [<dialog-name> ... ]]\n", argv[0]);
		return 1;
	}

	LoadTranslators("/usr/local/etc/guibuilder/tools/ncs2ics/trans.list");

	PProcessTransedObj process;
	void *param;

	if(argc == 2){
		process = (PProcessTransedObj)DumpTransedObj;
		param = (void*)stdout;
	}
	else
	{
		//process = (PProcessTransedObj) save_gui_config;
		process = (PProcessTransedObj) save_gui_source;
		ssi.dest_file = argv[2];
		ssi.names = argc >= 4 ? &argv[3] : NULL;
		ssi.name_count = argc - 3;
		ssi.current = 0;
		param = &ssi;
	}
	
	TranslateProject(argv[1], process, param);
	return 0;
}


