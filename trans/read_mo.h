/* Reading binary .mo files.
   Copyright (C) 1995-1998, 2000-2003 Free Software Foundation, Inc.
   Written by Ulrich Drepper <drepper@gnu.ai.mit.edu>, April 1995.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#ifndef _READ_MO_H
#define _READ_MO_H

#include <errno.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned int uint32_t;
/* This include file describes the main part of binary .mo format.  */
/* Header for binary .mo file format.  */
struct mo_file_header
{
  /* The magic number.  */
  uint32_t magic;
  /* The revision number of the file format.  */
  uint32_t revision;

  /* The number of strings pairs.  */
  uint32_t nstrings;
  /* Offset of table with start offsets of original strings.  */
  uint32_t orig_tab_offset;
  /* Offset of table with start offsets of translated strings.  */
  uint32_t trans_tab_offset;
  /* Size of hash table.  */
  uint32_t hash_tab_size;
  /* Offset of first hash table entry.  */
  uint32_t hash_tab_offset;
};

/* Some compilers, like SunOS4 cc, don't have offsetof in <stddef.h>.  */
#ifndef offsetof
# define offsetof(type,ident) ((size_t)&(((type*)0)->ident))
#endif

/* Separator between msgctxt and msgid in .mo files.  */
#define MSGCTXT_SEPARATOR '\004'  /* EOT */

/* We read the file completely into memory.  This is more efficient than
   lots of lseek().  This struct represents the .mo file in memory.  */
struct binary_mo_file
{
	const char *filename;
	char *data;
	size_t size;
};

typedef struct _message_ty message_ty;
struct _message_ty
{
	/* The msgctxt string, if present.  */
	const char *msgctxt;

	/* The msgid string.  */
	const char *msgid;

	/* The msgid string.  */
	size_t msgid_len;

	/* The msgstr strings.  */
	const char *msgstr;

	/* The number of bytes in msgstr, including the terminating NUL.  */
	size_t msgstr_len;
};

typedef struct _message_list_ty message_list_ty;
struct _message_list_ty
{
	message_ty *item;
	//size_t hashtable_offset;
	uint32_t *hash_tab;
	size_t hash_size;
	size_t nitems;
	struct binary_mo_file bf;
};

/* Reads an existing .mo file*/
void read_mo_file (message_list_ty *m_list, const char *filename);

const char* msd_gettext(const char *msgid);

int msd_locale_init(const char *mylocale, const char *lang_path);

void free_mo_info();

#endif /* _READ_MO_H */
