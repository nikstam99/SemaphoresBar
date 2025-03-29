#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>
#include "../include/shared_mem_init.h"

int main(int argc, char* argv[]) {
    sem_t *mutex = sem_open(MUTEX, 0);                          // Άνοιγμα του mutex semaphore
    if (mutex == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }
    int ordertime;
    char* shmid;
    if (argc != 5) {
		printf("Error!\n");
		return(1);
	}

    for (int i = 1; i < 4; i+=2) {
        if (!strcmp(argv[i], "-d")) {
            ordertime = atoi(argv[i+1]); 
        }
        else if (!strcmp(argv[i], "-s")) {
            shmid = argv[i+1];
        }
    }

    int fd = shm_open(SHMID, O_RDWR, 0666);                   // Άνοιγμα της κοινής μνήμης
    if (fd == -1) {
        perror("shm_open");
        exit(1);
    }

    Shared_memory shm = mmap(NULL, sizeof(struct shared_memory), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if (shm == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }
    // Ο receptionist δουλεύει μέχρι το μέγιστο αριθμό των επισκεπτών
    while(shm->number_of_visitors < MAX_VISITORS) {

        // Αν δεν υπάρχει διαθέσιμος χώρος στο μπαρ περιμένουμε να αδειάσει κάποιο τραπέζι
        if (shm->current_visitors == 12 ||( shm->table1.flag == 0 && shm->table2.flag == 0 && shm->table3.flag == 0)) sem_wait(&shm->full);
        
        // Ανοίγουμε το visit semaphore για τους επισκέπτες που θέλουν να μπουν στο buffer
        sem_post(&shm->visit);
        // Περιμένουμε να εξυπηρετηθεί ο πελάτης στη θέση start index
        sem_wait(&shm->visitors[shm->start_index]);
        // mutual exclusion
        sem_wait(mutex);

        // Προσομοίωση ετοιμασίας της παραγγελίας με sleep για [0.5*ordertime, ordertime] secs
        int max = ordertime;
        int min = 0.5*ordertime;
        srand(time(0));
        sleep(rand() % (max-min+1)+min);
        sem_post(mutex);

        // Τέλος δουλειάς του receptionist
        sem_post(&shm->receptionist);
    }

    // Περιμένουμε το semaphore που σηματοδοτεί το τέλος της ημέρας 
    // και εκτυπώνουμε στο tty τα στατιστικά του μπαρ
    sem_wait(&shm->end);
    printf("-----------------------------\n");
    printf("%d visitors!\n", shm->number_of_visitors);
    printf("-----------------------------\n");
    printf("Total Consumtion: \n");
    printf("Cheese:%d\nSalad: %d\nWine: %d\nWater: %d\n", shm->cheese, shm->salad, shm->wine, shm->water);
    printf("-----------------------------\n");
    printf("Average waiting time: %f secs\nAverage stay time: %f secs\n", shm->average_waiting/(double)shm->number_of_visitors, shm->average_stay/(double)shm->number_of_visitors);
    printf("-----------------------------\n");
    close(fd);

    // Destroy/unlink όλα τα semaphores και την κοινή μνήμη
    for (int i = 0; i < SEM_NUM; i++)
    sem_destroy(&shm->visitors[i]);
    sem_destroy(&shm->end);
    sem_destroy(&shm->receptionist);
    sem_destroy(&shm->visit);
    sem_destroy(&shm->full);
    sem_unlink(MUTEX);

    // Deallocate shared memory
    if (munmap(shm, sizeof(struct shared_memory)) == -1) {
        printf("Munmap error!\n");
        exit(1);
    }
    shm_unlink(SHMID);
    return 0;
}