/*
 * client.c
 *
 * $Id: $
 * $HeadURL: $
 * $LastChangedRevision: $
 * $LastChangedDate: $
 * $LastChangedBy: $
 *
 */

#include <sys/stat.h>
#include <python2.7/Python.h>

#include "message.h"
#include "results.h"
#include "process.h"
#include "client.h"
#include "string.h"
#include "logger.h"
#include "base64.h"
#include "quickft.h"
#include "file.h"
#include "time.h"
#include "gz.h"

/**
 * Initializes a QuickFT client
 *
 * @param addr                                    server addr
 * @param port                                    server port
 * @param timeout                                 timeout for parts of the messages, can be 0 for default
 * @param timeout_ack                             timeout for ack messages, can be 0 for default
 * @return                                        pointer of type quickft_client_t or NULL
 */
quickft_client_t * client_initialize( char * addr, int port, long timeout, long timeout_ack ) {
  
  quickft_client_t * new_client;

  LOGGER(__FUNCTION__, "Initializes QUICKFT client.");

  gl_timeout = DEFAULT_TIMEOUT;
  client_timeout_ack = DEFAULT_TIMEOUT_ACK;

  if (timeout != 0) {
    gl_timeout = timeout;
  }
  
  if (timeout_ack != 0) {
    client_timeout_ack = timeout_ack;
  }

  // Initializes the library's sockets functionalities
  if ( ! SOCKET_INIT() ) {
    return NULL;
  }

  new_client = malloc( sizeof(quickft_client_t) );
  memset(new_client, 0x00, sizeof(quickft_client_t));

  // Creates a connection
  new_client->connection = SOCKET_NEW_CLNT(addr, port);
  if ( new_client->connection == NULL ) {

    free(new_client);
    new_client = NULL;
  }
    
  return new_client;
}

/**
 * Closes the client and free()s memory
 *
 * @param client                        client's data structure
 * @return                              TRUE or FALSE
 */
int client_finalize ( quickft_client_t ** client ) {

  LOGGER(__FUNCTION__, "Finalizes QUICKFT client.");

  if ( (*client) != NULL ) {
    
    // Ends connection and closes socket
    SOCKET_SHUTDOWN( &(*client)->connection );
    SOCKET_CLOSE( &(*client)->connection );
    
    // Frees allocated memory
    free(*client);

    // Finalizes library's sockets functionalities
    SOCKET_DEINIT();
    
    return TRUE;
  }

  return FALSE;
}

/**
 * Completes a 'File Receive' operation and returns the result
 *
 * @param incoming_message        received response message
 * @param incoming_message_len    response message length
 * @param local_filename          local name of the file being received
 *
 * @return                        TRUE o FALSE
 */
