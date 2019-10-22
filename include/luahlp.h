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

#ifndef LUAHLP_H_
#define LUAHLP_H_

#ifdef __cplusplus
extern "C"{
#endif

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

typedef struct lua_reg_lib_info {
	const char* lib_name;
	const luaL_reg * reg_libs;
}lua_reg_lib_info;

int call_lua_function(const char* lua_file,
		const char* lua_func,
		lua_reg_lib_info* lib_info,
		const char* arg_sigs,
		...);

enum lua_ret{
	luar_ok = 0,
	luar_failed,
	luar_initerror,
	luar_invalid_argsigs,
	luar_callfailed
};
const char* lua_error_msg(void);

#ifdef __cplusplus
}
#endif

#endif /* LUAHLP_H_ */
