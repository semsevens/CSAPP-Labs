/* Code to implement readers/writers that preserves FIFO priority */

#include "csapp.h"
#include <stdbool.h>

/* Represents individual thread's position in queue */
typedef struct TOK {
    bool is_reader;
    sem_t enable;     // Enables access
    struct TOK *next; // Allows chaining as linked list
} rw_token_t;

/* Queue data structure */
typedef struct {
    sem_t mutex;       // Mutual exclusion
    int reading_count; // Number of active readers
    int writing_count; // Number of active writers
    // FIFO queue implemented as linked list with tail pointer
    rw_token_t *head;
    rw_token_t *tail;
} rw_queue_t;

void rw_queue_init(rw_queue_t *q);
void rw_queue_request_read(rw_queue_t *q, rw_token_t *t);
void rw_queue_request_write(rw_queue_t *q, rw_token_t *t);
void rw_queue_release(rw_queue_t *q);
/* Print information about queue */
void rw_queue_status(rw_queue_t *q);
