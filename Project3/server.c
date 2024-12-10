// team: Zarif + Riana

#include "server.h"

int chat_serv_sock_fd; //server socket
struct room *roomsHead = NULL;


/////////////////////////////////////////////
// USE THESE LOCKS AND COUNTER TO SYNCHRONIZE

int numReaders = 0; // keep count of the number of readers

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;  // mutex lock
pthread_mutex_t rw_lock = PTHREAD_MUTEX_INITIALIZER;  // read/write lock

/////////////////////////////////////////////


char const *server_MOTD = "Thanks for connecting to the BisonChat Server.\n\nchat>";

struct node *head = NULL;

int main(int argc, char **argv) {
   signal(SIGINT, sigintHandler);

    if (pthread_mutex_init(&user_mutex, NULL) != 0) {
        perror("Failed to initialize user mutex");
        exit(EXIT_FAILURE);
    }

    if (pthread_mutex_init(&room_mutex, NULL) != 0) {
        perror("Failed to initialize room mutex");
        exit(EXIT_FAILURE);
    }

    pthread_mutex_lock(&rw_lock);
    roomsHead = createRoom(DEFAULT_ROOM);
    if (!roomsHead) {
      fprintf(stderr, "Failed to create default room.\n");
      exit(EXIT_FAILURE);
   }
    pthread_mutex_unlock(&rw_lock);
    printf("Default room 'Lobby' created.\n");
    
   //////////////////////////////////////////////////////
   // create the default room for all clients to join when 
   // initially connecting
   //  
   //    TODO
   //////////////////////////////////////////////////////

   // Open server socket
   chat_serv_sock_fd = get_server_socket();

   // step 3: get ready to accept connections
   if(start_server(chat_serv_sock_fd, BACKLOG) == -1) {
      printf("start server error\n");
      exit(1);
   }
   
   printf("Server Launched! Listening on PORT: %d\n", PORT);
    
   //Main execution loop
    while(1) {
        // Accept a connection, start a thread
        int new_client = accept_client(chat_serv_sock_fd);
        if (new_client != -1) {
            printf("Accepting client connection, spawning thread...\n");
            pthread_t new_client_thread;
            pthread_create(&new_client_thread, NULL, client_receive, (void *)&new_client);
            if (pthread_create(&new_client_thread, NULL, client_receive, (void *)&new_client) != 0) {
              perror("Failed to create thread");
          } else {
              printf("Thread created for client: %d\n", new_client);
          }
        } else {
            printf("Error accepting client connection\n");
        }
    }
  close(chat_serv_sock_fd);

  pthread_mutex_destroy(&user_mutex);
  pthread_mutex_destroy(&room_mutex);

  return 0;
}


int get_server_socket(char *hostname, char *port) {
    int opt = TRUE;   
    int master_socket;
    struct sockaddr_in address; 
    
    //create a master socket  
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)   
    {   
        perror("socket failed");   
        exit(EXIT_FAILURE);   
    }   
     
    //set master socket to allow multiple connections ,  
    //this is just a good habit, it will work without this  
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,  
          sizeof(opt)) < 0 )   
    {   
        perror("setsockopt");   
        exit(EXIT_FAILURE);   
    }   
     
    //type of socket created  
    address.sin_family = AF_INET;   
    address.sin_addr.s_addr = INADDR_ANY;   
    address.sin_port = htons( PORT );   
         
    //bind the socket to localhost port 8888  
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)   
    {   
        perror("bind failed");   
        exit(EXIT_FAILURE);   
    }   

   return master_socket;
}


int start_server(int serv_socket, int backlog) {
   int status = 0;
   if ((status = listen(serv_socket, backlog)) == -1) {
      printf("socket listen error\n");
   }
   return status;
}


// int accept_client(int serv_sock) {
//    int reply_sock_fd = -1;
//    socklen_t sin_size = sizeof(struct sockaddr_storage);
//    struct sockaddr_storage client_addr;
//    //char client_printable_addr[INET6_ADDRSTRLEN];

//    // accept a connection request from a client
//    // the returned file descriptor from accept will be used
//    // to communicate with this client.
//    if ((reply_sock_fd = accept(serv_sock,(struct sockaddr *)&client_addr, &sin_size)) == -1) {
//       printf("socket accept error\n");
//    }
//    return reply_sock_fd;
// }

int accept_client(int serv_sock) {
   int reply_sock_fd = -1;
   socklen_t sin_size = sizeof(struct sockaddr_storage);
   struct sockaddr_storage client_addr;
   char client_ip[INET6_ADDRSTRLEN];  // Buffer for IP address

   // Accept a connection request from a client
   if ((reply_sock_fd = accept(serv_sock, (struct sockaddr *)&client_addr, &sin_size)) == -1) {
      printf("socket accept error\n");
   } else {
      // Print client connection details
      if (client_addr.ss_family == AF_INET) {
         struct sockaddr_in *client_addr_in = (struct sockaddr_in *)&client_addr;
         inet_ntop(AF_INET, &client_addr_in->sin_addr, client_ip, sizeof(client_ip));
         printf("New client connected from IP: %s, PORT: %d\n", client_ip, ntohs(client_addr_in->sin_port));
      } else {
         // Handle IPv6 if needed (for future expansion)
         printf("New client connected, but address type not supported yet.\n");
      }
   }

   return reply_sock_fd;
}



/* Handle SIGINT (CTRL+C) */
void sigintHandler(int sig_num) {
  printf("Error:Forced Exit.\n");

  struct node *current_user = head;
  while (current_user){
    char shutdown_message[] = "server is shutting down. You are going to be disconnected";
    send(current_user->socket,shutdown_message,strlen(shutdown_message),0);
    current_user = current_user->next;
  }

  sleep(3);

  current_user = head;
  while (current_user) {
    struct node *temp = current_user;
    close(current_user->socket);
    current_user = current_user->next;
    free(temp);
  }

  struct room *current_room = roomsHead;
  while (current_room) {
    struct room *temp = current_room;
    current_room = current_room->next;
    free(temp);
  }

  pthread_mutex_destroy(&user_mutex);
  pthread_mutex_destroy(&room_mutex);
  pthread_mutex_destroy(&rw_lock);


   printf("--------CLOSING ACTIVE USERS--------\n");

   close(chat_serv_sock_fd);
   exit(0);
}