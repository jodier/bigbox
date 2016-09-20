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

/*!
 * @file bigbox_list.h
 * @brief Circular doubly-linked lists implementation entirely based on C macros (libctnr)
 * @author Jerome ODIER
 *
 * Circular doubly-linked lists implementation entirely based on C macros.
 */

/*-------------------------------------------------------------------------*/

#ifndef __BIGBOX_LIST_H
#define __BIGBOX_LIST_H

/*-------------------------------------------------------------------------*/

/*! @brief Build a BIGBOX list.
 *  @return The new BIGBOX list.
 */

#define BIGBOX_LIST_NEW (NULL)

/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/

/*! @brief Initialize a BIGBOX list.
 *  @param list the list.
 *  @param prev the 1st chaining variable name (prev item).
 *  @param next the 2nt chaining variable name (next item).
 *  @return Nothing.
 */

#define bigbox_list_init_named(list, prev, next)				\
	((list) = NULL)

/*-------------------------------------------------------------------------*/

/*! @brief Initialize a BIGBOX list as a singleton.
 *  @param list the list.
 *  @param item the item to be inserted.
 *  @param prev the 1st chaining variable name (prev item).
 *  @param next the 2nt chaining variable name (next item).
 *  @return Nothing.
 */

#define bigbox_list_init_singleton_named(list, item, prev, next)		\
	((list) = (item)->prev = (item)->next = (item))

/*-------------------------------------------------------------------------*/

/*! @brief Checks whether the list is empty.
 *  @param list the list.
 *  @param prev the 1st chaining variable name (prev item).
 *  @param next the 2nt chaining variable name (next item).
 *  @return 'true' if the list is empty and 'false' if the list is not empty.
 */

#define bigbox_list_is_empty_named(list, prev, next)				\
	((list) == NULL)

/*-------------------------------------------------------------------------*/

/*! @brief Checks whether the list is a singleton.
 *  @param list the list.
 *  @param prev the 1st chaining variable name (prev item).
 *  @param next the 2nt chaining variable name (next item).
 *  @return 'true' if the list is a singleton and 'false' if the list is not a singleton.
 */

#define bigbox_list_is_singleton_named(list, prev, next)			\
	((list) != NULL && (list) == (list)->prev && (list) == (list)->next)

/*-------------------------------------------------------------------------*/

/*! @brief Get the item at the head of the list.
 *  @param list the list.
 *  @param prev the 1st chaining variable name (prev item).
 *  @param next the 2nt chaining variable name (next item).
 *  @return The item at the head of the list.
 */

#define bigbox_list_get_head_named(list, prev, next)				\
	((list) != NULL ? (list) /* */ : NULL)

/*-------------------------------------------------------------------------*/

/*! @brief Get the item at the tail of the list.
 *  @param list the list.
 *  @param prev the 1st chaining variable name (prev item).
 *  @param next the 2nt chaining variable name (next item).
 *  @return The item at the tail of the list.
 */

#define bigbox_list_get_tail_named(list, prev, next)				\
	((list) != NULL ? (list)->prev : NULL)

/*-------------------------------------------------------------------------*/

/*! @brief Checks whether the item is chained in a list.
 *  @param item the item.
 *  @param prev the 1st chaining variable name (prev item).
 *  @param next the 2nt chaining variable name (next item).
 *  @return 'true' if the item is chained in the list and 'false' if the item is not chained in the list.
 */

#define bigbox_list_item_is_chained_named(item, prev, next)			\
	((item)->prev != NULL || (item)->next != NULL)

/*-------------------------------------------------------------------------*/

/*! @brief Internal macro, do not use.
 *  @return Nothing.
 */

#define __list_insert_atleft_named(existing_item, item, prev, next)		\
{										\
	(existing_item)->prev->next = (item);					\
	(item)->prev = (existing_item)->prev;					\
	(existing_item)->prev = (item);						\
	(item)->next = (existing_item);						\
}

/*-------------------------------------------------------------------------*/

/*! @brief Internal macro, do not use.
 *  @return Nothing.
 */

#define __list_insert_atright_named(existing_item, item, prev, next)		\
{										\
	(existing_item)->next->prev = (item);					\
	(item)->next = (existing_item)->next;					\
	(existing_item)->next = (item);						\
	(item)->prev = (existing_item);						\
}

/*-------------------------------------------------------------------------*/

