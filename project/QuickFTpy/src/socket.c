/*
 * socket.c
 *
 * $Id: $
 * $HeadURL: $
 * $LastChangedRevision: $
 * $LastChangedDate: $
 * $LastChangedBy: $
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "mutex.h"
#include "socket.h"

/**
 * Initializes the library's socket functionalities
 *
 * @return                      TRUE or FALSE
 */
int socket_init() {

#ifdef _WIN32
  WSADATA wsa_data;
  char buffer[512];
  int res = 0;

  // Inicializa winsock
  res = WSAStartup(MAKEWORD(2,2), &wsa_data);
  if (res != 0) {        
    sprintf(buffer, "WSAStartup fallo en la inicializacion con el error: %d\n", res);
    return FALSE;
  }
#endif

  return TRUE;

}

/**
 * Finalizes the library's socket functionalities
 *
 * @return                      TRUE or FALSE
 */
int socket_deinit() {

#ifdef _WIN32
  if ( WSACleanup() == 0 ) {
    return TRUE;
  }
  
  return FALSE;
#endif
  return TRUE;

}

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
SOCKET_T* socket_create(int side, char* addr, int port, int max_connections, int nonblocking) {

  SOCKET_T* new_socket = NULL;
  struct sockaddr_in service;
  
  char buffer[_BUFFER_SIZE_S];
  int res;

  int socket_handle;

  // Validates paramter
  if (side < 0 || side > 1) {
    sprintf(buffer, "socket_create fail: parametros no validos");
    return NULL;
  }

  // Configurates the service
  service.sin_family = AF_INET;
  service.sin_port = htons((unsigned short)port);
  if (addr == NULL) {
    service.sin_addr.s_addr = htonl(INADDR_ANY);
  } else {
    service.sin_addr.s_addr = inet_addr(addr);
  }

  //
  // Creates socket and does the binding and listening
  // operations for server or connect for clients
  //

  // Creates socket
  if (nonblocking == TRUE) {
    socket_handle = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
  } else {
    socket_handle = socket(AF_INET, SOCK_STREAM, 0);
  }
    
  if (socket_handle == -1) {
      
    sprintf(buffer, "socket failed with error: %d\n", errno);
    LOGGER(__FUNCTION__, buffer);
    return NULL;
  }  

  // If it is server
  if (side == 1) {

    // Binds the socket to the port
    res = bind (socket_handle, (struct sockaddr *) &service, sizeof (service));
    if (res == -1)
    {
      sprintf(buffer, "socket failed with error: %d\n", errno);
      LOGGER(__FUNCTION__, buffer);

      shutdown(socket_handle, SHUT_RDWR);
      return NULL;
    }

    // Starts listening on the port
    res = listen(socket_handle, max_connections);
    if (res == -1) {

      sprintf(buffer, "listen fallo con el error: %d\n", errno);
      LOGGER(__FUNCTION__, buffer);
      
      shutdown(socket_handle, SHUT_RDWR);      
      return NULL;
    }
  
  }
  // If it is client
  else {

    // Connects to the server
    res = connect(socket_handle, (struct sockaddr *) &service, sizeof (service));
    if ( res == -1) {

      // Unless connection is in progress
      if ( errno != EINPROGRESS ) {

        sprintf(buffer, "connect failed with error: %d\n", errno);
        LOGGER(__FUNCTION__, buffer);
      
        shutdown(socket_handle, SHUT_RDWR);      
        return NULL;  
      } 

    }
  
  }

  // Configures timeouts for read/write
  {
    struct timeval timeout;      
    timeout.tv_sec = RW_TIMEOUT;
    timeout.tv_usec = 0;

    res = setsockopt(socket_handle, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
    if (res == -1) {
      
      sprintf(buffer, "setsockopt failed with error: %d\n", errno);
      LOGGER(__FUNCTION__, buffer);
    }

    res = setsockopt(socket_handle, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout));
    if (res == -1) {
      
      sprintf(buffer, "setsockopt failed with error: %d\n", errno);
      LOGGER(__FUNCTION__, buffer);
    }
  }

  // Allocates space for the socket structure
  new_socket = malloc(sizeof( SOCKET_T ));
  memset(new_socket, 0x00, sizeof( SOCKET_T ));

  // Creates mutex for thread-safe socket
  new_socket->mutex = (MUTEX_T*)malloc(sizeof(struct _mutex_t) );
  memset(new_socket->mutex, 0x00, sizeof(struct _mutex_t));
  MUTEX_CREATE(&new_socket->mutex);
  
  // Copies handle to the socket structure
  new_socket->handle = socket_handle;
    
  return new_socket;
  
}

