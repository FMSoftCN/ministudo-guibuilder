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

#ifndef _G_TRANSLATER_H
#define _G_TRANSLATER_H



class GTranslater : public Translater
{
private :
	string gURL;
	pthread_t pt;
	pthread_mutex_t mutex;
	static GTranslater *instance;
	string curl_data;

	GTranslater ();
	~GTranslater ();

public :
	static GTranslater *getInstance(void);

	const char *getResult();

	int translate ();
	int getProgress ();

	void setupURL(void);

	static size_t curl_save (void *ptr, size_t size, size_t nmemb, void *stream);

	int curl_deal (const char *url);
	int json_parse (const char *body);
};


#endif
