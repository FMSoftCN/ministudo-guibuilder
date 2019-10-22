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

#include <string>
using namespace std;

#include <pthread.h>

#include <curl/curl.h>

#include "json/json.h"

//#include "mgheads.h"
//#include "mgfcheads.h"

#include "translater.h"
#include "GTranslater.h"



GTranslater *GTranslater::instance;

GTranslater::GTranslater ()
{

}

GTranslater::~GTranslater ()
{
    delete instance;
}

GTranslater *GTranslater ::getInstance(void)
{
	if (instance == NULL)
		instance = new GTranslater();
	instance->progress = 0;
	return instance;
}

void GTranslater::setupURL(void)
{
	char hexchars[] = "0123456789ABCDEF";
	char *start, *end;
	unsigned char c;
	char *from, *to;
	from = (char *)srcString.c_str();
	end = from + srcString.length();

	start = to = (char *) malloc(3 * srcString.length() + 1);

	while (from < end) {
		  c = *from++;

		  if (c == ' ') {
				  *to++ = '+';
		  } else if ((c < '0' && c != '-' && c != '.') ||
		  (c < 'A' && c > '9') ||
		  (c > 'Z' && c < 'a' && c != '_') ||
		  (c > 'z')) {
				  to[0] = '%';
				  to[1] = hexchars[c >> 4];
				  to[2] = hexchars[c & 15];
				  to += 3;
		  } else {
				  *to++ = c;
		  }
	}
	*to = 0;

	gURL = "http://ajax.googleapis.com/ajax/services/language/translate?v=1.0&q=";
	gURL += start;
	gURL += "&langpair=";
	gURL.append (sr_lang);
	gURL += "|";
	gURL.append (ta_lang);

	free(start);
}


const char * GTranslater::getResult()
{
	return result.c_str();
}

int GTranslater::translate ()
{
	setupURL();
	progress = 20;

	curl_deal(gURL.c_str());
	progress = 60;

	json_parse (curl_data.c_str());
	progress = 100;
	return 0;
}

int GTranslater::getProgress ()
{
	int prog = 0;
	prog = progress;
	return prog;
}

size_t GTranslater::curl_save (void *ptr, size_t size, size_t nmemb, void *_data)
{
	//printf("curl get data ---- %s \n", (char *)ptr);
	GTranslater *ts = (GTranslater *)_data;
	ts->curl_data = (char *)ptr;

	return size;
}

int GTranslater::curl_deal (const char *url)
{
	CURL *h_curl;

	if (curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK) {
		fprintf(stderr, "curl_global_init() failed\n");
		return -1;
	}

	if ((h_curl = curl_easy_init()) == NULL) {
		fprintf(stderr, "curl_easy_init() failed\n");
		curl_global_cleanup();
		return -1;
	}

	curl_easy_setopt (h_curl, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt (h_curl, CURLOPT_URL, url );
	curl_easy_setopt (h_curl, CURLOPT_WRITEFUNCTION, curl_save);
	curl_easy_setopt (h_curl, CURLOPT_WRITEDATA, this);

	curl_easy_perform (h_curl);

	curl_easy_cleanup (h_curl);
	curl_global_cleanup ();

	return 0;
}

#define MAP_ENTITY(_entityName, _entity) \
    if(strncmp(_entityName, entityName, sizeof(_entityName)-1)  == 0) {\
        *pref_size = sizeof(_entityName) -1; \
        entity[0] = _entity; \
        return 1; }
static int entity_from_name(char* entity, const char* entityName, int *pref_size)
{
    if(*entityName == '#')
    {
        const char* strend = strchr(entityName, ';');
        if(strend)
            *pref_size = strend - entityName ;
        else
            *pref_size = strlen(entityName);

        int value = 0;
        if(entityName[1] == 'x') {
            value = strtol(entityName + 2, NULL, 16);
        }
        else {
            value = strtol(entityName + 1, NULL, 10);
        }

        char text[10];
        int n = 0;
        while(value != 0) {
            text[n++] = (value & 0xFF);
            value >>= 8;
        }
        for(int i = n - 1; i >= 0; i--)
            entity[i] = text[n-i-1];
        
        return n;

    }
    else 
    {
        MAP_ENTITY("quot", '\"')
        MAP_ENTITY("amp", '&')
        MAP_ENTITY("apos", '\'')
        MAP_ENTITY("lt", '<')
        MAP_ENTITY("gt", '>')
    }
    return 0;
}


static void set_translate_result(string & dst, const char* src)
{
    dst="";
    if(!src)
        return;

    char szText[1024];
    while(*src) {
        int i = 0;
        while(*src && i < sizeof(szText)-1) {
            if(*src == '&'){
                int size = 0;
                int c = entity_from_name(szText+i, src + 1, &size);
                if(c == 0) {
                    szText[i++] = *src ;
                }
                else {
                    src += (size+1) ;
                    i += c;
                }
            }
            else
            {
                szText[i++] = *src;
            }
            src ++;
        }
        szText[i] = 0;
        dst += szText;
    }
}

static int parse_content (struct json_object *obj, GTranslater *_ths)
{
	int ret = 200;
	char *reslt;
	json_object_object_foreach( obj, key, value )
	{
		if(!value) continue;

		if (json_object_is_type (value, json_type_object))
		{
			parse_content (value, _ths);
		}
		else if(json_object_is_type (value, json_type_string)
				&& strcmp(key, "translatedText") == 0)
		{
			reslt = (char *)json_object_to_json_string(value);
			set_translate_result(_ths->result,reslt);
			if (_ths->result.at(0) == '"') {
				_ths->result.erase(0, 1);
			}
			if (_ths->result.at(_ths->result.length()-1) == '"') {
				_ths->result.erase(_ths->result.length()-1, _ths->result.length());
			}
		}
		else if(json_object_is_type(value, json_type_int)
				&& strcmp(key, "responseStatus") == 0)
		{
			ret = atoi(json_object_to_json_string(value));
		}
	}

	if (ret != 200){
		_ths->result = "";
		return -1;
	}

	return 0;
}


int GTranslater::json_parse (const char *body)
{
	  struct json_object *obj;

	  obj = json_tokener_parse((char *)body);
	  if(!obj)
	      return -1;

	  parse_content(obj, this);

	  json_object_put(obj);

	  return 0;
}

