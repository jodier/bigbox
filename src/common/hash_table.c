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

#include <stdlib.h>
#include <string.h>

#include "internal.h"

/*-------------------------------------------------------------------------*/

static const uint64_t _seed1 = 0x99;
static const uint64_t _seed2 = 0xAA;

/*-------------------------------------------------------------------------*/

bool bigbox_hash_table_init(bigbox_hash_table_t *hash_table, size_t dim)
{
	size_t size = dim * sizeof(buff_t);

	buff_t table = malloc(size);

	if(table != NULL)
	{
		hash_table->dim = dim;
		hash_table->list = NULL;
		hash_table->table = table;

		memset(table, 0x00000000, size);

		pthread_mutex_init(&hash_table->mutex, NULL);
	
		return true;
	}
	else
	{
		return false;
	}
}

/*-------------------------------------------------------------------------*/

void bigbox_hash_table_final(bigbox_hash_table_t *hash_table)
{
	hash_table->dim = 0x00;

	if(hash_table->table != NULL)
	{
		free(hash_table->table);

		hash_table->table = NULL;
	}
}

/*-------------------------------------------------------------------------*/

bool bigbox_hash_table_put_by_hash(bigbox_hash_table_t *hash_table, uint64_t hash, buff_t buff, size_t size)
{
	/*-----------------------------------------------------------------*/

	struct bigbox_hash_table_item_s *item = malloc(sizeof(struct bigbox_hash_table_item_s) + size);

	if(item == NULL)
	{
		return false;
	}

	item->hash = hash;
	memcpy(item->buff, buff, size);
	item->size = size;

	/*-----------------------------------------------------------------*/

	size_t indx = hash % hash_table->dim;

	/*-----------------------------------------------------------------*/

	pthread_mutex_lock(&hash_table->mutex);

	/**/	item->next = hash_table->table[indx];
	/**/
	/**/	hash_table->table[indx] = (((item)));

	pthread_mutex_unlock(&hash_table->mutex);

	/*-----------------------------------------------------------------*/

	return true;
}

/*-------------------------------------------------------------------------*/

bool bigbox_hash_table_put(bigbox_hash_table_t *hash_table, const char *key, buff_t buff, size_t size)
{
	uint64_t hash = bigbox_hash(key, strlen(key), _seed1, _seed2);

	return bigbox_hash_table_put_by_hash(hash_table, hash, buff, size);
}

/*-------------------------------------------------------------------------*/

bool bigbox_hash_table_get_by_hash(bigbox_hash_table_t *hash_table, uint64_t hash, buff_t *buff, size_t *size)
{
	/*-----------------------------------------------------------------*/

	struct bigbox_hash_table_item_s *item = hash_table->table[hash % hash_table->dim];

	/*-----------------------------------------------------------------*/

	pthread_mutex_lock(&hash_table->mutex);

	/**/	for(;;)
	/**/	{
	/**/		/*-------------------------------------------------*/
	/**/
	/**/		if(item->hash == hash)
	/**/		{
	/**/			if(buff) {
	/**/				*buff = item->buff;
	/**/			}
	/**/
	/**/			if(size) {
	/**/				*size = item->size;
	/**/			}
	/**/
	/**/			pthread_mutex_unlock(&hash_table->mutex);
	/**/
	/**/			return true;
	/**/		}
	/**/
	/**/		/*-------------------------------------------------*/
	/**/
	/**/		if(item->next == NULL)
	/**/		{
	/**/			if(buff) {
	/**/				*buff = NULL;
	/**/			}
	/**/
	/**/			if(size) {
	/**/				*size = 0x00;
	/**/			}
	/**/
	/**/			pthread_mutex_unlock(&hash_table->mutex);
	/**/
	/**/			return false;
	/**/		}
	/**/
	/**/		/*-------------------------------------------------*/
	/**/
	/**/		item = item->next;
	/**/
	/**/		/*-------------------------------------------------*/
	/**/	}


	/*-----------------------------------------------------------------*/
}

/*-------------------------------------------------------------------------*/

bool bigbox_hash_table_get(bigbox_hash_table_t *hash_table, const char *key, buff_t *buff, size_t *size)
{
	uint64_t hash = bigbox_hash(key, strlen(key), _seed1, _seed2);

	return bigbox_hash_table_get_by_hash(hash_table, hash, buff, size);
}

/*-------------------------------------------------------------------------*/

bool bigbox_hash_table_del_by_hash(bigbox_hash_table_t *hash_table, uint64_t hash)
{
	/*-----------------------------------------------------------------*/

	struct bigbox_hash_table_item_s *item = hash_table->table[hash % hash_table->dim], *prev = item;

	/*-----------------------------------------------------------------*/

	pthread_mutex_lock(&hash_table->mutex);

	/**/	for(;;)
	/**/	{
	/**/		/*-------------------------------------------------*/
	/**/
	/**/		if(item->hash == hash)
	/**/		{
	/**/			prev->next = item->next;
	/**/
	/**/			pthread_mutex_unlock(&hash_table->mutex);
	/**/
	/**/			free((void *) item);
	/**/
	/**/			return true;
	/**/		}
	/**/
	/**/		/*-------------------------------------------------*/
	/**/
	/**/		if(item->next == NULL)
	/**/		{
	/**/			pthread_mutex_unlock(&hash_table->mutex);
	/**/
	/**/			return false;
	/**/		}
	/**/
	/**/		/*-------------------------------------------------*/
	/**/
	/**/		item = (prev = item)->next;
	/**/
	/**/		/*-------------------------------------------------*/
	/**/	}

	/*-----------------------------------------------------------------*/
}

/*-------------------------------------------------------------------------*/

bool bigbox_hash_table_del(bigbox_hash_table_t *hash_table, const char *key)
{
	uint64_t hash = bigbox_hash(key, strlen(key), _seed1, _seed2);

	return bigbox_hash_table_del_by_hash(hash_table, hash);
}

/*-------------------------------------------------------------------------*/
