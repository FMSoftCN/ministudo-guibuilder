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
#if defined (WIN32)
#include "mmap-win.h"
#else
#include <sys/mman.h>
#endif

#ifdef WIN32
#define _MGRM_PRINTF
#else
#define _MGRM_PRINTF(fmt...)
#endif

#define Package2TypeMaps(header) \
    (NCSRM_TYPEITEM*)((char*)header + sizeof (NCSRM_HEADER))

typedef struct _MAPFILELIST MAPFILELIST;
typedef MAPFILELIST* PMAPFILELIST;
struct _MAPFILELIST
{
    void         *data;
    long         data_size;
    Uint32       filename_id;
    PMAPFILELIST prev;
    PMAPFILELIST next;
};

typedef struct _RPINFO
{
    PMAPFILELIST head;
	int          ref;
    void         *data;
    long         data_size;
}RPINFO;

typedef RPINFO* PRPINFO;

static int append_list(HPACKAGE package, void* data,
        Uint32 filename_id, long data_size)
{
    PMAPFILELIST head = ((PRPINFO)package)->head;
    PMAPFILELIST last, list;

    list = (PMAPFILELIST)calloc(1, sizeof(MAPFILELIST));

    if (!list) return -1;

    list->data = data;
    list->filename_id = filename_id;
    list->data_size = data_size;
    list->prev = NULL;
    list->next = NULL;

    if (head == NULL) {
        ((PRPINFO)package)->head = list;
        return 0;
    }

    last = head;

    while (last->next) {
        last = last->next;
    }

    last->next = list;
    list->prev = last;

    return 0;
}

static PMAPFILELIST find_list(HPACKAGE package, Uint32 filename_id)
{
    PMAPFILELIST head = ((PRPINFO)package)->head;
    PMAPFILELIST first;

    if (!head) {
        return NULL;
    }

    first = head;

    while(first->next) {
        if (first->filename_id == filename_id) {
            return first;
        }
        first = first->next;
    }

    return NULL;
}

static void del_alllist(HPACKAGE package)
{
    PMAPFILELIST head, list;

    head = ((PRPINFO)package)->head;

    if (!head)
        return;

    ((PRPINFO)package)->head = NULL;

    while(head) {
#ifdef WIN32
		win_munmap(head->data);
#else
#if HAVE_MMAP
        munmap (head->data, head->data_size);
#else
        free (head->data);
#endif
#endif
        list = head;
        head = head->next;

        free(list);
    }
    return;
}

static int del_list(HPACKAGE package, Uint32 filename_id)
{
    PMAPFILELIST head = ((PRPINFO)package)->head;
    PMAPFILELIST first, list;
    BOOL found = FALSE;

    if (!head) {
        return -1;
    }

    first = head;

    if (first->filename_id == filename_id) {
        ((PRPINFO)package)->head = first->next;
        if (first->next)
            first->next->prev = NULL;

        list = first;
        found = TRUE;
    }
    else {
        while(first->next) {
            if (first->next->filename_id == filename_id) {
                list = first->next;
                if(list->next){
                    first->next = list->next;
                    first->next->prev = list->prev;
                } else { // the found at the tail of the list.
                    first->next = NULL;
                }
                found = TRUE;
                break;
            }
            first = first->next;
        }
    }

    if (found == TRUE) {
#ifdef WIN32
		win_munmap(list->data);
#else
#if HAVE_MMAP
        munmap (list->data, list->data_size);
#else
        free (list->data);
#endif
#endif
        free(list);
        return 0;
    }

    return -1;
}

static NCSRM_TYPEITEM* search_type_item (NCSRM_HEADER *header, Uint16 type)
{
    NCSRM_TYPEITEM   *type_item;
    int i = 0;

    type_item = Package2TypeMaps(header);

    while (i < header->nr_sect) {
        if (type & type_item->type)
            return type_item;

        type_item ++;
    }

    return NULL;
}

