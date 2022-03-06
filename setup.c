// Kacper Dobek index 148247
// setup file unlinks semaphores and shared memory objects
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>

int main()
{
    sem_unlink("/producers");
    sem_unlink("/consumers");
    sem_unlink("/q1");
    sem_unlink("/q2");
    sem_unlink("/Ccheck");
    sem_unlink("/Pcheck");
    shm_unlink("/main_buffer");
    shm_unlink("/q1fd");
    shm_unlink("/q2fd");
    shm_unlink("/pa");
    shm_unlink("/pr");
    shm_unlink("/ca");
    shm_unlink("/cr");
    return 0;
}