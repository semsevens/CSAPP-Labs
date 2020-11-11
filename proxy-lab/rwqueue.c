#include "rwqueue.h"

void rw_queue_init(rw_queue_t *q) {
    Sem_init(&q->mutex, 0, 1);
    q->reading_count = 0;
    q->writing_count = 0;
    q->head = NULL;
    q->tail = NULL;
}

/*
 * Helper routine to insert element in queue
 * Assume mutually exclusive access
 */
static void enqueue(rw_queue_t *q, rw_token_t *t) {
    if (q->tail == NULL) {
        q->tail = q->head = t;
    } else {
        q->tail->next = t;
        q->tail = t;
    }
    t->next = NULL;
}

/*
 * Helper routine to retrieve element at head of queue
 */
static rw_token_t *peekqueue(rw_queue_t *q) {
    return q->head;
}

/*
 * Helper routine to remove first element from queue
 */
static void dequeue(rw_queue_t *q) {
    rw_token_t *t = q->head;
    if (t == NULL)
        return;
    q->head = t->next;
    if (q->head == NULL)
        q->tail = NULL;
}

void rw_queue_request_read(rw_queue_t *q, rw_token_t *t) {
    bool block = false;
    P(&q->mutex);
    if (q->head == NULL && q->writing_count == 0)
        /* Can bypass queue */
        q->reading_count++;
    else {
        /* Must add to queue */
        t->is_reader = true;
        enqueue(q, t);
        Sem_init(&t->enable, 0, 0);
        block = true;
    }
    V(&q->mutex);
    if (block)
        P(&t->enable);
}

void rw_queue_request_write(rw_queue_t *q, rw_token_t *t) {
    bool block = false;
    P(&q->mutex);
    if (q->head == NULL && q->writing_count == 0 && q->reading_count == 0)
        /* Can bypass queue */
        q->writing_count++;
    else {
        t->is_reader = false;
        enqueue(q, t);
        Sem_init(&t->enable, 0, 0);
        block = true;
    }
    V(&q->mutex);
    if (block)
        P(&t->enable);
}

void rw_queue_release(rw_queue_t *q) {
    rw_token_t *t;
    P(&q->mutex);
#ifdef DEBUG
    printf("Start of release: ");
    rw_queue_status(q);
#endif
    if (q->writing_count > 0)
        q->writing_count--;
    else
        q->reading_count--;

    t = peekqueue(q);
    if (!t) {
        V(&q->mutex);
        return;
    }
    bool wanttoread = t->is_reader;
    if (!wanttoread && q->reading_count == 0) {
        q->writing_count++;
        V(&t->enable);
        dequeue(q);
    } else {
        while (wanttoread) {
            q->reading_count++;
            V(&t->enable);
            dequeue(q);
            t = peekqueue(q);
            wanttoread = t && t->is_reader;
        }
    }
#ifdef DEBUG
    printf("End of release: ");
    rw_queue_status(q);
#endif
    V(&q->mutex);
}

void rw_queue_status(rw_queue_t *q) {
    rw_token_t *t;
    printf("Readers = %d, Writers = %d, Queue=[", q->reading_count, q->writing_count);
    for (t = q->head; t; t = t->next)
        putchar(t->is_reader ? 'r' : 'w');
    printf("]\n");
}
