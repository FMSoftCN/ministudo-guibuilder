/**
 *  Create by wangjian.
 *  Date : 2009-6-27
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
	if (instance)
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
			_ths->result = reslt;
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

