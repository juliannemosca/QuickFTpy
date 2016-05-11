/*
 * process.h
 *
 * $Id: $
 * $HeadURL: $
 * $LastChangedRevision: $
 * $LastChangedDate: $
 * $LastChangedBy: $
 *
 */
 
#ifndef PROCESS_H
#define PROCESS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "server.h"
#include "thread.h"

#define ROOT_DIR      "/"
#define MAX_PROCESSES 512
  
typedef struct _process_data_t {

  SOCKET_T * connection; 
  char * received_message;
  int received_msg_len;

  int keep_going;
  int process_id;
  
} PROCESS_DATA_T;

typedef struct _process_t {

  int is_active;
  thread_t* work;
  PROCESS_DATA_T * proc_data;
  
} PROCESS_T;

/**
 * Initializes processes structures for threads
 */
void process_init();

/**
 * Finalizes processes structures for threads
 */
void process_deinit();

/**
 * Processes a request message through a socket
 *
 * @param connection              socket that holds the conexion
 */
void process_incoming_request( SOCKET_T ** connection );

/**
 * Thread function for processing an incoming message
 *
 * @param proc_data_arg           data structure with the thread parameters
 */
void process_incoming_request_worker( void * proc_data_arg );

/**
 * Processes a File Receive message from the client, 
 * performs and finalizes the operation
 *
 * @param proc_data_arg           data structure with connection parameters 
 *                                and received message
 */
void process_file_receive( PROCESS_DATA_T * proc_data );

/**
 * Processes a File Send message from the client, 
 * performs and finalizes the operation
 *
 * @param proc_data_arg           data structure with connection parameters 
 *                                and received message
 */
void process_file_send( PROCESS_DATA_T * proc_data );

/**
 * Processes a File Delete message from the client, 
 * performs and finalizes the operation
 *
 * @param proc_data_arg           data structure with connection parameters 
 *                                and received message
 */
void process_file_delete( PROCESS_DATA_T * proc_data );

/**
 * Sends a synchronous message through a connected node
 * 
 * @param connection            conexion on which the message will be sent
 * @param outgoing_message      outgoing message
 * @param outgoing_message_len  outgoing message length
 *
 * @return                      TRUE if message could be sent, otherwise FALSE
 */ 
int process_outgoing_message( SOCKET_T * connection, char * outgoing_message, int outgoing_message_len );

#endif // PROCESS_H
