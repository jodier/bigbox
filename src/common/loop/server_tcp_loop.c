/* Author  : Jerome ODIER
 *
 * Email   : jerome.odier@cern.ch
 *           jerome.odier@lpsc.in2p3.fr
 *
 * Version : 1.0 (2014-2015)
 */

/*-------------------------------------------------------------------------*/

#include "internal.h"

/*-------------------------------------------------------------------------*/

static void __loop_handler(svr_thread_t *thread)
{
	if(thread->user_handler_ptr != NULL)
	{
		((svr_tcp_handler_ptr_t) thread->user_handler_ptr)(thread->client_sock);
	}
}

/*-------------------------------------------------------------------------*/

void svr_tcp_loop(svr_server_ctx_t *server_ctx, svr_pooler_ctx_t *pooler_ctx, svr_tcp_handler_ptr_t handler_ptr, int nb_of_threads)
{
	svr_pooler(server_ctx, pooler_ctx, __loop_handler, handler_ptr, nb_of_threads);
}

/*-------------------------------------------------------------------------*/
