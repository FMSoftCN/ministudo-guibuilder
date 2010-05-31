/*
 * luahlp.h
 *
 *  Created on: 2009-6-27
 *      Author: dongjunjie
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
