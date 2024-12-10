//Zarif + Riana

#include "list.h"
#include <sys/socket.h>
#include <string.h> 

struct node *insertFirstU(struct node *head, int clientSocket, char *username) {
    struct node *newUser = malloc(sizeof(struct node));
    newUser->socket = clientSocket;
    strncpy(newUser->username, username, sizeof(newUser->username) - 1);
    newUser->next = head;  // Point the next to the current head (old user)
    return newUser;  // The new user becomes the new head
}

// Find a user by username
struct node* findU(struct node *head, const char* username) {
    pthread_mutex_lock(&user_mutex);

    struct node* current = head;
    while (current) {
        if (strcmp(current->username, username) == 0) {
            pthread_mutex_unlock(&user_mutex);
            return current;
        }
        current = current->next;
    }

    pthread_mutex_unlock(&user_mutex);
    return NULL;
}

// Remove a user from the user list by their socket
void removeUser(struct node **head, int socket) {
    pthread_mutex_lock(&user_mutex);

    struct node *current = *head;
    struct node *prev = NULL;

    while (current) {
        if (current->socket == socket) {
            if (prev) {
                prev->next = current->next;
            } else {
                *head = current->next;
            }
            free(current);
            pthread_mutex_unlock(&user_mutex);
            return;
        }
        prev = current;
        current = current->next;
    }

    pthread_mutex_unlock(&user_mutex);
}

// Create a new room
struct room *createRoom(char *name) {
    pthread_mutex_lock(&room_mutex);

    struct room *newRoom = (struct room *)malloc(sizeof(struct room));
    if (!newRoom) {
        perror("Failed to allocate memory for new room");
        pthread_mutex_unlock(&room_mutex);
        return NULL;
    }

    strcpy(newRoom->name, name);
    newRoom->users = NULL;
    newRoom->next = NULL;

    pthread_mutex_unlock(&room_mutex);
    return newRoom;
}

// Find a room by name
struct room *findRoom(struct room *head, char *name) {
    pthread_mutex_lock(&room_mutex);

    struct room *current = head;
    while (current) {
        if (strcmp(current->name, name) == 0) {
            pthread_mutex_unlock(&room_mutex);
            return current;
        }
        current = current->next;
    }

    pthread_mutex_unlock(&room_mutex);
    return NULL;
}

// Add a user to a room
void addUserToRoom(struct room *room, struct node *user) {
    pthread_mutex_lock(&room_mutex);

    if (!room->users) {
        room->users = user;
    } else {
        struct node *current = room->users;
        while (current->next) {
            if (current->socket == user->socket) {
                pthread_mutex_unlock(&room_mutex);
                return; // Avoid duplicates
            }
            current = current->next;
        }
        current->next = user;
    }

    pthread_mutex_unlock(&room_mutex);
}

// Remove a user from a room
void removeUserFromRoom(struct room *room, struct node *user) {
    pthread_mutex_lock(&room_mutex);

    struct node *current = room->users;
    struct node *prev = NULL;

    while (current) {
        if (current->socket == user->socket) {
            if (prev) {
                prev->next = current->next;
            } else {
                room->users = current->next;
            }
            free(current);
            break;
        }
        prev = current;
        current = current->next;
    }

    pthread_mutex_unlock(&room_mutex);
}

// Get a string with the list of all rooms
void getAllRooms(struct room *head, char *buffer) {
    pthread_mutex_lock(&room_mutex);  // Lock room list for thread safety

    struct room *current = head;
    while (current) {
        strcat(buffer, current->name);  // Append room name to buffer
        strcat(buffer, "\n");
        current = current->next;
    }

    pthread_mutex_unlock(&room_mutex);  // Unlock after reading the list
}

// Remove a user from all rooms
void removeUserFromAllRooms(struct room *head, struct node *user) {
    pthread_mutex_lock(&room_mutex);  // Lock room list for thread safety

    struct room *currentRoom = head;
    while (currentRoom) {
        removeUserFromRoom(currentRoom, user);  // Remove user from current room
        currentRoom = currentRoom->next;
    }

    pthread_mutex_unlock(&room_mutex);  // Unlock after modification
}

struct room* insertFirstRoom(struct room *head, const char *name) {
    struct room *newRoom = (struct room*) malloc(sizeof(struct room));
    strncpy(newRoom->name, name, sizeof(newRoom->name) - 1);
    newRoom->users = NULL;
    newRoom->next = head;
    return newRoom;
}

struct room* findUserRoom(struct room *roomsHead, const char *username) {
    struct room *currentRoom = roomsHead;  // Start from the head of the rooms list

    // Iterate through all rooms
    while (currentRoom != NULL) {
        struct node *user = currentRoom->users;

        // Check each user in the room's user list
        while (user != NULL) {
            if (strcmp(user->username, username) == 0) {
                return currentRoom;  // User found in this room
            }
            user = user->next;
        }

        currentRoom = currentRoom->next;  // Move to the next room
    }

    return NULL;  // User not found in any room
}

struct node *findUserBySocket(struct node *head, int socket) {
    struct node *current = head;
    while (current != NULL) {
        if (current->socket == socket) {
            return current;  // Return the user node if the socket matches
        }
        current = current->next;
    }
    return NULL;  // Return NULL if no matching user is found
}