/**
 * Accepts a connection and returns a socket
 * for the recently accepted connection
 *
 * @param listen_socket         socket in a 'listening' state
 * @param accept_socket         pointer by reference to store the accepted connection socket
 *
 * @return                      TRUE if connection was accepted, otherwise FALSE
 */
int socket_accept(SOCKET_T * listen_socket, SOCKET_T ** accept_socket) {

  int socket_handle;
  SOCKET_T* new_acc_socket;
  struct sockaddr sa_client;
  unsigned int sa_client_size = sizeof(sa_client);
  char buffer[1024];

  MUTEX_LOCK(listen_socket->mutex);

  if ( listen_socket != NULL ) {

    // Allocates space for the socket structure
    new_acc_socket = malloc(sizeof( SOCKET_T ));
    memset(new_acc_socket, 0x00, sizeof( SOCKET_T ));

    // Accepts connection
    socket_handle = accept(listen_socket->handle, (struct sockaddr *) &sa_client, &sa_client_size);
    if (socket_handle == -1) {

      if ( errno == EAGAIN || errno == EWOULDBLOCK ) {

        // Returns invalid, no connections pending
        free(new_acc_socket);
        MUTEX_UNLOCK(listen_socket->mutex);
        return FALSE;

      } else {
        
        sprintf(buffer, "accept failed with error: %d\n", errno);
        LOGGER(__FUNCTION__, buffer);

        free(new_acc_socket);
        MUTEX_UNLOCK(listen_socket->mutex);
        return FALSE;
      }

    } else {

      // Stores the new socket handle in the structure
      new_acc_socket->handle = socket_handle;

      // Copies the structure to the output parameter
      *accept_socket = new_acc_socket;

      // Creates mutex for the new socket
      (*accept_socket)->mutex = (MUTEX_T*)malloc(sizeof(struct _mutex_t) );
      memset( (*accept_socket)->mutex, 0x00, sizeof(struct _mutex_t));
      MUTEX_CREATE( &((*accept_socket)->mutex) );

      MUTEX_UNLOCK(listen_socket->mutex);

      return TRUE;
    }
  }

  MUTEX_UNLOCK(listen_socket->mutex);

  return FALSE;

}

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
int socket_select(int timeout, SOCKET_T * select_socket, int operation_type) {

  fd_set* readfds = NULL;
  fd_set* writefds = NULL;
  struct timeval tval_timeout;
  char buffer[1024];
  int retval = 0;
  int res;

  // Validates parameters
  if ( ( select_socket == NULL ) || ( (operation_type != S_READ) && (operation_type != S_WRITE) && (operation_type != S_RW) ) ) {

    sprintf(buffer, "socket_select fail: invalid parameters");
    LOGGER(__FUNCTION__, buffer);
    return -1;

  }

  // Configures timeout
  tval_timeout.tv_sec = timeout;
  tval_timeout.tv_usec = 0;

  // Allocates memory for sets
  readfds = (fd_set*)malloc(sizeof(fd_set));
  writefds = (fd_set*)malloc(sizeof(fd_set));

  // Initializes sets
  FD_ZERO(readfds);
  FD_ZERO(writefds);

  // Locks the socket's mutex
  MUTEX_LOCK(select_socket->mutex);

  // If has to check for reading
  if (operation_type & S_READ) {
    
    // Adds socket to the read set
    FD_SET( select_socket->handle, readfds );
  }

  // If has to check for writing
  if (operation_type & S_WRITE) {

    // Adds socket to the write set
    FD_SET(select_socket->handle, writefds);
  }

  // Calls select function
  res = select( (select_socket->handle)+1, readfds, writefds, NULL, &tval_timeout );
  if (res == -1) {

    sprintf(buffer, "select failed with error: %d\n", errno);
    LOGGER(__FUNCTION__, buffer);
    
    retval = -1;
    goto end_select;
  }

  // If socket is on the read result set
  // adds to return value
  if ( FD_ISSET(select_socket->handle, readfds ) ) {
    retval += S_READ;
  }

  // If socket is on the write result set
  // adds to return value
  if ( FD_ISSET(select_socket->handle, writefds ) ) {
    retval += S_WRITE;
  }

end_select:

  // Unlocks the socket's mutex
  MUTEX_UNLOCK(select_socket->mutex);

  // Frees allocated memory
  free(readfds);
  free(writefds);

  return retval;

}

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
int socket_select_multiple(int timeout, list_t* read_s, list_t* write_s ) {

  list_node_t * seeker = NULL;
  list_node_t * remover = NULL;
  SOCKET_T * ptr_socket = NULL;
  fd_set* readfds = NULL;
  fd_set* writefds = NULL;
  struct timeval tval_timeout;
  int lockstate;
  char buffer[1024];
  int res;

  // configures timeout
  tval_timeout.tv_sec = timeout;
  tval_timeout.tv_usec = 0;

  // Reserva memoria para los sets
  readfds = (fd_set*)malloc(sizeof(fd_set));
  writefds = (fd_set*)malloc(sizeof(fd_set));

  // Initializes sets
  FD_ZERO(readfds);
  FD_ZERO(writefds);

  // Iterates through the read sockets list
  seeker = read_s->first;
  while ( seeker != NULL) {

    ptr_socket = seeker->content;
    if (ptr_socket != NULL) {

      // Puts a lock on the socket's mutex
      MUTEX_LOCK(ptr_socket->mutex);

      // Adds socket to the read set
      FD_SET( ptr_socket->handle, readfds);

    }

    seeker = seeker->next;
  
  }

  // Iterates through the write sockets list
  seeker = write_s->first;
  while ( seeker != NULL) {

    ptr_socket = seeker->content;
    if (ptr_socket != NULL) {

      // If it is not locked
      lockstate = MUTEX_IS_LOCKED(ptr_socket->mutex); 
      if ( lockstate == FALSE ) {

        // Puts a lock on the socket's mutex
        MUTEX_LOCK(ptr_socket->mutex);

      }

      // Adds socket to the write set
      FD_SET( ptr_socket->handle, writefds );
    
    }

    seeker = seeker->next;
  
  }

  // Calls the select function with the generated sets
  res = select(0, readfds, writefds, NULL, &tval_timeout);
  if (res == -1) {

    sprintf(buffer, "select failed with error: %d\n", errno);
    LOGGER(__FUNCTION__, buffer);
    
    // Frees memory
    free(readfds);
    free(writefds);

    return FALSE;

  }
  
  //
  // Goes through the socket lists checking for the available ones and
  // removing the ones that are not available
  //

  //
  // Goes through the read list
  //
  seeker = read_s->first;
  while ( seeker != NULL) {

    ptr_socket = seeker->content;
    if (ptr_socket != NULL) {

      // If a socket is not available for reading
      if ( !FD_ISSET( ptr_socket->handle, readfds) ) {
        
        // Removes the socket from the list
        remover = seeker;
        seeker = seeker->next;

        LIST_REMOVE(read_s, remover, NULL);

        // Removes the lock on the mutex
        MUTEX_UNLOCK(ptr_socket->mutex);

        // Moves to the next
        continue;

      } else {

        // Removes the lock on the mutex
        MUTEX_UNLOCK(ptr_socket->mutex);
      
      }
    
    }

    // Moves to the next
    seeker = seeker->next;
  }

  //
  // Goes through the write list
  //
  seeker = write_s->first;
  while ( seeker != NULL) {

    ptr_socket = seeker->content;
    if (ptr_socket != NULL) {

      // If it is not locked
      lockstate = MUTEX_IS_LOCKED(ptr_socket->mutex); 
      if ( lockstate == FALSE ) {

        // Puts a lock on the socket's mutex
        MUTEX_LOCK(ptr_socket->mutex);
      
      }

      // If socket is not available for writing
      if ( !FD_ISSET( ptr_socket->handle, writefds) ) {

        // Removes the socket from the list
        remover = seeker;
        seeker = seeker->next;

        LIST_REMOVE(write_s, remover, NULL);

        // Removes the lock on the socket's mutex
        MUTEX_UNLOCK(ptr_socket->mutex);
        
        // Moves to the next
        continue;
      
      } else {
        
        // Removes the lock on the socket's mutex      
        MUTEX_UNLOCK(ptr_socket->mutex);        
      
      }

    }

    // Moves to the next
    seeker = seeker->next;
  
  }

  // Frees memory
  free(readfds);
  free(writefds);

  return TRUE;

}

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
int socket_recv(SOCKET_T* recv_socket, char** data_buffer, int* len) {

  char buffer[1024];
  int flags = 0;
  ssize_t res;

  if ( recv_socket != NULL ) {

    // Puts a lock on the socket's mutex
    MUTEX_LOCK(recv_socket->mutex);

    // Initializes buffer
    memset(*data_buffer, 0x00, *len);

    // Attempts to receive data
    res = recv(recv_socket->handle, *data_buffer, *len, flags);
    if ( res == -1 ) {

      // Updates the result value
      *len = 0;

      // removes the lock
      MUTEX_UNLOCK(recv_socket->mutex);

      if ( errno == EAGAIN || errno == EWOULDBLOCK ) {
        return TRUE;
      } else {
        sprintf(buffer, "recv failed with error: %d\n", errno);
        LOGGER(__FUNCTION__, buffer);
        return FALSE;
      } 
      
    }

    // Updates the result value
    *len = res;

    // Removes the lock
    MUTEX_UNLOCK(recv_socket->mutex);

    return TRUE;

  }

  return FALSE;

}

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
int socket_send(SOCKET_T* send_socket, char* send_buffer, int len, int * bytes_sent) {

  char buffer[1024];
  int flags = MSG_NOSIGNAL;
  int res;

  if ( (send_socket != NULL) && (send_buffer != NULL) ) {

    MUTEX_LOCK(send_socket->mutex);

    // Attempts to send data
    res = send(send_socket->handle, send_buffer, len, flags);
    if ( res == -1 ) {

      // Updates value of result
      *bytes_sent = 0;

      // Removes the lock
      MUTEX_UNLOCK(send_socket->mutex);

      if ( errno == EAGAIN || errno == EWOULDBLOCK ) {
        return TRUE;
      } else {
        sprintf(buffer, "send failed with error: %d\n", errno);
        LOGGER(__FUNCTION__, buffer);
        return FALSE;
      } 

    }

    // Updates value of result
    *bytes_sent = res;

    // Removes the lock
    MUTEX_UNLOCK(send_socket->mutex);

    return TRUE;

  }

  return FALSE;

}

