#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>
#include "../include/shared_mem_init.h"

// Το monitor απεικονίζει την τρέχουσα κατάσταση των τραπεζιών του μπαρ
// καθώς και την συνολική κατανάλωση εώς τώρα
int main(int argc, char* argv[]) {

    sem_t *mutex = sem_open(MUTEX, 0);                              // Άνοιγμα του mutex semaphore
    if (mutex == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }
    char* shmid;
    if (!strcmp(argv[1], "-s")) {
        shmid = argv[2];
    }
    int fd = shm_open(shmid, O_RDWR, 0666);                         // Άνοιγμα της κοινής μνήμης
    if (fd == -1) {
        perror("shm_open");
        exit(1);
    }
    Shared_memory shm = mmap(NULL, sizeof(struct shared_memory), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if (shm == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }
    // Εκτύπωση της κατάστασης του μπαρ
    printf("-----------------------------\n");
    printf("Table 1 Free seats: %d\n", shm->table1.empty_seats);
    printf("Seat 1: %d  Seat 2: %d  Seat 3: %d  Seat 4: %d\n", shm->table1.ids[0], 
    shm->table1.ids[1], shm->table1.ids[2], shm->table1.ids[3]);

    printf("-----------------------------\n");
    printf("Table 2 Free seats: %d\n", shm->table2.empty_seats);
    printf("Seat 1: %d  Seat 2: %d  Seat 3: %d  Seat 4: %d\n", shm->table2.ids[0], 
    shm->table2.ids[1], shm->table2.ids[2], shm->table2.ids[3]);

    printf("-----------------------------\n");
    printf("Table 3 Free seats: %d\n", shm->table3.empty_seats);
    printf("Seat 1: %d  Seat 2: %d  Seat 3: %d  Seat 4: %d\n", shm->table3.ids[0], 
    shm->table3.ids[1], shm->table3.ids[2], shm->table3.ids[3]);

    printf("-----------------------------\n");
    printf("Total Consumtion until now: \n");
    printf("Cheese:%d\nSalad: %d\nWine: %d\nWater: %d\n", shm->cheese, shm->salad, shm->wine, shm->water);
    printf("-----------------------------\n");
    printf("Average waiting time: %f\nAverage stay time: %f\n", shm->average_waiting/(double)shm->number_of_visitors, shm->average_stay/(double)shm->number_of_visitors);
    printf("-----------------------------\n");
    close(fd);
    return 0;
}