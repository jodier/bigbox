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

#include "../internal.h"

/*-------------------------------------------------------------------------*/

static void __loop_handler(bigbox_server_thread_t *thread)
{
	if(thread->user_handler_ptr != NULL)
	{
		((bigbox_tcp_handler_ptr_t) thread->user_handler_ptr)(thread->client_sock);
	}
}

/*-------------------------------------------------------------------------*/

void bigbox_tcp_loop(bigbox_server_ctx_t *server_ctx, bigbox_pooler_ctx_t *pooler_ctx, bigbox_tcp_handler_ptr_t handler_ptr, int nb_of_threads)
{
	bigbox_server_pooler(server_ctx, pooler_ctx, __loop_handler, handler_ptr, nb_of_threads);
}

/*-------------------------------------------------------------------------*/
