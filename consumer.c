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
#include <stdbool.h>

#define N 5

int main()
{
    bool first = true;
    sem_t *Sp, *Sc, *Sq1, *Sq2, *Ccheck;
    Sq1 = sem_open("/q1", O_CREAT, 0600, 1);
    Sq2 = sem_open("/q2", O_CREAT, 0600, 1);
    Sp = sem_open("/producers", O_CREAT, 0600, N);
    Sc = sem_open("/consumers", O_CREAT, 0600, 0);
    Ccheck = sem_open("/Ccheck", O_CREAT | O_EXCL, 0600, 0); // I use this semaphore with O_EXCL flag just to check if there are any producers already running
    if (Ccheck == SEM_FAILED)
        first = false;
    if (Sc == SEM_FAILED || Sq1 == SEM_FAILED || Sq2 == SEM_FAILED)
    {
        perror("sem_open");
        exit(1);
    }
    int q1_fd = shm_open("/q1fd", O_CREAT | O_RDWR, 0600);  // the queue for producers
    ftruncate(q1_fd, N * sizeof(int));
    int *q1 = mmap(NULL, N * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, q1_fd, 0);
    if (q1 == MAP_FAILED)
    {
        printf("Q1 mapping failed\n");
        exit(1);
    }
    int q2_fd = shm_open("/q2fd", O_CREAT | O_RDWR, 0600);  // the queue for consumers
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
    int pa_fd = shm_open("/pa", O_CREAT | O_RDWR, 0600);    // the addition index of q1 (producers)
    ftruncate(pa_fd, sizeof(int));
    int *pa = (int *)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, pa_fd, 0);
    int cr_fd = shm_open("/cr", O_CREAT | O_RDWR, 0600);    // the removal index of q2 (consumers)
    ftruncate(cr_fd, sizeof(int));
    int *cr = (int *)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, cr_fd, 0);
    srand(time(NULL));
    if (first == true)  // if no consumers already exist, set pa and cr to 0
    {
        *pa = 0;
        *cr = 0;
    }
    while (1)
    {
        sem_wait(Sc);
        sem_wait(Sq2);
        int r = q2[*cr];
        *cr = (*cr + 1) % N;
        sem_post(Sq2);
        sleep(rand() % 5 + 4);
        int product = buffer[r];
        printf("Consumed %d\n", product);
        sem_wait(Sq1);
        q1[*pa] = r;
        *pa = (*pa + 1) % N;
        sem_post(Sq1);
        sleep(rand() % 4 + 2);
        sem_post(Sp);
    }
    sem_close(Sp);
    sem_close(Sc);
    sem_close(Sq1);
    sem_close(Sq2);
    sem_close(Ccheck);
    munmap(q1, N * sizeof(int));
    munmap(buffer, N * sizeof(int));
    munmap(q2, N * sizeof(int));
    munmap(pa, sizeof(int));
    munmap(cr, sizeof(int));
    sem_unlink("/producers");
    sem_unlink("/consumers");
    sem_unlink("/q1");
    sem_unlink("/q2");
    shm_unlink("/q1fd");
    shm_unlink("/q2fd");
    shm_unlink("/main_buffer");
}