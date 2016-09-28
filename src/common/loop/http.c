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
  #include <fcntl.h>

  #include <sys/select.h>
#else
  #include <winsock2.h>
#endif


#include "../internal.h"

/*-------------------------------------------------------------------------*/

static char __hex2char(char c)
{
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

static size_t __deserialize_params(bigbox_http_param_t param_array[], char *s)
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

		param_array[result].name = p;
		param_array[result].value = q;

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
		if(size == 0 || bigbox_rio_read(sock, &c, 1) != 1)
		{
			return -1;
		}

		/**/

		if(c == '\r')
		{
			continue;
		}

		if(c == '\n')
		{
			*p = '\0';

			return (ssize_t) p - (ssize_t) buffer;
		}

		/**/

		*p++ = c;

		size--;
	}
}

/*-------------------------------------------------------------------------*/

static struct timeval _timeout = {0, 0};

/*-------------------------------------------------------------------------*/

static void __loop_handler(bigbox_server_thread_t *thread)
{
	char buffer[2048], in_resource_path[2048] = "", in_content_type[2048] = "", in_origin[2048] = "";

	ssize_t size;

	char *p, *q;

	int ret;

	/*-----------------------------------------------------------------*/
	/*                                                                 */
	/*-----------------------------------------------------------------*/

	fd_set read_fd_set;

	FD_ZERO(&read_fd_set);

	FD_SET(thread->client_sock, &read_fd_set);

	/*-----------------------------------------------------------------*/
	/*                                                                 */
	/*-----------------------------------------------------------------*/

	bigbox_http_response_t response;

	response.content_type = "text/plain";
	response.content_buff = ((((NULL))));
	response.content_size = ((((0x00))));

	response.done_handler_ptr = NULL;
	response.done_handler_arg = NULL;

	/*-----------------------------------------------------------------*/
	/*                                                                 */
	/*-----------------------------------------------------------------*/

	bigbox_http_request_t request;

	request.method = BIGBOX_HTTP_METHOD_UNKNOWN;

	request.resource_path = in_resource_path;
	request.content_type = in_content_type;

	request.nb_of_params = 0;

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
			q = strstr(p = buffer + 4, " HTTP");

			if(q != NULL)
			{
				strncpy(in_resource_path, p, q - p)[q - p] = '\0';

				request.method = BIGBOX_HTTP_METHOD_GET;
			}
		}

		/*---------------------------------------------------------*/

		else if(strncmp(buffer, "POST ", 5) == 0)
		{
			q = strstr(p = buffer + 5, " HTTP");

			if(q != NULL)
			{
				strncpy(in_resource_path, p, q - p)[q - p] = '\0';

				request.method = BIGBOX_HTTP_METHOD_POST;
			}
		}

		/*---------------------------------------------------------*/

		else if(strncmp(buffer, "PUT ", 4) == 0)
		{
			q = strstr(p = buffer + 4, " HTTP");

			if(q != NULL)
			{
				strncpy(in_resource_path, p, q - p)[q - p] = '\0';

				request.method = BIGBOX_HTTP_METHOD_PUT;
			}
		}

		/*---------------------------------------------------------*/

		else if(strncmp(buffer, "DELETE ", 7) == 0)
		{
			q = strstr(p = buffer + 7, " HTTP");

			if(q != NULL)
			{
				strncpy(in_resource_path, p, q - p)[q - p] = '\0';

				request.method = BIGBOX_HTTP_METHOD_DEL;
			}
		}

		/*---------------------------------------------------------*/

		else if(strncasecmp(buffer, "Content-Type: ", 14) == 0)
		{
			strcpy(in_content_type, buffer + 14);
		}

		/*---------------------------------------------------------*/

		else if(strncasecmp(buffer, "Origin: ", 8) == 0)
		{
			strcpy(in_origin, buffer + 8);
		}

		/*---------------------------------------------------------*/
	}

	/*-----------------------------------------------------------------*/

	if(thread->user_handler_ptr != NULL)
	{
		/*---------------------------------------------------------*/
		/*                                                         */
		/*---------------------------------------------------------*/

		char *params = strchr(in_resource_path, '?');

		if(params != NULL)
		{
			*params++ = '\0';

			request.nb_of_params += __deserialize_params(request.param_array + request.nb_of_params, params);
		}

		/*---------------------------------------------------------*/
		/*                                                         */
		/*---------------------------------------------------------*/

		ssize_t in_content_incr = 0;
		ssize_t in_content_size = 0;

		char *in_content_buff = NULL;

		/*---------------------------------------------------------*/

		for(;;)
		{
			/*-------------------------------------------------*/

			select(thread->client_sock + 1, &read_fd_set, NULL, NULL, &_timeout);

			if(FD_ISSET(thread->client_sock, &read_fd_set) == 0)
			{
				break;
			}

			/*-------------------------------------------------*/

			in_content_buff = (char *) realloc((void *) in_content_buff, in_content_size + 256 + 1);

			if(in_content_buff == NULL)
			{
				in_content_size = 0x00;

				break;
			}

			/*-------------------------------------------------*/

			in_content_incr = bigbox_rio_read(thread->client_sock, in_content_buff + in_content_size, 256);

			if(in_content_incr < 0)
			{
				free((void *) in_content_buff);

				in_content_buff = NULL;
				in_content_size = 0x00;

				break;
			}

			if(in_content_incr == 0)
			{
				break;
			}

			/*-------------------------------------------------*/

			in_content_size += in_content_incr;

			/*-------------------------------------------------*/
		}

		/*---------------------------------------------------------*/

		if(in_content_buff != NULL)
		{
			in_content_buff[in_content_size] = '\0';

			if(strcmp(in_content_type, "application/x-www-form-urlencoded") == 0)
			{
				request.nb_of_params += __deserialize_params(request.param_array + request.nb_of_params, in_content_buff);

				in_content_size = 0x00;
			}
		}

		/*---------------------------------------------------------*/
		/*                                                         */
		/*---------------------------------------------------------*/

		request.content_buff = in_content_buff;
		request.content_size = in_content_size;

		((bigbox_http_handler_ptr_t) thread->user_handler_ptr)(
			&response,
			&request
		);

		/*---------------------------------------------------------*/
		/*                                                         */
		/*---------------------------------------------------------*/

		if(in_content_buff != NULL)
		{
			free((void *) in_content_buff);
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
		response.content_type,
		response.content_size,
		in_origin
	);

	/*-----------------------------------------------------------------*/

	ret = bigbox_rio_write(thread->client_sock, buffer, strlen(buffer));

	if(ret < 0)
	{
		bigbox_log(LOG_TYPE_ERROR, "could not send data!\n");
	}

	/*-----------------------------------------------------------------*/

	if(response.content_buff != NULL)
	{
		ret = bigbox_rio_write(thread->client_sock, response.content_buff, response.content_size);

		if(ret < 0)
		{
			bigbox_log(LOG_TYPE_ERROR, "could not send data!\n");
		}
	}

	/*-----------------------------------------------------------------*/
	/*                                                                 */
	/*-----------------------------------------------------------------*/

	if(response.done_handler_ptr != NULL)
	{
		response.done_handler_ptr(response.done_handler_arg);
	}

	/*-----------------------------------------------------------------*/
}

/*-------------------------------------------------------------------------*/

int bigbox_http_loop(bigbox_server_ctx_t *server_ctx, bigbox_pooler_ctx_t *pooler_ctx, bigbox_http_handler_ptr_t handler_ptr, int nb_of_threads)
{
	return bigbox_server_pooler(server_ctx, pooler_ctx, __loop_handler, handler_ptr, nb_of_threads);
}

/*-------------------------------------------------------------------------*/
