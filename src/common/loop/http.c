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

#define _GNU_SOURCE

/*-------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if !defined(_WIN32) && !defined(_WIN64)
  #include <sys/socket.h>
#else
  #include <winsock2.h>
#endif


#include "../internal.h"

/*-------------------------------------------------------------------------*/

static char __hex2char(char c) {

	char result;

	/**/ if(c >= '0' && c <= '9') {
		result = c - '0' + 0x00;
	}
	else if(c >= 'a' && c <= 'f') {
	 	result = c - 'a' + 0x0A;
	}
	else if(c >= 'A' && c <= 'F') {
	 	result = c - 'A' + 0x0A;
	}
	else {
		result = 0;
	}

	return result;
}

/*-------------------------------------------------------------------------*/

static char *__decode_uri(char *s)
{
	register char *p = s;
	register char *q = s;

	while(*p != '\0')
	{
		/**/ if(*p == '%')
		{
			if(p[1] && p[2])
			{
				*q++ = __hex2char(p[1]) << 4
				       |
				       __hex2char(p[2]) << 0
				;

				p += 2;
			}
		}
		else if(*p == '+')
		{
			*q++ = ' ';
		}
		else
		{
			*q++ = *p;
		}

		p++;
	}

	*q = '\0';

	return s;
}

/*-------------------------------------------------------------------------*/

static size_t __deserialize_args(bigbox_http_arg_t arg_array[], char *s)
{
	char *lasts;

	register char *p;
	register char *q;

	size_t result = 0;

	for(p = strtok_r(s, "&", &lasts); p != NULL; p = strtok_r(0, "&", &lasts))
	{
		/*---------------------------------------------------------*/
		/*                                                         */
		/*---------------------------------------------------------*/

		q = strchr(p, '=');

		if(q != NULL)
		{
			q[0] = '\0';

			q = __decode_uri(q + 1);
		}

		/*---------------------------------------------------------*/
		/*                                                         */
		/*---------------------------------------------------------*/

		arg_array[result].name = p;
		arg_array[result].value = q;

		result++;

		/*---------------------------------------------------------*/
	}

	return result;
}

/*-------------------------------------------------------------------------*/

static ssize_t __readline(int sock, char *buffer, size_t size)
{
	register char *p = buffer;

	char c;

	for(;;)
	{
		if(size == 0 || recv(sock, &c, 1, 0) != 1)
		{
			return -1;
		}

		if(c == '\r')
		{
			continue;
		}

		if(c == '\n')
		{
			*p = '\0';

			return (ssize_t) p - (ssize_t) buffer;
		}

		*p++ = c;

		size--;
	}
}

/*-------------------------------------------------------------------------*/

