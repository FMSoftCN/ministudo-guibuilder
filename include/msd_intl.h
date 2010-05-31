/*
 * msd_intl.h
 *
 *  Created on: 2010-2-8
 *      Author: joshua
 */

#ifndef MSD_INTL_H_
#define MSD_INTL_H_

#include "gbconfig.h"

#ifdef _MSTUDIO_LOCALE

const char* msd_gettext(const char *msgid);

int msd_locale_init(const char *mylocale, const char *lang_path);

void free_mo_info();

#define _ msd_gettext

#else

#define _

#endif

#endif /* MSD_INTL_H_ */
