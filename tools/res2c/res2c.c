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

#include "resource.h"

#include <string>
#include <set>

#ifndef WIN32
#include <getopt.h>
#endif

#define OUTDIR outdir

#ifdef WIN32

#include <stdarg.h>
static inline void _PRINTF(const char* format, ...)
{
#ifdef _DEBUG
	va_list va;
	va_start(va, format);
	vprintf(format, va);
#endif
}

#else
#ifdef _DEBUG
#define _PRINTF(format...)  fprintf(stderr, format)
#else
#define _PRINTF(format...)
#endif
#endif

#ifdef WIN32
static char *basename(char* path){
	const char* str;
	if(path == NULL)
		return NULL;

	str = strrchr(path, '\\');
	if(!str)
		str = strrchr(path, '/');
	if(!str)
		return path;
	return (char*)str + 1;

}
#else
#include <libgen.h>
#endif


/*=================================================================*/
static char project[1024] = {};
static char resname[1024] = {};
static char fullres[1024] = {};
static char projname[256] = {};
static char outdir[1024] = "incore-res";

static void set_proj_name(char* project)
{
    if (strcmp(project, ".") == 0 || strcmp(project, "./") == 0) {
        strcpy (projname, basename(getenv("PWD")));
        return;
    } 

    char *p = strrchr(project, '/');
	if(!p)
		p = strrchr(project, '\\');
    if (p) {
        int len = strlen(project) - 1;

        if (p == (project + len)) {
            project[len] = '\0';
            set_proj_name(project);
        }
        else {
            strcpy(projname, p+1);
        }
    }
    else
        strcpy(projname, project);
}

static void set_package_file(char* project, char* resname)
{
    if (!project || !resname || strcmp(project, "") == 0)
        return;

    if (strcmp(resname, "") == 0) {
        sprintf (fullres, "%s/res/%s.res",  project, projname);
    }
    else 
        sprintf (fullres, "%s/res/%s", project, resname);

    _PRINTF("----- fullres:[%s] \n", fullres);
}

#ifndef WIN32
struct option longopts[] = {
    { "project", required_argument, NULL, 'p'},
    { "resname", required_argument, NULL, 'r'},
    { "outdir", optional_argument, NULL, 'd'},
    { 0, 0, 0, 0},
};
#endif

static int parseArgs(int argc, char* argv[])
{
#ifndef WIN32
    int c;
    while ((c = getopt_long(argc, argv, "p:r:d:", longopts, NULL)) != -1) 
    {
        switch (c) {
            case 'p':
                if (optarg) 
                    strcpy(project, optarg);
                break;

            case 'r':
                if (optarg) 
                    strcpy(resname, optarg);
                break;
            case 'd':
                if (optarg)
                    strcpy(outdir, optarg);
                break;
        }
    }
#else
	int i;
#define CHECKARG(long_arg, short_arg) \
		((argv[i][0] == '-' && strcmp(&argv[i][2], long_arg) == 0)  \
			|| strcmp(&argv[i][1], short_arg)==0)

	for(i = 0; i < argc; i ++)
	{
		if(argv[i][0] == '-') {
			if(CHECKARG("project", "p")){
				strcpy(project, argv[++i]);
			}
			else if(CHECKARG("resname", "r")) {
				strcpy(resname, argv[++i]);
			}
			else if(CHECKARG("outdir", "d")) {
				strcpy(outdir, argv[++i]);
			}

		}
	}
#undef CHECKARG

#endif

    if (strcmp(project, "") == 0) {
        return 1;
    }

    _PRINTF("\n----- project:[%s], resname:[%s] \n", project, resname);
    //current path
    if (strncmp(project, ".", strlen(project)) == 0 
            || strncmp(project, "./", strlen(project)) == 0) {
        strcpy (project, getenv("PWD"));
    } 

    set_proj_name(project);
    set_package_file(project, resname);

    return 0;
}

