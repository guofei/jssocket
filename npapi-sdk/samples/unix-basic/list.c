#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include "ll.h"
#include "list.h"

typedef struct list_entry {
	ll_t list;
	void *data;
} list_entry_t;

/* call while holding lock */
static int
list_check(list_t *ptr)
{
	assert(ptr->count == ll_check(&ptr->head));
	return (1);
}

int
list_init(list_t *ptr)
{
	ptr->count = 0;
	ll_init(&ptr->head);
	pthread_mutex_init(&ptr->lock,NULL);
	assert(pthread_mutex_lock(&ptr->lock) == 0 &&
	       list_check(ptr) &&
	       pthread_mutex_unlock(&ptr->lock) == 0);
	return 0;
}

int
list_push(list_t *ptr, void *item)
{
	list_entry_t *e = malloc(sizeof(*e));
	if (e == NULL)
		return (-1);
	e->data = item;
	pthread_mutex_lock(&ptr->lock);
	assert(list_check(ptr));
	++ptr->count;
	ll_enqueue(&ptr->head, &e->list);
	pthread_mutex_unlock(&ptr->lock);
	return (0);
}

void *
list_pop(list_t *ptr)
{
	list_entry_t *e,*data;
	pthread_mutex_lock(&ptr->lock);
	e = (list_entry_t *)ll_dequeue(&ptr->head);
	if(e != NULL){
		ptr->count--;
		data = e->data;
		free(e);
	}
	else{
		data = NULL;
	}
	pthread_mutex_unlock(&ptr->lock);
	return data;
}

struct wrap {
	int (*func)(void *, void *);
	void *user;
};

static int
wrapper(void *e, void *u)
{
	list_entry_t *q = e;
	struct wrap *w = u;
	return (w->func(q->data, w->user));
}

int
list_traverse(list_t *ptr, int (*func)(void *, void *), void *user)
{
	list_entry_t *ret;
	struct wrap wrap;
	int return_value = 0;

	wrap.func = func;
	wrap.user = user;
	pthread_mutex_lock(&ptr->lock);
	assert(list_check(ptr));
	ret = (list_entry_t *)ll_traverse(&ptr->head, wrapper, &wrap);
	if (ret) {
		free(ret);
		--ptr->count;
		return_value = -1;
	}
	assert(list_check(ptr));
	pthread_mutex_unlock(&ptr->lock);
	return (return_value);
}

static int
matchit(void *data, void *compare)
{
	return (data == compare ? -1 : 0);
}

int
list_remove(list_t *ptr, void *item)
{
	return (list_traverse(ptr, matchit, item));
}

int
list_destroy(list_t *ptr)
{
	list_entry_t *e;

	while ((e = (list_entry_t *)ll_dequeue(&ptr->head)) != NULL)
		free(e);
	return (pthread_mutex_destroy(&ptr->lock));
}