static void __loop_handler(bigbox_server_thread_t *thread)
{
	char buffer[2048], path[2048], content_length[1024], origin[1024];

	int method = SVR_HTTP_METHOD_UNKNOWN;

	register char *p;

	ssize_t size;

	int ret;

	/*-----------------------------------------------------------------*/
	/*                                                                 */
	/*-----------------------------------------------------------------*/

	path[0] = '\0';
	content_length[0] = '\0';
	origin[0] = '\0';

	/*-----------------------------------------------------------------*/
	/*                                                                 */
	/*-----------------------------------------------------------------*/

	for(;;)
	{
		/*---------------------------------------------------------*/

		size = __readline(thread->client_sock, buffer, 2048);

		if(size < 0)
		{
			bigbox_log(LOG_TYPE_ERROR, "could not read header!\n");

			return;
		}

		if(size == 0)
		{
			break;
		}

		/*---------------------------------------------------------*/

		/**/ if(strncmp(buffer, "GET ", 4) == 0)
		{
			p = strstr(buffer, " HTTP");

			if(p != NULL)
			{
				*p = '\0';

				strcpy(path, buffer + 4);

				method = SVR_HTTP_METHOD_GET;
			}
		}

		/*---------------------------------------------------------*/

		else if(strncmp(buffer, "POST ", 5) == 0)
		{
			p = strstr(buffer, " HTTP");

			if(p != NULL)
			{
				*p = '\0';

				strcpy(path, buffer + 5);

				method = SVR_HTTP_METHOD_POST;
			}
		}

		/*---------------------------------------------------------*/

		else if(strncmp(buffer, "PUT ", 4) == 0)
		{
			p = strstr(buffer, " HTTP");

			if(p != NULL)
			{
				*p = '\0';

				strcpy(path, buffer + 4);

				method = SVR_HTTP_METHOD_PUT;
			}
		}

		/*---------------------------------------------------------*/

		else if(strncmp(buffer, "DELETE ", 7) == 0)
		{
			p = strstr(buffer, " HTTP");

			if(p != NULL)
			{
				*p = '\0';

				strcpy(path, buffer + 7);

				method = SVR_HTTP_METHOD_PUT;
			}
		}

		/*---------------------------------------------------------*/

		else if(strncmp(buffer, "Content-Length: ", 16) == 0)
		{
			strcpy(content_length, buffer + 16);
		}

		/*---------------------------------------------------------*/

		else if(strncmp(buffer, "Origin: ", 8) == 0)
		{
			strcpy(origin, buffer + 8);
		}

		/*---------------------------------------------------------*/
	}

	/*-----------------------------------------------------------------*/
	/*                                                                 */
	/*-----------------------------------------------------------------*/

	const char *content_type = "text/plain";

	buff_t content_buff = NULL;
	size_t content_size = 0x00;

	void (* post_handler)(void *) = NULL;

	void *post_handler_arg = NULL;

	/*-----------------------------------------------------------------*/

	if(thread->user_handler_ptr != NULL)
	{
		char *PARAMS = NULL;
		char *CONTENT = NULL;

		size_t nb_of_args = 0;

		bigbox_http_arg_t arg_array[128];

		/*---------------------------------------------------------*/
		/*                                                         */
		/*---------------------------------------------------------*/

		PARAMS = strchr(path, '?');

		if(PARAMS != NULL)
		{
			*PARAMS++ = '\0';

			nb_of_args += __deserialize_args(&arg_array[nb_of_args], PARAMS);
		}

		/*---------------------------------------------------------*/
		/*                                                         */
		/*---------------------------------------------------------*/

		int __content_length = atoi(content_length);

		/*---------------------------------------------------------*/

		if(__content_length > 0)
		{
			/*-------------------------------------------------*/
			/*                                                 */
			/*-------------------------------------------------*/

			CONTENT = (char *) malloc(__content_length + 1);

			/*-------------------------------------------------*/
			/*                                                 */
			/*-------------------------------------------------*/

			if(CONTENT != NULL)
			{
				int i;

				for(i = 0; i < __content_length; i++)
				{
					if(recv(thread->client_sock, &CONTENT[i], 1, 0) != 1)
					{
						break;
					}
				}
			}
			else
			{
				bigbox_log(LOG_TYPE_ERROR, "out of memory!\n");
			}

			/*-------------------------------------------------*/
			/*                                                 */
			/*-------------------------------------------------*/

			nb_of_args += __deserialize_args(&arg_array[nb_of_args], CONTENT);

			/*-------------------------------------------------*/
		}

		/*---------------------------------------------------------*/
		/*                                                         */
		/*---------------------------------------------------------*/

		((bigbox_http_handler_ptr_t) thread->user_handler_ptr)(
			&content_type,
			&content_buff,
			&content_size,
			&post_handler,
			&post_handler_arg,
			method,
			nb_of_args,
			arg_array,
			path
		);

		/*---------------------------------------------------------*/
		/*                                                         */
		/*---------------------------------------------------------*/

		if(CONTENT != NULL)
		{
			free((void *) CONTENT);
		}

		/*---------------------------------------------------------*/
	}

	/*-----------------------------------------------------------------*/
	/*                                                                 */
	/*-----------------------------------------------------------------*/

	sprintf(buffer,
		"HTTP/1.1 200 OK\n"
		"Connection: close\n"
		"Content-Type: %s\n"
		"Content-length: %lu\n"
		"Access-Control-Allow-Credentials: true\n"
		"Access-Control-Allow-Origin: %s\n"
		"\n",
		content_type,
		content_size,
		origin
	);

	ret = bigbox_rio_write(thread->client_sock, buffer, strlen(buffer));

	if(ret < 0)
	{
		bigbox_log(LOG_TYPE_ERROR, "could not send data!\n");
	}

	/*-----------------------------------------------------------------*/
	/*                                                                 */
	/*-----------------------------------------------------------------*/

	if(content_buff != NULL
	   &&
	   content_size != 0x00
	 ) {
		ret = bigbox_rio_write(thread->client_sock, content_buff, content_size);

		if(ret < 0)
		{
			bigbox_log(LOG_TYPE_ERROR, "could not send data!\n");
		}
	}

	/*-----------------------------------------------------------------*/
	/*                                                                 */
	/*-----------------------------------------------------------------*/

	if(post_handler != NULL)
	{
		post_handler(post_handler_arg);
	}

	/*-----------------------------------------------------------------*/
}

/*-------------------------------------------------------------------------*/

void bigbox_http_loop(bigbox_server_ctx_t *server_ctx, bigbox_pooler_ctx_t *pooler_ctx, bigbox_http_handler_ptr_t handler_ptr, int nb_of_threads)
{
	bigbox_server_pooler(server_ctx, pooler_ctx, __loop_handler, handler_ptr, nb_of_threads);
}

/*-------------------------------------------------------------------------*/
