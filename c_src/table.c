#include <stdlib.h>
#include "../include/table.h"

Table table_create() {
    Table table = malloc(sizeof(struct table));             // Αρχικοποίηση των δεδομένων του τραπεζιού
    table->empty_seats = 4;
    table->flag = 1;                                       
    for (int i = 0; i < 4; i++) {
        table->ids[i] = 0;
        table->state[i] = EMPTY;
    }
    return table;
}

void table_destroy(Table table) {                        // Διαγραφή του τραπεζιού
    free(table);
}