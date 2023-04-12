#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

// These will be initialized in main() from the command line.
int seat_count;
int broker_count;
int *seat_taken;  // Array of seats
int transaction_count;

int seat_taken_count = 0;

pthread_mutex_t seat_taken_lock = PTHREAD_MUTEX_INITIALIZER;


int reserve_seat(int n)
{
    // Attempt to reserve seat number n
    //
    // If the seat is already taken, return -1
    // Otherwise mark the seat as taken and return 0
    //
    // This function should also increment seat_taken_count if the seat
    // wasn't already taken.
    int return_code = 0;
    pthread_mutex_lock(&seat_taken_lock);
    if (seat_taken[n] == 0)
    {
        seat_taken[n] = 1;
        seat_taken_count += 1;
    } else {
        return_code = -1;
    }
    pthread_mutex_unlock(&seat_taken_lock);
    return return_code;
}

int free_seat(int n)
{
    // Attempt to free (unreserve) seat number n
    //
    // If the seat is already free, return -1
    // Otherwise mark the seat as free and return 0
    //
    // This function should also decrement seat_taken_count if the seat
    // wasn't already free.
    int return_code = 0;
    pthread_mutex_lock(&seat_taken_lock);
    if (seat_taken[n] == 1)
    {
        seat_taken[n] = 0;
        seat_taken_count -= 1;
    } else {
        return_code = -1;
    }
    pthread_mutex_unlock(&seat_taken_lock);
    return return_code;
}

int is_free(int n) {
    // Returns true if the given seat is available.
    pthread_mutex_lock(&seat_taken_lock);
    int is_free = (seat_taken[n] == 0);
    pthread_mutex_unlock(&seat_taken_lock);
    return is_free;
}

int verify_seat_count(void) {
    // This function counts all the taken seats in the seat_taken[]
    // array.
    //
    // It then compares the count with the seat_count global variable.
    //
    // It returns true if they are the same, false otherwise
    //
    // You MAY modify this function, but the intended functionality must
    // still work properly.

    int count = 0;

    pthread_mutex_lock(&seat_taken_lock);

    // Count all the taken seats
    for (int i = 0; i < seat_count; i++)
        if (seat_taken[i])
            count++;

    // Return true if it's the same as seat_taken_count
    int valid_count = (count == seat_taken_count);
    pthread_mutex_unlock(&seat_taken_lock);
    return valid_count;
}

// ------------------- DO NOT MODIFY PAST THIS LINE -------------------

void *seat_broker(void *arg)
{
    int *id = arg;

    for (int i = 0; i < transaction_count; i++) {
        int seat = rand() % seat_count;
        if (rand() & 1) {
            // buy a random seat
            reserve_seat(seat);

        } else {
            // sell a random seat
            free_seat(seat);
        }

        if (!verify_seat_count()) {
            printf("Broker %d: the seat count seems to be off! " \
                   "I quit!\n", *id);
            return NULL;
        }
    }

    printf("Broker %d: That all seemed to work very well.\n", *id);

    return NULL;
}

int main(int argc, char *argv[])
{
    // Parse command line
    if (argc != 4) {
        fprintf(stderr, "usage: reservations seat_count broker_count xaction_count\n");
        exit(1);
    }

    seat_count = atoi(argv[1]);
    broker_count = atoi(argv[2]);
    transaction_count = atoi(argv[3]);

    // Allocate the seat-taken array
    seat_taken = calloc(seat_count, sizeof *seat_taken);

    // Allocate thread handle array for all brokers
    pthread_t *thread = calloc(broker_count, sizeof *thread);

    // Allocate thread ID array for all brokers
    int *thread_id = calloc(broker_count, sizeof *thread_id);

    srand(time(NULL) + getpid());
    
    // Launch all brokers
    for (int i = 0; i < broker_count; i++) {
        thread_id[i] = i;
        pthread_create(thread + i, NULL, seat_broker, thread_id + i);
    }

    // Wait for all brokers to complete
    for (int i = 0; i < broker_count; i++)
        pthread_join(thread[i], NULL);
}