static char* getProjName()
{
    return projname;
}

static char* getFullProjName()
{
    return project;
}

static char* getResName()
{
    return resname;
}

static char* getFullResName()
{
    return fullres;
}
/*=================================================================*/

using namespace std;
static set <string> images;

static const char* add_export(const char* filename)
{
	int idx = 0;
	int i;
	const char* begin; 
	const char* ext;
    static char strname[1024];

	begin = strrchr(filename, '/');
	if(begin == NULL) 
        begin = filename;
    else
        begin += 1;
	
	ext = strrchr(begin, '.');
	i = (ext?(ext-begin):strlen(begin))+10;
	
	strname[idx++] = '_';
	if(ext){
		for(i=1;ext[i]; i++)
			strname[idx++] = ext[i];
		strname[idx++] = '_';
	}

	for(i=0;(ext && &begin[i]<ext)||(!ext && begin[i]); i++)
	{
		if(isalnum(begin[i]))
			strname[idx++] = begin[i];
		else
			strname[idx++] = '_';
	}
	
	if(strname[idx-1] == '_')
		strcpy(strname+idx, "data");
	else
		strcpy(strname+idx, "_data");

	return strname;
}

#define BUFSIZE 16384            /* Increase buffer size by this amount */
typedef unsigned char Bytef;
typedef unsigned long uLongf;

static void bin2c(const char* in, const char* out, const char* varname)
{
    int     i, j;
    FILE    *outfile=NULL;       /* The output file*/
    char    outname[1024];
    Bytef   *source=NULL;       /* Buffer containing uncompressed data */
    Bytef   *dest=NULL;         /* Buffer containing compressed data */
    uLongf  sourceBufSize=0;   /* Buffer size */
    uLongf  sourceLen;         /* Length of uncompressed data */
    uLongf  destLen;           /* Length of compressed data */
    FILE    *infile=NULL;        /* The input file containing binary data */
    char    *tmpname;

    infile = fopen (in, "rb");
    if (infile == NULL) return;

    if (out) {
        outfile = fopen (out, "w");
    }
    else {
        sprintf (outname, "%s/%s/%s.c.data", getFullProjName(), OUTDIR, add_export(in));
        //sprintf (outname, "%s.c", in);
        outfile = fopen (outname, "w");
    }
    fprintf (stderr, "res2c=>%s to c[%s]\n", in, out?out:outname);

    if (outfile == NULL) {
        fprintf (stderr, "res2c=> can't open %s for writing\n", out?out:outname);
        return;
    }

    sourceLen = 0;
    while (!feof (infile)) {
        if (sourceLen + BUFSIZE > sourceBufSize) {
            sourceBufSize += BUFSIZE;
            source = (Bytef*)realloc (source, sourceBufSize);
            if (source == NULL) return;
        }
        sourceLen += fread (source+sourceLen, 1, BUFSIZE, infile);
        if (ferror (infile)) return;
    }
    fclose (infile);

	if(sourceLen <= 0)
	{
		fclose(outfile);
		return ;
	}

    destLen = sourceLen;
    dest = source;

    tmpname = (char*)strrchr(out?out:outname, '/');
    if (tmpname)
        tmpname += 1;

    fprintf (outfile, "/*\n"
            " * Filename: %s \n"
            " * This file is generated automatically by res2c, please don't edit.\n"
            " *\n"
            " *    Copyright (C) 2009 Feynman Software\n"
            " *    All rights reserved by Feynman Software.\n"
            " */\n\n#ifdef _MGNCS_INCORE_RES\n\n", tmpname? tmpname : (out?out:outname));

    /* Output dest buffer as C source code to outfile */
    fprintf (outfile, "static const unsigned char %s[] = {\n",
            varname ? varname : add_export(in));

    for (j=0; j<destLen-1; j++) {
        switch (j%8) {
            case 0:
                fprintf (outfile, "  0x%02x, ", ((unsigned) dest[j]) & 0xffu);
                break;
            case 7:
                fprintf (outfile, "0x%02x,\n", ((unsigned) dest[j]) & 0xffu);
                break;
            default:
                fprintf (outfile, "0x%02x, ", ((unsigned) dest[j]) & 0xffu);
                break;
        }
    }

    if ((destLen-1)%8 == 0) fprintf (outfile, "  0x%02x\n};\n\n", ((unsigned) dest[destLen-1]) & 0xffu);
    else fprintf (outfile, "0x%02x\n};\n\n", ((unsigned) dest[destLen-1]) & 0xffu);
    fprintf(outfile, "#endif /* _MGNCS_INCORE_RES */\n\n");
    free (source);
    fclose (outfile);
}