static NCSRM_IDITEM*
binary_search_iditem (Uint32 *sect_base, Uint32 res_id)
{
    NCSRM_IDITEM     *id_item, *base_iditem;
    int             low, high, mid;
    NCSRM_SECTHEADER *sect_header = (NCSRM_SECTHEADER *)sect_base;

    if (!sect_base)
        return NULL;

    base_iditem =
        (NCSRM_IDITEM *)((char*)sect_base + sizeof(NCSRM_SECTHEADER));

    low = 0;
    high = sect_header->size_maps;

    while (low < high) {
        mid = (low + high)/2;
        id_item = base_iditem + mid;

        if (id_item->id < res_id)
            low = mid + 1;
        else
            high = mid;
    }

    if ((low < sect_header->size_maps)
            && ((base_iditem + low)->id == res_id))
        return base_iditem + low;
    else
        return 0;
}

static char* mmap_file (const char *file_name, long *file_size)
{
#ifdef WIN32
	return (char*)win_mmap(file_name);
#else
    FILE* fp = NULL;
    void* data;

    _MGRM_PRINTF ("mmap file:[%s] \n", file_name);
    if (!file_name)
        return NULL;

    if ((fp = fopen (file_name, "rb")) == NULL) {
        _MGRM_PRINTF ("RESMANAGER>mmap: can't open file: %s\n", file_name);
        return NULL;
    }

    /* get file size*/
    fseek (fp, 0, SEEK_END);
    *file_size = ftell (fp);
#if HAVE_MMAP
    data = mmap (0, *file_size, PROT_READ, MAP_PRIVATE, fileno(fp), 0);
#else
    data = (char*)malloc(*file_size);
    fseek (fp, 0, SEEK_SET);
    fread(data, 1, *file_size, fp);
#endif
    fclose (fp);

    if (!data || (data == MAP_FAILED)) {
#if HAVE_MMAP
        _MGRM_PRINTF ("RESMANAGER>mmap: mmap file[%s] error.\n", file_name);
#else
        _MGRM_PRINTF ("RESMANAGER>mmap: read file[%s] error.\n", file_name);
#endif
        return 0;
    }

    return (char*)data;
#endif
}

static char* find_mmap_file(HPACKAGE package, Uint32 filename_id)
{
    PMAPFILELIST list;
    long         size;
    char         *data;

    if (filename_id <= 0)
        return NULL;

    list = find_list(package, filename_id);

    if (list) {
        //reference count++
        return (char*)(list->data);
    }

    data = mmap_file (ncsGetString(package, filename_id), &size);

    if (data == NULL) {
        return NULL;
    }

    append_list(package, data, filename_id, size);

    return data;
}

char* get_res_sectaddr (HPACKAGE hPackage, int type)
{
    NCSRM_HEADER     *res_head;
    NCSRM_TYPEITEM   *type_item;

	if(!hPackage)
		return NULL;

	res_head = (NCSRM_HEADER*)((PRPINFO)hPackage)->data;

    type_item = search_type_item (res_head, type);

    if (!type_item) {
        return 0;
    }

    if (type_item->offset == 0) {
        return find_mmap_file (hPackage, type_item->filename_id);
    }
    else
        return (char*)res_head + type_item->offset;
}

static char en_locale[8] = "en_US";
static char gLocaleValue[8] = "en_US";

const char* ncsGetDefaultLocale(void)
{
    return gLocaleValue;
}

const char* ncsSetDefaultLocale (char* language, char* country)
{
    sprintf (gLocaleValue, "%s_%s", language, country);
    return gLocaleValue;
}

static char* _get_locale_base_addr (HPACKAGE handle,
        NCSRM_SECTHEADER* sect,
        NCSRM_LOCALEITEM* locale, const char* def_locale)
{
    char cur_locale[20];
    int i = 0;
    char *sect_addr = NULL;

    while (i < sect->size_maps) {
        sprintf (cur_locale, "%s_%s", locale->language, locale->country);

        if (strcmp(def_locale, cur_locale) == 0) {
            if (locale->offset == 0) {
                sect_addr = find_mmap_file (handle, locale->filename_id);
                if (sect_addr == NULL)
                    return NULL;
            }
            else
                sect_addr = (char*)sect + locale->offset;
            break;
        }
        locale++;
        i++;
    }
    return sect_addr;
}