int client_get_file_receive_response_result(char * incoming_message, int incoming_message_len, char * local_filename) {

  const int NOT_FOUND = -1;

  char l_msg[_BUFFER_SIZE_S];
  char destination_dir[2048];

  int result = RESULT_UNDEFINED;

  int search_length_res;
  int search_content_res;
  int search_result_res;

  char * content  = NULL;
  int length = 0;

  char * response = NULL;
  
  // Moves response to a safe buffer for searching
  response = (char*)malloc(sizeof(char) * incoming_message_len + 1);
  memcpy(response, incoming_message, incoming_message_len);
  response[incoming_message_len] = '\0';

  // Parses parameters
  search_length_res = string_search(response, PARAM_LENGTH, HEADER_LEN);
  search_content_res = string_search(response, PARAM_CONTENT, HEADER_LEN);
  search_result_res = string_search(response, PARAM_RESULT, HEADER_LEN);

  // Finds mandatory 'result' parameter
  if (search_result_res == NOT_FOUND ) {

    snprintf(l_msg, _BUFFER_SIZE_S, "ERROR: invalid response parameters: [%s]", response);
    LOGGER(__FUNCTION__, l_msg);
    
    result = RESULT_INVALID_RESPONSE;
    goto END_FILE_RECEIVE_RESULT;
  }
  else {

    char * result_string; 

    result_string = (char*)malloc(sizeof(char) * (RESULT_VALUE_LEN + 1));
    memcpy(result_string, &( response[ ( search_result_res + strlen(PARAM_RESULT) ) ] ), RESULT_VALUE_LEN );
    result_string[RESULT_VALUE_LEN] = '\0';

    result = message_result_string_to_code(result_string);

    free(result_string);

    if (result != RESULT_SUCCESS) {

      goto END_FILE_RECEIVE_RESULT;
    }
  }

  if (search_length_res == NOT_FOUND || search_content_res == NOT_FOUND) {
    snprintf(l_msg, _BUFFER_SIZE_S, "ERROR: invalid response parameters: [%s]", response);
    LOGGER(__FUNCTION__, l_msg);
    
    result = RESULT_INVALID_RESPONSE;
    goto END_FILE_RECEIVE_RESULT;
  }

  length = atoi( &( response[ ( search_length_res + strlen(PARAM_LENGTH) ) ] ) );

  if (length == 0) {
    
    snprintf(l_msg, _BUFFER_SIZE_S, "ERROR: invalid response parameters: [%s]", response);
    LOGGER(__FUNCTION__, l_msg);

    result = RESULT_INVALID_RESPONSE;
    goto END_FILE_RECEIVE_RESULT;
  }

  content = (char*)malloc(sizeof(char) * (incoming_message_len - search_content_res));
  memcpy(content, &( response[ ( search_content_res + strlen(PARAM_CONTENT) ) ] ), length );

  // Prepares directory
  file_get_base_path(local_filename, destination_dir);
  if (destination_dir != NULL)
  {
    // @HACK: checks if destination directory is not root
    if (strcmp(destination_dir, ROOT_DIR) != 0 ) {

      if ( ! file_directory_exists(destination_dir)) {

        if ( ! file_mkdir_parent(destination_dir) ) {

          sprintf(l_msg, "ERROR: destination directory could not be created %s", destination_dir);
          LOGGER(__FUNCTION__, l_msg);

          result = RESULT_COULD_NOT_CREATE_DESTINATION_DIRECTORY;
          goto END_FILE_RECEIVE_RESULT;

        }
      }
    }
  }
  else {
      
    sprintf(l_msg, "ERROR: destination directory %s is invalid.", local_filename);
    LOGGER(__FUNCTION__, l_msg);

    result = RESULT_INVALID_DESTINATION_DIRECTORY;
    goto END_FILE_RECEIVE_RESULT;
  }

  //
  // Downloads response to a base64 file,then decodes and decompresses the file
  //
  {

    char gzip_filename[256];
    char b64_filename[256];
    FILE * f = NULL;

    sprintf(gzip_filename, "%s.gz", local_filename );
    sprintf(b64_filename, "%s.b64", local_filename );

    f = fopen(b64_filename, "w");
    if (f != NULL) {
      fwrite( content, sizeof(char), length, f );
      fclose(f);

      if ( base64_process_file('d', b64_filename, gzip_filename, incoming_message_len) == TRUE ) {
  
        if ( gz_unpack_file(gzip_filename, local_filename) == TRUE )
        {      
          remove(b64_filename);
          remove(gzip_filename);
        }
        else {
          snprintf(l_msg, _BUFFER_SIZE_S, "Error processing file (%s)", gzip_filename);
          LOGGER(__FUNCTION__, l_msg);

          result = RESULT_FILE_DECOMPRESS_ERROR;
        }
  
      }
      else {
        snprintf(l_msg, _BUFFER_SIZE_S, "Error processing file (%s)", b64_filename);
        LOGGER(__FUNCTION__, l_msg);

        result = RESULT_FILE_DECODE_ERROR;
      }

    }
    
  }

END_FILE_RECEIVE_RESULT:

  // Cleanup
  if (response != NULL) {
    free(response);
  }
  if (content != NULL) {
    free(content);
  }

  return result;
}

/**
 * Completes a 'File Send' operation and returns the result
 *
 * @param incoming_message        received response message
 * @param incoming_message_len    received response message length
 *
 * @return                        TRUE o FALSE
 */
