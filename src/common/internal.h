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

#include "../../include/bigbox/common.h"

/*-------------------------------------------------------------------------*/
/*                                                                         */
/*-------------------------------------------------------------------------*/

struct bigbox_hash_table_item_s
{
	uint64_t hash;

	buff_t buff;
	size_t size;

	struct bigbox_hash_table_item_s *next;
};

/*-------------------------------------------------------------------------*/
/*                                                                         */
/*-------------------------------------------------------------------------*/

#define closesocket(s) \
		close(s)

#define h_addr h_addr_list[0]

typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;

/*-------------------------------------------------------------------------*/

typedef struct bigbox_server_thread_s
{
	/*-----------------------------------------------------------------*/

	struct bigbox_server_ctx_s *server_ctx;
	struct bigbox_pooler_ctx_s *pooler_ctx;

	/*-----------------------------------------------------------------*/

	void (* loop_handler_ptr)(struct bigbox_server_thread_s *);

	void *user_handler_ptr;

	/*-----------------------------------------------------------------*/

	pthread_t pthread;

	int client_sock;

	/*-----------------------------------------------------------------*/

} bigbox_server_thread_t;

/*-------------------------------------------------------------------------*/

void bigbox_server_pooler(
	struct bigbox_server_ctx_s *server_ctx,
	struct bigbox_pooler_ctx_s *pooler_ctx,
	void (* loop_handler_ptr)(struct bigbox_server_thread_s *),
	void *user_handler_ptr,
	int nb_of_threads
);

/*-------------------------------------------------------------------------*/
