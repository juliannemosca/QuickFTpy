/*
 * process.c
 *
 * $Id: $
 * $HeadURL: $
 * $LastChangedRevision: $
 * $LastChangedDate: $
 * $LastChangedBy: $
 *
 */


#include <sys/types.h>
#include <sys/stat.h>

#include "process.h"
#include "message.h"
#include "results.h"
#include "server.h"
#include "string.h"
#include "base64.h"
#include "quickft.h"
#include "gz.h"
#include "socket.h"
#include "time.h"
#include "file.h"

static PROCESS_T processes[MAX_PROCESSES];
static int abort_processes;

/**
 * @NOTE:
 *   the functions on this unit are exclusively used by the server, with
 *   the sole exception of the function 'process_outgoing_message' which
 *   is also used by the client.
 *   For the rest of the operations, client implements its own functions
 *   located on the 'client.c' unit.
 */

/**
 * Initializes processes structures for threads
 */
void process_init() {

  memset(processes, 0x00, ( sizeof(PROCESS_T) * MAX_PROCESSES ) );
  abort_processes = FALSE;

}

/**
 * Finalizes processes structures for threads
 */
void process_deinit() {

  int iter;
  abort_processes = TRUE;
  
  for (iter = 0; iter < MAX_PROCESSES; iter++) {
   
    if (processes[iter].is_active == TRUE) {

      SOCKET_SHUTDOWN(&(processes[iter].proc_data->connection));
            
      // Waits for the thread to end and then destroys it
      THREAD_JOIN(processes[iter].work, TRUE);
      free(processes[iter].work);

      processes[iter].is_active = FALSE;

      SOCKET_CLOSE(&(processes[iter].proc_data->connection));

      free(processes[iter].proc_data);

    }
    
  }

}

/**
 * Processes a request message through a socket
 *
 * @param connection              socket that holds the conexion
 */
void process_incoming_request(SOCKET_T** connection) {

  int iter;

  // Finds the first available node
  for (iter = 0; iter < MAX_PROCESSES; iter++) {

    if ( processes[iter].is_active == FALSE ) {

      processes[iter].is_active = TRUE;
      processes[iter].proc_data = (PROCESS_DATA_T*)malloc(sizeof(PROCESS_DATA_T));
      processes[iter].proc_data->process_id = iter;
      processes[iter].proc_data->connection = *connection;
      
      processes[iter].work = (thread_t*)malloc(sizeof(thread_t));
      THREAD_CREATE(&processes[iter].work, (void *)&process_incoming_request_worker, (void *)processes[iter].proc_data );

      break;

    }

  }

}

/**
 * Thread function for processing an incoming message
 *
 * @param proc_data_arg           data structure with the thread parameters
 */
