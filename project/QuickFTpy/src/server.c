/*
 * server.c
 *
 * $Id: $
 * $HeadURL: $
 * $LastChangedRevision: $
 * $LastChangedDate: $
 * $LastChangedBy: $
 *
 */

#include <unistd.h>
#define Sleep(ms) usleep(ms*1000)

#include "macros.h"

#include "server.h"
#include "message.h"
#include "process.h"
#include "quickft.h"


// Server handle
static SERVER_T * gl_server_handle = NULL;

/**
 * Initializes server for sending and receiving messages
 *
 */
PyObject * server_initialize( PyObject * self, PyObject * args ) {
  
  SERVER_T * new_server;
  char l_msg[1024];
  
  int port = 0;
  int max_connections = 0;
  int timeout = 0;
  
  PyObject * py_log_writer;
  
  // Initializes python threading
  PyEval_InitThreads();
  PyEval_ReleaseLock();
  
  // Parses arguments
  if (!PyArg_ParseTuple(args, "iiiO", &port, &max_connections, &timeout, &py_log_writer)) {
    return Py_BuildValue("i", FALSE);
  }
  
  // Makes sure fourth argument is a function
  if (!PyCallable_Check(py_log_writer)) {
    PyErr_SetString(PyExc_TypeError, "Argument is not a function.");  
  }
  
  // Stores the log writer function
  gl_py_log_writer = py_log_writer;
  
  // Initializes log
  LOGGER_INIT;
  
  LOGGER(__FUNCTION__, "Initializing...");
  
  gl_timeout = DEFAULT_TIMEOUT;
  if (timeout != 0) {
    gl_timeout = timeout;
  }
  
  // Initializes the library's socket functionalities
  if ( ! SOCKET_INIT() ) {
    return Py_BuildValue("i", FALSE);
  }

  // Allocates and sets server structure
  new_server = malloc( sizeof(SERVER_T) );
  memset(new_server, 0x00, sizeof(SERVER_T));

  // Creates a connection
  new_server->connection = SOCKET_NEW_SRVR(NULL, port, max_connections);
  if ( new_server->connection != NULL ) {

    // Sets initialization variable
    new_server->initialized = TRUE;
    
    // Begins thread for listening
    server_listen_begin( new_server );

    // Saves server instance
    gl_server_handle = new_server;
    
    sprintf(l_msg, "Server running on port %d", port);
    LOGGER(__FUNCTION__, l_msg);
    
    return Py_BuildValue("i", TRUE);
    
  } 
  
  free(new_server);
  return Py_BuildValue("i", FALSE);
  
}

/**
 * Ends the connections and finalizes the server
 *
 */
PyObject * server_finalize ( PyObject * self ) {

  if ( gl_server_handle != NULL ) {

    // Sets initialized variable to false
    gl_server_handle->initialized = FALSE;

    // Finalizes listen thread
    server_listen_finalize( gl_server_handle );
    
    // Ends connection and closes socket
    SOCKET_CLOSE( &(gl_server_handle)->connection );
    
    // Frees memory previously allocated for server structure
    free(gl_server_handle);
    (gl_server_handle) = NULL;

    // Finalizes library's socket functionalities
    SOCKET_DEINIT();
    
    // Finalizes log
    LOGGER_DEINIT;
    
    return Py_BuildValue("i", TRUE);
  }

  // Finalizes log
  LOGGER_DEINIT;
  
  //PyGILState_Release(gl_py_gstate);
  
  return Py_BuildValue("i", FALSE);
}

/**
 * Performs the server listen loop
 *
 */
void * server_listen_function ( void * server_l ) {
  
  SOCKET_T * accepted_socket;
  SERVER_T * server = (SERVER_T * ) server_l;

  while ( server->udata.keep_going == TRUE ) {

    // Locks mutex on the thread
    MUTEX_LOCK(server->udata.mutex);

    if ( TRUE == SOCKET_ACCEPT(server->connection, &accepted_socket) ) 
    {
      process_incoming_request(&accepted_socket);
    }
    
    // Removes lock from mutex
    MUTEX_UNLOCK(server->udata.mutex);

    Sleep(100);

  }
  
  // Sets the thread state
  server->udata.is_running=FALSE;
  LOGGER(__FUNCTION__, "stops listen");
  return 0;
}

/**
 * Starts the thread for the server's listen loop
 *
 * @param server                        server data structure
 *
 */
void server_listen_begin (  SERVER_T * server  ) {

  // Initializes processes structures
  process_init();

  // Creates a thread for the server_listen
  server->udata.keep_going = TRUE;
  server->udata.is_running = TRUE;
  
  // Allocates memory for the mutex
  server->udata.mutex = (MUTEX_T*)malloc(sizeof(MUTEX_T));
  memset(server->udata.mutex, 0x00, sizeof(MUTEX_T));
  
  // Creates the mutex for the new thread
  MUTEX_CREATE (&server->udata.mutex);

  // Creates the thread
  server->listen_thread = (thread_t*) malloc(sizeof(thread_t));
  THREAD_CREATE(&server->listen_thread, &server_listen_function, (void *)server);

}

/**
 * Finalizes the thread's listen loop
 *
 * @param server                        server data structure
 */
void server_listen_finalize (  SERVER_T * server ) {

  // Sets flag to stop the thread
  server->udata.keep_going = FALSE;
  
  //  Waits for the thread to end and destroys it
  THREAD_JOIN(server->listen_thread, TRUE);
  free(server->listen_thread);

  // Destroys the thread's mutex
  MUTEX_DESTROY(&server->udata.mutex);
  free(server->udata.mutex);

  // Finalizes the processes structures
  process_deinit();
}
