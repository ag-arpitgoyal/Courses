/*
 * MTL458 Assignment 3: Free-Space Management
 *
 * Do not modify this file. The autograder compiles your a3.c with its own
 * copy of it.
 */
#ifndef MYALLOC_H
#define MYALLOC_H

#include <stddef.h>

/* Placement policies, for my_set_policy */
enum { FIRST_FIT, NEXT_FIT, BEST_FIT, WORST_FIT, BUDDY };

/* Parts 1 and 3 */
void *my_malloc(size_t n);
void my_free(void *ptr);
void *my_calloc(size_t nmemb, size_t size);
void *my_realloc(void *ptr, size_t n);

/* Parts 2, 4 and 5 */
void my_set_policy(int policy);
void my_set_delay(int k);

/* Output */
void my_heap_dump(void);
void my_leak_report(void);

#endif
