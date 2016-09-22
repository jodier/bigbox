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

static size_t __deserialize_params(bigbox_http_param_t arg_array[], char *s)
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
	char buffer[2048], in_path[2048], in_content_length[1024], in_content_type[1024], in_origin[1024];

	int in_method = SVR_HTTP_METHOD_UNKNOWN;

	register char *p;

	ssize_t size;

	int ret;

	/*-----------------------------------------------------------------*/
	/*                                                                 */
	/*-----------------------------------------------------------------*/

	in_path[0] = '\0';
	in_content_length[0] = '\0';
	in_content_type[0] = '\0';
	in_origin[0] = '\0';

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

				strcpy(in_path, buffer + 4);

				in_method = SVR_HTTP_METHOD_GET;
			}
		}

		/*---------------------------------------------------------*/

		else if(strncmp(buffer, "POST ", 5) == 0)
		{
			p = strstr(buffer, " HTTP");

			if(p != NULL)
			{
				*p = '\0';

				strcpy(in_path, buffer + 5);

				in_method = SVR_HTTP_METHOD_POST;
			}
		}

		/*---------------------------------------------------------*/

		else if(strncmp(buffer, "PUT ", 4) == 0)
		{
			p = strstr(buffer, " HTTP");

			if(p != NULL)
			{
				*p = '\0';

				strcpy(in_path, buffer + 4);

				in_method = SVR_HTTP_METHOD_PUT;
			}
		}

		/*---------------------------------------------------------*/

		else if(strncmp(buffer, "DELETE ", 7) == 0)
		{
			p = strstr(buffer, " HTTP");

			if(p != NULL)
			{
				*p = '\0';

				strcpy(in_path, buffer + 7);

				in_method = SVR_HTTP_METHOD_DEL;
			}
		}

		/*---------------------------------------------------------*/

		else if(strncmp(buffer, "Content-Length: ", 16) == 0)
		{
			strcpy(in_content_length, buffer + 16);
		}

		/*---------------------------------------------------------*/

		else if(strncmp(buffer, "Content-Type: ", 14) == 0)
		{
			strcpy(in_content_type, buffer + 14);
		}

		/*---------------------------------------------------------*/

		else if(strncmp(buffer, "Origin: ", 8) == 0)
		{
			strcpy(in_origin, buffer + 8);
		}

		/*---------------------------------------------------------*/
	}

	/*-----------------------------------------------------------------*/
	/*                                                                 */
	/*-----------------------------------------------------------------*/

	const char *out_content_type = "text/plain";
	buff_t out_content_buff = NULL;
	size_t out_content_size = 0x00;

	void (* done_handler_ptr)(void *) = NULL;
	void *done_handler_arg = NULL;

	/*-----------------------------------------------------------------*/

	if(thread->user_handler_ptr != NULL)
	{
		size_t nb_of_args = 0;

		bigbox_http_param_t arg_array[128];

		/*---------------------------------------------------------*/
		/*                                                         */
		/*---------------------------------------------------------*/

		char *params = strchr(in_path, '?');

		if(params != NULL)
		{
			*params++ = '\0';

			nb_of_args += __deserialize_params(&arg_array[nb_of_args], params);
		}

		/*---------------------------------------------------------*/
		/*                                                         */
		/*---------------------------------------------------------*/

		buff_t in_content_buff = 0x000000000000000000000;
		size_t in_content_size = atoi(in_content_length);

		/*---------------------------------------------------------*/

		if(in_content_size > 0)
		{
			in_content_buff = malloc(in_content_size + 1);

			if(in_content_buff != NULL)
			{
				/*-----------------------------------------*/
				/*                                         */
				/*-----------------------------------------*/

				char *p = (char *) in_content_buff;

				for(size_t i = 0; i < in_content_size; i++)
				{
					if(recv(thread->client_sock, p++, 1, 0) != 1)
					{
						break;
					}
				}

				*p = '\0';

				/*-----------------------------------------*/
				/*                                         */
				/*-----------------------------------------*/

				if(strcmp(in_content_type, "application/x-www-form-urlencoded") == 0)
				{
					nb_of_args += __deserialize_params(arg_array + nb_of_args, in_content_buff);

					in_content_size = 0;
				}

				/*-----------------------------------------*/
			}
			else
			{
				in_content_size = 0;
			}
		}

		/*---------------------------------------------------------*/
		/*                                                         */
		/*---------------------------------------------------------*/

		((bigbox_http_handler_ptr_t) thread->user_handler_ptr)(
			&out_content_type,
			&out_content_buff,
			&out_content_size,
			&done_handler_ptr,
			&done_handler_arg,
			in_method,
			in_path,
			in_content_type,
			in_content_buff,
			in_content_size,
			nb_of_args,
			arg_array
		);

		/*---------------------------------------------------------*/
		/*                                                         */
		/*---------------------------------------------------------*/

		if(in_content_buff != NULL)
		{
			free(in_content_buff);
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
		out_content_type,
		out_content_size,
		in_origin
	);

	/*-----------------------------------------------------------------*/

	ret = bigbox_rio_write(thread->client_sock, buffer, strlen(buffer));

	if(ret < 0)
	{
		bigbox_log(LOG_TYPE_ERROR, "could not send data!\n");
	}

	/*-----------------------------------------------------------------*/
	/*                                                                 */
	/*-----------------------------------------------------------------*/

	if(out_content_buff != NULL
	   &&
	   out_content_size != 0x00
	 ) {
		ret = bigbox_rio_write(thread->client_sock, out_content_buff, out_content_size);

		if(ret < 0)
		{
			bigbox_log(LOG_TYPE_ERROR, "could not send data!\n");
		}
	}

	/*-----------------------------------------------------------------*/
	/*                                                                 */
	/*-----------------------------------------------------------------*/

	if(done_handler_ptr != NULL)
	{
		done_handler_ptr(done_handler_arg);
	}

	/*-----------------------------------------------------------------*/
}

/*-------------------------------------------------------------------------*/

int bigbox_http_loop(bigbox_server_ctx_t *server_ctx, bigbox_pooler_ctx_t *pooler_ctx, bigbox_http_handler_ptr_t handler_ptr, int nb_of_threads)
{
	return bigbox_server_pooler(server_ctx, pooler_ctx, __loop_handler, handler_ptr, nb_of_threads);
}

/*-------------------------------------------------------------------------*/
