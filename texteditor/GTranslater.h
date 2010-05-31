/**
 *  Create by wangjian.
 *  Date : 2009-6-27
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
