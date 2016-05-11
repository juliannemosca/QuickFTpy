/*
 * server.h
 *
 * $Id: $
 * $HeadURL: $
 * $LastChangedRevision: $
 * $LastChangedDate: $
 * $LastChangedBy: $
 *
 */

#ifndef SERVER_H
#define SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <python2.7/Python.h>
  
#include "mutex.h"
#include "socket.h"
#include "thread.h"

// Declares the user_data structure for threads
typedef struct _user_data_t {
  int is_running;
  int keep_going;
  MUTEX_T* mutex;
} USER_DATA_T;

// Definition for server data structure type
typedef struct _server_t {

  // Connection information structure
  SOCKET_T * connection;
  int initialized;
  
  // Information on the thread's context
  // for the node's listen process
  thread_t * listen_thread;
  USER_DATA_T udata;

} SERVER_T;

/**
 * Initializes server for sending and receiving messages
 *
 */
PyObject * server_initialize(PyObject * self, PyObject * args);

/**
 * Ends the connections and finalizes the server
 *
 */
PyObject * server_finalize (PyObject * self);

/**
 * Starts the thread for the server's listen loop
 *
 * @param server                        server data structure
 *
 */
void server_listen_begin ( SERVER_T * server_context );

/**
 * Finalizes the thread's listen loop
 *
 * @param server                        server data structure
 */
void server_listen_finalize ( SERVER_T * server );

#endif // SERVER_H