int client_get_file_send_response_result(char * incoming_message, int incoming_message_len) {

  const int NOT_FOUND = -1;

  char l_msg[_BUFFER_SIZE_S];

  int result = RESULT_UNDEFINED;
  int search_result_res;

  char * response;

  // Moves response to a safe buffer for searching
  response = (char*)malloc(sizeof(char) * incoming_message_len + 1);
  memcpy(response, incoming_message, incoming_message_len);
  response[incoming_message_len] = '\0';

  // Parses parameters
  search_result_res = string_search(response, PARAM_RESULT, HEADER_LEN);

  if (search_result_res == NOT_FOUND ) {
    snprintf(l_msg, _BUFFER_SIZE_S, "ERROR: invalid response parameters [%s]", response);
    LOGGER(__FUNCTION__, l_msg);
    
    result = RESULT_INVALID_RESPONSE;
    
  }
  else {

    char * result_string; 

    result_string = (char*)malloc(sizeof(char) * (RESULT_VALUE_LEN + 1));
    memcpy(result_string, &( response[ ( search_result_res + strlen(PARAM_RESULT) ) ] ), RESULT_VALUE_LEN );
    result_string[RESULT_VALUE_LEN] = '\0';

    result = message_result_string_to_code(result_string);

    free(result_string);

  }

  free(response);

  return result;
}

/**
 * Completes a 'File Delete' operation and returns the result
 *
 * @param incoming_message        mensaje de response recibido
 * @param incoming_message_len    longitud del mensaje de response
 *
 * @return                        TRUE o FALSE
 */
int client_get_file_delete_response_result(char * incoming_message, int incoming_message_len) {

  const int NOT_FOUND = -1;

  char l_msg[_BUFFER_SIZE_S];

  int result = RESULT_UNDEFINED;
  int search_result_res;

  char * response;

  // Moves response to a safe buffer for searching
  response = (char*)malloc(sizeof(char) * incoming_message_len + 1);
  memcpy(response, incoming_message, incoming_message_len);
  response[incoming_message_len] = '\0';

  // Parses parameters
  search_result_res = string_search(response, PARAM_RESULT, HEADER_LEN);

  if (search_result_res == NOT_FOUND ) {
    snprintf(l_msg, _BUFFER_SIZE_S, "ERROR: invalid response parameters: [%s]", response);
    LOGGER(__FUNCTION__, l_msg);
    
    result = RESULT_INVALID_RESPONSE;
    
  }
  else {

    char * result_string; 

    result_string = (char*)malloc(sizeof(char) * (RESULT_VALUE_LEN + 1));
    memcpy(result_string, &( response[ ( search_result_res + strlen(PARAM_RESULT) ) ] ), RESULT_VALUE_LEN );
    result_string[RESULT_VALUE_LEN] = '\0';

    result = message_result_string_to_code(result_string);

    free(result_string);

  }

  free(response);

  return result;
}

/**
 * Function for processing an incoming acknowledgment message
 *
 * @param client                  client's data structure
 *
 * @return                        RESULT_SUCCESS, RESULT_CONNECTION_ERROR o RESULT_INVALID_RESPONSE
 */
int client_get_ack( quickft_client_t * client ) {

  int brecv = 0;
  int selectval = 0;
  
  char * recbuf = NULL;
  unsigned long exec_timeout = 0;  
  int result = RESULT_CONNECTION_ERROR;
  
  brecv = strlen(MESSAGE_ACK);

  // Allocates initial space for message
  recbuf = malloc(sizeof(char) * brecv);

  // Updates time of next timeout
  exec_timeout = GetTickCount() + client_timeout_ack;

  while (1) {

    // Evaluates if timeout has been reached to cancel the operation
    if (GetTickCount() > exec_timeout) {

      result = RESULT_CONNECTION_ERROR;
      break;
    }

    selectval = SOCKET_SELECT(S_TIMEOUT, client->connection, S_READ);          
    if ( selectval == S_READ ) {

      // Attempts to receive message
      if ( ! SOCKET_RECV(client->connection, &recbuf, &brecv) ) {
    
        // Produces error on fail
        LOGGER(__FUNCTION__, "ERROR: A connection error occurred while attempting to receive the message." );
        break;        
      }

      if ( brecv > 0) {

        unsigned int received = brecv;
        if ( received == strlen(MESSAGE_ACK) ) {

          if (memcmp(recbuf, MESSAGE_ACK, strlen(MESSAGE_ACK)) == 0) {

            result = RESULT_SUCCESS;
            break;
          }

        }

      }

    }

  }
  
  // Cleanup
  if (recbuf != NULL) {
    free(recbuf);
  }

  return result;
}

