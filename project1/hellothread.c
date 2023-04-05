#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

void *run_loop(void *ptr)
{
    // Convert the input ptr to a char**
    char **thread_name = (char **)ptr;

    // Print out the thread name and loop index 5 times
    for (int i = 0; i < 5; i++)
    {
        printf("%s: %i\n", *thread_name, i);
        sleep(1);
    }
    return NULL;
}

int main(void)
{
    // Define thread ids
    pthread_t id_1;
    pthread_t id_2;

    // Define thread names
    char *thread_name1 = "Thread 1";
    char *thread_name2 = "Thread 2";

    // Create threads
    pthread_create(&id_1, NULL, run_loop, &thread_name1);
    pthread_create(&id_2, NULL, run_loop, &thread_name2);

    // Join threads
    pthread_join(id_1, NULL);
    pthread_join(id_2, NULL);

    // Print completion message
    printf("Threads complete!");
}