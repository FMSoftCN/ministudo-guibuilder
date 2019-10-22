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

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#if HAVE_SYSLOG_H
# include <syslog.h>
#endif /* HAVE_SYSLOG_H */

#if HAVE_UNISTD_H
# include <unistd.h>
#endif /* HAVE_UNISTD_H */

#if HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif /* HAVE_SYS_PARAM_H */

#include "debug.h"

static int _syslog = 0;
static int _debug = 0;

void mc_set_debug(int debug) { _debug = debug; }
int mc_get_debug() { return _debug; }

extern void mc_set_syslog(int syslog)
{
  _syslog = syslog;
}

void mc_abort(const char *msg, ...)
{
  va_list ap;
  va_start(ap, msg);
#if HAVE_VSYSLOG
  if(_syslog) {
	  vsyslog(LOG_ERR, msg, ap);
  } else
#endif
	  vprintf(msg, ap);
  va_end(ap);
  exit(1);
}


void mc_debug(const char *msg, ...)
{
  va_list ap;
  if(_debug) {
    va_start(ap, msg);
#if HAVE_VSYSLOG
    if(_syslog) {
		vsyslog(LOG_DEBUG, msg, ap);
	} else
#endif
		vprintf(msg, ap);
    va_end(ap);
  }
}

void mc_error(const char *msg, ...)
{
  va_list ap;
  va_start(ap, msg);
#if HAVE_VSYSLOG
    if(_syslog) {
		vsyslog(LOG_ERR, msg, ap);
	} else
#endif
		vfprintf(stderr, msg, ap);
  va_end(ap);
}

void mc_info(const char *msg, ...)
{
  va_list ap;
  va_start(ap, msg);
#if HAVE_VSYSLOG
    if(_syslog) {
		vsyslog(LOG_INFO, msg, ap);
	} else 
#endif
		vfprintf(stderr, msg, ap);
  va_end(ap);
}
