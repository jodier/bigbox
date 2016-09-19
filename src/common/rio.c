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

#if !defined(_WIN32) && !defined(_WIN64)
  #include <unistd.h>

  #include <sys/time.h>
  #include <sys/socket.h>
#else
  #include <winsock2.h>
#endif

#include "internal.h"

/*-------------------------------------------------------------------------*/

int bigbox_rio_read_timeout(int sock, int seconds)
{
	struct timeval tv;

	tv.tv_sec = seconds;
	tv.tv_usec = 0x00000;

	return setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct timeval));
}

/*-------------------------------------------------------------------------*/

int bigbox_rio_write_timeout(int sock, int seconds)
{
	struct timeval tv;

	tv.tv_sec = seconds;
	tv.tv_usec = 0x00000;

	return setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(struct timeval));
}

/*-------------------------------------------------------------------------*/

ssize_t bigbox_rio_read(int sock, buff_t buff, size_t size)
{
	ssize_t size_done;
	char *ptr = (char *) buff;

__loop:
	size_done = recv(sock, ptr, size, 0);

	if(size_done < 0)
	{
		if(errno == EINTR)
		{
			ptr += size_done;
			size -= size_done;

			goto __loop;
		}
		else
		{
			return -1;
		}
	}

	return (ssize_t) (ptr + size_done) - (ssize_t) buff;
}

/*-------------------------------------------------------------------------*/

ssize_t bigbox_rio_write(int sock, BUFF_t buff, size_t size)
{
	ssize_t size_done;
	char *ptr = (char *) buff;

__loop:
	size_done = send(sock, ptr, size, 0);

	if(size_done < 0)
	{
		if(errno == EINTR)
		{
			ptr += size_done;
			size -= size_done;

			goto __loop;
		}
		else
		{
			return -1;
		}
	}

	return (ssize_t) (ptr + size_done) - (ssize_t) buff;
}

/*-------------------------------------------------------------------------*/

int bigbox_rio_close(int sock)
{
	int ret = 0;

	if(sock >= 0)
	{
		if(shutdown(sock, SHUT_RDWR) < 0)
		{
			ret = -1;
		}

		if(closesocket(sock) < 0)
		{
			ret = -1;
		}
	}

	return ret;
}

/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
#if defined(_WIN32) || defined(_WIN64)
__CONSTRUCTOR(C) static void C(void)
{
	WSADATA wsadata;

	int err = WSAStartup(MAKEWORD(2, 2), &wsadata);

	if(err < 0)
	{

	}
}
#endif
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
#if defined(_WIN32) || defined(_WIN64)
__DESTRUCTOR(D) static void D(void)
{
	WSACleanup();
}
#endif
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
