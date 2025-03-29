#include "table.h"
#include <semaphore.h>

// Το ID της κοινής μνήμης
#define SHMID "sdi2200169"

// Δίνουμε στο named semaphore που χρησιμοποιείται για mutual exclusion το όνομα mutex
#define MUTEX "/mutex"

// Ο αριθμός των μέγιστων επισκεπτών που μπορεί να εξυπηρετήσει το μπαρ σε μία ημέρα
#define MAX_VISITORS 20

// Ο αριθμός των semaphores που υπάρχουν μέσα στο buffer για τη σωστή εξυπηρέτηση σε μορφή FCFS των επισκεπτών
#define SEM_NUM 100

// Ο χρόνος που κάθονται οι επισκέπτες στο μπαρ αφού λάβουν την παραγγελία τους
#define REST_TIME "5"

typedef struct shared_memory* Shared_memory;

// Το struct της κοινής μνήμης
struct shared_memory{
    struct table table1;                // Κάθε τραπέζι αποτελεί ένα table struct
    struct table table2;
    struct table table3;

    int water;                          // Σύνολο της κάθε επιλογής του μενού που έχει καταναλωθεί
    int cheese;                         // τη συγκεκριμένη ημέρα
    int wine;
    int salad;

    // Μέσος όρος αναμονής των επισκεπτών μέχρι να παραγγείλουν
    double average_waiting;

    // Μέσος όρος παραμονής των επισκεπτών από τη στιγμή που έλαβαν την παραγγελία τους
    double average_stay;

    // Αριθμός των συνολικών επισκεπτών
    int number_of_visitors;  

    // Οι επισκέπτες που βρίσκονται αυτή τη στιγμή στο μπαρ          
    int current_visitors;

    // Ένα index του buffer που δείχνει ποιος είναι ο επόμενος πελάτης μέσα στο buffer που περιμένει να κάνει παραγγελία               
    int start_index;

    // Στο semaphore visit περιμένουν όσοι επισκέπτες δεν χωράνε στο buffer με τα semaphores
    sem_t visit;

    // Μπλοκάρει την είσοδο επισκεπτών αν το μπαρ είναι γεμάτο
    sem_t full;

    // Ο buffer με τα semaphores που περιμένουν οι επισκέπτες να μπουν στο μπαρ
    sem_t visitors[SEM_NUM];

    // Σηματοδοτεί το τέλος του προγράμματος του receptionist 
    sem_t end;

    // Το semaphore που ξυπνάει και κοιμίζει τον receptionist αν έχει δουλειά ή όχι
    sem_t receptionist;
};