#define RES_KEY unsigned long
RES_KEY Str2Key(const char* str)
{
	int i,l;
	RES_KEY ret = 0;
	unsigned short *s;

	if(str == NULL)
		return (RES_KEY)-1;
	
	l = (strlen(str)+1) / 2;
	s = (unsigned short*) str;

	for(i=0; i<l; i++)
		ret ^= (s[i]<<(i&0x0f));

	return ret;
}

static int pack2c(void)
{
    FILE *resfile;
    char cname[1024];

    sprintf (cname, "%s/%s/mgncs_incore_package.c", getFullProjName(),OUTDIR);
    bin2c(getFullResName(), cname, "_mgncs_incore_package");

    resfile = fopen (cname, "a+");
    if (resfile == NULL) return 1;

    fprintf (resfile, 
            "#include <minigui/common.h>\n"
            "#include <minigui/minigui.h>\n"
            "#include <minigui/gdi.h>\n"
            "#include <minigui/window.h>\n"
            "#include <mgncs/mgncs.h>\n\n"
            "#ifdef _MGNCS_INCORE_RES\n"
            "#include \"_incore_images.c.data\"\n\n"
            "HPACKAGE ncsLoadIncoreResPackage(void)\n"
            "{\n"
            "    if(sizeof(_mgncs_incore_images) > 0 ) \n"
            "    { \n"
            "       if(AddInnerRes(_mgncs_incore_images, \n"
            "           sizeof(_mgncs_incore_images)/sizeof(INNER_RES), FALSE) != RES_RET_OK)\n"
            "       return HPACKAGE_NULL;\n"
            "    }\n"
            "    return ncsLoadResPackageFromMem((const void*)_mgncs_incore_package, sizeof(_mgncs_incore_package));\n"
            "}\n\n"
            "#endif\n");

    fclose (resfile);
}

static int image2c(set<string> filelist)
{
    set <string>::iterator it;
    FILE *resfile;
    char cname[1024];

    sprintf (cname, "%s/%s/_incore_images.c.data", getFullProjName(), OUTDIR);

    //an image file to a c file
    for (it  = filelist.begin(); it != filelist.end(); it++) {
        char name[1024];
        sprintf(name, "%s/%s", getFullProjName(), it->c_str());
        //in, out, varname
        bin2c(name, NULL, NULL);
    }

    //incore image data section
    resfile = fopen (cname, "w+");
    if (resfile == NULL) return 1;

    fprintf (resfile, "/*\n"
            " * Filename: %s \n"
            " * This file is generated automatically by res2c tools, please don't edit.\n"
            " *\n"
            " *    Copyright (C) 2009 Feynman Software\n"
            " *    All rights reserved by Feynman Software.\n"
            " */\n\n", strrchr(cname, '.')+1);


    fprintf (resfile, "#ifndef _INCORE_IMAGES_H\n"
            "#define _INCORE_IMAGES_H\n\n");

    fprintf (resfile, 
            "#include <minigui/common.h>\n"
            "#include <minigui/minigui.h>\n"
            "#include <minigui/gdi.h>\n"
            "#include <minigui/window.h>\n"
            "#include <mgncs/mgncs.h>\n\n");
    fprintf (resfile, "#ifdef _MGNCS_INCORE_RES\n");

    for (it  = filelist.begin(); it != filelist.end(); it++) {
        fprintf (resfile, "#include \"%s.c.data\"\n", add_export(strrchr(it->c_str(), '/') + 1));
    }

    fprintf (resfile, "\nstatic INNER_RES _mgncs_incore_images[] = {\n");

    for (it  = filelist.begin(); it != filelist.end(); it++) {

        const char *tmp = add_export(it->c_str());

        fprintf (resfile, "    {0x%0x, (void*)%s, sizeof(%s), \"%s\"},\n",
                (int)Str2Key(it->c_str()), tmp, tmp, strrchr(it->c_str(), '.') + 1);
    }
 
    fprintf (resfile, "};\n#endif\n");
    fprintf (resfile, "\n#endif\n");
    fclose (resfile);

    return 0;
}

