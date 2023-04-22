#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdbool.h>
#include "eventbuf.h" 
#include "helpers.h"

// Shared data buffer and needed semaphores
struct eventbuf *eb;
sem_t *mutex;
sem_t *items;
sem_t *spaces;

// Flag to indicate that there are no more events
bool done = false;

// Command line arguments
int num_producers;
int num_consumers;
int events_per_producer;
int max_outstanding;

void *producer (void *arg){
    /*
    * Producer thread function
    */
    // Get producer id
    int id = *((int *) arg);

    // Generate events
    for (int i = 0; i < events_per_producer; i++) {
        // Calculate event number
        int event_number = id * 100 + i;
        printf("P%d: adding event %d\n", id, event_number);
        
        // Add event to buffer
        sem_wait(spaces);
        sem_wait(mutex);
        eventbuf_add(eb, event_number);
        sem_post(mutex);
        sem_post(items);
    }
    printf("P%d: exiting\n", id);
    return NULL;
}

void *consumer (void *arg){
    /*
    * Consumer thread function
    */
    // Get consumer id
    int id = *((int *) arg);

    // Consume events
    while(!done) {
        // Get event from buffer
        sem_wait(items);
        sem_wait(mutex);
        if (eventbuf_empty(eb)) {
            // No more events, set done flag
            sem_post(mutex);
            sem_post(items);
            done = true;
        }
        int event = eventbuf_get(eb);
        sem_post(mutex);
        sem_post(spaces);

        // Print event
        printf("C%d: got event %d\n", id, event);
    }
    printf("C%d: exiting\n", id);

    return NULL;
}

int main(int argc, char *argv[]) {
    // Parse command line arguments
    if (argc != 5) {
        printf("Usage: %s <num_producers> <num_consumers> <events_per_producer> <max_outstanding>\n", argv[0]);
        return 1;
    }
    num_producers = atoi(argv[1]);
    num_consumers = atoi(argv[2]);
    events_per_producer = atoi(argv[3]);
    max_outstanding = atoi(argv[4]);

    // Initialize shared data buffer and needed semaphores
    eb = eventbuf_create();
    mutex = sem_open_temp("mutex_sem", 1);
    items = sem_open_temp("items_sem", 0);
    spaces = sem_open_temp("spaces_sem", max_outstanding);

    // Create Producers
    pthread_t producers[num_producers];
    int producer_ids[num_producers];
    for (int i = 0; i < num_producers; i++) {
        producer_ids[i] = i;
        pthread_create(&producers[i], NULL, producer, &producer_ids[i]);
    }

    // Create Consumers
    pthread_t consumers[num_consumers];
    int consumer_ids[num_consumers];
    for (int i = 0; i < num_consumers; i++) {
        consumer_ids[i] = i;
        pthread_create(&consumers[i], NULL, consumer, &consumer_ids[i]);
    }

    // Wait for Producers
    for (int i = 0; i < num_producers; i++) {
        pthread_join(producers[i], NULL);
    }

    // Wake up consumers so they will exit
    sem_post(items);

    // Wait for Consumers
    for (int i = 0; i < num_consumers; i++) {
        pthread_join(consumers[i], NULL);
    }

    // Cleanup
    eventbuf_free(eb);

    return 0;
}