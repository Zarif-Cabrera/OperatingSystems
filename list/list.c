// list/list.c
// 
// Implementation for linked list.
//
// <Author>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"

list_t *list_alloc() { 
  list_t* mylist =  (list_t *) malloc(sizeof(list_t)); 
  mylist->head = NULL;
}

void list_free(list_t *l) {
  if (l == NULL) {
    return;
  }

  node_t *current = l->head;
  node_t *next_node;

  while (current!= NULL) {
    next_node = current->next;
    free(current);
    current = next_node;
  }

  l->head = NULL;
}

void list_print(list_t *l) {
  if (l == NULL) {
    printf("List is NULL\n");
    return;
  }

  node_t *current = l->head;

  if (current == NULL) {
    printf("List is empty\n");
    return;
    
  }

  while (current != NULL) {
    printf("%d -> ", current->value);
    current = current->next;
  }

  printf("NULL\n");
}

char * listToString(list_t *l) {
  char* buf = (char *) malloc(sizeof(char) * 10024);
  char tbuf[20];

	node_t* curr = l->head;
  while (curr != NULL) {
    sprintf(tbuf, "%d->", curr->value);
    curr = curr->next;
    strcat(buf, tbuf);
  }
  strcat(buf, "NULL");
  return buf;
}

int list_length(list_t *l) { 

    node_t *current = l->head;

    int count = 0;


    while (current != NULL) {
        count++;
        current = current->next;
  }
  return count; 
  
  }

void list_add_to_back(list_t *l, elem value) {
     node_t *cur_node = (node_t *) getNode(value);

    if (l->head == NULL) {
      l->head = cur_node;

    } else {

      node_t *current = l->head;

      while (current->next!= NULL) {
        current = current ->next;
      }
      current->next = cur_node;
    }
  
}

void list_add_to_front(list_t *l, elem value) {
     node_t *cur_node = (node_t *) getNode(value);

     /* Insert to front */
     node_t *head = l->head;  // get head of list

     cur_node->next = head;
     l->head = cur_node;
}

node_t * getNode(elem value) {
  node_t *mynode;

  mynode = (node_t *) malloc(sizeof(node_t));
  mynode->value = value;
  mynode->next = NULL;

  return mynode;
}

void list_add_at_index(list_t *l, elem value, int index) {
     node_t *cur_node = (node_t *) getNode(value);

     node_t *current = l->head;
     int current_index = 0;

     if (index == 0) {
        list_add_to_front(l, value);
     }
     
     else {
        while(current != NULL && current_index< index - 1) {
          current = current->next;
          current_index++;
        }

        cur_node->next = current->next;
        current->next = cur_node;
     }

}

elem list_remove_from_back(list_t *l) { 
  
  if (l == NULL || l->head == NULL) {
    return -1;
  } 

  if (l->head->next == NULL) {
    elem value = l->head->value;
    free(l->head);
    l->head = NULL;
    return value;
  }

  node_t *current = l->head;
  while (current->next->next!= NULL) {
    current = current->next;
  }

  node_t *last_node = current->next;
  elem value = last_node->value;
  current->next = NULL;
  free(last_node);

  return value;


  }
   
elem list_remove_from_front(list_t *l) { 
   if (l == NULL || l->head == NULL) {
    return -1;
  } 

  node_t *node_to_remove = l->head;
  elem value = node_to_remove->value;
  l->head = node_to_remove->next;
  free(node_to_remove);

  return value;
}
elem list_remove_at_index(list_t *l, int index) {
   if (l == NULL || l->head == NULL) {
    return -1;
  } 

    if (index == 0){
      return list_remove_from_front(l);
    }

    node_t *current = l->head;
    int current_index = 0;

    while(current != NULL && current_index < index - 1) {
          current = current->next;
          current_index++;
        }
  
    if (current == NULL || current->next == NULL) {
        return -1; 
    }

    node_t *node_to_remove = current->next;
    elem value = node_to_remove->value; 
    current->next = node_to_remove->next; 
    free(node_to_remove); 

    return value;


 }

bool list_is_in(list_t *l, elem value) { 
  if (l == NULL){
    printf("false\n");
    return false; 
  }

  node_t *current = l->head;

  while (current!= NULL) {
    if (current->value == value) {
      printf("true\n");
      return true;
    }
    current = current->next;
  }
  printf("false\n");
  return false;

  }

elem list_get_elem_at(list_t *l, int index) { 
  if (l == NULL || index < 0) {
    return -1;
  }

  node_t *current = l->head;
  int current_index = 0;

  while (current!= NULL) {
    if (current_index == index) {
      return current->value;
    }
    current = current->next;
    current_index++;
  }

  return -1;
  }

int list_get_index_of(list_t *l, elem value) { 
  if (l == NULL) {
    return -1;
  }

  node_t *current = l->head;
  int current_index = 0;

  while (current != NULL) {
    if (current->value == value) {
      return current_index;
    }
    current = current->next;
    current_index++;
  }

  return -1;

  }