/**
 * Function for processing an incoming response message
 *
 * @param client                  client's data structure
 * @param response                pointer by reference to a non-allocated buffer to
 *                                store the response. Must be free()'d after usage. 
 * @param response_len            reference to a variable to store the response length
 *
 * @return                        type of message or error code
 */
int client_get_response( quickft_client_t * client, char ** response, unsigned long * response_len ) {

  int brecv = 0;

  int selectval = 0;
  
  char * recbuf = NULL;
  char * incoming_message = NULL;

  int message_complete = FALSE;

  long total_bytes_received = 0;
  long incoming_msg_len = 0;
  long var_part_size = 0;

  int header_complete = FALSE;

  unsigned long exec_timeout = 0;
  
  int result = RESULT_UNDEFINED;
  
  *response = NULL;
  *response_len = 0;

  // Attempts to obtain an ACK
  result = client_get_ack(client);
  if (result != RESULT_SUCCESS) {
    return result;
  }

  incoming_message = malloc(HEADER_LEN);
  brecv = incoming_msg_len = HEADER_LEN;

  // Allocates initial space for message
  recbuf = malloc(sizeof(char) * HEADER_LEN + VAR_PART_MINIMUM_LEN);

  // Updates the time of the next timeout
  exec_timeout = GetTickCount() + gl_timeout;

  while (message_complete != TRUE) {

    // Evaluates if timeout has been reached to cancel the operation
    if (GetTickCount() > exec_timeout) {

      result = RESULT_CONNECTION_ERROR;
      goto END_GET_RESPONSE;
    }

    selectval = SOCKET_SELECT(S_TIMEOUT, client->connection, S_READ);          
    if ( selectval == S_READ ) {

      // Attempts to receive message
      if ( ! SOCKET_RECV(client->connection, &recbuf, &brecv) ) {
    
        // Produces error on fail
        LOGGER(__FUNCTION__, "ERROR: A connection error occurred while attempting to receive the message." );
        break;        
      }

      if ( brecv > 0) {
     
        total_bytes_received += brecv;

        // Updates time for next timeout
        exec_timeout = GetTickCount() + gl_timeout;
      
        // Verifies if header was previously completed
        if ( header_complete == TRUE ) {
      
          // Copies the new message fragment
          memcpy( &incoming_message[total_bytes_received-brecv], recbuf, brecv );

          // If the size of received message has reached the total message size
          if ( total_bytes_received == ( HEADER_LEN + var_part_size ) ) {
          
            // Message is complete
            message_complete = TRUE;
            break;
          }
          else {

            int bytes_left = incoming_msg_len - total_bytes_received;
            if ( bytes_left < CHUNK_SIZE ) {
              brecv = bytes_left;
            }
            else {
              brecv = CHUNK_SIZE;
            }
          }
        }
        // Verifies if header is complete
        else if ( total_bytes_received == HEADER_LEN ) {
      
          // Copies the new message fragment
          memcpy( &incoming_message[0], recbuf, total_bytes_received );

          // Checks for valid header and gets incoming message type
          result = IS_VALID_HEADER( incoming_message, &var_part_size, ( FILE_SND_B + FILE_RCV_B + FILE_DEL_B ) );          
          if ( result > 0x00 ) {
        
            header_complete = TRUE;

            if ( var_part_size == 0 ) {
                            
              // If size of the variable part is 0 produces error
              LOGGER(__FUNCTION__, "ERROR: Length of variable part cannot be 0." );
              break;
            }
          
            if (var_part_size > CHUNK_SIZE) {
              brecv = CHUNK_SIZE;
            }
            else {
              brecv = var_part_size;
            }

            incoming_msg_len = HEADER_LEN + var_part_size;
            incoming_message = realloc( incoming_message, incoming_msg_len );
            memset( &incoming_message[HEADER_LEN], 0x00, var_part_size );

            recbuf = realloc( recbuf, incoming_msg_len );
            memset( &recbuf[HEADER_LEN], 0x00, var_part_size );
          }
          else {

            result = RESULT_INVALID_RESPONSE;

            // If header is not valid produces error
            LOGGER(__FUNCTION__, "ERROR: The message does not have a valid header." );
            break;
          }
      
        }

      }

    }

  }
  
END_GET_RESPONSE:

  // Cleanup
  if (recbuf != NULL) {
    free(recbuf);
  }

  if ( result <= 0 ) {
    free(incoming_message);
    incoming_message = NULL;
  }

  *response = incoming_message;
  *response_len = incoming_msg_len;

  return result;
}


