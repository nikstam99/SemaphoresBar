#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include <sys/wait.h>
#include "../include/shared_mem_init.h"

// Αρχικοποίηση όλων των δομών του προγράμματος
int main(int argc, char* argv[]) {
    char* shmid;
    if (argc != 3) {
		printf("Error!\n");
		return(1);
    }
    if (!strcmp(argv[1], "-s")) shmid = argv[2];        // Το όνομα της κοινής μνήμης

    // Αν δοθεί ως όρισμα το 0 στο πρόγραμμα, τότε εκτελείται ένα παράδειγμα με visits
    // μέχρι να φτάσουμε στο μέγιστο αριθμό επισκεπτών
    if (!strcmp(argv[1], "0")) {
        int parent = getpid();
        int status;
        int last;

        // Κάνουμε MAX VISITORS forks για να στείλουμε επισκέπτες στο μπαρ
        for (int i = 0; i < MAX_VISITORS; i++) {
            fork();
            if (getpid() != parent && i == MAX_VISITORS-1) last = getpid();
            if (getpid() == parent) continue;
            sleep(1);

            // Χρήση του exec για να εκτελεστεί το visitor για κάθε fork
            execlp("./visitor", "visitor", "-d", REST_TIME, "-s", SHMID, NULL);
            perror("Visitor exec failure.. ");
			exit(1);
        }
        waitpid(last, &status, 0);
        // Τέλος παραδείγματος
        return 0;
    }
    // Σβήνουμε τα δεδομένα του logging file
    fclose(fopen("logging.txt", "w"));
    sem_unlink(MUTEX);

    // Δημιουργία του mutex semaphore
    sem_t *mutex = sem_open(MUTEX, O_CREAT, 0666, 1);
    if (mutex == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }

    // Δημιουργία της κοινής μνήμης
    int fd = shm_open(shmid, O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        perror("shm_open");
        exit(1);
    }
    
    // Τοποθετούμε την κοινή μνήμη σε έναν δείκτη στο struct shared memory
    ftruncate(fd, sizeof(struct shared_memory));
    Shared_memory shm = mmap(0, sizeof(struct shared_memory), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shm == MAP_FAILED) {
        perror("mmap");
        exit(1);
    } 
    // Δημιουργία όλων των unnamed semaphores
    for (int i = 0; i < SEM_NUM; i++)
    sem_init(&shm->visitors[i], 1, 0);
    sem_init(&shm->end, 1, 0);
    sem_init(&shm->receptionist, 1, 0);
    sem_init(&shm->visit, 1, 0);
    sem_init(&shm->full, 1, 0);

    // Περιμένει για mutual exclusion και αρχικοποιεί όλα τα δεδομένα της κοινής μνήμης
    sem_wait(mutex);
    Table table1 = table_create();
    shm->table1 = *table1;
    Table table2 = table_create();
    shm->table2 = *table2;
    Table table3 = table_create();
    shm->table3 = *table3;
    shm->cheese = 0;
    shm->salad = 0;
    shm->water = 0;
    shm->wine = 0;
    shm->number_of_visitors = 0;
    shm->current_visitors = 0;
    shm->start_index = 0;
    shm->average_stay = 0;
    shm->average_waiting = 0;
    sem_post(mutex);
    close(fd);
    // Free τα τραπέζια από αυτό το πρόγραμμα
    table_destroy(table1);
    table_destroy(table2);
    table_destroy(table3);
    return 0;

}