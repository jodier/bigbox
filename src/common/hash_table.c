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

#define SEED1 0x38F9D3B5DE1C7094ULL
#define SEED2 0x549CAD91603F99F6ULL

/*-------------------------------------------------------------------------*/

bool bigbox_hash_table_initialize(bigbox_hash_table_t *hash_table, size_t dim)
{
	size_t size = dim * sizeof(buff_t);

	buff_t table = malloc(size);

	if(table != NULL)
	{
		hash_table->dim = dim;
		hash_table->list = NULL;
		hash_table->table = table;

		memset(table, 0, size);

		pthread_mutex_init(&hash_table->mutex, NULL);
	
		return true;
	}
	else
	{
		return false;
	}
}

/*-------------------------------------------------------------------------*/

bool bigbox_hash_table_finalize(bigbox_hash_table_t *hash_table)
{
	/*-----------------------------------------------------------------*/

	bigbox_hash_table_item_t *hash_table_item;

	while((hash_table_item = bigbox_list_get_tail_named(hash_table->list, list_prev, list_next)) != NULL)
	{
		bigbox_list_remove_named(hash_table->list, hash_table_item, list_prev, list_next);

		free(hash_table_item);
	}

	/*-----------------------------------------------------------------*/

	hash_table->dim = 0x00;

	if(hash_table->table != NULL)
	{
		free(hash_table->table);

		hash_table->table = NULL;
	}

	/*-----------------------------------------------------------------*/
	
	return true;
}

/*-------------------------------------------------------------------------*/

void bigbox_hash_table_item_release_unsafe(bigbox_hash_table_t *hash_table, bigbox_hash_table_item_t *hash_table_item, size_t indx)
{
	/**/	if(--hash_table_item->refcnt < 1)
	/**/	{
	/**/		bigbox_list_remove_named(hash_table->list, hash_table_item, list_prev, list_next);
	/**/
	/**/		bigbox_list_remove_named(hash_table->table[indx], hash_table_item, table_prev, table_next);
	/**/
	/**/		free(hash_table_item);
	/**/	}
}

/*-------------------------------------------------------------------------*/

void bigbox_hash_table_item_release(bigbox_hash_table_t *hash_table, bigbox_hash_table_item_t *hash_table_item)
{
	/*-----------------------------------------------------------------*/

	int indx = hash_table_item->hash
	           %
		   hash_table->dim
	;

	/*-----------------------------------------------------------------*/

	pthread_mutex_lock(&hash_table->mutex);

	/**/	bigbox_hash_table_item_release_unsafe(hash_table, hash_table_item, indx);

	pthread_mutex_unlock(&hash_table->mutex);

	/*-----------------------------------------------------------------*/
}

/*-------------------------------------------------------------------------*/

bool bigbox_hash_table_put_by_hash(bigbox_hash_table_t *hash_table, uint64_t hash, buff_t buff, size_t size, uint32_t expire)
{
	if(buff == NULL
	   ||
	   size == 0x00
	 ) {
		return true;
	}

	/*-----------------------------------------------------------------*/

	bigbox_hash_table_item_t *hash_table_item = malloc(sizeof(bigbox_hash_table_item_t) + size);

	if(hash_table_item == NULL)
	{
		bigbox_log(LOG_TYPE_FATAL, "out of memory!\n");

		return false;
	}

	hash_table_item->refcnt = 0x0001;
	hash_table_item->expire = expire;

	hash_table_item->hash = hash;
	hash_table_item->buff = memcpy(hash_table_item + 1, buff, size);
	hash_table_item->size = size;

	/*-----------------------------------------------------------------*/

	size_t indx = hash % hash_table->dim;

	/*-----------------------------------------------------------------*/

	pthread_mutex_lock(&hash_table->mutex);

	/**/	bigbox_list_append_tail_named(hash_table->list, hash_table_item, list_prev, list_next);
	/**/
	/**/	bigbox_list_append_tail_named(hash_table->table[indx], hash_table_item, table_prev, table_next);

	pthread_mutex_unlock(&hash_table->mutex);

	/*-----------------------------------------------------------------*/

	return true;
}

