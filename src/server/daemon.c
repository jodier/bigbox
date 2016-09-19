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

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "internal.h"

/*-------------------------------------------------------------------------*/

int main(int argc, char **argv)
{
	/*-----------------------------------------------------------------*/

	int port = 6969, c;

	static struct option long_options[] = {
		{"port", 1, 0, 'p'},
		{"version", 0, 0, 'v'},
		{"help", 0, 0, 'h'},
		{NULL, 0, NULL, 0}
	};

	while((c = getopt_long(argc, argv, "p:vh", long_options, NULL)) != -1)
	{
		switch(c)
		{
			case 'p':
				port = atoi(optarg);
				break;

			case 'v':
				return 0;

			case 'h':
				return 0;

			default:
				return 1;
		}
	}

	/*-----------------------------------------------------------------*/

	printf("Starting BigBox on port %d...\n", port);

	/*-----------------------------------------------------------------*/

	return 0;
}

/*-------------------------------------------------------------------------*/