/**
 * Finds, compresses and encodes a file and generates content for a send file
 * operation message
 *
 * @param local_filename        local name of the file being sent
 * @param content               pointer by reference to a non-allocated buffer to
 *                              store the content. Must be free()'d after usage. May return NULL.
 * @param content_len           reference to a variable to store the content length
 *
 * @return                      result type of operation
 */
int client_generate_content_from_file(char * local_filename, char ** content, unsigned long * content_len) {

  char l_msg[_BUFFER_SIZE_S];

  int result = RESULT_UNDEFINED;

  *content = NULL;
  *content_len = 0;

  if ( ! file_exists(local_filename) ) 
  {
    LOGGER(__FUNCTION__, "No files were found in the directory for the specified mask.");
    result = RESULT_FILE_NOT_FOUND;
  }
  else {

    long long filesize = file_size(local_filename);
    if (filesize > 0) 
    {
      char gzip_output[256];
      char b64_output[256];

      sprintf(gzip_output, "%s.gz", local_filename );
      sprintf(b64_output, "%s.b64", local_filename );

      if ( gz_pack_file(local_filename, gzip_output) == TRUE )
      {
        LOGGER(__FUNCTION__, "Content to send succesfully packed.");
        filesize = file_size(gzip_output);

        snprintf(l_msg, _BUFFER_SIZE_S, "Preparing to encode %lld bytes.", filesize);
        LOGGER(__FUNCTION__, l_msg);

        if ( base64_process_file('e', gzip_output, b64_output, filesize) == TRUE )
        {
          unsigned long bytes_read;
          char * buffer = NULL;
          FILE * f = NULL;

          LOGGER(__FUNCTION__, "Content to send succesfully encoded.");

          filesize = file_size(b64_output);          
          buffer = (char*)malloc(sizeof(char) * filesize);

          f = fopen(b64_output, "r");
          if (f != NULL)
          {
            bytes_read = fread(buffer, 1, filesize, f);
              
            snprintf(l_msg, _BUFFER_SIZE_S, "%lu bytes read from file %s to process and send.", bytes_read, b64_output);
            LOGGER(__FUNCTION__, l_msg);

            fclose(f);

            // Copy results to the output params
            *content = buffer;
            *content_len = strlen(buffer);

            result = RESULT_SUCCESS;
              
            // Deletes any temporary files generated
            remove(gzip_output);
            remove(b64_output);

          }
          else {
            snprintf(l_msg, _BUFFER_SIZE_S, "Error opening file (%s)", b64_output);
            LOGGER(__FUNCTION__, l_msg);

            result = RESULT_FILE_READ_ERROR;
          }
  
        }
        else {
          snprintf(l_msg, _BUFFER_SIZE_S, "Error encoding file (%s)", gzip_output);
          LOGGER(__FUNCTION__, l_msg);

          result = RESULT_FILE_ENCODE_ERROR;
        }

      }
      else {
        snprintf(l_msg, _BUFFER_SIZE_S, "Error packing file (%s)", local_filename);
        LOGGER(__FUNCTION__, l_msg);

        result = RESULT_FILE_COMPRESS_ERROR;
      }

    } else {

      snprintf(l_msg, _BUFFER_SIZE_S, "Content of the file is null (%s)", local_filename);
      LOGGER(__FUNCTION__, l_msg);
    }

  }

  return result;
}

/**
 * Performs a 'File Receive' operation for the client
 *
 */
