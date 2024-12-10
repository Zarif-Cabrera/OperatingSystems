// team: Zarif + Riana

#include "server.h"

#define DEFAULT_ROOM "Lobby"

// USE THESE LOCKS AND COUNTER TO SYNCHRONIZE
extern int numReaders;
extern pthread_mutex_t rw_lock;
extern pthread_mutex_t mutex;

extern struct node *head;
extern char *server_MOTD;

// Helper function to trim whitespace
char *trimwhitespace(char *str) {
    char *end;

    // Trim leading space
    while (isspace((unsigned char)*str)) str++;

    if (*str == 0)  // All spaces?
        return str;

    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;

    // Write new null terminator character
    end[1] = '\0';

    return str;
}

void *client_receive(void *ptr) {
    int client = *(int *)ptr;  // socket
    int received, i;
    char buffer[MAXBUFF], sbuffer[MAXBUFF];  // data buffer of 2K
    char tmpbuf[MAXBUFF];  // data temp buffer of 1K
    char cmd[MAXBUFF], username[20];
    char *arguments[80];

    struct node *currentUser;
    struct room *roomsHead = NULL;

    // Send MOTD (Message of the Day)
    send(client, server_MOTD, strlen(server_MOTD), 0);

    if (client <= 0) {
        printf("Invalid client socket: %d\n", client);
        return NULL;  // Exit if client socket is invalid
    }

    // Assign a guest username
    sprintf(username, "guest%d", client);
    pthread_mutex_lock(&mutex); // Ensure thread safety when modifying the user list
    head = insertFirstU(head, client, username);
    pthread_mutex_unlock(&mutex);

    // Add the guest to the default room (Lobby)
    pthread_mutex_lock(&rw_lock);  // Lock read-write lock to ensure no other thread is modifying the rooms
    struct room *lobby = findRoom(roomsHead, DEFAULT_ROOM);
    pthread_mutex_unlock(&rw_lock);

    printf("Client thread started for client socket(V63): %d\n", client);  // Debugging log

    if (!lobby) {
        // Room not found, so create it
        lobby = createRoom(DEFAULT_ROOM);  // Create the Lobby room
        pthread_mutex_lock(&rw_lock);
        roomsHead = insertFirstRoom(roomsHead, lobby->name);  // Add it to the room list
        pthread_mutex_unlock(&rw_lock);
    }

    if (lobby) {
        pthread_mutex_lock(&rw_lock);  // Lock write access to the room list to prevent race conditions
        lobby->users = insertFirstU(lobby->users, client, username);
        pthread_mutex_unlock(&rw_lock);

        printf("User %s added to Lobby.\n", username);
    } else {
        fprintf(stderr, "Default room 'Lobby' not found!\n");
    }

    // Send a welcome message to the user
    sprintf(buffer, "Welcome %s! You have been added to the default room: %s.\nchat>", username, DEFAULT_ROOM);
    send(client, buffer, strlen(buffer), 0);

    while (1) {
        printf("Waiting for commands...\n");
        if ((received = read(client, buffer, MAXBUFF)) != 0) {
            buffer[received] = '\0';
            strcpy(cmd, buffer);
            strcpy(sbuffer, buffer);

            // Tokenize the input in buf (split it on whitespace)
            arguments[0] = strtok(cmd, delimiters);

            i = 0;
            while (arguments[i] != NULL) {
                arguments[++i] = strtok(NULL, delimiters);
                strcpy(arguments[i - 1], trimwhitespace(arguments[i - 1]));
            }

            // Debugging - print the received command and arguments
            printf("Received command: %s\n", arguments[0]);
            for (int j = 0; j < i; j++) {
                printf("Argument %d: %s\n", j, arguments[j]);
            }

            // Handle commands with synchronization
            if (strcmp(arguments[0], "login") == 0) {
                // Handle login: User sets their username
                strcpy(username, arguments[1]);

                // Check if the username is already taken
                pthread_mutex_lock(&mutex);  // Lock mutex to ensure thread safety for user list
                currentUser = findU(head, username);
                pthread_mutex_unlock(&mutex);

                if (currentUser) {
                    // Username already taken
                    sprintf(buffer, "Username '%s' already taken.\nchat>", username);
                } else {
                    pthread_mutex_lock(&mutex);  // Lock mutex to safely remove user
                    removeUser(&head, client);
                    removeUser(&head, client);
                    pthread_mutex_unlock(&mutex);

                    // Send success message
                    sprintf(buffer, "Logged in as %s.\nchat>", username);

                    pthread_mutex_lock(&mutex);  // Insert user after checking for uniqueness
                    head = insertFirstU(head, client, username);  // Add to users list
                    pthread_mutex_unlock(&mutex);
                }

                // Send the response to the client
                send(client, buffer, strlen(buffer), 0);
            }
            else if (strcmp(arguments[0], "create") == 0) {
                // Handle room creation with synchronization
                pthread_mutex_lock(&rw_lock);  // Lock write access to the room list
                struct room *newRoom = createRoom(arguments[1]);
                if (newRoom) {
                    roomsHead = insertFirstRoom(roomsHead, newRoom->name);
                    sprintf(buffer, "Room '%s' created successfully!\nchat>", arguments[1]);
                } else {
                    sprintf(buffer, "Failed to create room '%s'.\nchat>", arguments[1]);
                }
                pthread_mutex_unlock(&rw_lock);

                send(client, buffer, strlen(buffer), 0);
            }
            else if (strcmp(arguments[0], "join") == 0) {
                // Handle joining a room
                pthread_mutex_lock(&rw_lock);  // Lock read-write lock to ensure thread safety for room list
                struct room *room = findRoom(roomsHead, arguments[1]);
                pthread_mutex_unlock(&rw_lock);

                if (room) {
                    pthread_mutex_lock(&mutex);  // Lock mutex for user list
                    struct node *user = findU(head, username);
                    addUserToRoom(room, user);
                    pthread_mutex_unlock(&mutex);
                    sprintf(buffer, "Joined room '%s'.\nchat>", arguments[1]);
                } else {
                    sprintf(buffer, "Room '%s' not found.\nchat>", arguments[1]);
                }
                send(client, buffer, strlen(buffer), 0);
            }
            else if (strcmp(arguments[0], "leave") == 0) {
                // Handle leaving a room
                pthread_mutex_lock(&rw_lock);  // Lock room list for thread safety
                struct room *room = findRoom(roomsHead, arguments[1]);
                pthread_mutex_unlock(&rw_lock);

                if (room) {
                    pthread_mutex_lock(&mutex);  // Lock user list for thread safety
                    struct node *user = findU(head, username);
                    removeUserFromRoom(room, user);
                    pthread_mutex_unlock(&mutex);
                    sprintf(buffer, "Left room '%s'.\nchat>", arguments[1]);
                } else {
                    sprintf(buffer, "Room '%s' not found.\nchat>", arguments[1]);
                }
                send(client, buffer, strlen(buffer), 0);
            }
            else if (strcmp(arguments[0], "users") == 0) {
                // Handle listing users with synchronization
                struct node *allUsers = head;
                struct room *currentRoom = findUserRoom(roomsHead, username);
                struct node *roomUsers = currentRoom ? currentRoom->users : NULL;

                sprintf(buffer, "Connected Users:\n");
                while (allUsers) {
                    sprintf(buffer + strlen(buffer), "- %s\n", allUsers->username);
                    allUsers = allUsers->next;
                }

                sprintf(buffer + strlen(buffer), "\nUsers in Room '%s':\n", currentRoom ? currentRoom->name : "None");
                if (roomUsers) {
                    while (roomUsers) {
                        sprintf(buffer + strlen(buffer), "- %s\n", roomUsers->username);
                        roomUsers = roomUsers->next;
                    }
                } else {
                    sprintf(buffer + strlen(buffer), "No users in your current room.\n");
                }

                sprintf(buffer + strlen(buffer), "\nchat>");  // Append the chat prompt
                send(client, buffer, strlen(buffer), 0);
            }
            else if (strcmp(arguments[0], "rooms") == 0) {
                // List all rooms with synchronization
                pthread_mutex_lock(&rw_lock);  // Lock read-write lock to safely access rooms
                sprintf(buffer, "Rooms:\n");
                getAllRooms(roomsHead, buffer + strlen(buffer));
                pthread_mutex_unlock(&rw_lock);

                sprintf(buffer + strlen(buffer), "\nchat>");  // Append the chat prompt
                send(client, buffer, strlen(buffer), 0);
            }
            else if (strcmp(arguments[0], "connect") == 0) {
              pthread_mutex_lock(&mutex);  // Lock mutex to safely access user list
              struct node *targetUser = findU(head, arguments[1]);
              struct node *currentUser = findUserBySocket(head, client);  // Get the current user by socket
              pthread_mutex_unlock(&mutex);

              if (targetUser && currentUser) {
                  // Set the connected user for both current and target user
                  strncpy(currentUser->connectedTo, targetUser->username, sizeof(currentUser->connectedTo) - 1);
                  strncpy(targetUser->connectedTo, currentUser->username, sizeof(targetUser->connectedTo) - 1);

                  // Notify the current user that they are now connected
                  sprintf(buffer, "You are now connected to %s.\nchat>", arguments[1]);
                  send(client, buffer, strlen(buffer), 0);
                  
                  // Notify the target user that they are connected to the current user
                  sprintf(buffer, "User '%s' has connected to you.\nchat>", currentUser->username);
                  send(targetUser->socket, buffer, strlen(buffer), 0);
              } else {
                  sprintf(buffer, "User '%s' not found.\nchat>", arguments[1]);
                  send(client, buffer, strlen(buffer), 0);  // Inform the client that the user wasn't found
              }
          }
          else if (strcmp(arguments[0], "disconnect") == 0) {
              pthread_mutex_lock(&mutex);
              struct node *currentUser = findUserBySocket(head, client);  // Get the current user by socket
              pthread_mutex_unlock(&mutex);

              if (currentUser && strlen(currentUser->connectedTo) > 0) {
                  // Get the user they are connected to
                  struct node *targetUser = findU(head, currentUser->connectedTo);
                  
                  // Reset the 'connectedTo' field for both users
                  memset(currentUser->connectedTo, 0, sizeof(currentUser->connectedTo));
                  if (targetUser) {
                      memset(targetUser->connectedTo, 0, sizeof(targetUser->connectedTo));
                      // Notify the target user they have been disconnected
                      sprintf(buffer, "User '%s' has disconnected from you.\nchat>", currentUser->username);
                      send(targetUser->socket, buffer, strlen(buffer), 0);
                  }

                  // Notify the current user they have disconnected
                  sprintf(buffer, "Disconnected from user '%s'.\nchat>", currentUser->connectedTo);
                  send(client, buffer, strlen(buffer), 0);
    } else {
        // Inform the user they are not connected
        sprintf(buffer, "You are not connected to anyone.\nchat>");
        send(client, buffer, strlen(buffer), 0);
    }
}
            else if (strcmp(arguments[0], "exit") == 0 || strcmp(arguments[0], "logout") == 0) {
                printf("Exit command received from client %d.\n", client);

                // Attempt to remove the user with synchronization
                pthread_mutex_lock(&mutex);
                removeUser(&head, client);
                pthread_mutex_unlock(&mutex);

                // Close the client socket
                close(client);
                printf("Socket %d closed.\n", client);

                // Send a farewell message to the client
                sprintf(buffer, "Goodbye %s.\n", username);
                send(client, buffer, strlen(buffer), 0);
                printf("Farewell message sent to client %d.\n", client);

                return NULL;
            } else {
              struct node *currentUser = findUserBySocket(head, client);  // Get current user by socket
              if (currentUser != NULL) {
                if (strlen(currentUser->connectedTo) > 0) {
                  // If user is connected, send the message to the connected user only
                  struct node *targetUser = findU(head, currentUser->connectedTo);
                  if (targetUser != NULL) {
                        sprintf(tmpbuf, "\n::%s> %s\nchat>", username, sbuffer);
                        strcpy(sbuffer, tmpbuf);
                        send(targetUser->socket, sbuffer, strlen(sbuffer), 0);  // Send to the connected user only
                        printf("Message sent to %s: %s\n", targetUser->username, sbuffer);
                    }
                } else {
                    // If user is not connected to anyone, send the message to all users
                    sprintf(tmpbuf, "\n::%s> %s\nchat>", username, sbuffer);
                    strcpy(sbuffer, tmpbuf);

                    pthread_mutex_lock(&mutex);
                    struct node *broadcastUser = head;
                    while (broadcastUser != NULL) {
                        if (client != broadcastUser->socket) {  // Don't send to yourself
                            send(broadcastUser->socket, sbuffer, strlen(sbuffer), 0);
                        }
                        broadcastUser = broadcastUser->next;
                    }
                    pthread_mutex_unlock(&mutex);

                    printf("Message broadcasted to all users: %s\n", sbuffer);
                }
            }
        }
        memset(buffer, 0, sizeof(1024));

    }

}
return NULL;

}
