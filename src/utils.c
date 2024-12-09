#include "../include/utils.h"
#include "../include/server.h"
#include <arpa/inet.h> // inet_addr()
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h> // bzero()
#include <sys/socket.h>
#include <unistd.h> // read(), write(), close()
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <pthread.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/time.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <limits.h>
#include <stdint.h>

//TODO: Declare a global variable to hold the file descriptor for the server socket
int master_fd;
//TODO: Declare a global variable to hold the mutex lock for the server socket
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;;
//TODO: Declare a gloabl socket address struct to hold the address of the server
struct sockaddr_in server_addr; 
/*
################################################
##############Server Functions##################
################################################
*/

/**********************************************
 * init
   - port is the number of the port you want the server to be
     started on
   - initializes the connection acception/handling system
   - if init encounters any errors, it will call exit().
************************************************/
void init(int port) {
   //TODO: create an int to hold the socket file descriptor
   //TODO: create a sockaddr_in struct to hold the address of the server
    int sd;
    struct sockaddr_in addr;

   /**********************************************
    * IMPORTANT!
    * ALL TODOS FOR THIS FUNCTION MUST BE COMPLETED FOR THE INTERIM SUBMISSION!!!!
    **********************************************/
   
   
   
   // TODO: Create a socket and save the file descriptor to sd (declared above)
    sd = socket (PF_INET, SOCK_STREAM, 0);
    if (sd < 0) {
      perror("Socket creation failed");
      exit(EXIT_FAILURE);
    }
   // TODO: Change the socket options to be reusable using setsockopt(). 
    int enable = 1;
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char*)&enable, sizeof(int));

   // TODO: Bind the socket to the provided port.
    addr.sin_family= AF_INET;
    addr.sin_addr.s_addr= htonl(INADDR_ANY);
    addr.sin_port= htons(port); //server picks the port
    if (bind(sd,(struct sockaddr*)&addr, sizeof(addr))<0) {
      perror("Bind failed");
      close(sd);
      exit(EXIT_FAILURE);
    }

   // TODO: Mark the socket as a pasive socket. (ie: a socket that will be used to receive connections)
    if (listen(sd, 5) < 0) {
      perror("Listen failed");
      close(sd);
      exit(EXIT_FAILURE);
    } 
   
   
   
   // We save the file descriptor to a global variable so that we can use it in accept_connection().
   // TODO: Save the file descriptor to the global variable master_fd
  master_fd = sd;
   printf("UTILS.O: Server Started on Port %d\n",port);
   fflush(stdout);

}


/**********************************************
 * accept_connection - takes no parameters
   - returns a file descriptor for further request processing.
   - if the return value is negative, the thread calling
     accept_connection must should ignore request.
***********************************************/
int accept_connection(void) {

   //TODO: create a sockaddr_in struct to hold the address of the new connection
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
   
   /**********************************************
    * IMPORTANT!
    * ALL TODOS FOR THIS FUNCTION MUST BE COMPLETED FOR THE INTERIM SUBMISSION!!!!
    **********************************************/
   
   
   
   // TODO: Aquire the mutex lock
    pthread_mutex_lock(&lock);
   // TODO: Accept a new connection on the passive socket and save the fd to newsock
    int newsock = accept(master_fd, (struct sockaddr*)&addr, &addr_len);
    if (newsock < 0) {
      perror("Accept failed");
      pthread_mutex_unlock(&lock);  // Ensure the lock is released before returning
      return -1; 
    }
   // TODO: Release the mutex lock
    pthread_mutex_unlock(&lock);

   // TODO: Return the file descriptor for the new client connection
   return newsock;
}