void process_incoming_request_worker( void * proc_data_arg ) {

  int brecv = 0;

  int selectval = 0;

  char * recbuf = NULL;
  char * incoming_message = NULL;

  int message_complete = FALSE;
  unsigned long message_type = 0;

  long total_bytes_received = 0;
  long incoming_msg_len = 0;
  long var_part_size = 0;
  
  int header_complete = FALSE;

  unsigned long exec_timeout = 0;
  
  PROCESS_DATA_T * proc_data = ( PROCESS_DATA_T * ) proc_data_arg;

  incoming_message = malloc(HEADER_LEN);
  brecv = incoming_msg_len = HEADER_LEN;

  // Allocates initial space for message
  recbuf = malloc(sizeof(char) * HEADER_LEN + VAR_PART_MINIMUM_LEN);

  // Update moment of next timeout
  exec_timeout = (GetTickCount() + gl_timeout);
  
  while (message_complete != TRUE && abort_processes == FALSE) {

    // Evaluates if operation timed out and cancels
    if (GetTickCount() > exec_timeout) {

      LOGGER(__FUNCTION__, "ERROR: Message transfer operation timed out.");
      goto END_PROCESS_INCOMING_REQUEST;
    }

    selectval = SOCKET_SELECT(S_TIMEOUT, proc_data->connection, S_READ);
    if ( selectval == S_READ ) {

      // Attempts to receive the message
      if ( ! SOCKET_RECV(proc_data->connection, &recbuf, &brecv) ) {
    
        // Produces error on fail
        LOGGER(__FUNCTION__, "ERROR: A connection problem occurred while attemting to receive message." );
        break;        
      }

      if ( brecv > 0) {
     
        total_bytes_received += brecv;

        // Updates moment of next timeout
        exec_timeout = GetTickCount() + gl_timeout;

        // Checks if header was previously completed
        if ( header_complete == TRUE ) {
      
          // Copies the new message fragment
          memcpy( &incoming_message[total_bytes_received-brecv], recbuf, brecv );

          // If received message size has reached the total message size
          if ( total_bytes_received == ( HEADER_LEN + var_part_size ) ) {
          
            // Message completed
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
        // Checks if header is complete
        else if ( total_bytes_received == HEADER_LEN ) {
      
          // Copies the new fragment of message
          memcpy( &incoming_message[0], recbuf, total_bytes_received );

          // Validates header and gets message type
          message_type = IS_VALID_HEADER( incoming_message, &var_part_size, ( FILE_SND_B + FILE_RCV_B + FILE_DEL_B ) );          
          if ( message_type > 0x00 ) {
        
            header_complete = TRUE;

            if ( var_part_size == 0 ) {
                            
              // If var part size is 0 fails
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
            recbuf = realloc( recbuf, incoming_msg_len );
          }
          else {
        
            // If header is not valid fails
            LOGGER(__FUNCTION__, "ERROR: The message does not have a valid header." );
            break;
          }
      
        }

      }

    }

  }

  proc_data->received_message = incoming_message;
  proc_data->received_msg_len = incoming_msg_len;

  // Sends an ACK message to client
  if ( ! process_outgoing_message(proc_data->connection, MESSAGE_ACK, strlen(MESSAGE_ACK)) ) {

    LOGGER(__FUNCTION__, "ERROR: Acknowledgment message could not be sent.");
    goto END_PROCESS_INCOMING_REQUEST;
  }

  // If it is a File Receive message
  if ( message_type == FILE_RCV_B ) {
      
    // Process the message
    process_file_receive( proc_data );    
  }
      
  // If it is a File Send message
  if ( message_type == FILE_SND_B ) {
      
    // Process the message
    process_file_send( proc_data );      
  }
      
  // If it is a File Delete message
  if ( message_type == FILE_DEL_B ) {
      
    // Process the message
    process_file_delete( proc_data );      
  }

END_PROCESS_INCOMING_REQUEST:

  // Cleanup
  if (incoming_message != NULL) {
    free(incoming_message);
  }
  if (recbuf != NULL) {
    free(recbuf);
  }

  {
    int proc_id = proc_data->process_id;

    SOCKET_CLOSE(&(proc_data->connection));
    
    free(processes[proc_id].proc_data);
    processes[proc_id].proc_data = NULL;

    processes[proc_id].is_active = FALSE;
    
  }

  return;
}

/**
 * Processes a File Receive message from the client, 
 * performs and finalizes the operation
 *
 * @param proc_data_arg           data structure with connection parameters 
 *                                and received message
 */
void process_file_receive( PROCESS_DATA_T * proc_data ) {

  const int NOT_FOUND = -1;

  char * filename   = NULL;
  char l_msg[4096];

  char * request    = NULL;
  char * response   = NULL;

  int search_filename_res    = 0;
  unsigned long response_len = 0;

  int result = RESULT_UNDEFINED;
  const int param_len = (proc_data->received_msg_len - HEADER_LEN - strlen(PARAM_FILENAME));

  // Copies the request to a safe buffer for searching
  request = (char*)malloc(sizeof(char) * proc_data->received_msg_len + 1);
  memcpy(request, proc_data->received_message, proc_data->received_msg_len);
  request[proc_data->received_msg_len] = '\0';

  search_filename_res = STR_SEARCH( request, PARAM_FILENAME, HEADER_LEN);
  if ( search_filename_res == NOT_FOUND ) {

    result = RESULT_INVALID_REQUEST;
    goto END_PROCESS_FILE_RECEIVE;
  }
  
  // Gets name of file to send
  filename = (char*)malloc(sizeof(char) * proc_data->received_msg_len);
  memcpy( filename, &(request[ ( search_filename_res + strlen(PARAM_FILENAME) ) ]), param_len );
  filename[param_len] = '\0';

  sprintf(l_msg, "A request has been received to send the following file: %s", filename);
  LOGGER(__FUNCTION__, l_msg);

  //
  // Find, pack, and encode file
  //
  {
    long long filesize = file_size(filename);

    if ( ! file_exists(filename) || filesize <= 0 ) {

      LOGGER(__FUNCTION__, "No files have been found for the specified mask.");
      result = RESULT_FILE_NOT_FOUND;
    } else {

      char gzip_output[256];
      char b64_output[256];

      sprintf(gzip_output, ".\\%d-%lu.gz", proc_data->process_id, GetTickCount() );
      sprintf(b64_output, ".\\%d-%lu.b64", proc_data->process_id, GetTickCount() );

      if ( gz_pack_file(filename, gzip_output) == TRUE )
      {

        if ( base64_process_file('e', gzip_output, b64_output, filesize) == TRUE )
        {
          unsigned long bytes_read;
          char * buffer = NULL;
          FILE * f = NULL;

          filesize = file_size(b64_output);
          buffer = (char*)malloc(sizeof(char) * filesize + 1);
          memset(buffer, 0x00, filesize + 1);

          f = fopen(b64_output, "r");
          if (f != NULL)
          {
            bytes_read = fread(buffer, 1, filesize, f);

            sprintf(l_msg, "%lu bytes read from file %s to process and send.", bytes_read, b64_output);
            LOGGER(__FUNCTION__, l_msg);

            fclose(f);

            result = RESULT_SUCCESS;

            // Generates response message with file content
            response = message_file_receive_response( result, strlen(buffer), buffer, &response_len );

            // Sends a File Receive response message
            if ( !process_outgoing_message( proc_data->connection, response, response_len ) ) {
    
              LOGGER(__FUNCTION__, "File Receive message could not be sent.");
            }
            
            // Deletes generated temporary files
            remove(gzip_output);
            remove(b64_output);

            // Frees response
            free(response);

          } else {

            sprintf(l_msg, "Error opening file (%s)", b64_output);
            LOGGER(__FUNCTION__, l_msg);

            result = RESULT_FILE_READ_ERROR;
          }

          // Frees buffer
          free(buffer);
          
        } else {

          sprintf(l_msg, "Error encoding file (%s)", gzip_output);
          LOGGER(__FUNCTION__, l_msg);

          result = RESULT_FILE_ENCODE_ERROR;
        }

      } else {

        sprintf(l_msg, "Error packing file (%s)", filename);
        LOGGER(__FUNCTION__, l_msg);

        result = RESULT_FILE_COMPRESS_ERROR;
      }

    }

  }

END_PROCESS_FILE_RECEIVE:

  // Cleanup
  if (request != NULL) {
    free(request);
  }
  if (filename != NULL) {
    free(filename);
  }

  if (result == RESULT_SUCCESS) {

    return;
  }
  else {

    // Generates response
    response = message_file_receive_response( result, 0, NULL, &response_len );

    // Sends a File Receive response message
    if ( !process_outgoing_message( proc_data->connection, response, response_len ) ) {
    
      LOGGER(__FUNCTION__, "File Receive response message could not be sent.");
    }

    // Libera la respuesta
    free(response);
  }

  return;
}

/**
 * Processes a File Send message from the client, 
 * performs and finalizes the operation
 *
 * @param proc_data_arg           data structure with connection parameters 
 *                                and received message
 */
void process_file_send( PROCESS_DATA_T * proc_data ) {

  const int NOT_FOUND = -1;

  char * filename = NULL;
  char * content  = NULL;

  char * request  = NULL;
  char * response = NULL;

  int search_path_res         = 0;
  int search_length_res       = 0;
  int search_content_res      = 0;

  unsigned long response_len  = 0;

  char destination_dir[2048];
  char l_msg[4096];

  int result = RESULT_UNDEFINED;  
    
  unsigned long content_len = 0;
  int param_path_len = 0;//= (proc_data->received_msg_len - HEADER_LEN - strlen(PARAM_PATH));
 
  // Copies the request to a safe buffer for searching
  request = (char*)malloc(sizeof(char) * proc_data->received_msg_len + 1);
  memcpy(request, proc_data->received_message, proc_data->received_msg_len);
  request[proc_data->received_msg_len] = '\0';

  // Finds parameters positions
  search_path_res         = STR_SEARCH( request, PARAM_PATH, HEADER_LEN);
  search_length_res       = STR_SEARCH( request, PARAM_LENGTH, HEADER_LEN);
  search_content_res      = STR_SEARCH( proc_data->received_message, PARAM_CONTENT, HEADER_LEN);

  if (search_path_res == NOT_FOUND || search_length_res == NOT_FOUND || search_content_res == NOT_FOUND) {
    
    result = RESULT_INVALID_REQUEST;
    goto END_PROCESS_FILE_SEND;
  }

  param_path_len = ( search_length_res - ( HEADER_LEN + strlen(MSG_SEPARATOR) + strlen(PARAM_PATH) ) );

  // Gets name of file to receive
  filename = (char*)malloc(sizeof(char) * param_path_len + 1);  
  memcpy(filename, &request[search_path_res + strlen(PARAM_PATH)], param_path_len);
  filename[param_path_len] = '\0';

  sprintf(l_msg, "A request has been received to receive the file: %s", filename);
  LOGGER(__FUNCTION__, l_msg);
  
  //
  // Gets content length
  //
  content_len = atoi( &( proc_data->received_message[ ( search_length_res + strlen(PARAM_LENGTH) ) ] ) );
  if (content_len == 0) {

    result = RESULT_INVALID_REQUEST;
    goto END_PROCESS_FILE_SEND;
  }

  //
  // Gets content
  //
  content = (char*)malloc(sizeof(char) * content_len + 1);
  memcpy(content, &(proc_data->received_message[ ( search_content_res + strlen(PARAM_CONTENT) ) ]), content_len);
  content[content_len] = '\0';
  
  // Makes a backup copy if file exists
  if (file_exists(filename) == TRUE) {
  
    char new_name[2048];    
    sprintf(new_name, "%s.bkp", filename);
  
    if ( ! file_copy(filename, new_name, TRUE) ) {
    
      sprintf(l_msg, "ERROR: Could not make backup copy of file (%s).", filename);
      LOGGER(__FUNCTION__, l_msg);
    
    }
    
  }
  
  // Prepares directories
  file_get_base_path(filename, destination_dir);
  if (destination_dir != NULL)
  {
    // If destination directory is not root
    if (strcmp(destination_dir, ROOT_DIR) != 0 ) {

      if ( ! file_directory_exists(destination_dir)) {

        if ( ! file_mkdir_parent(destination_dir) ) {

          result = RESULT_COULD_NOT_CREATE_DESTINATION_DIRECTORY;
          goto END_PROCESS_FILE_SEND;
        }
      }
    }
  }
  else {
      
    sprintf(l_msg, "ERROR: The directory specified for the file %s is not valid.", filename);
    LOGGER(__FUNCTION__, l_msg);

    result = RESULT_INVALID_DESTINATION_DIRECTORY;
    goto END_PROCESS_FILE_SEND;
  }

  //
  // Writes base64 to a file, then decodes and unpacks file
  //
  {    
    char b64_filename[2048];
    char gz_filename[2048];
    FILE * f = NULL; 
    
    sprintf(b64_filename, "%s.b64", filename);
    sprintf(gz_filename, "%s.gz", filename);

    f = fopen(b64_filename, "w");
    if (f != NULL) {
      fwrite( content, sizeof(char), content_len, f );
      fclose(f);

      if ( base64_process_file( 'd', b64_filename, gz_filename, content_len ) ) {
    
        if ( gz_unpack_file(gz_filename, filename) ) {
      
          sprintf(l_msg, "file was succesfully unpacked (%s).", gz_filename);
          LOGGER( __FUNCTION__, l_msg );

          result = RESULT_SUCCESS;

        }
        else {

          sprintf(l_msg, "ERROR: could not unpack file (%s).", gz_filename);
          LOGGER( __FUNCTION__, l_msg );

          result = RESULT_FILE_DECOMPRESS_ERROR;
        }      
      
      }
      else {
    
        sprintf(l_msg, "ERROR: could not decode file (%s).", b64_filename);
        LOGGER( __FUNCTION__, l_msg );

        result = RESULT_FILE_DECODE_ERROR;
      }
    
      // Delete generated temporary files
      if ( file_exists(b64_filename) ) {
        remove(b64_filename);
      }
    
      if ( file_exists(gz_filename) ) {
        remove(gz_filename);
      }

    }
    
  }
  
END_PROCESS_FILE_SEND:

  // Generates a response message
  response = message_file_send_response( result, &response_len );  

  // Sends the response
  if ( !process_outgoing_message( proc_data->connection, response, response_len ) ) {
    
    LOGGER(__FUNCTION__, "File Receive operation response message could not be sent.");
  }

  // Cleanup
  if (request != NULL) {
    free(request);
  }
  if (response != NULL) {
    free(response);
  }
  if (filename != NULL) {
    free(filename);
  }
  if (content != NULL) {
    free(content);
  }

  return;
}

/**
 * Processes a File Delete message from the client, 
 * performs and finalizes the operation
 *
 * @param proc_data_arg           data structure with connection parameters 
 *                                and received message
 */
void process_file_delete( PROCESS_DATA_T * proc_data ) {

  const int NOT_FOUND = -1;

  char * filename   = NULL;
  char l_msg[4096];

  char * request    = NULL;
  char * response   = NULL;

  unsigned long response_len  = 0;
  int search_filename_res     = 0;

  int result = RESULT_UNDEFINED;
  const int param_len = (proc_data->received_msg_len - HEADER_LEN - strlen(PARAM_FILENAME));

  // Copies the request to a safe buffer for searching
  request = (char*)malloc(sizeof(char) * proc_data->received_msg_len + 1);
  memcpy(request, proc_data->received_message, proc_data->received_msg_len);
  request[proc_data->received_msg_len] = '\0';

  // Gets name of file to delete
  search_filename_res = STR_SEARCH( proc_data->received_message, PARAM_FILENAME, HEADER_LEN);
  if ( search_filename_res == NOT_FOUND ) {

    result = RESULT_INVALID_REQUEST;
    goto END_PROCESS_FILE_DELETE;
  }

  filename = (char*)malloc(sizeof(char) * proc_data->received_msg_len);
  memcpy( filename, &(request[ ( search_filename_res + strlen(PARAM_FILENAME) ) ]), param_len );
  filename[param_len] = '\0';
  
  sprintf(l_msg, "A request has been received to delete the file: %s", filename);
  LOGGER(__FUNCTION__, l_msg);
  
  if (file_exists(filename) == TRUE) {
  
    if (file_delete(filename) == TRUE) {
    
      result = RESULT_SUCCESS;
    }
    else {
      result = RESULT_FILE_DELETE_ERROR;
    }    
  }
  else {
    result = RESULT_FILE_NOT_FOUND;
  }
    
  // Generates response message
  response = message_file_delete_response( result, &response_len );  

  // Sends response message
  if ( !process_outgoing_message( proc_data->connection, response, response_len ) ) {
    
    LOGGER(__FUNCTION__, "File Receive response message could not be sent.");
  }

END_PROCESS_FILE_DELETE:

  // Cleanup
  if (request != NULL) {
    free(request);
  }
  if (response != NULL) {
    free(response);
  }
  if (filename != NULL) {
    free(filename);
  }

  return;
}

/**
 * Sends a synchronous message through a connected node
 * 
 * @param connection            conexion on which the message will be sent
 * @param outgoing_message      outgoing message
 * @param outgoing_message_len  outgoing message length
 *
 * @return                      TRUE if message could be sent, otherwise FALSE
 */ 
int process_outgoing_message( SOCKET_T * connection, char * outgoing_message, int outgoing_message_len ) {

  int bsent = 0;
  int selectval = 0;  
  
  long total_bytes_sent = 0;
  long total_bytes_left = 0;
  long chunk_size = HEADER_LEN;
  
  int send_error = FALSE;
  int timed_out = FALSE;
  int message_send_success = FALSE;    
  unsigned long exec_timeout = 0;
  char * progress;
  
  // Updates moment for next timeout
  exec_timeout = GetTickCount() + gl_timeout;

  // Send message loop
  while ( selectval != S_WRITE && abort_processes == FALSE ) {

    // If operation timed out cancel
    if (GetTickCount() > exec_timeout) {
      timed_out = TRUE;
      goto END_PROCESS_OUTGOING_MESSAGE;
    }

    // Once the header was sent sends the rest of the message...
    while (total_bytes_sent < outgoing_message_len && abort_processes == FALSE) {

      // If operation timed out cancel
      if (GetTickCount() > exec_timeout) {
        timed_out = TRUE;
        goto END_PROCESS_OUTGOING_MESSAGE;
      }

      // ...but first it sends only the header
      while (total_bytes_sent < HEADER_LEN && abort_processes == FALSE) {

        // If operation timed out cancel
        if (GetTickCount() > exec_timeout) {
          timed_out = TRUE;
          goto END_PROCESS_OUTGOING_MESSAGE;
        }

        selectval = SOCKET_SELECT(S_TIMEOUT, connection, S_WRITE);
          
        if ( selectval == S_WRITE ) {

          // Attempts to send the message
          if ( ! SOCKET_SEND(connection, outgoing_message, chunk_size, &bsent) ) {

            // Produces error on fail
            send_error = TRUE;
            break; 
          }

          total_bytes_sent += bsent;
        }

      }

      // If message is only a header then it is complete
      if (total_bytes_sent == outgoing_message_len) {
        break;
      }

      progress = &outgoing_message[total_bytes_sent];

      // Updates bytes left to send count
      total_bytes_left = outgoing_message_len - total_bytes_sent;
      if (total_bytes_left > CHUNK_SIZE) {
        chunk_size = CHUNK_SIZE;
      }
      else {
        chunk_size = total_bytes_left;
      }

      selectval = SOCKET_SELECT(S_TIMEOUT, connection, S_WRITE);
          
      if ( selectval == S_WRITE ) {

        // Attempts to send the message following the header
        if ( ! SOCKET_SEND(connection, progress, chunk_size, &bsent) ) {

          // Produces error on fail
          send_error = TRUE;
          break; 
        }

        total_bytes_sent += bsent;
      }

    }

    // If bytes sent has reached total message size
    if ( total_bytes_sent == outgoing_message_len ) {
        
      message_send_success = TRUE;
      break;
    }

  }
  
END_PROCESS_OUTGOING_MESSAGE:

  // If an error occurred or operation timed out
  if ( send_error == TRUE || timed_out) {
  
    char log[_BUFFER_SIZE_L];
    memset(log, 0x00, _BUFFER_SIZE_L);

    strcpy( log, "Failed while attempting to send the following message: ");
    if (outgoing_message_len >= _BUFFER_SIZE_L) {
      memcpy(log, outgoing_message, _BUFFER_SIZE_L - 1);
    }
    else {
      memcpy(log, outgoing_message, outgoing_message_len);
    }
    LOGGER(__FUNCTION__, log);
    
    sprintf( log, "[Number of bytes sent:%ld]", total_bytes_sent );
    LOGGER(__FUNCTION__, log);
  }

  return message_send_success;
    
}
