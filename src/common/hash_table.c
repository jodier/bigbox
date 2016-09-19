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

void bigbox_hash_table_init(bigbox_hash_table_t *hash_table, size_t dim)
{
	/*-----------------------------------------------------------------*/

	size_t size = dim * sizeof(void *);

	void *table = malloc(size);

	if(table == NULL)
	{
		bigbox_log(LOG_TYPE_FATAL, "out of memory!\n");
	}

	memset(table, 0x00, size);

	/*-----------------------------------------------------------------*/

	hash_table->dim = dim;
	hash_table->table = table;

	/*-----------------------------------------------------------------*/

	pthread_mutex_init(&hash_table->mutex, NULL);

	/*-----------------------------------------------------------------*/
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

void bigbox_hash_table_set_by_hash(bigbox_hash_table_t *hash_table, uint64_t hash, buff_t buff, size_t size)
{
	/*-----------------------------------------------------------------*/

	struct bigbox_hash_table_item_s *item = malloc(sizeof(struct bigbox_hash_table_item_s) + size);

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
}

/*-------------------------------------------------------------------------*/

void bigbox_hash_table_set(bigbox_hash_table_t *hash_table, const char *key, buff_t buff, size_t size)
{
	uint64_t hash = bigbox_hash(key, strlen(key), _seed1, _seed2);

	bigbox_hash_table_set_by_hash(hash_table, hash, buff, size);
}

/*-------------------------------------------------------------------------*/

void bigbox_hash_table_get_by_hash(bigbox_hash_table_t *hash_table, uint64_t hash, buff_t *buff, size_t *size)
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
	/**/			break;
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
	/**/			break;
	/**/		}
	/**/
	/**/		/*-------------------------------------------------*/
	/**/
	/**/		item = item->next;
	/**/
	/**/		/*-------------------------------------------------*/
	/**/	}

	pthread_mutex_unlock(&hash_table->mutex);

	/*-----------------------------------------------------------------*/
}

/*-------------------------------------------------------------------------*/

void bigbox_hash_table_get(bigbox_hash_table_t *hash_table, const char *key, buff_t *buff, size_t *size)
{
	uint64_t hash = bigbox_hash(key, strlen(key), _seed1, _seed2);

	bigbox_hash_table_get_by_hash(hash_table, hash, buff, size);
}

/*-------------------------------------------------------------------------*/
