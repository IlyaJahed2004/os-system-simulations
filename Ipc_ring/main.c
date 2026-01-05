#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <fcntl.h>

#define SHM_KEY 0x1111
#define NUM_PUS 8
#define MAILBOX_SIZE 8
#define NUM_PRIMES 10


typedef struct {
    int value;
    int counter;
} prime_t;


typedef struct {
    prime_t mailbox[NUM_PUS][MAILBOX_SIZE];
    int head[NUM_PUS];
    int tail[NUM_PUS];
} shared_mem_t;




// Prime Generator
int is_prime(int x) {
    if (x < 2) return 0;
    for (int i = 2; i * i <= x; i++)
        if (x % i == 0) return 0;
    return 1;
}



int main() {
    int shmid = shmget(SHM_KEY, sizeof(shared_mem_t), IPC_CREAT | 0666);
    shared_mem_t *shm = shmat(shmid, NULL, 0);

    for (int i = 0; i < NUM_PUS; i++)
        shm->head[i] = shm->tail[i] = 0;

    // Semaphores 

    sem_t *empty[NUM_PUS], *full[NUM_PUS], *mutex[NUM_PUS];
    char name[64];

    for (int i = 0; i < NUM_PUS; i++) {
        sprintf(name, "/p1_empty_%d", i);
        sem_unlink(name);
        empty[i] = sem_open(name, O_CREAT, 0666, MAILBOX_SIZE);

        sprintf(name, "/p1_full_%d", i);
        sem_unlink(name);
        full[i] = sem_open(name, O_CREAT, 0666, 0);

        sprintf(name, "/p1_mutex_%d", i);
        sem_unlink(name);
        mutex[i] = sem_open(name, O_CREAT, 0666, 1);
    }

    //Fork PU Processes

    for (int pu = 0; pu < NUM_PUS; pu++) {
        if (fork() == 0) {
            int id = pu;

            while (1) {
                sem_wait(full[id]);
                sem_wait(mutex[id]);

                prime_t item = shm->mailbox[id][shm->tail[id]];
                shm->tail[id] = (shm->tail[id] + 1) % MAILBOX_SIZE;

                sem_post(mutex[id]);
                sem_post(empty[id]);

                if (item.counter == -1)
                    break;

                item.value += id;
                item.counter--;

                printf("PU %d: value=%d counter=%d\n",
                       id, item.value, item.counter);
                fflush(stdout);

                int next = (id + 1) % NUM_PUS;

                sem_wait(empty[next]);
                sem_wait(mutex[next]);

                shm->mailbox[next][shm->head[next]] = item;
                shm->head[next] = (shm->head[next] + 1) % MAILBOX_SIZE;

                sem_post(mutex[next]);
                sem_post(full[next]);
            }

            shmdt(shm);
            exit(0);
        }
    }

    // Parent: Send Primes

    int sent = 0;
    for (int x = 2; sent < NUM_PRIMES; x++) {
        if (!is_prime(x)) continue;

        prime_t p;
        p.value = x;
        p.counter = x;

        int target = x % NUM_PUS;

        sem_wait(empty[target]);
        sem_wait(mutex[target]);

        shm->mailbox[target][shm->head[target]] = p;
        shm->head[target] = (shm->head[target] + 1) % MAILBOX_SIZE;

        sem_post(mutex[target]);
        sem_post(full[target]);

        sent++;
    }

    // Termination

    for (int i = 0; i < NUM_PUS; i++) {
        sem_wait(empty[i]);
        sem_wait(mutex[i]);

        shm->mailbox[i][shm->head[i]].counter = -1;
        shm->head[i] = (shm->head[i] + 1) % MAILBOX_SIZE;

        sem_post(mutex[i]);
        sem_post(full[i]);
    }

    for (int i = 0; i < NUM_PUS; i++)
        wait(NULL);

    shmdt(shm);
    shmctl(shmid, IPC_RMID, NULL);
    return 0;
}