/*-------------------------------------------------------------------------*/

bool bigbox_hash_table_put(bigbox_hash_table_t *hash_table, const char *key, buff_t buff, size_t size, uint32_t expire)
{
	uint64_t hash = bigbox_hash(key, strlen(key), SEED1, SEED2);

	return bigbox_hash_table_put_by_hash(hash_table, hash, buff, size, expire);
}

/*-------------------------------------------------------------------------*/

bool bigbox_hash_table_get_by_hash(bigbox_hash_table_t *hash_table, uint64_t hash, bigbox_hash_table_item_t **result)
{
	/*-----------------------------------------------------------------*/

	size_t indx = hash % hash_table->dim;

	/*-----------------------------------------------------------------*/

	bigbox_hash_table_item_t *hash_table_list = hash_table->table[indx], *hash_table_item;

	/*-----------------------------------------------------------------*/

	pthread_mutex_lock(&hash_table->mutex);

	/**/	if(hash_table_list != NULL)
	/**/	{
	/**/		int i;
	/**/
	/**/		for(hash_table_item = hash_table_list, i = 0; (i == 0) || (hash_table_item != hash_table_list); hash_table_item = hash_table_item->list_next)
	/**/		{
	/**/			if(hash_table_item->hash == hash)
	/**/			{
	/**/				goto _found;
	/**/			}
	/**/		}
	/**/	}

	/*-----------------------------------------------------------------*/

	/**/	if(result != NULL)
	/**/	{
	/**/		/***********************/;
	/**/
	/**/		*result = 0x0000000000000;
	/**/	}

	pthread_mutex_unlock(&hash_table->mutex);

	return false;

	/*-----------------------------------------------------------------*/
_found:
	/**/	if(result != NULL)
	/**/	{
	/**/		hash_table_item->refcnt++;
	/**/
	/**/		*result = hash_table_item;
	/**/	}

	pthread_mutex_unlock(&hash_table->mutex);

	return true;

	/*-----------------------------------------------------------------*/
}

/*-------------------------------------------------------------------------*/

bool bigbox_hash_table_get(bigbox_hash_table_t *hash_table, const char *key, bigbox_hash_table_item_t **result)
{
	uint64_t hash = bigbox_hash(key, strlen(key), SEED1, SEED2);

	return bigbox_hash_table_get_by_hash(hash_table, hash, result);
}

/*-------------------------------------------------------------------------*/

bool bigbox_hash_table_del_by_hash(bigbox_hash_table_t *hash_table, uint64_t hash)
{
	/*-----------------------------------------------------------------*/

	size_t indx = hash % hash_table->dim;

	/*-----------------------------------------------------------------*/

	bigbox_hash_table_item_t *hash_table_list = hash_table->table[indx], *hash_table_item;

	/*-----------------------------------------------------------------*/

	pthread_mutex_lock(&hash_table->mutex);

	/**/	if(hash_table_list != NULL)
	/**/	{
	/**/		int i;
	/**/
	/**/		for(hash_table_item = hash_table_list, i = 0; (i == 0) || (hash_table_item != hash_table_list); hash_table_item = hash_table_item->list_next)
	/**/		{
	/**/			if(hash_table_item->hash == hash)
	/**/			{
	/**/				goto _found;
	/**/			}
	/**/		}
	/**/	}

	/*-----------------------------------------------------------------*/

	/**/	/**********************************************************************/;

	pthread_mutex_unlock(&hash_table->mutex);

	return false;

	/*-----------------------------------------------------------------*/
_found:
	/**/	bigbox_hash_table_item_release_unsafe(hash_table, hash_table_item, indx);

	pthread_mutex_unlock(&hash_table->mutex);

	return true;

	/*-----------------------------------------------------------------*/
}

/*-------------------------------------------------------------------------*/

bool bigbox_hash_table_del(bigbox_hash_table_t *hash_table, const char *key)
{
	uint64_t hash = bigbox_hash(key, strlen(key), SEED1, SEED2);

	return bigbox_hash_table_del_by_hash(hash_table, hash);
}

/*-------------------------------------------------------------------------*/
