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
#include <stdlib.h>

#if !defined(_WIN32) && !defined(_WIN64)
  #include <sys/socket.h>

  #include <arpa/inet.h>
#else
  #include <winsock2.h>
#endif

#include "internal.h"

/*-------------------------------------------------------------------------*/

static void *__handler(void *args)
{
	bigbox_server_thread_t *thread = (bigbox_server_thread_t *) args;

	/*-----------------------------------------------------------------*/
	/* THREAD LOOP                                                     */
	/*-----------------------------------------------------------------*/

	pthread_mutex_lock(&thread->pooler_ctx->mutex);

	for(;;)
	{
		/**/	/*-------------------------------------------------*/
		/**/	/* WAIT FOR NEW CLIENT                             */
		/**/	/*-------------------------------------------------*/
		/**/
		/**/	while(thread->pooler_ctx->client_sock < 0)
		/**/	{
		/**/		pthread_cond_wait(&thread->pooler_ctx->new_client, &thread->pooler_ctx->mutex);
		/**/
		/**/		if(thread->pooler_ctx->alive == false)
		/**/		{
		/**/			goto __bye;
		/**/		}
		/**/	}
		/**/
		/**/	/*-------------------------------------------------*/
		/**/	/* SET CLIENT SOCK                                 */
		/**/	/*-------------------------------------------------*/
		/**/
		/**/	thread->client_sock = thread->pooler_ctx->client_sock;
		/**/
		/**/	/*-------------------------------------------------*/
		/**/	/* UPDATE STATE                                    */
		/**/	/*-------------------------------------------------*/
		/**/
		/**/	thread->pooler_ctx->nb_of_free_thread -= 1;
		/**/	thread->pooler_ctx->client_sock = -1;
		/**/
		/**/	/*-------------------------------------------------*/

		pthread_mutex_unlock(&thread->pooler_ctx->mutex);

		/*---------------------------------------------------------*/
		/* LOOP                                                    */
		/*---------------------------------------------------------*/

		if(thread->loop_handler_ptr != NULL)
		{
			thread->loop_handler_ptr(thread);
		}

		/*---------------------------------------------------------*/
		/* CLOSE CLIENT SOCK                                       */
		/*---------------------------------------------------------*/

		bigbox_rio_close(thread->client_sock);

		/*---------------------------------------------------------*/

		pthread_mutex_lock(&thread->pooler_ctx->mutex);

		/**/	/*-------------------------------------------------*/
		/**/	/* UPDATE STATE                                    */
		/**/	/*-------------------------------------------------*/
		/**/
		/**/	thread->pooler_ctx->nb_of_free_thread += 1;
		/**/
		/**/	/*-------------------------------------------------*/
		/**/	/* SIGNAL FREE THREAD                              */
		/**/	/*-------------------------------------------------*/
		/**/
		/**/	pthread_cond_signal(&thread->pooler_ctx->free_thread);

		/*---------------------------------------------------------*/
	}
__bye:
	pthread_mutex_unlock(&thread->pooler_ctx->mutex);

	pthread_exit(0);
}

/*-------------------------------------------------------------------------*/

