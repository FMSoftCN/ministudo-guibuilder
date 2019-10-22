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

#ifndef _TRANSLATER_H
#define _TRANSLATER_H


class Translater
{
protected :
	int progress;
	char sr_lang[6];
	char ta_lang[6];
	string srcString;

public :
	string result;
	Translater();
	virtual ~Translater();

	void setSourceLangue(const char *lang, const char *country)
	{
		if (lang && checkLang(lang))
			strncpy(sr_lang, lang, 2);
		sr_lang[2]= '\0';

		//FIXME, only the Chinese need to differentiate zh-CN from zh-TW
		if (strcmp(ta_lang, "zh")!= 0)
			return;

		strcat(sr_lang, "-");

		if (country && checkCountry(country))
			strncat(sr_lang, country, 2);

		sr_lang[5] = '\0';
	}

	void setTargetLangue(const char *lang, const char *country)
	{
		if (lang && checkLang(lang))
			strncpy(ta_lang, lang, 2);
		ta_lang[2]= '\0';

		//FIXME, only the Chinese need to differentiate zh-CN from zh-TW
		if (strcmp(ta_lang, "zh")!= 0)
			return;

		strcat(ta_lang, "-");

		if (country && checkCountry(country))
			strncat(ta_lang, country, 2);

		ta_lang[5] = '\0';
	}

	const char * getSourceLangue(void){ return sr_lang;}
	const char * getTargetLangue(void){ return ta_lang;}

	void setSourceString (const char *string) { srcString = string;}
	const char * getSourceString (void) { return srcString.c_str();}

	virtual const char *getResult() = 0;

	virtual int translate () = 0;
	int getProgress () { return progress; }
	void reset (void) { progress = 0; }

private:
	int checkLang(const char *lang)
	{
		//TOD0
		return 1;
	}
	int checkCountry(const char *country)
	{
		//TOD0
		return 1;
	}

};

#endif
