#include "BENSCHILLIBOWL.h"

#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

bool IsEmpty(BENSCHILLIBOWL* bcb);
bool IsFull(BENSCHILLIBOWL* bcb);
void AddOrderToBack(Order **orders, Order *order);

MenuItem BENSCHILLIBOWLMenu[] = { 
    "BensChilli", 
    "BensHalfSmoke", 
    "BensHotDog", 
    "BensChilliCheeseFries", 
    "BensShake",
    "BensHotCakes",
    "BensCake",
    "BensHamburger",
    "BensVeggieBurger",
    "BensOnionRings",
};
int BENSCHILLIBOWLMenuLength = 10;

/* Select a random item from the Menu and return it */
MenuItem PickRandomMenuItem() {
  int index = rand() % BENSCHILLIBOWLMenuLength;
  return BENSCHILLIBOWLMenu[index];
}

/* Allocate memory for the Restaurant, then create the mutex and condition variables needed to instantiate the Restaurant */

BENSCHILLIBOWL* OpenRestaurant(int max_size, int expected_num_orders) {
  BENSCHILLIBOWL* bcb = (BENSCHILLIBOWL*) malloc(sizeof(BENSCHILLIBOWL));
  assert(bcb != NULL);

  bcb->orders = NULL;
  bcb->current_size = 0;
  bcb->max_size = max_size;
  bcb->next_order_number = 0;
  bcb->orders_handled = 0;
  bcb->expected_num_orders = expected_num_orders;

  pthread_mutex_init(&(bcb->mutex), NULL);
  pthread_cond_init(&(bcb->can_add_orders), NULL);
  pthread_cond_init(&(bcb->can_get_orders), NULL);

  printf("Restaurant is open!\n");
  return bcb;
}

/* check that the number of orders received is equal to the number handled (ie.fullfilled). Remember to deallocate your resources */

void CloseRestaurant(BENSCHILLIBOWL* bcb) {
  if (bcb->orders_handled != bcb->expected_num_orders){
    printf("Warning: Num of orders fulfilled (%d) does not match the expected amount (%d)\n",bcb->orders_handled,bcb->expected_num_orders);
  }

  Order* current = bcb->orders;
  while (current != NULL) {
    Order* temp = current;
    current = current->next;
    free(temp);
  }

  pthread_mutex_destroy(&(bcb->mutex));
  pthread_cond_destroy(&(bcb->can_add_orders));
  pthread_cond_destroy(&(bcb->can_get_orders));

  free(bcb);
  printf("Restaurant is closed!\n");
}

/* add an order to the back of queue */
int AddOrder(BENSCHILLIBOWL* bcb, Order* order) {
  pthread_mutex_lock(&(bcb->mutex));

  // Wait if the restaurant is full
  while (IsFull(bcb)) {
    pthread_cond_wait(&(bcb->can_add_orders), &(bcb->mutex));
  }

  order->order_number = bcb->next_order_number++; // assign an order number
  AddOrderToBack(&(bcb->orders), order); // add to order list
  printf("Adding order %d to the queue...\n", order->order_number);

  bcb->current_size++; // increase current size

  // Signal all cooks that orders are available (use broadcast)
  pthread_cond_broadcast(&(bcb->can_get_orders));
  pthread_mutex_unlock(&(bcb->mutex));

  return order->order_number;
}

/* remove an order from the queue */
Order* GetOrder(BENSCHILLIBOWL* bcb) {
    while (IsEmpty(bcb)) {
        pthread_cond_wait(&(bcb->can_get_orders), &(bcb->mutex));
    }
    Order* order = bcb->orders;
    if (order) {
        bcb->orders = order->next; // Update the queue head
        bcb->current_size--; // Update size
        bcb->orders_handled++; // Track fulfilled orders
        printf("Order %d removed from queue. Orders handled: %d\n", order->order_number, bcb->orders_handled);
        pthread_cond_signal(&(bcb->can_add_orders));
    }
    return order;
}


// Optional helper functions (you can implement if you think they would be useful)
bool IsEmpty(BENSCHILLIBOWL* bcb) {
  return bcb->current_size == 0;
}

bool IsFull(BENSCHILLIBOWL* bcb) {
  return bcb->current_size == bcb->max_size;
}

/* this method adds order to the rear of the queue */
void AddOrderToBack(Order **orders, Order *order) {
  if (*orders == NULL) {
    *orders = order;
  } else {
    Order* current = *orders;
    while (current->next != NULL) {
      current = current->next;
    }
    current->next = order;
  }
  order->next = NULL;
}