/**********************************************
 * send_file_to_client
   - socket is the file descriptor for the socket
   - buffer is the file data you want to send
   - size is the size of the file you want to send
   - returns 0 on success, -1 on failure 
************************************************/
int send_file_to_client(int socket, char * buffer, int size) 
{
    //TODO: create a packet_t to hold the packet data
    packet_t packet;
    packet.size = size;
    //var to track data sent
    int total_data_sent = 0;
    int current_data_send = 0;


    //TODO: send the file size packet
    if (write(socket, &packet.size, sizeof(packet.size)) < 0) {
        perror("Failed to send file size");
        return -1;
    }


    //TODO: send the file data
    while (total_data_sent < size) {
        // Send the remaining data
        current_data_send = write(socket, buffer + total_data_sent, size - total_data_sent);
        //Fail case
        if (current_data_send < 0) {
            perror("Failed to send file data");
            return -1;
        }
        //update tracking of bytes
        total_data_sent += current_data_send;
    }
  
    //TODO: return 0 on success, -1 on failure
    return 0;

}


/**********************************************
 * get_request_server
   - fd is the file descriptor for the socket
   - filelength is a pointer to a size_t variable that will be set to the length of the file
   - returns a pointer to the file data
************************************************/
char * get_request_server(int fd, size_t *filelength)
{
    //TODO: create a packet_t to hold the packet data
    packet_t packet;
 
    //TODO: receive the response packet
    if (read(fd, &packet.size, sizeof(packet.size)) <= 0) {
        perror("Failed to receive file size");
        return NULL;
    }
  
    //TODO: get the size of the image from the packet
    *filelength = packet.size; // Assign the file size to the output parameter
    int size_of_img = packet.size;

    char *buffer = (char *)malloc(size_of_img);
    if (!buffer) {
        perror("Memory allocation failed");
        return NULL; // Return NULL if memory allocation fails
    }

    //TODO: recieve the file data and save into a buffer variable.
    int total_data_received = 0; // Initialize total bytes received
    int current_data_receive;

    while (total_data_received < size_of_img) {
        current_data_receive = read(fd, buffer + total_data_received, size_of_img - total_data_received);
        if (current_data_receive <= 0) {
            perror("Failed to receive file data");
            free(buffer); // Free allocated memory on failure
            return NULL;
        }
        total_data_received += current_data_receive; // Update the total received data
    }
    //TODO: return the buffer
    return buffer;
}


/*
################################################
##############Client Functions##################
################################################
*/

/**********************************************
 * setup_connection
   - port is the number of the port you want the client to connect to
   - initializes the connection to the server
   - if setup_connection encounters any errors, it will call exit().
************************************************/
int setup_connection(int port)
{
    //TODO: create a sockaddr_in struct to hold the address of the server
    struct sockaddr_in server_addr;   

    //TODO: create a socket and save the file descriptor to sockfd
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);\
    //fail case
    if (sockfd < 0) {
        perror("Socket creation failed");
        return -1;
    }
   
    //TODO: assign IP, PORT to the sockaddr_in struct
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    //TODO: connect to the server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection to the server failed");
        close(sockfd); // Close the socket on failure
        return -1;
    }
   
    //TODO: return the file descriptor for the socket
    return sockfd;
}


/**********************************************
 * send_file_to_server
   - socket is the file descriptor for the socket
   - file is the file pointer to the file you want to send
   - size is the size of the file you want to send
   - returns 0 on success, -1 on failure
************************************************/
int send_file_to_server(int socket, FILE *file, int size) 
{
    //TODO: send the file size packet
   

    //TODO: send the file data
   

    // TODO: return 0 on success, -1 on failure
   
}

/**********************************************
 * receive_file_from_server
   - socket is the file descriptor for the socket
   - filename is the name of the file you want to save
   - returns 0 on success, -1 on failure
************************************************/
int receive_file_from_server(int socket, const char *filename) 
{
    //TODO: create a buffer to hold the file data
    

    //TODO: open the file for writing binary data
   
    
   //TODO: create a packet_t to hold the packet data
    
   //TODO: receive the response packet
  

    //TODO: get the size of the image from the packet
   
   

   //TODO: recieve the file data and write it to the file
    
    //TODO: return 0 on success, -1 on failure
}

