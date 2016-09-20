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
#include <string.h>

#include "internal.h"

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

void __exit(int signal)
{
	printf("stopping bigbox...\n");

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

static void http_handler(const char **content_type, buff_t *content_buff, size_t *content_size, int *free_content, int method, size_t nb_of_args, bigbox_http_arg_t arg_array[], const char *path)
{
	char *p = "<html><body>Hello World!</body></html>";

	*content_buff = /****/(p);
	*content_size = strlen(p);
	*free_content = 0;
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
