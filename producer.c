// Kacper Dobek index 148247
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <time.h>
#include <errno.h>
#include <stdbool.h>

#define N 5

int main()
{
    bool first = true;
    sem_t *Sp, *Sc, *Sq1, *Sq2, *Pcheck;
    Sq1 = sem_open("/q1", O_CREAT, 0600, 1);
    Sq2 = sem_open("/q2", O_CREAT, 0600, 1);
    Sp = sem_open("/producers", O_CREAT, 0600, N);
    Sc = sem_open("/consumers", O_CREAT, 0600, 0);
    Pcheck = sem_open("/Pcheck", O_CREAT | O_EXCL, 0600, 0); // I use this semaphore with O_EXCL flag just to check if there are any producers already running
    if (Pcheck == SEM_FAILED)
        first = false;
    if (Sp == SEM_FAILED || Sq1 == SEM_FAILED || Sq2 == SEM_FAILED || Sc == SEM_FAILED)
    {
        perror("sem_open");
        exit(1);
    }
    int q1_fd = shm_open("/q1fd", O_CREAT | O_RDWR, 0600); // the queue for producers
    ftruncate(q1_fd, N * sizeof(int));
    int *q1 = mmap(NULL, N * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, q1_fd, 0);
    if (q1 == MAP_FAILED)
    {
        printf(strerror(errno));
        printf("Q1 mapping failed\n");
        exit(1);
    }
    int q2_fd = shm_open("/q2fd", O_CREAT | O_RDWR, 0600); // the queue for consumers
    ftruncate(q2_fd, N * sizeof(int));
    int *q2 = mmap(NULL, N * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, q2_fd, 0);
    if (q2 == MAP_FAILED)
    {
        printf("Q2 mapping failed\n");
        exit(1);
    }
    int buffer_fd = shm_open("/main_buffer", O_CREAT | O_RDWR, 0600);   // the main buffer for produced items
    ftruncate(buffer_fd, N * sizeof(int));
    int *buffer = mmap(NULL, N * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, buffer_fd, 0);
    if (buffer == MAP_FAILED)
    {
        printf("Buffer mapping failed\n");
        exit(1);
    }
    int pr_fd = shm_open("/pr", O_CREAT | O_RDWR, 0600); // the removal index of q1 (producers)
    ftruncate(pr_fd, sizeof(int));
    int *pr = (int *)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, pr_fd, 0);
    int ca_fd = shm_open("/ca", O_CREAT | O_RDWR, 0600); // the addition index of q2 (consumers)
    ftruncate(ca_fd, sizeof(int));
    int *ca = (int *)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, ca_fd, 0);
    srand(time(NULL));
    if (first == true) // if no producers already exist, initialize q1 with values 0..N-1 and set pr and ca to 0
    {
        for (int i = 0; i < N; i++)
            q1[i] = i;
        *pr = 0;
        *ca = 0;
    }

    while (1)
    {
        sem_wait(Sp);
        sem_wait(Sq1);
        int r = q1[*pr];
        *pr = (*pr + 1) % N;
        sem_post(Sq1);
        int product = rand() % 100; // the produced items are random numbers %100
        sleep(rand() % 5 + 5);
        buffer[r] = product;
        printf("Produced %d\n", product);
        sem_wait(Sq2);
        q2[*ca] = r;
        *ca = (*ca + 1) % N;
        sem_post(Sq2);
        sleep(rand() % 5 + 3);
        sem_post(Sc);
    }
    sem_close(Sp);
    sem_close(Sq1);
    sem_close(Sq2);
    sem_close(Sc);
    munmap(q1, N * sizeof(int));
    munmap(buffer, N * sizeof(int));
    munmap(q2, N * sizeof(int));
    munmap(pr, sizeof(int));
    munmap(ca, sizeof(int));
    sem_unlink("/producers");
    sem_unlink("/consumers");
    sem_unlink("/q1");
    sem_unlink("/q2");
    sem_unlink("/Pcheck");
    shm_unlink("/q1fd");
    shm_unlink("/q2fd");
    shm_unlink("/main_buffer");
}