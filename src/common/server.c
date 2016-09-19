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

#if !defined(_WIN32) && !defined(_WIN64)
  #include <unistd.h>

  #include <arpa/inet.h>
#else
  #include <winsock2.h>
#endif

#include "internal.h"

/*-------------------------------------------------------------------------*/

void bigbox_server_initialize(bigbox_server_ctx_t *server_ctx)
{
	memset(server_ctx, 0x00, sizeof(bigbox_server_ctx_t));

	server_ctx->sock = -1;
}

/*-------------------------------------------------------------------------*/

void bigbox_server_finalize(bigbox_server_ctx_t *ctx)
{
	bigbox_rio_close(ctx->sock);
}

/*-------------------------------------------------------------------------*/

int bigbox_server_listen(bigbox_server_ctx_t *server_ctx, int port, int backlog)
{
	int ret;

	/*-----------------------------------------------------------------*/
	/* CREATE SOCKET                                                   */
	/*-----------------------------------------------------------------*/

	int sock = socket(AF_INET, SOCK_STREAM, 0);

	if(sock < 0)
	{
		return -1;
	}

	/*-----------------------------------------------------------------*/

	SOCKADDR_IN sockaddr = {0};

	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(port);
	sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	/*-----------------------------------------------------------------*/

	int option = 1;

	ret = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

	if(ret < 0)
	{
		closesocket(sock);

		return -1;
	}

	/*-----------------------------------------------------------------*/
	/* BIND SOCKET                                                     */
	/*-----------------------------------------------------------------*/

	ret = bind(sock, (SOCKADDR *) &sockaddr, sizeof(sockaddr));

	if(ret < 0)
	{
		closesocket(sock);

		return -1;
	}

	/*-----------------------------------------------------------------*/
	/* LISTEN SOCKET                                                   */
	/*-----------------------------------------------------------------*/

	ret = listen(sock, backlog);

	if(ret < 0)
	{
		closesocket(sock);

		return -1;
	}

	/*-----------------------------------------------------------------*/
	/* PATCH CONTEXT                                                   */
	/*-----------------------------------------------------------------*/

	server_ctx->port = port;
	server_ctx->sock = sock;

	/*-----------------------------------------------------------------*/

	return 0;
}

/*-------------------------------------------------------------------------*/
