/* Author : Jerome ODIER
 * Email : odier@hypnos-solutions.com
 *
 * Version : 1.0 (2016-![VALUE YEAR])
 *
 *
 * This file is part of BIGBOX.
 *
 *
 * Hypnos-Solutions RESTRICTED
 * ___________________________
 *
 *  Hypnos-Solutions
 *  All Rights Reserved.
 *
 * NOTICE:  All information contained herein is, and remains
 * the property of Hypnos-Solutions and its suppliers, if any.
 * The intellectual and technical concepts contained herein are
 * proprietary to Hypnos-Solutions and its suppliers and are
 * protected by trade secret or copyright law. Dissemination of
 * this information or reproduction of this material is strictly
 * forbidden unless prior written permission is obtained from
 * Hypnos-Solutions.
 */

/*-------------------------------------------------------------------------*/

#include <string.h>

#include "lua/src/lua.h"
#include "lua/src/lauxlib.h"
#include "lua/src/lualib.h"

#include "internal.h"

/*-------------------------------------------------------------------------*/

static pthread_mutex_t _mutex = PTHREAD_MUTEX_INITIALIZER;

/*-------------------------------------------------------------------------*/

bool bigbox_lua_initialize(bigbox_lua_ctx_t *lua_ctx)
{
	/*-----------------------------------------------------------------*/
	/*                                                                 */
	/*-----------------------------------------------------------------*/

	lua_State *state = luaL_newstate();

	if(state == NULL)
	{
		return false;
	}

	luaL_openlibs(state);

	/*-----------------------------------------------------------------*/
	/*                                                                 */
	/*-----------------------------------------------------------------*/



	/*-----------------------------------------------------------------*/
	/*                                                                 */
	/*-----------------------------------------------------------------*/

	lua_ctx->state = (void *) state;

	return true;
}

/*-------------------------------------------------------------------------*/

bool bigbox_lua_finalize(bigbox_lua_ctx_t *lua_ctx)
{
	if(lua_ctx->state != NULL)
	{
		lua_close((lua_State *) lua_ctx->state);
	}

	return true;
}

/*-------------------------------------------------------------------------*/

const char *bigbox_lua_execute(bigbox_lua_ctx_t *lua_ctx, const char *code)
{
	char *result;

	pthread_mutex_lock(&_mutex);

	/**/	if(luaL_dostring((lua_State *) lua_ctx->state, code) == 0)
	/**/	{
	/**/		result = strdup(lua_tostring((lua_State *) lua_ctx->state, -1));
	/**/	}
	/**/	else
	/**/	{
	/**/		result = strdup("done with error");
	/**/	}

	pthread_mutex_unlock(&_mutex);

	return result;
}

/*-------------------------------------------------------------------------*/
