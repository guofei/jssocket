#ifndef _LIST_H_
#define _LIST_H_

/* thread-safe verison of linked list */

#include "ll.h"
#include <pthread.h>

typedef struct list {
	int count;
	pthread_mutex_t lock;
	llh_t head;
} list_t;

int list_init(list_t *ptr);
int list_push(list_t *ptr, void *item);
void *list_pop(list_t *ptr);
int list_traverse(list_t *ptr, int (*func)(void *, void *), void *user);
int list_remove(list_t *ptr, void *item);
int list_destroy(list_t *ptr);

#endif /* _LIST_H_ */
