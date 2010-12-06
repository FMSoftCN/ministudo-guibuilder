/*
 * luahlp.c
 *
 *  Created on: 2009-6-27
 *      Author: dongjunjie
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "luahlp.h"

static char err_msg[128];

const char* lua_error_msg(){
	return err_msg;
}

static int call_va (lua_State *L, const char* func, const char *sig, va_list vl)
{
  int narg, nres;		/* number of arguments and results */

  lua_getglobal(L, func);

  /* push arguments */
  narg = 0;
  while (*sig)
    {				/* push arguments */
      switch (*sig++)
	{

	case 'd':		/* double argument */
	  lua_pushnumber (L, va_arg (vl, double));
	  break;

	case 'i':		/* int argument */
	  lua_pushnumber (L, va_arg (vl, int));
	  break;

	case 's':		/* string argument */
	  lua_pushstring (L, va_arg (vl, char *));
	  break;

	case '>':
	  goto endwhile;

	default:
	  //luaL_error (L, "invalid option (%c)", *(sig - 1));
#ifdef WIN32
		sprintf(err_msg, "invalid option (%c)", *(sig-1));
#else
		snprintf(err_msg, sizeof(err_msg)-1, "invalid option (%c)", *(sig-1));
#endif
		return luar_invalid_argsigs;
	}
      narg++;
      luaL_checkstack (L, 1, "too many arguments");
    }
endwhile:

  /* do the call */
  nres = strlen (sig);		/* number of expected results */
  if (lua_pcall (L, narg, nres, 0) != 0)	/* do the call */
  {
#ifdef WIN32
		sprintf(err_msg, "luaL_error running function `%s`: %s", func, lua_tostring(L,-1));
#else
		sprintf(err_msg, "luaL_error running function `%s`: %s", func, lua_tostring(L,-1));
#endif
    return luar_callfailed;
  }

  /* retrieve results */
  nres = -nres;			/* stack index of first result */
  while (*sig)
    {				/* get results */
      switch (*sig++)
	{

	case 'd':		/* double result */
	  if (!lua_isnumber (L, nres))
	    luaL_error (L, "wrong result type");
	  *va_arg (vl, double *) = lua_tonumber (L, nres);
	  break;

	case 'i':		/* int result */
	  if (!lua_isnumber (L, nres))
	    luaL_error (L, "wrong result type");
	  *va_arg (vl, int *) = (int) lua_tonumber (L, nres);
	  break;

	case 's':		/* string result */
	  if (!lua_isstring (L, nres))
	    luaL_error (L, "wrong result type");
	  *va_arg (vl, const char **) = lua_tostring (L, nres);
	  break;

	default:
#ifdef WIN32
		sprintf(err_msg, "invalid return option (%s)", sig-1);
#else
		sprintf(err_msg, "invalid return option (%s)", sig-1);
#endif
		return luar_invalid_argsigs;
	}
      nres++;
    }

  return luar_ok;
}

int call_lua_function(const char* lua_file,
		const char* lua_func,
		lua_reg_lib_info* lib_info,
		const char* arg_sigs,...)
{
	va_list vl;
	int ret;
	lua_State* L;

	va_start(vl, arg_sigs);

	if(!lua_file || !lua_func){
#ifdef WIN32
		sprintf(err_msg, "lua_file or lua_func is NULL");
#else
		snprintf(err_msg, sizeof(err_msg)-1,"lua_file or lua_func is NULL");
#endif
		return luar_failed;
	}

	//open lib
	L = luaL_newstate();

	if(L == NULL){
#ifdef WIN32
		sprintf(err_msg, "open lua state failed");
#else
		snprintf(err_msg, sizeof(err_msg)-1,"open lua state failed");
#endif
		return luar_initerror;
	}

	//open standare lib
	luaL_openlibs(L);

	if(lib_info && lib_info->lib_name && lib_info->reg_libs)
		luaL_register(L, lib_info->lib_name , lib_info->reg_libs);

	//run file
	(void)luaL_dofile(L, lua_file);

	ret = call_va(L, lua_func, arg_sigs, vl);

	lua_close(L);

	va_end(vl);

	return ret;
}
