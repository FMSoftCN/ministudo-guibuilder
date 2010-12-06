#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include <string>
using namespace std;

#include "translater.h"

Translater::Translater()
    :progress(0)
{
    sr_lang[0]='\0';
    ta_lang[0]='\0';
}

Translater::~Translater()
{

}

