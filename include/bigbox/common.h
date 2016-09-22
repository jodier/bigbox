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

#ifndef __BIGBOX_COMMON_H_
#define __BIGBOX_COMMON_H_

/*-------------------------------------------------------------------------*/

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <pthread.h>

#include <sys/types.h>

/*-------------------------------------------------------------------------*/

#define buff_t /* */ void *
#define BUFF_t const void *

/*-------------------------------------------------------------------------*/
/* VERSION                                                                 */
/*-------------------------------------------------------------------------*/

#define BIGBOX_VERSION "1.0.0"

/*-------------------------------------------------------------------------*/
/* LOG                                                                     */
/*-------------------------------------------------------------------------*/

typedef enum log_type_e
{
	LOG_TYPE_DEBUG
	,
	LOG_TYPE_OOOPS
	,
	LOG_TYPE_ERROR
	,
	LOG_TYPE_FATAL

} log_type_t;

/*-------------------------------------------------------------------------*/
/* HASH_TABLE                                                              */
/*-------------------------------------------------------------------------*/

typedef struct bigbox_hash_table_item_s
{
	uint32_t refcnt;
	uint32_t expire;

	uint64_t hash;

	buff_t buff;
	size_t size;

	struct bigbox_hash_table_item_s *list_prev;
	struct bigbox_hash_table_item_s *list_next;

	struct bigbox_hash_table_item_s *table_prev;
	struct bigbox_hash_table_item_s *table_next;

} bigbox_hash_table_item_t;

/*-------------------------------------------------------------------------*/

typedef struct bigbox_hash_table_s
{
	int dim;

	struct bigbox_hash_table_item_s *list;

	struct bigbox_hash_table_item_s **table;

	pthread_mutex_t mutex;

} bigbox_hash_table_t;

/*-------------------------------------------------------------------------*/
/* BASIC CLIENT                                                            */
/*-------------------------------------------------------------------------*/

typedef struct bigbox_client_s
{
	const char *addr;

	uint16_t port;

	int sock;

} bigbox_client_ctx_t;

/*-------------------------------------------------------------------------*/

#define bigbox_client_ctx_get_addr(client_ctx) \
	((client_ctx)->addr)

#define bigbox_client_ctx_get_port(client_ctx) \
	((client_ctx)->port)

#define bigbox_client_ctx_get_sock(client_ctx) \
	((client_ctx)->sock)

/*-------------------------------------------------------------------------*/
/* BASIC SERVER                                                            */
/*-------------------------------------------------------------------------*/

typedef struct bigbox_server_ctx_s
{
	uint16_t port;

	int sock;

} bigbox_server_ctx_t;

/*-------------------------------------------------------------------------*/

#define bigbox_server_ctx_get_port(server_ctx) \
	((server_ctx)->port)

#define bigbox_server_ctx_get_sock(server_ctx) \
	((server_ctx)->sock)

#define bigbox_server_ctx_get_nr(server_ctx) \
	((server_ctx)->nr)

/*-------------------------------------------------------------------------*/
/* BASIC POOLER                                                            */
/*-------------------------------------------------------------------------*/

typedef struct bigbox_pooler_ctx_s
{
	/*-----------------------------------------------------------------*/

	volatile int alive;
	volatile int client_sock;
	volatile int nb_of_free_thread;

	/*-----------------------------------------------------------------*/

	pthread_mutex_t mutex;

	pthread_cond_t new_client;
	pthread_cond_t free_thread;

	/*-----------------------------------------------------------------*/

} bigbox_pooler_ctx_t;

/*-------------------------------------------------------------------------*/
/* HIGH LEVEL SERVERS                                                      */
/*-------------------------------------------------------------------------*/

#define SVR_HTTP_METHOD_UNKNOWN	0
#define SVR_HTTP_METHOD_GET	1
#define SVR_HTTP_METHOD_POST	2
#define SVR_HTTP_METHOD_PUT	3
#define SVR_HTTP_METHOD_DEL	4

/*-------------------------------------------------------------------------*/

typedef struct bigbox_http_param_s
{
	const char *name;
	const char *value;

} bigbox_http_param_t;

/*-------------------------------------------------------------------------*/

typedef void (* bigbox_tcp_handler_ptr_t)(
	int client_sock
);

/*-------------------------------------------------------------------------*/

typedef void (* bigbox_http_handler_ptr_t)(

	const char **content_type,
	buff_t *content_buff,
	size_t *content_size,

	void (** done_handler_ptr)(void *),
	void **done_handler_arg,

	int method,
	const char *in_path,
	const char *in_content_type,
	buff_t in_content_buff,
	size_t in_content_size,

	size_t nb_of_params,
	bigbox_http_param_t param_array[]
);

