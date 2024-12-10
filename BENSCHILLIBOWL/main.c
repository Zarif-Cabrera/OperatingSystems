#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <semaphore.h>
#include "BENSCHILLIBOWL.h"

#define BENSCHILLIBOWL_SIZE 100
#define NUM_CUSTOMERS 90
#define NUM_COOKS 10
#define ORDERS_PER_CUSTOMER 3
#define EXPECTED_NUM_ORDERS NUM_CUSTOMERS * ORDERS_PER_CUSTOMER

// Global variables for synchronization
BENSCHILLIBOWL *bcb;
pthread_mutex_t bcb_mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex to protect the BENSCHILLIBOWL
sem_t empty_slots;  // Semaphore for empty slots in the restaurant's order queue
sem_t filled_slots; // Semaphore for filled orders ready to be processed

BENSCHILLIBOWL* OpenRestaurant(int max_size, int expected_num_orders);
void CloseRestaurant(BENSCHILLIBOWL* bcb);

// Thread function for customers
void* BENSCHILLIBOWLCustomer(void* tid) {
    int customer_id = (int)(long) tid;

    for (int i = 0; i < ORDERS_PER_CUSTOMER; i++) {
        Order* order = (Order*) malloc(sizeof(Order));
        if (!order) {
            perror("Failed to allocate memory for order");
            return NULL;
        }

        order->menu_item = PickRandomMenuItem();
        order->customer_id = customer_id;
        order->order_number = -1;
        order->next = NULL;

        sem_wait(&empty_slots); // Wait for an empty slot
        pthread_mutex_lock(&bcb_mutex);

        int order_number = AddOrder(bcb, order);
        order->order_number = order_number;
        printf("Customer %d placed order %d for %s\n", customer_id, order_number, order->menu_item);

        pthread_mutex_unlock(&bcb_mutex);
        sem_post(&filled_slots); // Signal a filled slot
    }

    return NULL;
}

// Thread function for cooks
void* BENSCHILLIBOWLCook(void* tid) {
    int cook_id = (int)(long) tid;
    int orders_fulfilled = 0;

    while (1) {
        sem_wait(&filled_slots); // Wait for a filled order
        pthread_mutex_lock(&bcb_mutex);

        if (bcb->orders_handled >= bcb->expected_num_orders) {
            pthread_mutex_unlock(&bcb_mutex);
            sem_post(&filled_slots); // Allow other cooks to exit
            break; // Exit when all orders are handled
        }

        Order* order = GetOrder(bcb);
        if (order) {
            printf("Cook %d fulfilled order %d for customer %d\n", cook_id, order->order_number, order->customer_id);
            free(order); // Free the order memory
            orders_fulfilled++;
            sem_post(&empty_slots); // Signal an empty slot
        }

        pthread_mutex_unlock(&bcb_mutex);
    }

    printf("Cook %d fulfilled %d orders\n", cook_id, orders_fulfilled);
    return NULL;
}

// Main function
int main() {
    // Initialize the semaphores
    sem_init(&empty_slots, 0, BENSCHILLIBOWL_SIZE); // Initially, all slots are empty
    sem_init(&filled_slots, 0, 0);                 // Initially, no orders are filled

    // Open the restaurant
    bcb = OpenRestaurant(BENSCHILLIBOWL_SIZE, EXPECTED_NUM_ORDERS);

    // Create customer threads
    pthread_t customers[NUM_CUSTOMERS];
    for (int i = 0; i < NUM_CUSTOMERS; i++) {
        if (pthread_create(&customers[i], NULL, BENSCHILLIBOWLCustomer, (void*)(long) i) != 0) {
            perror("Failed to create customer thread");
            exit(EXIT_FAILURE);
        }
    }

    // Create cook threads
    pthread_t cooks[NUM_COOKS];
    for (int i = 0; i < NUM_COOKS; i++) {
        if (pthread_create(&cooks[i], NULL, BENSCHILLIBOWLCook, (void*)(long) i) != 0) {
            perror("Failed to create cook thread");
            exit(EXIT_FAILURE);
        }
    }

    // Wait for all customer threads to finish
    for (int i = 0; i < NUM_CUSTOMERS; i++) {
        if (pthread_join(customers[i], NULL) != 0) {
            perror("Failed to join customer thread");
            exit(EXIT_FAILURE);
        }
    }

    // Notify cooks to exit
    for (int i = 0; i < NUM_COOKS; i++) {
        sem_post(&filled_slots); // Signal cooks to exit
    }

    // Wait for all cook threads to finish
    for (int i = 0; i < NUM_COOKS; i++) {
        if (pthread_join(cooks[i], NULL) != 0) {
            perror("Failed to join cook thread");
            exit(EXIT_FAILURE);
        }
    }

    // Close the restaurant
    CloseRestaurant(bcb);

    // Destroy semaphores and mutex
    sem_destroy(&empty_slots);
    sem_destroy(&filled_slots);
    pthread_mutex_destroy(&bcb_mutex);

    return 0;
}
