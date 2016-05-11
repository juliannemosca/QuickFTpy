/*
 * client.h
 *
 * $Id: $
 * $HeadURL: $
 * $LastChangedRevision: $
 * $LastChangedDate: $
 * $LastChangedBy: $
 *
 */

#ifndef _CLIENT_H
#define _CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "socket.h"

// Timeout for ACK Messages
unsigned long client_timeout_ack;

// Data structure definition for client nodes
typedef struct _quickft_client_t {

  // Connection data structure
  SOCKET_T * connection;
  
} quickft_client_t;

/**
 * Performs a 'File Receive' operation for the client
 *
 */
PyObject * client_file_receive( PyObject * self, PyObject * args );

/**
 * Performs a 'File Send' operation for the client
 *
 */
PyObject * client_file_send( PyObject * self, PyObject * args );

/**
 * Performs a 'File Delete' operation for the client on the server
 *
 */
PyObject * client_file_delete( PyObject * self, PyObject * args );

#ifdef __cplusplus
}
#endif

#endif  // _CLIENT_H