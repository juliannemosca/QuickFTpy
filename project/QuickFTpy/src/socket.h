/*
 * socket.h
 *
 * $Id: $
 * $HeadURL: $
 * $LastChangedRevision: $
 * $LastChangedDate: $
 * $LastChangedBy: $
 *
 */

#ifndef SOCKET_H
#define SOCKET_H

#ifdef __cplusplus
extern "C" {
#endif

#include "macros.h"
#include "list.h"

// Macros:
#define SOCKET_INIT             socket_init
#define SOCKET_DEINIT           socket_deinit
#define SOCKET_CREATE           socket_create
#define SOCKET_ACCEPT           socket_accept
#define SOCKET_SELECT           socket_select
#define SOCKET_RECV             socket_recv
#define SOCKET_SEND             socket_send
#define SOCKET_CLOSE            socket_close
#define SOCKET_SHUTDOWN         socket_shutdown

#define SOCKET_NEW_SRVR(a,p,m)  SOCKET_CREATE(1, a, p, m, TRUE)
#define SOCKET_NEW_CLNT(a,p)    SOCKET_CREATE(0, a, p, 0, TRUE)

#define S_READ                  0x01
#define S_WRITE                 0x02
#define S_RW                    0x03

// Defines timeout for the sockets select operations
#define S_TIMEOUT 2

// Defines timeout for read/write operations
#define RW_TIMEOUT (10 * 1000)

/**
 * Socket information structure
 */
typedef struct _socket_t {

  int handle;
  struct _mutex_t* mutex;

} SOCKET_T;

/**
 * Initializes the library's socket functionalities
 *
 * @return                      TRUE or FALSE
 */
int socket_init();

/**
 * Finalizes the library's socket functionalities
 *
 * @return                      TRUE or FALSE
 */
int socket_deinit();

/**
 * Creates a new socket and connects to server if it is a client,
 * or does the binding and starts listening on a port if it is server.
 * Returns a reference to the newly created connection.
 *
 * @param side                  0 if it is a client, 1 if it is a server
 * @param addr                  address to connect for clients or to listen on
 *                              for server, if NULL for server listens on INADDR_ANY
 * @param port                  port to connect or to listen on
 * @param max_connections       max number of connections that the server can
 *                              accept. does not apply for clients.
 * @param nonblocking           TRUE if nonblocking is desired, otherwise FALSE
 * 
 * @return                      pointer to the newly created socket, on error
 *                              returns NULL
 */
SOCKET_T* socket_create(int side, char* addr, int port, int max_connections, int nonblocking);

/**
 * Accepts a connection and returns a socket
 * for the recently accepted connection
 *
 * @param listen_socket         socket in a 'listening' state
 * @param accept_socket         pointer by reference to store the accepted connection socket
 *
 * @return                      TRUE if connection was accepted, otherwise FALSE
 */
int socket_accept(SOCKET_T * listen_socket, SOCKET_T ** accept_socket);

/**
 * Checks if a socket is available for reading and/or writing
 *
 * @param timeout               timeout, or 0 for blocking
 * @param socket                socket to check state on
 * @param operation_type        operation type: S_READ, S_WRITE, S_RW
 *
 * @return                      type of operation the socket is available for,
 *                              can be S_READ, S_WRITE, S_RW, 
 *                              or -1 if an error occurred
 */
int socket_select(int timeout, SOCKET_T * select_socket, int operation_type);

/**
 * Checks if a group of sockets is available for read/write
 * 
 * Upon return the lists will be already updated, having removed de nodes correponding
 * to sockets that were not available for the requested operations (read/write)
 * 
 * @param timeout               timeout, or 0 for blocking
 * @param read_s                pointer to a socket list to check for read availability, or NULL if does not apply
 * @param write_s               pointer to a socket list to check for write availability, or NULL if does not apply
 *
 * @return                      TRUE, o FALSE si la operacion dio error o timeout
 */
int socket_select_multiple(int timeout, list_t* read_s, list_t* write_s );

/**
 * Receives data on a connected socket.
 * 
 * If the socket is non-blocking and has no data in the queue the function
 * returns TRUE and the 'len' parameter will be set with the value 0.
 * 
 * You should always check for the number of received bytes before attempting
 * to read the content of data_buffer after returning from function
 *
 *
 * @param recv_socket           connected socket for receiving data
 * @param data_buffer           buffer by reference to store the received data
 * @param len                   size of the buffer passed by reference, the value will be updated
 *                              after the operation with the number of received bytes, which are
 *                              stored in data_buffer.
 * 
 * @return                      TRUE or FALSE
 *
 */
int socket_recv(SOCKET_T* recv_socket, char** data_buffer, int* len);

/**
 * Sends data through a connected socket.
 * 
 * If the socket is non-blocking and the operation turns out to be blocking
 * the function returns immediatly with a TRUE return value.
 * 
 * @param send_socket           connected socket for sending data
 * @param data_buffer           buffer containing the data to be sent
 * @param len                   buffer size without including the terminating null character,
 *                              like for example the value returned by strlen.
 * @param bytes_sent            output parameter, returns number of bytes sent.
 *
 * @return                      TRUE or FALSE
 */
int socket_send(SOCKET_T* send_socket, char* send_buffer, int len, int * bytes_sent);

/**
 * Finalizes, closes, and destroys a socket previously created with SOCKET_CRATE
 *
 * @param close_socket          pointer by reference to the socket data structure
 *
 * @return                      TRUE or FALSE
 */
int socket_close(SOCKET_T** close_socket);

/**
 * Disables both send and receive operations in a socket
 * without checking if the socket is being used.
 *
 * @param socket                pointer by reference to the socket structure to shut down
 *
 * @return                      TRUE or FALSE
 */
int socket_shutdown(SOCKET_T** socket);

#endif // WIN32