PyObject * client_file_receive( PyObject * self, PyObject * args ) {

  char l_msg[_BUFFER_SIZE_S];

  char * request  = NULL;
  char * response = NULL;

  unsigned long request_len;
  unsigned long response_len; 

  int message_type = 0;
  int result = RESULT_UNDEFINED;

  quickft_client_t * client;

  // Function parameters
  char * remote_filename;
  char * local_filename;
  char * addr;
  char * port; 
  int timeout; 
  int timeout_ack;
  PyObject * py_log_writer;
  
  // Parses arguments
  if (!PyArg_ParseTuple(args, "ssssiiO",&remote_filename, 
                                        &local_filename, 
                                        &addr, 
                                        &port,
                                        &timeout,
                                        &timeout_ack,
                                        &py_log_writer)) {
    return Py_BuildValue("i", FALSE);
  }
  
  // Makes sure fourth argument is a function
  if (!PyCallable_Check(py_log_writer)) {
    PyErr_SetString(PyExc_TypeError, "Argument is not a function.");  
  }
  
  // Stores the log writer function
  gl_py_log_writer = py_log_writer;
  
  // Initializes the log
  LOGGER_INIT;
  
  LOGGER(__FUNCTION__, "Begins a File Receive operation.");

  if (addr == NULL) {
    LOGGER(__FUNCTION__, "Server Addr can not be null");
    return Py_BuildValue("i", result);
  }
  
  // Initializes a client
  if (port == NULL) {
    client = client_initialize( addr, DEFAULT_PORT, timeout, timeout_ack );
  }
  else {
    client = client_initialize( addr, atoi(port), timeout, timeout_ack );
  }

  if (client == NULL) {

    LOGGER(__FUNCTION__, "Error on client initialization.");
    return Py_BuildValue("i", result);
  }

  // Generates request message
  request = message_file_receive_request(strlen(remote_filename), remote_filename, &request_len);

  // Sends the request
  if ( process_outgoing_message(client->connection, request, request_len) == TRUE ) {

    message_type = client_get_response(client, &response, &response_len);
    if ( message_type == FILE_RCV_B ) {
      
      if (response != NULL && response_len != 0) {

        // Logs response
        LOGGER(__FUNCTION__, "Gets response from server...");
        if ( response_len >= _BUFFER_SIZE_S ) {
          snprintf(l_msg, _BUFFER_SIZE_S, "%s", response);
          l_msg[_BUFFER_SIZE_S - 1] = '\0';
        }
        else {
          snprintf(l_msg, response_len, "%s", response);
          l_msg[response_len] = '\0';
        }
        
        LOGGER(__FUNCTION__, l_msg);

        // Completes the operation and gets the result
        result = client_get_file_receive_response_result(response, response_len, local_filename);
      }
      else {

        sprintf(l_msg, "Could not get a valid response from the QUICKFT server at %s:%s", addr, port);
        LOGGER(__FUNCTION__, l_msg);
        result = RESULT_CONNECTION_ERROR;
      }
      
    }
    else {

      // If the response message type is not correct
      if (message_type > 0) {
        sprintf(l_msg, "Message type [%02d] is invalid for expected response.", message_type);
        LOGGER(__FUNCTION__, l_msg);

        result = RESULT_INVALID_RESPONSE;
      }
      // If an error occurred
      else {
        sprintf(l_msg, "An error ocurred when trying to read response [%d]", message_type);
        LOGGER(__FUNCTION__, l_msg);

        result = message_type;
      }

    }

  }
  else {
    LOGGER(__FUNCTION__, "An error occurred while trying to send the request.");
    result = RESULT_CONNECTION_ERROR;
  }

  // Finalizes the client data structure
  client_finalize(&client);

  // Frees allocated memory 
  if (request != NULL) {
    free(request);
  }
  if (response != NULL) {
    free(response);
  }

  LOGGER(__FUNCTION__, "Finalizes File Receive operation.");

  // Finalizes the log
  LOGGER_DEINIT;
  
  return Py_BuildValue("i", result);

}

/**
 * Performs a 'File Send' operation for the client
 *
 */
