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

// Δημιουργία μιας νέας (τυχαίας) παραγγελίας
struct order make_order() {
    struct order Order;
    srand(time(0));
    Order.drink = rand() % 3;
    Order.food = rand() % 4;
    return Order;
}

int main(int argc, char* argv[]) {
    sem_t *mutex = sem_open(MUTEX, 0);                          // Άνοιγμα του mutex semaphore
    if (mutex == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }
    time_t n;
    struct tm arrived;
    struct tm sat;
    struct tm left;
    int resttime;
    char* shmid;
    if (argc != 5) {
		printf("Error!\n");
		return(1);
	}
    for (int i = 1; i < 4; i+=2) {
        if (!strcmp(argv[i], "-d")) {
            resttime = atoi(argv[i+1]);                     // Rest time
        }
        else if (!strcmp(argv[i], "-s")) {                  // Όνομα κοινής μνήμης
            shmid = argv[i+1];
        }
    }
    int fd = shm_open(shmid, O_RDWR, 0666);                 // Άνοιγμα κοινής μνήμης
    if (fd == -1) {
        perror("shm_open");
        exit(1);
    }

    Shared_memory shm = mmap(NULL, sizeof(struct shared_memory), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if (shm == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }
    // mutual exclusion
    sem_wait(mutex);

    // Νέος επισκέπτης
    printf("New visitor! (%d)\n", getpid());

    // Αποθήκευση της ώρας που έφτασε
    time(&n);
    arrived = *localtime(&n);
    sem_post(mutex);

    // Περιμένουμε να ανοίξει το κοινό semaphore
    sem_wait(&shm->visit);
 
    // mutual exclusion
    sem_wait(mutex);

    struct order Order;
    int max = resttime;
    int min = 0.7*resttime;
    int pos;
    int table = 0;

    // Ο πελάτης ψάχνει για ελεύθερη θέση και αν δεν βρει περιμένει κάποιος να φύγει
    while(1) {

    // Έλεγχος για ελεύθερη θέση και δημιουργία παραγγελίας στο τραπέζι 1
    if (shm->table1.flag > 0) {
        for (int i = 0; i < 4; i++) {
            if (shm->table1.state[i] == EMPTY) {
                shm->table1.ids[i] = getpid();
                shm->table1.state[i] = OCCUPIED;
                shm->table1.empty_seats--;

                // Αν δεν υπάρχει άλλη κενή θέση αφού ο επισκέπτης καθίσει τότε το τραπέζι κλειδώνει μέχρι να αδειάσει
                if (shm->table1.empty_seats == 0) shm->table1.flag = 0;
                pos = i;
                table = 1;
                break;
            }
        }
        Order = make_order();
    }
    // Έλεγχος για ελεύθερη θέση και δημιουργία παραγγελίας στο τραπέζι 2
    if (shm->table2.flag > 0 && table == 0) {
        for (int i = 0; i < 4; i++) {
            if (shm->table2.state[i] == EMPTY) {
                shm->table2.ids[i] = getpid();
                shm->table2.state[i] = OCCUPIED;
                shm->table2.empty_seats--;

                // Αν δεν υπάρχει άλλη κενή θέση αφού ο επισκέπτης καθίσει τότε το τραπέζι κλειδώνει μέχρι να αδειάσει
                if (shm->table2.empty_seats == 0) shm->table2.flag = 0;
                pos = i;
                table = 2;
                break;
            }
        }
        Order = make_order();
    }
    // Έλεγχος για ελεύθερη θέση και δημιουργία παραγγελίας στο τραπέζι 3
    if (shm->table3.flag > 0 && table == 0) {
        for (int i = 0; i < 4; i++) {
            if (shm->table3.state[i] == EMPTY) {
                shm->table3.ids[i] = getpid();
                shm->table3.state[i] = OCCUPIED;
                shm->table3.empty_seats--;

                // Αν δεν υπάρχει άλλη κενή θέση αφού ο επισκέπτης καθίσει τότε το τραπέζι κλειδώνει μέχρι να αδειάσει
                if (shm->table3.empty_seats == 0) shm->table3.flag = 0;
                pos = i;
                table = 3;
                break;
            }
        }
        Order = make_order();
    }
    // Αν βρήκε θέση παραγγέλνει
    if (table > 0) break;
    // Αν δεν βρήκε βγαίνουμε από το mutual exclusion για να φύγει κάποιος
    else {
        sem_post(mutex);
        sem_wait(mutex);
    }
    }

    // Ενημέρωση των στατιστικών της κοινής μνήμης για τη νέα παραγγελία
    if (Order.drink == 0) {
        shm->water++;
    }
    else if (Order.drink == 1) {
        shm->wine++;
    }
    else if (Order.drink == 2) {
        shm->water++;
        shm->wine++;
    }
    if (Order.food == 0) {
        shm->salad++;
    }
    else if (Order.food == 1) {
        shm->cheese++;
    }
    else if (Order.food == 2) {
        shm->salad++;
        shm->cheese++;
    }
    // Τέλος της παραγγελίας
    sem_post(&shm->visitors[shm->start_index]);
    shm->number_of_visitors++;
    shm->current_visitors++;

    // Αύξηση του start index για να πάει στον επόμενο πελάτη που θα παραγγείλει
    shm->start_index = (shm->start_index+1) % SEM_NUM;
    sem_post(mutex);

    // Περιμένουμε τον receptionist να φτιάξει την παραγγελία
    sem_wait(&shm->receptionist);
    // Αποθήκευση της ώρας που ο επισκέπτης έκατσε
    time(&n);
    sat = *localtime(&n);

    // Προσομοίωση με sleep όσο ο πελάτης τρώει/πίνει
    srand(time(0));
    sleep(rand() % (max-min+1)+min);

    //mutual exclusion
    sem_wait(mutex);

    // Ο πελάτης φεύγει και ενημερώνει την κοινή μνήμη για να αδειάσει η θέση του
    shm->current_visitors--;
    if (table == 1) {
        shm->table1.empty_seats++;
        shm->table1.ids[pos] = 0;
        shm->table1.state[pos] = LEFT;

        // Αν οι κενές θέσεις είναι 4 το τραπέζει ανοίγει για επισκέπτες
        if (shm->table1.empty_seats == 4) {
            shm->table1.flag = 1;
            for (int i = 0; i < 4; i++) {
                shm->table1.state[i] = EMPTY;
            }
            sem_post(&shm->full);
        }
    }
    if (table == 2) {
        shm->table2.empty_seats++;
        shm->table2.ids[pos] = 0;
        shm->table2.state[pos] = LEFT;

        // Αν οι κενές θέσεις είναι 4 το τραπέζει ανοίγει για επισκέπτες
        if (shm->table2.empty_seats == 4) {
            shm->table2.flag = 1;
            for (int i = 0; i < 4; i++) {
                shm->table2.state[i] = EMPTY;
            }
            sem_post(&shm->full);
        }
    }
    if (table == 3) {
        shm->table3.empty_seats++;
        shm->table3.ids[pos] = 0;
        shm->table3.state[pos] = LEFT;

        // Αν οι κενές θέσεις είναι 4 το τραπέζει ανοίγει για επισκέπτες
        if (shm->table3.empty_seats == 4) {
            shm->table3.flag = 1;
            for (int i = 0; i < 4; i++) {
                shm->table3.state[i] = EMPTY;
            }
            sem_post(&shm->full);
        }
    }

    // Καταγραφή των στατιστικών του επισκέπτη στο logging file
    FILE *fd2;
    fd2 = fopen("/io_files/logging.txt", "a");
    // Aποθήκευση της ώρας που έφυγε
    time(&n);
    left = *localtime(&n);
    if (sat.tm_sec >= arrived.tm_sec)
        shm->average_waiting += sat.tm_sec - arrived.tm_sec + 60*(sat.tm_min - arrived.tm_min);
    else 
        shm->average_waiting += 60*(sat.tm_min - arrived.tm_min) - arrived.tm_sec + sat.tm_sec;

    if (left.tm_sec >= sat.tm_sec)
        shm->average_stay += left.tm_sec - sat.tm_sec + 60*(left.tm_min - sat.tm_min);
    else 
        shm->average_stay += 60*(left.tm_min - sat.tm_min) - sat.tm_sec + left.tm_sec;

    // Εκτύπωση της πραγματικής ώρας σε GMT για κάθε λειτουργία του επισκέπτη (έφτασε, έκατσε, έφυγε)
    fprintf(fd2, "Id: %d | Arrived: %02d:%02d:%02d | Took a Seat: %02d:%02d:%02d | Left: %02d:%02d:%02d | (Table %d, Seat: %d)\n", getpid(), arrived.tm_hour, arrived.tm_min, arrived.tm_sec, 
    sat.tm_hour, sat.tm_min, sat.tm_sec, left.tm_hour, left.tm_min, left.tm_sec, table, pos+1);
    fclose(fd2);
    sem_post(mutex);
    // Αν φτάσουμε τους μέγιστους επισκέπτες τότε ενημερώνουμε το end semaphore για να κλέισει ο receptionist
    if (shm->number_of_visitors == MAX_VISITORS) sem_post(&shm->end);
    close(fd);

    return 0;
}