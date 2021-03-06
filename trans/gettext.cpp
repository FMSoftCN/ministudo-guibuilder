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
#include <ctype.h>

#include "mgheads.h"

#include "msd_intl.h"
#include "read_mo.h"

#ifdef _MSTUDIO_LOCALE
#define LANG_SECT	"LANGUAGE"
#define LANG_KEY	"lang"
#define LANGPATH   "LANGPATH"
#define PATHKEY	"path"
/* We assume to have `unsigned long int' value with at least 32 bits.  */
#define HASHWORDBITS 32

/* Defines the so called `hashpjw' function by P.J. Weinberger
   [see Aho/Sethi/Ullman, COMPILERS: Principles, Techniques and Tools,
   1986, 1987 Bell Telephone Laboratories, Inc.]  */
static  unsigned long int
hash_string (const char *str_param)
{
	unsigned long int hval, g;
	const char *str = str_param;

	/* Compute the hash value for the given string.  */
	hval = 0;
	while (*str != '\0')
	{
		hval <<= 4;
		hval += (unsigned char) *str++;
		g = hval & ((unsigned long int) 0xf << (HASHWORDBITS - 4));
		if (g != 0)
		{
			hval ^= g >> (HASHWORDBITS - 8);
			hval ^= g;
		}
	}
	return hval;
}
/* -------------------------------------------------------------------------- */

static message_list_ty m_list;

#define msd_locale_nr   (sizeof(msd_locales)/sizeof(msd_locales[0]))

static const char*
get_string(uint32_t idx )
{
    return (const char*) m_list.item[idx].msgstr;
}

const char* msd_gettext(const char *msgid)
{
    size_t act = 0;
    size_t top, bottom;

    if(m_list.hash_tab == NULL || !msgid || *msgid == '\0')
        goto not_found;

    /* Locate the MSGID and its translation.  */
    if( m_list.hash_size > 2 && m_list.hash_tab ) {
        /* Use the hashing table.  */
        uint32_t hash_val = hash_string (msgid);
        uint32_t idx = hash_val % m_list.hash_size;
        uint32_t incr = 1 + (hash_val % (m_list.hash_size - 2));
        uint32_t nstr = m_list.hash_tab[idx];

        if ( !nstr ) /* Hash table entry is empty.  */
            goto not_found;

        if (!strcmp( msgid, m_list.item[nstr - 1].msgid) )
            return get_string( nstr - 1 );

        for(;;) {
            if (idx >= m_list.hash_size - incr)
                idx -= m_list.hash_size - incr;
            else
                idx += incr;

            nstr = m_list.hash_tab[idx];
            if( !nstr )
                goto not_found; /* Hash table entry is empty.  */

            if ( !strcmp (msgid, m_list.item[nstr - 1].msgid))
                return get_string(nstr-1 );
        }
        /* NOTREACHED */
    }

    /* Now we try the default method:  binary search in the sorted
     *        array of messages.  */
    bottom = 0;
    top = m_list.nitems;
    while( bottom < top ) {
        int cmp_val;

        act = (bottom + top) / 2;
        cmp_val = strcmp(msgid, m_list.item[act].msgid);
        if (cmp_val < 0)
            top = act;
        else if (cmp_val > 0)
            bottom = act + 1;
        else
            return get_string(act);
    }

not_found:
    return msgid;
}

int
msd_locale_init(const char* str_cfg)
{
	if(!str_cfg)
		return 0;

	if(m_list.item != NULL)
		return 1;

    char szLocale[16] = {0};
    char szLangPath[MAX_PATH] = {0};

    GetValueFromEtcFile(str_cfg, LANG_SECT, LANG_KEY, szLocale, sizeof(szLocale)-1);

    GetValueFromEtcFile(str_cfg, LANGPATH, PATHKEY, szLangPath, sizeof(szLangPath)-1);

	if (szLocale[0] == '\0' || szLangPath[0] == '\0')
		return 1;

	if (strcmp(szLocale, "en") == 0 || (strlen(szLocale) > 20))
	{
		return 1;
	}

	sprintf(szLangPath + strlen(szLangPath), "/lang/%s.mo", szLocale);
	read_mo_file(&m_list, szLangPath);

	return 0;
}

void free_mo_info()
{
    free(m_list.hash_tab);
    m_list.hash_tab = NULL;

    free(m_list.item);
    m_list.item = NULL;

    free(m_list.bf.data);
    m_list.bf.data = NULL;
}
#endif
