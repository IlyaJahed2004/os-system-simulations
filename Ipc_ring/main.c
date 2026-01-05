#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <fcntl.h>

// Constants 

#define SHARED_MEM_KEY     0x3456
#define NUM_PROCESS_UNITS  8
#define NUM_PRIMES         100
#define MAILBOX_CAPACITY   8
#define RESULT_CAPACITY    200


typedef struct {
    int value;      
    int counter;    
} pipeline_item_t;

typedef struct {
    pipeline_item_t inbox[NUM_PROCESS_UNITS][MAILBOX_CAPACITY];
    int inbox_head[NUM_PROCESS_UNITS];
    int inbox_tail[NUM_PROCESS_UNITS];

    pipeline_item_t results[RESULT_CAPACITY];
    int result_head;
    int result_tail;
} shared_data_t;

// Prime Generator
void generate_primes(int count, int *buffer) {
    int found = 0;
    int number = 2;

    while (found < count) {
        int is_prime = 1;
        for (int i = 2; i * i <= number; i++) {
            if (number % i == 0) {
                is_prime = 0;
                break;
            }
        }

        if (is_prime) {
            buffer[found++] = number;
            printf("[PARENT] found prime %d\n", number);
            fflush(stdout);
        }
        number++;
    }
}


int main() {

    setbuf(stdout, NULL);

    // Shared Memory Setup

    int shm_id = shmget(SHARED_MEM_KEY, sizeof(shared_data_t), IPC_CREAT | 0666);
    shared_data_t *shared = shmat(shm_id, NULL, 0);

    for (int i = 0; i < NUM_PROCESS_UNITS; i++)
        shared->inbox_head[i] = shared->inbox_tail[i] = 0;

    shared->result_head = shared->result_tail = 0;

    // Mailbox Semaphores

    sem_t *inbox_empty[NUM_PROCESS_UNITS];
    sem_t *inbox_full[NUM_PROCESS_UNITS];
    sem_t *inbox_mutex[NUM_PROCESS_UNITS];
    char sem_name[64];

    for (int i = 0; i < NUM_PROCESS_UNITS; i++) {

        sprintf(sem_name, "/inbox_empty_%d", i);
        sem_unlink(sem_name);
        inbox_empty[i] = sem_open(sem_name, O_CREAT, 0666, MAILBOX_CAPACITY);

        sprintf(sem_name, "/inbox_full_%d", i);
        sem_unlink(sem_name);
        inbox_full[i] = sem_open(sem_name, O_CREAT, 0666, 0);

        sprintf(sem_name, "/inbox_mutex_%d", i);
        sem_unlink(sem_name);
        inbox_mutex[i] = sem_open(sem_name, O_CREAT, 0666, 1);
    }

    // Result Buffer Semaphores

    sem_unlink("/result_empty");
    sem_unlink("/result_full");
    sem_unlink("/result_mutex");

    sem_t *result_empty = sem_open("/result_empty", O_CREAT, 0666, RESULT_CAPACITY);
    sem_t *result_full  = sem_open("/result_full",  O_CREAT, 0666, 0);
    sem_t *result_mutex = sem_open("/result_mutex", O_CREAT, 0666, 1);

    // Spawn Processing Units

    for (int pu_id = 0; pu_id < NUM_PROCESS_UNITS; pu_id++) {

        if (fork() == 0) {

            printf("[PU %d] started\n", pu_id);

            while (1) {

                sem_wait(inbox_full[pu_id]);
                sem_wait(inbox_mutex[pu_id]);

                pipeline_item_t item =
                    shared->inbox[pu_id][shared->inbox_tail[pu_id]];
                shared->inbox_tail[pu_id] =
                    (shared->inbox_tail[pu_id] + 1) % MAILBOX_CAPACITY;

                sem_post(inbox_mutex[pu_id]);
                sem_post(inbox_empty[pu_id]);

                if (item.counter == -1)
                    break;

                item.value += pu_id;
                item.counter--;

                if (item.counter > 0) {

                    int next_pu = (pu_id + 1) % NUM_PROCESS_UNITS;

                    sem_wait(inbox_empty[next_pu]);
                    sem_wait(inbox_mutex[next_pu]);

                    shared->inbox[next_pu][shared->inbox_head[next_pu]] = item;
                    shared->inbox_head[next_pu] =
                        (shared->inbox_head[next_pu] + 1) % MAILBOX_CAPACITY;

                    sem_post(inbox_mutex[next_pu]);
                    sem_post(inbox_full[next_pu]);

                    printf("[PU %d] forwarded value=%d counter=%d to PU %d\n",
                           pu_id, item.value, item.counter, next_pu);
                } else {

                    sem_wait(result_empty);
                    sem_wait(result_mutex);

                    shared->results[shared->result_head] = item;
                    shared->result_head =
                        (shared->result_head + 1) % RESULT_CAPACITY;

                    sem_post(result_mutex);
                    sem_post(result_full);

                    printf("[PU %d] finished value=%d\n", pu_id, item.value);
                }
            }

            shmdt(shared);
            exit(0);
        }
    }

    //Parent: Send Initial Primes

    int primes[NUM_PRIMES];
    generate_primes(NUM_PRIMES, primes);

    for (int i = 0; i < NUM_PRIMES; i++) {

        int prime = primes[i];
        int target_pu = prime % NUM_PROCESS_UNITS;

        sem_wait(inbox_empty[target_pu]);
        sem_wait(inbox_mutex[target_pu]);

        shared->inbox[target_pu][shared->inbox_head[target_pu]] =
            (pipeline_item_t){ .value = prime, .counter = prime };

        shared->inbox_head[target_pu] =
            (shared->inbox_head[target_pu] + 1) % MAILBOX_CAPACITY;

        sem_post(inbox_mutex[target_pu]);
        sem_post(inbox_full[target_pu]);

        usleep(500);
    }

    //Parent: Collect Results

    for (int i = 0; i < NUM_PRIMES; i++) {

        sem_wait(result_full);
        sem_wait(result_mutex);

        pipeline_item_t result =
            shared->results[shared->result_tail];

        shared->result_tail =
            (shared->result_tail + 1) % RESULT_CAPACITY;

        sem_post(result_mutex);
        sem_post(result_empty);

        printf("[RESULT %2d] final value = %d\n", i + 1, result.value);
    }

    //Shutdown Processing Units

    for (int i = 0; i < NUM_PROCESS_UNITS; i++) {

        sem_wait(inbox_empty[i]);
        sem_wait(inbox_mutex[i]);

        shared->inbox[i][shared->inbox_head[i]] =
            (pipeline_item_t){ .counter = -1 };

        shared->inbox_head[i] =
            (shared->inbox_head[i] + 1) % MAILBOX_CAPACITY;

        sem_post(inbox_mutex[i]);
        sem_post(inbox_full[i]);
    }

    for (int i = 0; i < NUM_PROCESS_UNITS; i++)
        wait(NULL);

    shmdt(shared);
    shmctl(shm_id, IPC_RMID, NULL);

    return 0;
}