/*-------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

/*-------------------------------------------------------------------------*/
/* LOG                                                                     */
/*-------------------------------------------------------------------------*/

int _bigbox_vlog(
	enum log_type_e type,
	const char *function_name,
	const char *format,
	va_list ap
);

int _bigbox_log(
	enum log_type_e type,
	const char *function_name,
	const char *format,
	...
);

/*-------------------------------------------------------------------------*/

#define bigbox_log(type, format, ...) \
		_bigbox_log(type, __func__, format, ##__VA_ARGS__)

/*-------------------------------------------------------------------------*/
/* HASH                                                                    */
/*-------------------------------------------------------------------------*/

uint64_t bigbox_hash(
	BUFF_t buff,
	size_t size,
	uint64_t seed1,
	uint64_t seed2
);

/*-------------------------------------------------------------------------*/
/* HASH TABLE                                                              */
/*-------------------------------------------------------------------------*/

bool bigbox_hash_table_initialize(
	struct bigbox_hash_table_s *hash_table,
	size_t dim
);

bool bigbox_hash_table_finalize(
	struct bigbox_hash_table_s *hash_table
);

/*-------------------------------------------------------------------------*/

void bigbox_hash_table_item_release(
	struct bigbox_hash_table_s *hash_table,
	struct bigbox_hash_table_item_s *hash_table_item
);

/*-------------------------------------------------------------------------*/

bool bigbox_hash_table_put_by_hash(
	struct bigbox_hash_table_s *hash_table,
	uint64_t hash,
	buff_t buff,
	size_t size,
	uint32_t expire
);

bool bigbox_hash_table_put(
	struct bigbox_hash_table_s *has_table,
	const char *key,
	buff_t buff,
	size_t size,
	uint32_t expire
);

/**/

bool bigbox_hash_table_get_by_hash(
	struct bigbox_hash_table_s *hash_table,
	uint64_t hash,
	struct bigbox_hash_table_item_s **result
);

bool bigbox_hash_table_get(
	struct bigbox_hash_table_s *hash_table,
	const char *key,
	struct bigbox_hash_table_item_s **result
);

/**/

bool bigbox_hash_table_del_by_hash(
	struct bigbox_hash_table_s *hash_table,
	uint64_t hash
);

bool bigbox_hash_table_del(
	struct bigbox_hash_table_s *hash_table,
	const char *key
);

/*-------------------------------------------------------------------------*/
/* ROBUST I/O                                                              */
/*-------------------------------------------------------------------------*/

int bigbox_rio_read_timeout(
	int sock,
	int seconds
);

int bigbox_rio_write_timeout(
	int sock,
	int seconds
);

/*-------------------------------------------------------------------------*/

ssize_t bigbox_rio_read(
	int sock,
	buff_t buff,
	size_t size
);

ssize_t bigbox_rio_write(
	int sock,
	BUFF_t buff,
	size_t size
);

/*-------------------------------------------------------------------------*/

int bigbox_rio_close(
	int sock
);

/*-------------------------------------------------------------------------*/
/* BASIC TCP CLIENT                                                        */
/*-------------------------------------------------------------------------*/

void bigbox_client_initialize(
	struct bigbox_client_s *ctx
);

int bigbox_client_finalize(
	struct bigbox_client_s *ctx
);

/*-------------------------------------------------------------------------*/

int bigbox_client_connect(
	struct bigbox_client_s *ctx,
	const char *host,
	uint16_t port
);

/*-------------------------------------------------------------------------*/
/* BASIC TCP SERVER                                                        */
/*-------------------------------------------------------------------------*/

void bigbox_server_initialize(
	struct bigbox_server_ctx_s *ctx
);

int bigbox_server_finalize(
	struct bigbox_server_ctx_s *ctx
);

/*-------------------------------------------------------------------------*/

int bigbox_server_listen(
	struct bigbox_server_ctx_s *ctx,
	int port,
	int backlog
);

/*-------------------------------------------------------------------------*/
/* HIGH LEVEL SERVERS                                                      */
/*-------------------------------------------------------------------------*/

int bigbox_tcp_loop(
	struct bigbox_server_ctx_s *server_ctx,
	struct bigbox_pooler_ctx_s *pooler_ctx,
	bigbox_tcp_handler_ptr_t handler_ptr,
	int nb_of_threads
);

int bigbox_http_loop(
	struct bigbox_server_ctx_s *server_ctx,
	struct bigbox_pooler_ctx_s *http_ctx,
	bigbox_http_handler_ptr_t handler_ptr,
	int nb_of_threads
);

/*-------------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

/*-------------------------------------------------------------------------*/

#endif /* __BIGBOX_COMMON_H_ */

/*-------------------------------------------------------------------------*/