void bigbox_server_pooler(bigbox_server_ctx_t *server_ctx, bigbox_pooler_ctx_t *pooler_ctx, void (* loop_handler_ptr)(bigbox_server_thread_t *), void *user_handler_ptr, int nb_of_threads)
{
	int ret;

	/*-----------------------------------------------------------------*/
	/* ?????                                                           */
	/*-----------------------------------------------------------------*/

	pooler_ctx->alive = 1;

	pooler_ctx->client_sock = -1;

	pooler_ctx->nb_of_free_thread = nb_of_threads;

	/*-----------------------------------------------------------------*/
	/* INIT MUTEX                                                      */
	/*-----------------------------------------------------------------*/

	ret = pthread_mutex_init(&pooler_ctx->mutex, NULL);

	if(ret < 0)
	{
		bigbox_log(LOG_TYPE_FATAL, "could not create pmutex!\n");
	}

	/*-----------------------------------------------------------------*/
	/* INIT CONDITION                                                  */
	/*-----------------------------------------------------------------*/

	ret = pthread_cond_init(&pooler_ctx->new_client, NULL);

	if(ret < 0)
	{
		bigbox_log(LOG_TYPE_FATAL, "could not create pcond!\n");
	}

	/*-----------------------------------------------------------------*/
	/* INIT CONDITION                                                  */
	/*-----------------------------------------------------------------*/

	ret = pthread_cond_init(&pooler_ctx->free_thread, NULL);

	if(ret < 0)
	{
		bigbox_log(LOG_TYPE_FATAL, "could not create pcond!\n");
	}

	/*-----------------------------------------------------------------*/
	/* INITIALIZE THREAD POOL                                          */
	/*-----------------------------------------------------------------*/

	bigbox_server_thread_t *thread_array = malloc(nb_of_threads * sizeof(bigbox_server_thread_t));

	if(thread_array == NULL)
	{
		bigbox_log(LOG_TYPE_FATAL, "out of memory!\n");
	}

	/*-----------------------------------------------------------------*/

	int i;

	for(i = 0; i < nb_of_threads; i++)
	{
		bigbox_server_thread_t *thread = &thread_array[i];

		/*---------------------------------------------------------*/
		/* INIT THREAD                                             */
		/*---------------------------------------------------------*/

		thread->server_ctx = server_ctx;
		thread->pooler_ctx = pooler_ctx;

		thread->loop_handler_ptr = loop_handler_ptr;
		thread->user_handler_ptr = user_handler_ptr;

		/*---------------------------------------------------------*/
		/* INIT PTHREAD                                            */
		/*---------------------------------------------------------*/

		ret = pthread_create(&thread->pthread, NULL, &__handler, thread);

		if(ret < 0)
		{
			bigbox_log(LOG_TYPE_FATAL, "could not create pthread!\n");
		}

		/*---------------------------------------------------------*/
	}

	/*-----------------------------------------------------------------*/
	/* SERVER LOOP                                                     */
	/*-----------------------------------------------------------------*/

	pthread_mutex_lock(&pooler_ctx->mutex);

	for(;;)
	{
		/*---------------------------------------------------------*/
		/* WAIT FOR FREE THREAD                                    */
		/*---------------------------------------------------------*/

		/**/	while(pooler_ctx->nb_of_free_thread == 0)
		/**/	{
		/**/		pthread_cond_wait(&pooler_ctx->free_thread, &pooler_ctx->mutex);
		/**/	}

		pthread_mutex_unlock(&pooler_ctx->mutex);

		/*---------------------------------------------------------*/
		/* ACCEPT CONNECTION                                       */
		/*---------------------------------------------------------*/

		SOCKADDR_IN sockaddr;

		int socklen = sizeof(sockaddr);

		int client_sock = accept(server_ctx->sock, (SOCKADDR *) &sockaddr, (socklen_t *) &socklen);

		if(pooler_ctx->alive == false)
		{
			goto __bye;
		}

		/*---------------------------------------------------------*/
		/* CHECK STATUS                                            */
		/*---------------------------------------------------------*/

		if(client_sock > 0)
		{
			bigbox_log(LOG_TYPE_DEBUG, "connection from %d.%d.%d.%d\n",
				(int) ((sockaddr.sin_addr.s_addr & 0xFF) >> 0),
				(int) ((sockaddr.sin_addr.s_addr & 0xFF00) >> 8),
				(int) ((sockaddr.sin_addr.s_addr & 0xFF0000) >> 16),
				(int) ((sockaddr.sin_addr.s_addr & 0xFF000000) >> 24)
			);
		}
		else
		{
			if(errno != EINTR && errno != ECONNABORTED)
			{
				bigbox_log(LOG_TYPE_ERROR, "could not accept client!\n");
			}

			pthread_mutex_lock(&pooler_ctx->mutex);

			continue;
		}

		/*---------------------------------------------------------*/

		pthread_mutex_lock(&pooler_ctx->mutex);

		/**/	/*-------------------------------------------------*/
		/**/	/* SET CLIENT SOCK                                 */
		/**/	/*-------------------------------------------------*/
		/**/
		/**/	pooler_ctx->client_sock = client_sock;
		/**/
		/**/	/*-------------------------------------------------*/
		/**/	/* SIGNAL NEW CLIENT                               */
		/**/	/*-------------------------------------------------*/
		/**/
		/**/	pthread_cond_signal(&pooler_ctx->new_client);

		/*---------------------------------------------------------*/
	}

	/*-----------------------------------------------------------------*/
	/* FINALIZE THREAD POOL                                            */
	/*-----------------------------------------------------------------*/
__bye:
	pthread_mutex_lock(&pooler_ctx->mutex);
	
	/**/	pthread_cond_broadcast(&pooler_ctx->new_client);

	pthread_mutex_unlock(&pooler_ctx->mutex);

	/*-----------------------------------------------------------------*/

	for(i = 0; i < nb_of_threads; i++)
	{
		pthread_join(thread_array[i].pthread, NULL);
	}

	/*-----------------------------------------------------------------*/

	free(thread_array);

	/*-----------------------------------------------------------------*/
}

/*-------------------------------------------------------------------------*/
