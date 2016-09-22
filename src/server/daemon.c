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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <signal.h>
#include <string.h>

#include "internal.h"

#include "html/index.h"

/*-------------------------------------------------------------------------*/

static void version(void)
{
	printf("%s\n", BIGBOX_VERSION);
}

/*-------------------------------------------------------------------------*/

static void help(const char *program_name, size_t default_dim, uint32_t default_port)
{
	printf("%s %zu %u\n", program_name, default_dim, default_port);
}

/*-------------------------------------------------------------------------*/

#define BIGBOX_DEFAULT_DIM 4096
#define BIGBOX_DEFAULT_PORT 6969

/*-------------------------------------------------------------------------*/

static bigbox_hash_table_t hash_table;
static bigbox_server_ctx_t server_ctx;
static bigbox_pooler_ctx_t pooler_ctx;

/*-------------------------------------------------------------------------*/

static void __exit(int signal)
{
	printf("\nStopping BigBox...\n");

	pooler_ctx.alive = 0;

	if(bigbox_server_finalize(&server_ctx) < 0)
	{
		bigbox_log(LOG_TYPE_ERROR, "could not finalize server!\n");
	}

	if(bigbox_hash_table_finalize(&hash_table) == false)
	{
		bigbox_log(LOG_TYPE_ERROR, "could not finalize hash table!\n");
	}
}

/*-------------------------------------------------------------------------*/

static void xxxx_xxxxx_xxxx(void *arg)
{
	bigbox_hash_table_item_release(&hash_table, arg);
}

/*-------------------------------------------------------------------------*/

static void http_handler(const char **content_type, buff_t *content_buff, size_t *content_size, void (** done_handler_ptr)(void *), void **done_handler_arg, int method, size_t nb_of_params, bigbox_http_param_t arg_array[], const char *path)
{
	*content_type = "text/html";

	/**/ if(strcmp(path, "/") == 0)
	{
		*content_buff = index_html_buff;
		*content_size = INDEX_HTML_SIZE;
	}
	else if(strncmp(path, "/key/", 5) == 0)
	{
		/*---------------------------------------------------------*/
		/* METHOD GET                                              */
		/*---------------------------------------------------------*/

		/**/ if(method == SVR_HTTP_METHOD_GET)
		{
			bigbox_hash_table_item_t *hash_table_item;

			if(bigbox_hash_table_get(&hash_table, path + 5, &hash_table_item))
			{
				*content_buff = hash_table_item->buff;
				*content_size = hash_table_item->size;

				*done_handler_ptr = xxxx_xxxxx_xxxx;
				*done_handler_arg = hash_table_item;
			}
			else
			{
				*content_buff = NULL;
				*content_size = 0x00;
			}
		}

		/*---------------------------------------------------------*/
		/* METHOD POST/PUT                                         */
		/*---------------------------------------------------------*/

		else if(method == SVR_HTTP_METHOD_POST
		        ||
			method == SVR_HTTP_METHOD_PUT
		 ) {
			bigbox_hash_table_put(&hash_table, path + 5, "Hello World!", 12, 1000000);
		}

		/*---------------------------------------------------------*/
		/* METHOD DEL                                              */
		/*---------------------------------------------------------*/

		else if(method == SVR_HTTP_METHOD_DEL)
		{
			if(bigbox_hash_table_del(&hash_table, path + 5))
			{
				*content_buff = "entry deleted";
				*content_size = 0x000000000000D;
			}
			else
			{
				*content_buff = "entry not found";
				*content_size = 0x00000000000000F;
			}
		}

		/*---------------------------------------------------------*/

		else
		{
			*content_buff = "invalid request";
			*content_size = 0x00000000000000F;
		}

		/*---------------------------------------------------------*/
	}
	else
	{
		*content_buff = "invalid request";
		*content_size = 0x00000000000000F;
	}
}

/*-------------------------------------------------------------------------*/

int main(int argc, char **argv)
{
	/*-----------------------------------------------------------------*/

	size_t dim = BIGBOX_DEFAULT_DIM;

	uint32_t port = BIGBOX_DEFAULT_PORT;

	static struct option long_options[] = {
		{"dim", 1, 0, 'd'},
		{"port", 1, 0, 'p'},
		{"version", 0, 0, 'v'},
		{"help", 0, 0, 'h'},
		{NULL, 0, NULL, 0}
	};

	/*-----------------------------------------------------------------*/

	int c;

	while((c = getopt_long(argc, argv, "dp:vh", long_options, NULL)) != -1)
	{
		switch(c)
		{
			case 'd':
				dim = strtoul(optarg, NULL, 0);
				break;

			case 'p':
				port = strtoul(optarg, NULL, 0);
				break;

			case 'v':
				version();
				return 0;

			case 'h':
				help(argv[0], BIGBOX_DEFAULT_DIM, BIGBOX_DEFAULT_PORT);
				return 0;

			default:
				help(argv[0], BIGBOX_DEFAULT_DIM, BIGBOX_DEFAULT_PORT);
				return 1;
		}
	}

	/*-----------------------------------------------------------------*/

	printf("Starting BigBox on port %d...\n", port);

	/*-----------------------------------------------------------------*/

	if(bigbox_hash_table_initialize(&hash_table, dim) == false)
	{
		bigbox_log(LOG_TYPE_FATAL, "could not initialize DB!\n");
	}

	bigbox_hash_table_put(&hash_table, "foo", "Hello World!", 12, 1000000);

	/*-----------------------------------------------------------------*/

	signal(SIGINT, __exit);

	/*-----------------------------------------------------------------*/

	bigbox_server_initialize(&server_ctx);

	if(bigbox_server_listen(&server_ctx, port, 100) < 0)
	{
		bigbox_server_finalize(&server_ctx);

		bigbox_hash_table_finalize(&hash_table);

		bigbox_log(LOG_TYPE_FATAL, "%s!\n", strerror(errno));
	}

	bigbox_http_loop(&server_ctx, &pooler_ctx, http_handler, 100);

	/*-----------------------------------------------------------------*/

	printf("Bye...\n");

	/*-----------------------------------------------------------------*/

	return 0;
}

/*-------------------------------------------------------------------------*/
