#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include <sys/wait.h>
#include "../include/shared_mem_init.h"

int main() {
    int fd = shm_open(SHMID, O_RDWR, 0666);
    if (fd == -1) {
        perror("shm_open");
        exit(1);
    }
    
    Shared_memory shm = mmap(0, sizeof(struct shared_memory), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shm == MAP_FAILED) {
        perror("mmap");
        exit(1);
    } 

    for (int i = 0; i < SEM_NUM; i++)
    sem_destroy(&shm->visitors[i]);
    sem_destroy(&shm->end);
    sem_destroy(&shm->receptionist);
    sem_destroy(&shm->visit);
    sem_destroy(&shm->full);
    sem_unlink(MUTEX);

    if (munmap(shm, sizeof(struct shared_memory)) == -1) {
        printf("Munmap error!\n");
        exit(1);
    }
    shm_unlink(SHMID);
    close(fd);
}