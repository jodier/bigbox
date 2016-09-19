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

#include "internal.h"

/*-------------------------------------------------------------------------*/

static pthread_mutex_t _mutex = PTHREAD_MUTEX_INITIALIZER;

/*-------------------------------------------------------------------------*/

int _bigbox_vlog(log_type_t type, const char *function_name, const char *format, va_list ap)
{
	pthread_mutex_lock(&_mutex);

	/**/	switch(type)
	/**/	{
	/**/		case LOG_TYPE_DEBUG:
	/**/			fprintf(stderr, "DEBUG, %s() - ", function_name);
	/**/			break;
	/**/
	/**/		case LOG_TYPE_OOOPS:
	/**/			fprintf(stderr, "OOOPS, %s() - ", function_name);
	/**/			break;
	/**/
	/**/		case LOG_TYPE_ERROR:
	/**/			fprintf(stderr, "ERROR, %s() - ", function_name);
	/**/			break;
	/**/
	/**/		case LOG_TYPE_FATAL:
	/**/			fprintf(stderr, "FATAL, %s() - ", function_name);
	/**/			break;
	/**/
	/**/		default:
	/**/			pthread_mutex_unlock(&_mutex);
	/**/
	/**/			return -EINVAL;
	/**/	}
	/**/
	/**/	vfprintf(stderr, format, ap);

	pthread_mutex_unlock(&_mutex);

	if(type == LOG_TYPE_FATAL)
	{
		exit(1);
	}

	return 0;
}

/*-------------------------------------------------------------------------*/

int _bigbox_log(log_type_t type, const char *function_name, const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	int ret = _bigbox_vlog(type, function_name, format, ap);
	va_end(ap);

	return ret;
}

/*-------------------------------------------------------------------------*/
