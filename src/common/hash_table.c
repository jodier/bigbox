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

#define SEED1 0x38F9D3B5De1C7094ULL
#define SEED2 0x549CAD91603F99F6ULL

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
	/*-----------------------------------------------------------------*/
	/*                                                                 */
	/*-----------------------------------------------------------------*/

	bigbox_hash_table_item_t *item;

	while((item = bigbox_list_get_tail_named(hash_table->list, list_prev, list_next)) != NULL)
	{
		bigbox_list_remove_named(hash_table->list, item, list_prev, list_next);

		free(item);
	}

	/*-----------------------------------------------------------------*/
	/*                                                                 */
	/*-----------------------------------------------------------------*/

	hash_table->dim = 0x00;

	if(hash_table->table != NULL)
	{
		free(hash_table->table);

		hash_table->table = NULL;
	}

	/*-----------------------------------------------------------------*/
}

/*-------------------------------------------------------------------------*/

bool bigbox_hash_table_put_by_hash(bigbox_hash_table_t *hash_table, uint64_t hash, buff_t buff, size_t size)
{
	/*-----------------------------------------------------------------*/

	bigbox_hash_table_item_t *item = malloc(sizeof(bigbox_hash_table_item_t) + size);

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

	/**/	bigbox_list_append_tail_named(hash_table->list, item, list_prev, list_next);
	/**/
	/**/	item->table_next = hash_table->table[indx];
	/**/
	/**/	hash_table->table[indx] = (((item)));

	pthread_mutex_unlock(&hash_table->mutex);

	/*-----------------------------------------------------------------*/

	return true;
}

/*-------------------------------------------------------------------------*/

bool bigbox_hash_table_put(bigbox_hash_table_t *hash_table, const char *key, buff_t buff, size_t size)
{
	uint64_t hash = bigbox_hash(key, strlen(key), SEED1, SEED2);

	return bigbox_hash_table_put_by_hash(hash_table, hash, buff, size);
}

/*-------------------------------------------------------------------------*/

bool bigbox_hash_table_get_by_hash(bigbox_hash_table_t *hash_table, uint64_t hash, bigbox_hash_table_item_t **result)
{
	/*-----------------------------------------------------------------*/

	bigbox_hash_table_item_t *item = hash_table->table[hash % hash_table->dim];

	/*-----------------------------------------------------------------*/

	pthread_mutex_lock(&hash_table->mutex);

	/**/	for(;;)
	/**/	{
	/**/		/*-------------------------------------------------*/
	/**/
	/**/		if(item->hash == hash)
	/**/		{
	/**/			if(result)
	/**/			{
	/**/				*result = item;
	/**/			}
	/**/
	/**/			pthread_mutex_unlock(&hash_table->mutex);
	/**/
	/**/			return true;
	/**/		}
	/**/
	/**/		/*-------------------------------------------------*/
	/**/
	/**/		if(item->table_next == NULL)
	/**/		{
	/**/			if(result)
	/**/			{
	/**/				*result = NULL;
	/**/			}
	/**/
	/**/			pthread_mutex_unlock(&hash_table->mutex);
	/**/
	/**/			return false;
	/**/		}
	/**/
	/**/		/*-------------------------------------------------*/
	/**/
	/**/		item = item->table_next;
	/**/
	/**/		/*-------------------------------------------------*/
	/**/	}

	/*-----------------------------------------------------------------*/
}

/*-------------------------------------------------------------------------*/

bool bigbox_hash_table_get(bigbox_hash_table_t *hash_table, const char *key, bigbox_hash_table_item_t **result)
{
	uint64_t hash = bigbox_hash(key, strlen(key), SEED1, SEED2);

	return bigbox_hash_table_get_by_hash(hash_table, hash, result);
}

/*-------------------------------------------------------------------------*/

bool bigbox_hash_table_del_by_hash(bigbox_hash_table_t *hash_table, uint64_t hash, bigbox_hash_table_item_t **result)
{
	/*-----------------------------------------------------------------*/

	bigbox_hash_table_item_t *item = hash_table->table[hash % hash_table->dim], *prev = item;

	/*-----------------------------------------------------------------*/

	pthread_mutex_lock(&hash_table->mutex);

	/**/	for(;;)
	/**/	{
	/**/		/*-------------------------------------------------*/
	/**/
	/**/		if(item->hash == hash)
	/**/		{
	/**/			/*-----------------------------------------*/
	/**/
	/**/			bigbox_list_remove_named(hash_table->list, item, list_prev, list_next);
	/**/
	/**/			prev->table_next = item->table_next;
	/**/
	/**/			item->table_next = NULL;
	/**/
	/**/			/*-----------------------------------------*/
	/**/
	/**/			if(item != NULL)
	/**/			{
	/**/				*result = item;
	/**/			}
	/**/
	/**/			pthread_mutex_unlock(&hash_table->mutex);
	/**/
	/**/			return true;
	/**/		}
	/**/
	/**/		/*-------------------------------------------------*/
	/**/
	/**/		if(item->table_next == NULL)
	/**/		{
	/**/			/*-----------------------------------------*/
	/**/
	/**/			/* NOT FOUND, NOT FOUND, NOT FOUND, NOT FOUND, NOT FOUND, NOT FOUND */
	/**/
	/**/			/* NOT FOUND, NOT FOUND, NOT FOUND */
	/**/
	/**/			/* NOT FOUND, NOT FOUND */
	/**/
	/**/			/*-----------------------------------------*/
	/**/
	/**/			if(item != NULL)
	/**/			{
	/**/				*result = NULL;
	/**/			}
	/**/
	/**/			pthread_mutex_unlock(&hash_table->mutex);
	/**/
	/**/			return false;
	/**/		}
	/**/
	/**/		/*-------------------------------------------------*/
	/**/
	/**/		item = (prev = item)->table_next;
	/**/
	/**/		/*-------------------------------------------------*/
	/**/	}

	/*-----------------------------------------------------------------*/
}

/*-------------------------------------------------------------------------*/

bool bigbox_hash_table_del(bigbox_hash_table_t *hash_table, const char *key, bigbox_hash_table_item_t **result)
{
	uint64_t hash = bigbox_hash(key, strlen(key), SEED1, SEED2);

	return bigbox_hash_table_del_by_hash(hash_table, hash, result);
}

/*-------------------------------------------------------------------------*/