/*! @brief Append the item at the head of the list.
 *  @param list the list.
 *  @param item the item.
 *  @param prev the 1st chaining variable name (prev item).
 *  @param next the 2nt chaining variable name (next item).
 *  @return Nothing.
 */

#define bigbox_list_append_head_named(list, item, prev, next)			\
{										\
	if((list) != NULL)							\
	{									\
		__list_insert_atleft_named(list, item, prev, next);		\
										\
		(list) = (item);						\
	}									\
	else									\
	{									\
		bigbox_list_init_singleton_named(list, item, prev, next);	\
	}									\
}

/*-------------------------------------------------------------------------*/

/*! @brief Append the item at the tail of the list.
 *  @param list the list.
 *  @param item the item.
 *  @param prev the 1st chaining variable name (prev item).
 *  @param next the 2nt chaining variable name (next item).
 *  @return Nothing.
 */

#define bigbox_list_append_tail_named(list, item, prev, next)			\
{										\
	if((list) != NULL)							\
	{									\
		__list_insert_atleft_named(list, item, prev, next);		\
										\
/*		(list) = (list);						\
 */	}									\
	else									\
	{									\
		bigbox_list_init_singleton_named(list, item, prev, next);	\
	}									\
}

/*-------------------------------------------------------------------------*/

/*! @brief Insert the item at the left of an existing chained item.
 *  @param list the list.
 *  @param existing_item the existing chained item.
 *  @param item the item.
 *  @param prev the 1st chaining variable name (prev item).
 *  @param next the 2nt chaining variable name (next item).
 *  @return Nothing.
 */

#define bigbox_list_insert_atleft_named(list, existing_item, item, prev, next)	\
{										\
	if((list) != NULL)							\
	{									\
		__list_insert_atleft_named(existing_item, item, prev, next);	\
										\
		if((list) == (existing_item))					\
		{								\
			(list) = (item);					\
		}								\
	}									\
	else									\
	{									\
		bigbox_list_init_singleton_named(list, item, prev, next);	\
	}									\
}

/*-------------------------------------------------------------------------*/

/*! @brief Insert the item at the right of an existing chained item.
 *  @param list the list.
 *  @param existing_item the existing chained item.
 *  @param item the item.
 *  @param prev the 1st chaining variable name (prev item).
 *  @param next the 2nt chaining variable name (next item).
 *  @return Nothing.
 */

#define bigbox_list_insert_atright_named(list, existing_item, item, prev, next)	\
{										\
	if((list) != NULL)							\
	{									\
		__list_insert_atright_named(existing_item, item, prev, next);	\
										\
/*		if((list) == (existing_item))					\
 *		{								\
 *			(list) = (list);					\
 *		}								\
 */	}									\
	else									\
	{									\
		bigbox_list_init_singleton_named(list, item, prev, next);	\
	}									\
}

/*-------------------------------------------------------------------------*/

/*! @brief Remove the item from the list.
 *  @param list the list.
 *  @param item the item.
 *  @param prev the 1st chaining variable name (prev item).
 *  @param next the 2nt chaining variable name (next item).
 *  @return Nothing.
 */

#define bigbox_list_remove_named(list, item, prev, next)			\
{										\
	if(((item)->prev == (item))						\
	   &&									\
	   ((item)->next == (item))						\
	 ) {									\
		(list) = NULL;							\
	}									\
	else									\
	{									\
		(item)->prev->next = (item)->next;				\
		(item)->next->prev = (item)->prev;				\
										\
		if((list) == (item))						\
		{								\
			(list) = (__typeof__(list)) (item)->next;		\
		}								\
	}									\
										\
	(item)->prev = (item)->next = NULL;					\
}

/*-------------------------------------------------------------------------*/

/*! @brief Pop the item from the head of the list.
 *  @param list the list.
 *  @param prev the 1st chaining variable name (prev item).
 *  @param next the 2nt chaining variable name (next item).
 *  @return The item of the head of the list.
 */

#define bigbox_list_pop_head_named(list, prev, next)				\
({										\
	__typeof__(list) result = bigbox_list_get_head_named(list, prev, next);	\
										\
	if(result != NULL)							\
	{									\
		bigbox_list_remove_named(list, result, prev, next);		\
	}									\
										\
	result;									\
})

/*-------------------------------------------------------------------------*/

