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
  #include <netdb.h>
  #include <unistd.h>

  #include <arpa/inet.h>
#else
  #include <winsock2.h>
#endif

#include "internal.h"

/*-------------------------------------------------------------------------*/

void bigbox_client_initialize(bigbox_client_ctx_t *client_ctx)
{
	memset(client_ctx, 0x00, sizeof(bigbox_client_ctx_t));

	client_ctx->sock = -1;
}

/*-------------------------------------------------------------------------*/

int bigbox_client_finalize(bigbox_client_ctx_t *client_ctx)
{
	return client_ctx->sock >= 0 ? bigbox_rio_close(client_ctx->sock)
	                             : 0
	;
}

/*-------------------------------------------------------------------------*/

int bigbox_client_connect(bigbox_client_ctx_t *client_ctx, const char *addr, uint16_t port)
{
	int ret;

	/*-----------------------------------------------------------------*/
	/* RESOLVE HOST NAME                                               */
	/*-----------------------------------------------------------------*/

	IN_ADDR in_addr;

	struct hostent *host = gethostbyname(addr);

	if(host != NULL)
	{
		in_addr = *(IN_ADDR *) host->h_addr;
	}
	else
	{
		in_addr.s_addr = inet_addr(addr);

		if(in_addr.s_addr == INADDR_NONE)
		{
			return -1;
		}
	}

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
	sockaddr.sin_addr = in_addr;

	/*-----------------------------------------------------------------*/
	/* CONNECT SOCKET                                                  */
	/*-----------------------------------------------------------------*/

	ret = connect(sock, (SOCKADDR *) &sockaddr, sizeof(sockaddr));

	if(ret < 0)
	{
		closesocket(sock);

		return -1;
	}

	/*-----------------------------------------------------------------*/
	/* PATCH CONTEXT                                                   */
	/*-----------------------------------------------------------------*/

	client_ctx->addr = addr;
	client_ctx->port = port;
	client_ctx->sock = sock;

	/*-----------------------------------------------------------------*/

	return 0;
}

/*-------------------------------------------------------------------------*/
