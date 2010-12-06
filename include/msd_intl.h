/*
 * msd_intl.h
 *
 *  Created on: 2010-2-8
 *      Author: joshua
 */

#ifndef MSD_INTL_H_
#define MSD_INTL_H_

#include "gbconfig.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _MSTUDIO_LOCALE

const char* msd_gettext(const char *msgid);

int msd_locale_init(const char*str_cfg);

void free_mo_info();

#define _ msd_gettext

#else

#define _

#endif

#ifdef __cplusplus
}
#endif

#endif /* MSD_INTL_H_ */