/*! @brief Pop the item from the tail of the list.
 *  @param list the list.
 *  @param prev the 1st chaining variable name (prev item).
 *  @param next the 2nt chaining variable name (next item).
 *  @return The item of the tail of the list.
 */

#define bigbox_list_pop_tail_named(list, prev, next)				\
({										\
	__typeof__(list) result = bigbox_list_get_tail_named(list, prev, next);	\
										\
	if(result != NULL)							\
	{									\
		bigbox_list_remove_named(list, result, prev, next);		\
	}									\
										\
	result;									\
})

/*-------------------------------------------------------------------------*/

/*! @brief Count the number of items in the list.
 *  @param list the list.
 *  @param prev the 1st chaining variable name (prev item).
 *  @param next the 2nt chaining variable name (next item).
 *  @return The number of items in the list..
 */

#define bigbox_list_count_named(list, prev, next)				\
({										\
	int result = 0;								\
										\
	if((list) != NULL)							\
	{									\
		__typeof__(list) item = list;					\
		__typeof__(list) stop = list;					\
										\
		do { result++; } while((item = item->next) != stop);		\
	}									\
										\
	result;									\
})

/*-------------------------------------------------------------------------*/

/*! @brief Iterate over all the items in the list from head to tail.
 *  @param list the list.
 *  @param iterator an iterator.
 *  @param nr a counter.
 *  @param prev the 1st chaining variable name (prev item).
 *  @param next the 2nt chaining variable name (next item).
 *  @return Nothing.
 */

#define bigbox_list_foreach_forward_named(list, iterator, nr, prev, next)	\
	if((list) != NULL) for((iterator) = bigbox_list_get_head_named(list, prev, next), (nr) = 0; ((nr) == 0) || ((iterator) != bigbox_list_get_head_named(list, prev, next)); (iterator) = (iterator)->next, (nr)++)

/*! @brief Iterate over all the items in the list from tail to head.
 *  @param list the list.
 *  @param iterator an iterator.
 *  @param nr a counter.
 *  @param prev the 1st chaining variable name (prev item).
 *  @param next the 2nt chaining variable name (next item).
 *  @return Nothing.
 */

#define bigbox_list_foreach_backward_named(list, iterator, nr, prev, next)	\
	if((list) != NULL) for((iterator) = bigbox_list_get_tail_named(list, prev, next), (nr) = 0; ((nr) == 0) || ((iterator) != bigbox_list_get_tail_named(list, prev, next)); (iterator) = (iterator)->prev, (nr)++)

/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/

/*! @brief Initialize a BIGBOX list. The chaining variables must be 'prev' and 'next'.
 *  @param list the list.
 *  @return Nothing.
 */

#define bigbox_list_init(list) \
		bigbox_list_init_named(list, prev, next)

/*-------------------------------------------------------------------------*/

/*! @brief Initialize a BIGBOX list as a singleton.
 *  @param list the list.
 *  @param item the item to be inserted.
 *  @return Nothing.
 */

#define bigbox_list_init_singleton(list, item) \
		bigbox_list_init_singleton_named(list, item, prev, next)

/*-------------------------------------------------------------------------*/

/*! @brief Checks whether the list is empty.
 *  @param list the list.
 *  @return 'true' if the list is empty and 'false' if the list is not empty.
 */

#define bigbox_list_is_empty(list) \
		bigbox_list_is_empty_named(list, prev, next)

/*-------------------------------------------------------------------------*/

/*! @brief Checks whether the list is a singleton.
 *  @param list the list.
 *  @return 'true' if the list is a singleton and 'false' if the list is not a singleton.
 */

#define bigbox_list_is_singleton(list) \
		bigbox_list_is_singleton_named(list, prev, next)

/*-------------------------------------------------------------------------*/

/*! @brief Get the item at the head of the list.
 *  @param list the list.
 *  @return The item at the head of the list.
 */

#define bigbox_list_get_head(list) \
		bigbox_list_get_head_named(list, prev, next)

/*! @brief Get the item at the taim of the list.
 *  @param list the list.
 *  @return The item at the tail of the list.
 */

#define bigbox_list_get_tail(list) \
		bigbox_list_get_tail_named(list, prev, next)

/*-------------------------------------------------------------------------*/

/*! @brief Checks whether the item is chained in a list.
 *  @param item the item.
 *  @return 'true' if the item is chained in the list and 'false' if the item is not chained in the list.
 */