PyObject * client_file_send( PyObject * self, PyObject * args ) {

  char l_msg[_BUFFER_SIZE_S];

  char * request  = NULL;
  char * response = NULL;

  char * content  = NULL;

  unsigned long request_len;
  unsigned long response_len;
  unsigned long content_len;

  int message_type = 0;
  int result = RESULT_UNDEFINED;

  quickft_client_t * client;

  // Function parameters
  char * remote_filename;
  char * local_filename;
  char * addr;
  char * port; 
  int timeout; 
  int timeout_ack;
  PyObject * py_log_writer;
  
  // Parses arguments
  if (!PyArg_ParseTuple(args, "ssssiiO",&remote_filename, 
                                        &local_filename, 
                                        &addr, 
                                        &port,
                                        &timeout,
                                        &timeout_ack,
                                        &py_log_writer)) {
    return Py_BuildValue("i", FALSE);
  }
  
  // Makes sure fourth argument is a function
  if (!PyCallable_Check(py_log_writer)) {
    PyErr_SetString(PyExc_TypeError, "Argument is not a function.");  
  }
  
  // Stores the log writer function
  gl_py_log_writer = py_log_writer;
  
  // Initializes the log
  LOGGER_INIT;
  
  LOGGER(__FUNCTION__, "Begins a File Send operation.");

  if (addr == NULL) {
    LOGGER(__FUNCTION__, "Server Addr can not be null");
    return Py_BuildValue("i", result);
  }
  
  // Initializes a client
  if (port == NULL) {
    client = client_initialize( addr, DEFAULT_PORT, timeout, timeout_ack );
  }
  else {
    client = client_initialize( addr, atoi(port), timeout, timeout_ack );
  }

  if (client == NULL) {

    LOGGER(__FUNCTION__, "Error on client initialization.");
    return Py_BuildValue("i", result);
  }

  // Generates content for request
  result = client_generate_content_from_file(local_filename, &content, &content_len);  
  if ( result == RESULT_SUCCESS ) {

    // Generates request message
    request = message_file_send_request(remote_filename, content_len, content, &request_len);

    // Sends the message
    if ( process_outgoing_message(client->connection, request, request_len) == TRUE) {

      message_type = client_get_response(client, &response, &response_len);
      if ( message_type == FILE_SND_B ) {
      
        if (response != NULL  && response_len != 0) {

          // Logs response
          LOGGER(__FUNCTION__, "Gets response from server...");
          if ( response_len >= _BUFFER_SIZE_S ) {
            snprintf(l_msg, _BUFFER_SIZE_S, "%s", response);
            l_msg[_BUFFER_SIZE_S - 1] = '\0';
          }
          else {
            snprintf(l_msg, response_len, "%s", response);
            l_msg[response_len] = '\0';
          }
        
          LOGGER(__FUNCTION__, l_msg);

          // Completes the operation and gets the result
          result = client_get_file_send_response_result(response, response_len);
        }
        else {

          sprintf(l_msg, "Could not get a valid response from the QUICKFT server at %s:%s", addr, port);
          LOGGER(__FUNCTION__, l_msg);
          result = RESULT_CONNECTION_ERROR;
        }
      
      }
      else {

        // If the response message type is not correct
        if (message_type > 0) {
          sprintf(l_msg, "Message type [%02d] is invalid for expected response.", message_type);
          LOGGER(__FUNCTION__, l_msg);

          result = RESULT_INVALID_RESPONSE;
        }
        // If an error occurred
        else {
          sprintf(l_msg, "An error ocurred when trying to read response [%d]", message_type);
          LOGGER(__FUNCTION__, l_msg);

          result = message_type;
        }

      }

    }
    else {
      LOGGER(__FUNCTION__, "An error occurred while trying to send the request.");
      result = RESULT_CONNECTION_ERROR;
    }

  }
  else {
    snprintf(l_msg, _BUFFER_SIZE_S, "An error occurred while trying to generate content from file [%s]", local_filename);
    LOGGER(__FUNCTION__, l_msg);
  }

  // Finalizes the client data structure
  client_finalize(&client);

  // Frees allocated memory
  if (request != NULL) {
    free(request);
  }
  if (response != NULL) {
    free(response);
  }
  if (content != NULL) {
    free(content);
  }

  LOGGER(__FUNCTION__, "Finalizes File Send operation.");

  // Finalizes the log
  LOGGER_DEINIT;
  
  return Py_BuildValue("i", result);

}

