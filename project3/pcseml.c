#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include "eventbuf.h" 
#include "helpers.h"

struct eventbuf *eb;
sem_t *sem_producer;
sem_t *sem_consumer;

int num_producers;
int num_consumers;
int events_per_producer;
int max_outstanding;

void *producer (void *arg){
    int id = *((int *) arg);
    printf("P%d: starting\n", id);
    return NULL;
}

void *consumer (void *arg){
    int id = *((int *) arg);
    printf("C%d: starting\n", id);
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
    sem_producer = sem_open_temp("producer_sem", 1);
    sem_consumer = sem_open_temp("consumer_sem", 1);


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

    // Notify Consumers that there are no more events
    

    // Wait for Consumers
    for (int i = 0; i < num_consumers; i++) {
        pthread_join(consumers[i], NULL);
    }

    eventbuf_free(eb);

    return 0;
}