#define bigbox_list_item_is_chained(item) \
		bigbox_list_item_is_chained_named(item, prev, next)

/*-------------------------------------------------------------------------*/

/*! @brief Append the item at the head of the list.
 *  @param list the list.
 *  @param item the item.
 *  @return Nothing.
 */

#define bigbox_list_append_head(list, item) \
		bigbox_list_append_head_named(list, item, prev, next)

/*! @brief Append the item at the tail of the list.
 *  @param list the list.
 *  @param item the item.
 *  @return Nothing.
 */

#define bigbox_list_append_tail(list, item) \
		bigbox_list_append_tail_named(list, item, prev, next)

/*-------------------------------------------------------------------------*/

/*! @brief Insert the item at the left of an existing chained item.
 *  @param list the list.
 *  @param existing_item the existing chained item.
 *  @param item the item.
 *  @return Nothing.
 */

#define bigbox_list_insert_atleft(list, existing_item, item) \
		bigbox_list_insert_atleft_named(list, existing_item, item, prev, next)

/*! @brief Insert the item at the right of an existing chained item.
 *  @param list the list.
 *  @param existing_item the existing chained item.
 *  @param item the item.
 *  @return Nothing.
 */

#define bigbox_list_insert_atright(list, existing_item, item) \
		bigbox_list_insert_atright_named(list, existing_item, item, prev, next)

/*-------------------------------------------------------------------------*/

/*! @brief Remove the item from the list.
 *  @param list the list.
 *  @param item the item.
 *  @return Nothing.
 */

#define bigbox_list_remove(list, item) \
		bigbox_list_remove_named(list, item, prev, next)

/*-------------------------------------------------------------------------*/

/*! @brief Pop the item from the head of the list.
 *  @param list the list.
 *  @return The item of the head of the list.
 */

#define bigbox_list_pop_head(list) \
		bigbox_list_pop_head_named(list, prev, next)

/*! @brief Pop the item from the tail of the list.
 *  @param list the list.
 *  @return The item of the tail of the list.
 */

#define bigbox_list_pop_tail(list) \
		bigbox_list_pop_tail_named(list, prev, next)

/*-------------------------------------------------------------------------*/

/*! @brief Count the number of items in the list.
 *  @param list the list.
 *  @return The number of items in the list..
 */

#define bigbox_list_count(list) \
		bigbox_list_count_named(list, prev, next)

/*-------------------------------------------------------------------------*/

/*! @brief Iterate over all the items in the list from head to tail.
 *  @param list the list.
 *  @param iterator an iterator.
 *  @param nr a counter.
 *  @return Nothing.
 */

#define bigbox_list_foreach_forward(list, iterator, nr) \
		bigbox_list_foreach_forward_named(list, iterator, nr, prev, next)

/*! @brief Iterate over all the items in the list from tail to head.
 *  @param list the list.
 *  @param iterator an iterator.
 *  @param nr a counter.
 *  @return Nothing.
 */

#define bigbox_list_foreach_backward(list, iterator, nr) \
		bigbox_list_foreach_backward_named(list, iterator, nr, prev, next)

/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/

/*! @brief Alias to 'bigbox_list_append_tail_named'.
 */

#define bigbox_list_append_named(list, item, prev, next) \
		bigbox_list_append_tail_named(list, item, prev, next)

/*! @brief Alias to 'bigbox_list_pop_tail_named'.
 */

#define bigbox_list_pop_named(list, prev, next) \
		bigbox_list_pop_tail_named(list, prev, next)

/*! @brief Alias to 'bigbox_list_foreach_forward_named'.
 */

#define bigbox_list_foreach_named(list, iterator, nr, prev, next) \
		bigbox_list_foreach_forward_named(list, iterator, nr, prev, next)

/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/

/*! @brief Alias to 'bigbox_list_append_tail'.
 */

#define bigbox_list_append(list, item) \
		bigbox_list_append_tail(list, item)

/*! @brief Alias to 'bigbox_list_pop_tail'.
 */

#define bigbox_list_pop(list) \
		bigbox_list_pop_tail(list)

/*! @brief Alias to 'bigbox_list_foreach_forward'.
 */

#define bigbox_list_foreach(list, iterator, nr) \
		bigbox_list_foreach_forward(list, iterator, nr)

/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/

#endif /* __BIGBOX_LIST_H */

/*-------------------------------------------------------------------------*/