static NCSRM_IDITEM* getIDItem (HPACKAGE handle, Uint32 res_id, char **sect_addr)
{
    NCSRM_SECTHEADER* sect;
    NCSRM_LOCALEITEM* locale;

    *sect_addr = get_res_sectaddr(handle, res_id>>16);
    if (*sect_addr == NULL)
        return NULL;

    //find locale file according to locale map info
    if (res_id>>16 == NCSRT_TEXT) {
        sect = (NCSRM_SECTHEADER*)(*sect_addr);
        locale = (NCSRM_LOCALEITEM*)((char*)(*sect_addr) + sizeof(NCSRM_SECTHEADER) + sizeof(NCSRM_DEFLOCALE_INFO));

        *sect_addr = _get_locale_base_addr(handle, sect, locale, ncsGetDefaultLocale());
        if (!*sect_addr) {
            *sect_addr = _get_locale_base_addr(handle, sect, locale, en_locale);
        }
    }

    return binary_search_iditem ((Uint32*)*sect_addr, res_id);
}

static void _set_locale_info(HPACKAGE hPackage)
{
    char* sect_addr = get_res_sectaddr(hPackage, NCSRT_TEXT);

    if (sect_addr) {
        NCSRM_DEFLOCALE_INFO* locale =
            (NCSRM_DEFLOCALE_INFO*)(sect_addr + sizeof(NCSRM_SECTHEADER));

        if (locale->language[0] && locale->country[0])
            ncsSetDefaultLocale(locale->language, locale->country);
    }
}

HPACKAGE ncsLoadResPackageFromFile (const char* fileName)
{
    void        *data;
    RPINFO      *package;
    NCSRM_HEADER *res_head;
    long        file_size;

    data = mmap_file (fileName, &file_size);

    if (data == NULL)
        return HPACKAGE_NULL;

    package =(RPINFO*)calloc (1, sizeof(RPINFO));
    if (!package)
        return HPACKAGE_NULL;

    if (append_list((HPACKAGE)package, data, -1, file_size)) {
        free (package);
        return HPACKAGE_NULL;
    }

    package->data = data;
    package->data_size = file_size;

    res_head = (NCSRM_HEADER *)data;

    //mgrp
    if (res_head->magic !=0x4d475250) {

        _MGRM_PRINTF ("RESMANAGER>header info:No MiniGUI res package:0x%0x.\n",
                res_head->magic);

        ncsUnloadResPackage((HPACKAGE)package);
        return HPACKAGE_NULL;
    }

    _set_locale_info((HPACKAGE)package);
    //set ncs system renderer for default renderer
    //ncsSetSystemRenderer(ncsGetString((HPACKAGE)package, NCSRM_SYSSTR_DEFRDR));

	package->ref = 1;
    return (HPACKAGE)package;
}

void ncsUnloadResPackage (HPACKAGE package)
{
	PRPINFO prpinfo;
    if (package == HPACKAGE_NULL)
        return;

	prpinfo = (PRPINFO)package;
	if(!prpinfo || --prpinfo->ref > 0)
		return ;

    del_alllist(package);
    free (prpinfo);
}

const char* ncsGetString (HPACKAGE package, Uint32 resId)
{
    char        *string;
    NCSRM_IDITEM *item;

    if (package == HPACKAGE_NULL && resId <= 0)
        return NULL;

    if ((resId >>16) != NCSRT_STRING && (resId >>16) != NCSRT_TEXT) {
        return NULL;
    }

    item = getIDItem (package, resId, &string);

    if (item == 0) {
        _MGRM_PRINTF ("RESMANAGER>Error: Not found string resource"
                      "by id:0x%0x.\n", resId);
        return NULL;
    }

    return string + item->offset;
}