/**
 * Finalizes, closes, and destroys a socket previously created with SOCKET_CRATE
 *
 * @param close_socket          pointer by reference to the socket data structure
 *
 * @return                      TRUE or FALSE
 */
int socket_close(SOCKET_T** close_socket) {

  char buffer[1024];
  int res;

  if ( close_socket && *close_socket ) {

    // Puts a lock on the mutex
    MUTEX_LOCK( (*(close_socket))->mutex );

    // Shuts down the socket for send and receive
    shutdown( (*(close_socket))->handle, SHUT_RDWR );
    
    // Closes the socket
    res = close( (*(close_socket))->handle );
    if (res == -1) {

      // Removes the lock on the mutex
      MUTEX_UNLOCK( (*(close_socket))->mutex );

      sprintf(buffer, "shutdown failed with error: %d\n", errno);
      LOGGER(__FUNCTION__, buffer);
  
      return FALSE;

    }

    // Removes the lock on the mutex and destroys it
    MUTEX_UNLOCK( (*(close_socket))->mutex );
    MUTEX_DESTROY( &( (*(close_socket))->mutex) );
    
    // Frees previously allocated memory
    free( (*(close_socket))->mutex );

    // Frees memory from the socket structure
    free( *close_socket );

    return TRUE;

  }

  return FALSE;

}

/**
 * Disables both send and receive operations in a socket
 * without checking if the socket is being used.
 *
 * @param socket                pointer by reference to the socket structure to shut down
 *
 * @return                      TRUE or FALSE
 */
int socket_shutdown(SOCKET_T** socket) {

  if ( socket && *socket ) {

    // Shuts down the socket for reading and writing
    shutdown( (*(socket))->handle, SHUT_RDWR);
    return TRUE;

  }
  
  return FALSE;

}