/**
 * Performs a 'File Delete' operation for the client on the server
 *
 */
PyObject * client_file_delete( PyObject * self, PyObject * args ) {

  char l_msg[_BUFFER_SIZE_S];

  char * request  = NULL;
  char * response = NULL;

  unsigned long request_len;
  unsigned long response_len; 

  int message_type = 0;
  int result = RESULT_UNDEFINED;

  quickft_client_t * client;

  // Function parameters
  char * remote_filename;
  char * addr;
  char * port; 
  int timeout; 
  int timeout_ack;
  PyObject * py_log_writer;
  
  // Parses arguments
  if (!PyArg_ParseTuple(args, "sssiiO",&remote_filename,  
                                       &addr, 
                                       &port,
                                       &timeout,
                                       &timeout_ack,
                                       &py_log_writer)) {
    return Py_BuildValue("i", FALSE);
  }
  
  // Makes sure fourth argument is a function
  if (!PyCallable_Check(py_log_writer)) {
    PyErr_SetString(PyExc_TypeError, "Argument is not a function.");  
  }
  
  // Stores the log writer function
  gl_py_log_writer = py_log_writer;
  
  // Initializes the log
  LOGGER_INIT;
  
  LOGGER(__FUNCTION__, "Begins a File Delete operation.");

  if (addr == NULL) {
    LOGGER(__FUNCTION__, "Server Addr can not be null");
    return Py_BuildValue("i", result);
  }
  
  // Initializes a client
  if (port == NULL) {
    client = client_initialize( addr, DEFAULT_PORT, timeout, timeout_ack );
  }
  else {
    client = client_initialize( addr, atoi(port), timeout, timeout_ack );
  }

  if (client == NULL) {

    LOGGER(__FUNCTION__, "Error on client initialization.");
    return Py_BuildValue("i", result);
  }

  // Generates request message
  request = message_file_delete_request(strlen(remote_filename), remote_filename, &request_len);

  // Sends the message
  if ( process_outgoing_message(client->connection, request, request_len) == TRUE ) {

    message_type = client_get_response(client, &response, &response_len);
    if ( message_type == FILE_DEL_B ) {
      
      if (response != NULL && response_len != 0) {

        // Logs response
        LOGGER(__FUNCTION__, "Gets response from server...");
        if ( response_len >= _BUFFER_SIZE_S ) {
          snprintf(l_msg, _BUFFER_SIZE_S, "%s", response);
          l_msg[_BUFFER_SIZE_S - 1] = '\0';
        }
        else {
          snprintf(l_msg, response_len, "%s", response);
          l_msg[response_len] = '\0';
        }
        
        LOGGER(__FUNCTION__, l_msg);

        // Completes the operation and gets the result
        result = client_get_file_delete_response_result(response, response_len);
      }
      else {

        sprintf(l_msg, "Could not get a valid response from the QUICKFT server at %s:%s", addr, port);
        LOGGER(__FUNCTION__, l_msg);
        result = RESULT_CONNECTION_ERROR;
      }
      
    }
    else {

      // If the response message type is not correct
      if (message_type > 0) {
        sprintf(l_msg, " Message type [%02d] is invalid for expected response.", message_type);
        LOGGER(__FUNCTION__, l_msg);

        result = RESULT_INVALID_RESPONSE;
      }
      // If an error occurred
      else {
        sprintf(l_msg, "An error ocurred when trying to read response [%d]", message_type);
        LOGGER(__FUNCTION__, l_msg);

        result = message_type;
      }

    }

  }
  else {
    LOGGER(__FUNCTION__, "An error occurred while trying to send the request.");
    result = RESULT_CONNECTION_ERROR;
  }

  // Finalizes the client data structure
  client_finalize(&client);

  // Frees allocated memory
  if (request != NULL) {
    free(request);
  }
  if (response != NULL) {
    free(response);
  }

  LOGGER(__FUNCTION__, "Finalizes File Delete operation.");
  
  // Finalizes the log
  LOGGER_DEINIT;
  
  return Py_BuildValue("i", result);

}
