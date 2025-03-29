typedef struct table* Table;
enum seat_state{EMPTY, OCCUPIED, LEFT};             // Το state της κάθε θέσης

enum drink{WATER, WINE, BOTH};                      // Παραγγελία ποτού
enum food{SALAD, CHEESE, ALL, NONE};                // Παραγγελία φαγητού

// Το struct για κάθε τραπέζι
struct table{
    // Οι κενές θέσεις του τραπεζιού
    int empty_seats;
    // Τα id των επισκεπτών σε κάθε θέση
    int ids[4];
    
    // Το state της κάθε θέσης
    int state[4];

    // Ένα flag που δείχνει αν το τραπέζει είναι ανοιχτό για επισκέπτες ή όχι
    int flag;
};

// Το struct για κάθε παραγγελία
struct order{
    int food;
    int drink;
};

// Δημιουργεί ένα καινούριο τραπέζι
Table table_create();

// Διαγράφει ένα τραπέζι 
void table_destroy(Table table);