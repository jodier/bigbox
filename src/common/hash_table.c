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

#include "internal.h"

/*-------------------------------------------------------------------------*/

void bigbox_hash_table_init(bigbox_hash_table_t *hash_table, size_t dim)
{
	void *table = malloc(dim * sizeof(struct bigbox_hash_table_item_s));

	if(table == NULL)
	{
		bigbox_log(LOG_TYPE_FATAL, "out of memory!\n");
	}

	hash_table->dim = dim;
	hash_table->table = table;
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
