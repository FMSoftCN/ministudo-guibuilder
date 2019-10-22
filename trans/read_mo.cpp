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

/* Specification.  */
#include "msd_intl.h"
#include "read_mo.h"

#ifdef _MSTUDIO_LOCALE
/* Read the contents of the given input stream.  */
static void
read_binary_mo_file (struct binary_mo_file *bfp,
		FILE *fp, const char *filename)
{
	char *buf = NULL;
	size_t alloc = 0;
	size_t size = 0;
	size_t count;

	while (!feof (fp))
	{
		const size_t increment = 4096;
		if (size + increment > alloc)
		{
			alloc = alloc + alloc / 2;
			if (alloc < size + increment)
				alloc = size + increment;
			buf = (char *) realloc (buf, alloc);
		}
		count = fread (buf + size, 1, increment, fp);
		if (count == 0)
		{
			if (ferror (fp))
				fprintf(stderr, "fp error\n");
		}
		else
			size += count;
	}
	buf = (char *) realloc (buf, size);
	bfp->filename = filename;
	bfp->data = buf;
	bfp->size = size;
}

/* Get a 32-bit number from the file, at the given file position.  */
	static uint32_t
get_uint32 (const struct binary_mo_file *bfp, size_t offset)
{
	uint32_t b0, b1, b2, b3;

	if (offset + 4 > bfp->size)
		fprintf(stderr, "get_uint32 size exceeds\n");

	b0 = *(unsigned char *) (bfp->data + offset + 0);
	b1 = *(unsigned char *) (bfp->data + offset + 1);
	b2 = *(unsigned char *) (bfp->data + offset + 2);
	b3 = *(unsigned char *) (bfp->data + offset + 3);
	return b0 | (b1 << 8) | (b2 << 16) | (b3 << 24);

}

/* Get a static string from the file, at the given file position.  */
	static char *
get_string (const struct binary_mo_file *bfp, size_t offset, size_t *lengthp)
{
	/* See 'struct string_desc'.  */
	uint32_t s_length = get_uint32 (bfp, offset);
	uint32_t s_offset = get_uint32 (bfp, offset + 4);

	if (s_offset + s_length + 1 > bfp->size)
		fprintf(stderr, "get_string size exceeds\n");

	if (bfp->data[s_offset + s_length] != '\0')
		fprintf(stderr, "get_string size exceeds\n");

	*lengthp = s_length + 1;
	return bfp->data + s_offset;
}

/* Reads an existing .mo file */
	void
read_mo_file (message_list_ty *m_list, const char *filename)
{
	FILE *fp;
	struct mo_file_header header;
	unsigned int i;

	fp = fopen (filename, "rb");
	if (fp == NULL)
	{
		fprintf(stderr, "err: open mo file\n");
		return;
	}
	/* Read the file contents into memory.  */
	read_binary_mo_file (&(m_list->bf), fp, filename);

	/* Get a 32-bit number from the file header.  */
# define GET_HEADER_FIELD(field) \
	get_uint32 (&(m_list->bf), offsetof (struct mo_file_header, field))

	header.nstrings = GET_HEADER_FIELD (nstrings);
	header.orig_tab_offset = GET_HEADER_FIELD (orig_tab_offset);
	header.trans_tab_offset = GET_HEADER_FIELD (trans_tab_offset);
	header.hash_tab_size = GET_HEADER_FIELD (hash_tab_size);
	header.hash_tab_offset = GET_HEADER_FIELD (hash_tab_offset);

	m_list->hash_size = header.hash_tab_size;
	m_list->hash_tab = (uint32_t *)malloc(m_list->hash_size * sizeof(uint32_t));
	m_list->nitems = header.nstrings;
	m_list->item = (message_ty *)malloc(m_list->nitems*(sizeof(message_ty)));

	for (i = 0; i < m_list->hash_size; i++)
	{
		 m_list->hash_tab[i] = get_uint32(&(m_list->bf), header.hash_tab_offset + i*4);
	}

	for (i = 0; i < header.nstrings; i++)
	{
		char *msgctxt;
		char *msgid;
		size_t msgid_len;
		char *separator;
		char *msgstr;
		size_t msgstr_len;

		/* Read the msgid.  */
		msgid = get_string (&(m_list->bf), header.orig_tab_offset + i * 8,
				&msgid_len);

		separator = strchr (msgid, MSGCTXT_SEPARATOR);
		if (separator != NULL)
		{
			*separator = '\0';
			msgctxt = msgid;
			msgid = separator + 1;
			msgid_len -= msgid - msgctxt;
		}
		else
			msgctxt = NULL;

		/* Read the msgstr.  */
		msgstr = get_string (&(m_list->bf), header.trans_tab_offset + i * 8,
				&msgstr_len);

		m_list->item[i].msgid = msgid;
		m_list->item[i].msgid_len = msgid_len;
		m_list->item[i].msgstr = msgstr;
		m_list->item[i].msgctxt = msgctxt;
		m_list->item[i].msgstr_len = msgstr_len;
	}

	fclose (fp);
}
/*
int
main()
{
	int i;

	struct message_list_ty m_list;
	read_mo_file(&m_list, "test.mo");

	for (i = 0; i < m_list.nitems; i++)
	{
		fprintf(stderr, "str[%d] is %s\n", i, m_list.item[i].msgid);
		fprintf(stderr, "str[%d] is %s\n", i, m_list.item[i].msgstr);
	}

	return 0;
}
*/
#endif
