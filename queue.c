#include <stdlib.h>
#include <stdbool.h>
#include "lib/contracts.h"
#include "lib/xalloc.h"
#include "queue.h"

typedef struct list_node list;
struct list_node {
  void* data;
  list* next;
};

typedef struct queue_header queue;
struct queue_header {
  list* front;
  list* back;
  int size;
};

bool is_inclusive_segment(list* start, list* end, int i) {
  if (i < 0) return false;
  if (start == NULL && i == 0) return true;
  else if (start == end && start->next == NULL && i == 1) return true;
  else if (start != end && i > 0 && start->next != NULL &&
           is_inclusive_segment(start->next,end, i-1)) return true;
  return false;
}

bool is_queue(queue* Q) {
  if (Q == NULL) return false;
  if (is_inclusive_segment(Q->front, Q->back, Q->size)) return true;
  return false;
}

queue* queue_new()
//@ensures \result != NULL;
{
  queue* Q = xmalloc(sizeof(queue));
  list* l = xmalloc(sizeof(list));
  Q->front = NULL;
  Q->back = l;
  Q->size = 0;
  l->data = NULL;
  l->next = NULL;
  ENSURES(Q != NULL);
  return Q;
}

size_t queue_size(queue* Q)
//@requires Q != NULL;
//@ensures \result >= 0;
{
  REQUIRES(Q != NULL);
  REQUIRES(is_queue(Q));
  ENSURES(Q->size >= 0);
  return Q->size;
}

void enq(queue* Q, void* x)
//@requires Q != NULL;
{
  REQUIRES(Q != NULL);
  REQUIRES(is_queue(Q));
  list* l = xmalloc(sizeof(list));
  l->data = NULL;
  l->next = NULL;
  if (Q->front == NULL && Q->size == 0) { //empty queue
    Q->front = l;
    Q->back = l;
    Q->front->next = NULL;
    Q->front->data = x;
    Q->size = 1;
  }
  else {
    Q->back->next = l;
    Q->back = l;
    Q->back->next = NULL;
    Q->back->data = x;
    Q->size = Q->size + 1;
  }
  ENSURES(Q != NULL);
}

void* deq(queue* Q)
//@requires Q != NULL && queue_size(Q) > 0;
{
  REQUIRES(Q != NULL);
  REQUIRES(queue_size(Q) > 0);
  if (Q->front == Q->back && Q->size == 1) {
    void* x = Q->front->data;
    Q->size = 0;
    return x;
  }
  void* x = Q->front->data;
  Q->front = Q->front->next;
  Q->size = Q->size - 1;
  return x;
}

void* queue_peek(queue* Q, size_t i)
//@requires Q != NULL && 0 <= i && i < queue_size(Q);
{
  REQUIRES(Q != NULL);
  REQUIRES(i < queue_size(Q));
  list* l = Q->front;
  for (size_t index = 0; index <= i; index++) {
    if (index == i) {
      void* x = l->data;
      return x;
    }
    l = l->next;
  }
  return NULL;
}

void queue_reverse(queue* Q)
//@requires Q != NULL;
{
  REQUIRES(Q != NULL);
  REQUIRES(is_queue(Q));
  if (Q->front == NULL && Q->size == 0) return;
  list* cur = Q->front;
  list* nxt = NULL;
  list* prev = NULL;
  while (cur != NULL) {
    nxt = cur->next;
    cur->next = prev;
    prev = cur;
    cur = nxt;
  }
  Q->back = Q->front;
  Q->front = prev;
}

bool queue_all(queue* Q, check_property_fn* P)
//@requires Q != NULL && P != NULL;
{
  REQUIRES(Q != NULL);
  REQUIRES(P != NULL);
  REQUIRES(is_queue(Q));
  if (Q->front == NULL && Q->size == 0) return true;
  list* l = Q->front;
  for (size_t index = 0; index < queue_size(Q); index++) {
    if ((*P)(l->data) == false) return false;
    l = l->next;
  }
  return true;
}

void* queue_iterate(queue* Q, void* base, iterate_fn* F)
//@requires Q != NULL && F != NULL;
{
  REQUIRES(Q != NULL);
  REQUIRES(F != NULL);
  REQUIRES(is_queue(Q));
  if (Q->front == NULL && Q->size == 0) return base;
  list* l = Q->front;
  for (size_t index = 0; index < queue_size(Q); index++) {
    base = (*F)(base, l->data);
    l = l->next;
  }
  return base;
}

void queue_free(queue* Q, free_fn* F)
//@requires Q != NULL;
{
  REQUIRES(Q != NULL);
  REQUIRES(is_queue(Q));
  if (F == NULL) {
    free(Q->front);
    free(Q->back);
  }
  else if (F != NULL) {
    list* cur = Q->front;
    list* nxt = Q->front->next;
    for (size_t index = 0; index < queue_size(Q); index++) {
      (*F)(cur);
      nxt = nxt->next;
      cur = nxt;
    }
  }
  free(Q);
}