extern char* get_res_sectaddr (HPACKAGE hPackage, int type);
static int get_image_list(void)
{
    long    size;
    int     i = 0, found = 0;
    NCSRM_HEADER     *head;
    NCSRM_TYPEITEM   *pType;
    NCSRM_SECTHEADER *sect_head;
    NCSRM_IDITEM     *item; 

    HPACKAGE hPackage = ncsLoadResPackage(getFullResName());

    if (hPackage == HPACKAGE_NULL) {
        fprintf (stderr, "res2c=> load resource package %s failure.\n", getFullResName());
        return 1;
    }

    //insert list
    sect_head = (NCSRM_SECTHEADER*) get_res_sectaddr(hPackage, NCSRT_IMAGE);
    item = (NCSRM_IDITEM*)((char*)sect_head + sizeof(NCSRM_SECTHEADER));

    for (i = 0; i < sect_head->size_maps; i++, item++) {
        const char* filename = ncsGetString(hPackage, item->filename_id);
        if (filename)
            images.insert(filename);
    }

    ncsUnloadResPackage(hPackage);
    return 0;
}

#ifdef WIN32
#include <dirent.h>
#define MKDIR(szPath,len, dir_name) do{ \
	sprintf(szPath+len, "/%s", dir_name); \
	mkdir(szPath,0644); \
}while(0)
#else 
#include <sys/types.h>
#include <sys/stat.h>
#define MKDIR(szPath,len, dir_name) do{ \
	sprintf(szPath+len, "/%s", dir_name); \
	mkdir(szPath,S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IXUSR); \
}while(0)
#endif 

static void check_out_dir()
{
    char szPath[1024];
    strcpy(szPath, getFullProjName());
    MKDIR(szPath, strlen(szPath), OUTDIR);
}


int res2c(int argc, char *argv[])
{
    char *p;
    char suffix[]=".res";

    if (parseArgs(argc, argv)) {
        fprintf (stderr, "\nUsage: ./res2c --project test --resname test.res\n"
                "The options: \n"
                "   --project(-p) Required.\n"
                "   --resname(-r) Optional.\n"
                "   --outdir(-d) Optional.\n");
        return 1;
    }

    //check suffix
    p = strstr(getFullResName(), suffix);

    if (!p  || *(p + strlen(suffix))) {
        fprintf (stderr, "res2c=> %s have error suffix.\n", getFullResName());
        return 1;
    }

    check_out_dir();

    if (get_image_list()) {
        fprintf (stderr, "res2c=> %s get image information failure.\n", getFullResName());
        return 1;
    }

    if (image2c(images)) {
        fprintf (stderr, "res2c=> %s convert image to incore resouce failure.\n", getFullResName());
        return 1;
    }

    if (pack2c()) {
        fprintf (stderr, "res2c=> %s convert resource package failure.\n", getFullResName());
        return 1;
    }

    fprintf (stderr, "res2c=> success.\n\n");
    return 0;
}

int main(int argc, char *argv[])
{
    return res2c(argc, argv);
}
