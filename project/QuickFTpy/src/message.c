/*
 * message.c
 *
 * $Id: $
 * $HeadURL: $
 * $LastChangedRevision: $
 * $LastChangedDate: $
 * $LastChangedBy: $
 *
 */

#include "message.h"
#include "results.h"
#include "string.h"

// Defines the valid message routine
#define VALID_MESSAGE_ROUTINE   memcpy( var_part_size_buf, &header[PCOL_NAME_LEN+1+VERSION_LEN+1+MSG_TYPE_LEN+1], SIZE_LEN );\
                                var_part_size_buf[SIZE_LEN] = '\0';\
                                *var_part_size = strtol(var_part_size_buf, NULL, 16);

// Swap routine
#define swap(x,y) do\
                  { unsigned char swap_temp[sizeof(x) == sizeof(y) ? (signed)sizeof(x) : -1];\
                    memcpy(swap_temp,&y,sizeof(x));\
                    memcpy(&y,&x,       sizeof(x));\
                    memcpy(&x,swap_temp,sizeof(x));\
                  } while(0)

/**
 * Implementation of _itoa for compatibility with Win32
 */
char * _itoa(int num, char* str, int base) {

  int i = 0;
  int is_negative = FALSE;
 
  /* Handle 0 explicitely, otherwise empty string is printed for 0 */
  if (num == 0)
  {
      str[i++] = '0';
      str[i] = '\0';
      return str;
  }
 
  // In standard itoa(), negative numbers are handled only with 
  // base 10. Otherwise numbers are considered unsigned.
  if (num < 0 && base == 10)
  {
      is_negative = TRUE;
      num = -num;
  }
 
  // Process individual digits
  while (num != 0)
  {
      int rem = num % base;
      str[i++] = (rem > 9)? (rem-10) + 'a' : rem + '0';
      num = num/base;
  }
 
  // If number is negative, append '-'
  if (is_negative)
      str[i++] = '-';
 
  str[i] = '\0'; // Append string terminator
 
  // Reverse the string
  {
    int length = i;
    int start = 0;
    int end = length -1;
    while (start < end)
    {
      swap(*(str+start), *(str+end));
      start++;
      end--;
    }
  }
 
  return str;  
}

/**
 * Generates a File Receive request message
 *
 * @param len                 filename length
 * @param filename            message content
 * @param msg_len             output parameter returns generated message length
 *
 * @return                    generated message, NOT terminated with NULL,
 *                            must be free()d after usage
 */
char * message_file_receive_request( int len, char * filename, unsigned long * msg_len ) {

  char * msg;
  char header[HEADER_LEN];
  char * var_part;
  char size[SIZE_LEN+1];
  char size_len[SIZE_LEN+1];

  int index = 0;  

  memset(header, 0x00, HEADER_LEN);
  var_part = malloc( len + VAR_PART_MINIMUM_LEN );
  memset(var_part, 0x00, VAR_PART_MINIMUM_LEN);
  memset(size, 0x00, (SIZE_LEN + 1) );
  *msg_len = 0;

  // Protocol Name
  memcpy(header, PCOL_NAME, PCOL_NAME_LEN);
  header[index += PCOL_NAME_LEN] = '=';
  
  // Protocol Version
  memcpy(&header[++index], VERSION, VERSION_LEN);
  header[index += VERSION_LEN] = '=';
  
  // Message Code
  memcpy(&header[++index], FILE_RECEIVE, MSG_TYPE_LEN);
  header[index += MSG_TYPE_LEN] = '=';

  // Builds variable part parameters
  // Adds filename param
  sprintf(var_part, "%s=filename:%s", MSG_SEPARATOR, filename);
  
  // Gets var part size in hex.
  _itoa( strlen(var_part), size, 16 );

  // Applies padding to var part size
  string_left_padding( size, SIZE_LEN, '0', size_len);

  // Inserts var part size value in header
  memcpy(&header[++index], size_len, SIZE_LEN);

  // Allocates memory for the message
  *msg_len = HEADER_LEN + strlen(var_part);
  msg = malloc(*msg_len);
  
  // Copies header, separator, and var part of the message
  memcpy( msg, header, HEADER_LEN );
  memcpy( &msg[HEADER_LEN], var_part, (*msg_len - HEADER_LEN) );

  // Frees memory
  free(var_part);

  // Returns message
  return msg;
}

/**
 * Generates a File Receive response message
 * 
 * @param result_code         operation result code, TRUE or FALSE
 * @param len                 content length
 * @param content             message content
 * @param msg_len             output parameter returns generated message length
 *
 * @return                    generated message, NOT terminated with NULL,
 *                            must be free()d after usage
 */
char * message_file_receive_response( int result_code, unsigned long len, char * content, unsigned long * msg_len ) {

  char * msg;
  char header[HEADER_LEN];
  char * var_part;
  char size[SIZE_LEN+1];
  char size_len[SIZE_LEN+1];
  char result_string[RESULT_VALUE_LEN+1];

  int index = 0;  

  memset(header, 0x00, HEADER_LEN);
  var_part = malloc( len + VAR_PART_MINIMUM_LEN );
  memset(var_part, 0x00, VAR_PART_MINIMUM_LEN);
  memset(size, 0x00, (SIZE_LEN + 1) );
  *msg_len = 0;

  // Protocol Name
  memcpy(header, PCOL_NAME, PCOL_NAME_LEN);
  header[index += PCOL_NAME_LEN] = '=';
  
  // Protocol Version
  memcpy(&header[++index], VERSION, VERSION_LEN);
  header[index += VERSION_LEN] = '=';
  
  // Message Code
  memcpy(&header[++index], FILE_RECEIVE, MSG_TYPE_LEN);
  header[index += MSG_TYPE_LEN] = '=';

  // Result
  sprintf(result_string, "%s", message_result_code_to_string(result_code, result_string) );  

  if (result_code == RESULT_SUCCESS) {

    // Builds variable part parameters,
    // adds length and content
    sprintf(var_part, "%s=result:%s=length:%lu=content:%s", MSG_SEPARATOR, result_string, len, content);
  }
  else {
    sprintf(var_part, "%s=result:%s", MSG_SEPARATOR, result_string);
  }  

  // Gets var part size in hex.
  _itoa( strlen(var_part), size, 16 );

  // Applies padding to var part size
  string_left_padding( size, SIZE_LEN, '0', size_len);

  // Inserts var part size value in header
  memcpy(&header[++index], size_len, SIZE_LEN);

  // Allocates memory for the message
  *msg_len = HEADER_LEN + strlen(var_part);
  msg = malloc(*msg_len);
  
  // Copies header, separator, and var part of the message
  memcpy( msg, header, HEADER_LEN );
  memcpy( &msg[HEADER_LEN], var_part, (*msg_len - HEADER_LEN) );

  // Frees memory
  free(var_part);

  // Returns message
  return msg;
}

/**
 * Generates a File Send request message
 * 
 * @param path                filepath in destination
 * @param len                 content length
 * @param content             message content
 * @param msg_len             output parameter returns generated message length
 *
 * @return                    generated message, NOT terminated with NULL,
 *                            must be free()d after usage
 */
char * message_file_send_request( char * path, unsigned long len, char * content, unsigned long * msg_len ) {

  char * msg;
  char header[HEADER_LEN];
  char * var_part;
  char size[SIZE_LEN+1];
  char size_len[SIZE_LEN+1];

  int index = 0;  

  memset(header, 0x00, HEADER_LEN);
  var_part = malloc( len + VAR_PART_MINIMUM_LEN );
  memset(var_part, 0x00, VAR_PART_MINIMUM_LEN);
  memset(size, 0x00, (SIZE_LEN + 1) );
  *msg_len = 0;

  // Protocol Name
  memcpy(header, PCOL_NAME, PCOL_NAME_LEN);
  header[index += PCOL_NAME_LEN] = '=';
  
  // Protocol Version
  memcpy(&header[++index], VERSION, VERSION_LEN);
  header[index += VERSION_LEN] = '=';
  
  // Message Code
  memcpy(&header[++index], FILE_SEND, MSG_TYPE_LEN);
  header[index += MSG_TYPE_LEN] = '=';

  // Builds variable part parameters,
  // ads length and content
  sprintf(var_part, "%s=path:%s=length:%lu=content:%s", MSG_SEPARATOR, path, len, content);  

  // Gets var part size in hex.
  _itoa( strlen(var_part), size, 16 );

  // Applies padding to var part size
  string_left_padding( size, SIZE_LEN, '0', size_len);

  // Inserts var part size value in header
  memcpy(&header[++index], size_len, SIZE_LEN);

  // Allocates memory for the message
  *msg_len = HEADER_LEN + strlen(var_part);
  msg = malloc(*msg_len);
  
  // Copies header, separator, and var part of the message
  memcpy( msg, header, HEADER_LEN );
  memcpy( &msg[HEADER_LEN], var_part, (*msg_len - HEADER_LEN) );

  // Frees memory
  free(var_part);

  // Returns message
  return msg;
}

/**
 * Generates a File Send response message
 *
 * @param result_code         operation result code
 * @param msg_len             output parameter returns generated message length
 *
 * @return                    generated message, NOT terminated with NULL,
 *                            must be free()d after usage
 */
char * message_file_send_response( int result_code, unsigned long * msg_len ) {

  char * msg;
  char header[HEADER_LEN];
  char * var_part;
  char size[SIZE_LEN+1];
  char size_len[SIZE_LEN+1];
  char result_string[RESULT_VALUE_LEN+1];

  int index = 0;  

  memset(header, 0x00, HEADER_LEN);
  var_part = malloc( VAR_PART_MINIMUM_LEN );
  memset(var_part, 0x00, VAR_PART_MINIMUM_LEN);
  memset(size, 0x00, (SIZE_LEN + 1) );
  *msg_len = 0;

  // Protocol Name
  memcpy(header, PCOL_NAME, PCOL_NAME_LEN);
  header[index += PCOL_NAME_LEN] = '=';
  
  // Protocol Version
  memcpy(&header[++index], VERSION, VERSION_LEN);
  header[index += VERSION_LEN] = '=';
  
  // Message Code
  memcpy(&header[++index], FILE_SEND, MSG_TYPE_LEN);
  header[index += MSG_TYPE_LEN] = '=';

  // Result
  sprintf(result_string, "%s", message_result_code_to_string(result_code, result_string) );  
  sprintf(var_part, "%s=result:%s", MSG_SEPARATOR, result_string); 

  // Gets var part size in hex.
  _itoa( strlen(var_part), size, 16 );

  // Applies padding to var part size
  string_left_padding( size, SIZE_LEN, '0', size_len);

  // Inserts var part size value in header
  memcpy(&header[++index], size_len, SIZE_LEN);

  // Allocates memory for the message
  *msg_len = HEADER_LEN + strlen(var_part);
  msg = malloc(*msg_len);
  
  // Copies header, separator, and var part of the message
  memcpy( msg, header, HEADER_LEN );
  memcpy( &msg[HEADER_LEN], var_part, (*msg_len - HEADER_LEN) );

  // Frees memory
  free(var_part);

  // Returns message
  return msg;
}

/**
 * Generates a File Delete request message
 *
 * @param len                 filename length
 * @param filename            message content
 * @param msg_len             output parameter returns generated message length
 *
 * @return                    generated message, NOT terminated with NULL,
 *                            must be free()d after usage
 */
char * message_file_delete_request( int len, char * filename, unsigned long * msg_len ) {

  char * msg;
  char header[HEADER_LEN];
  char * var_part;
  char size[SIZE_LEN+1];
  char size_len[SIZE_LEN+1];

  int index = 0;  

  memset(header, 0x00, HEADER_LEN);
  var_part = malloc( len + VAR_PART_MINIMUM_LEN );
  memset(var_part, 0x00, VAR_PART_MINIMUM_LEN);
  memset(size, 0x00, (SIZE_LEN + 1) );
  *msg_len = 0;

  // Protocol Name
  memcpy(header, PCOL_NAME, PCOL_NAME_LEN);
  header[index += PCOL_NAME_LEN] = '=';
  
  // Protocol Version
  memcpy(&header[++index], VERSION, VERSION_LEN);
  header[index += VERSION_LEN] = '=';
  
  // Message Code
  memcpy(&header[++index], FILE_DELETE, MSG_TYPE_LEN);
  header[index += MSG_TYPE_LEN] = '=';

  // Builds variable part parameters,
  // adds filename
  sprintf(var_part, "%s=filename:%s", MSG_SEPARATOR, filename);
  
  // Gets var part size in hex.
  _itoa( strlen(var_part), size, 16 );

  // Applies padding to var part size
  string_left_padding( size, SIZE_LEN, '0', size_len);

  // Inserts var part size value in header
  memcpy(&header[++index], size_len, SIZE_LEN);

  // Allocates memory for the message
  *msg_len = HEADER_LEN + strlen(var_part);
  msg = malloc(*msg_len);
  
  // Copies header, separator, and var part of the message
  memcpy( msg, header, HEADER_LEN );
  memcpy( &msg[HEADER_LEN], var_part, (*msg_len - HEADER_LEN) );

  // Frees memory
  free(var_part);

  // Returns message
  return msg;
}

/**
 * Generates a File Delete response message
 *
 * @param result_code         operation result code
 * @param msg_len             output parameter returns generated message length
 *
 * @return                    generated message, NOT terminated with NULL,
 *                            must be free()d after usage
 */
char * message_file_delete_response( int result_code, unsigned long * msg_len ) {

  char * msg;
  char header[HEADER_LEN];
  char * var_part;
  char size[SIZE_LEN+1];
  char size_len[SIZE_LEN+1];
  char result_string[RESULT_VALUE_LEN+1];

  int index = 0;  

  memset(header, 0x00, HEADER_LEN);
  var_part = malloc( VAR_PART_MINIMUM_LEN );
  memset(var_part, 0x00, VAR_PART_MINIMUM_LEN);
  memset(size, 0x00, (SIZE_LEN + 1) );
  *msg_len = 0;

  // Protocol Name
  memcpy(header, PCOL_NAME, PCOL_NAME_LEN);
  header[index += PCOL_NAME_LEN] = '=';
  
  // Protocol Version
  memcpy(&header[++index], VERSION, VERSION_LEN);
  header[index += VERSION_LEN] = '=';
  
  // Message Code
  memcpy(&header[++index], FILE_DELETE, MSG_TYPE_LEN);
  header[index += MSG_TYPE_LEN] = '=';

  // Result
  sprintf(result_string, "%s", message_result_code_to_string(result_code, result_string) );  
  sprintf(var_part, "%s=result:%s", MSG_SEPARATOR, result_string);

  // Gets var part size in hex.
  _itoa( strlen(var_part), size, 16 );

  // Applies padding to var part size
  string_left_padding( size, SIZE_LEN, '0', size_len);

  // Inserts var part size value in header
  memcpy(&header[++index], size_len, SIZE_LEN);

  // Allocates memory for the message
  *msg_len = HEADER_LEN + strlen(var_part);
  msg = malloc(*msg_len);
  
  // Copies header, separator, and var part of the message
  memcpy( msg, header, HEADER_LEN );
  memcpy( &msg[HEADER_LEN], var_part, (*msg_len - HEADER_LEN) );

  // Frees memory
  free(var_part);

  // Returns message
  return msg;
}

/**
 * Evaluates if a heaeder is valid, and if it is returns
 * the size of the variable part of the message that follows
 * 
 * @param header            header to evaluate
 * @param var_part_size     returns variable part size
 *                          if the header is valid, otherwise returns 0.
 * @param type              definse a value or binary addition of values for
 *                          message codes
 *   
 * @return                  TRUE if header is valid, otherwise FALSE
 */
int message_is_valid_header ( char * header, long * var_part_size, unsigned long type ) {
  
  char var_part_size_buf[SIZE_LEN+1];
  
  *var_part_size = 0;    
  memset(var_part_size_buf, 0x00, SIZE_LEN+1);
  
  // Checks for valid protocol name and version
  if ( ( memcmp( header, PCOL_NAME, PCOL_NAME_LEN ) == 0 ) &&
       ( memcmp( &header[PCOL_NAME_LEN+1], VERSION, VERSION_LEN) == 0 ) ) {
  
    //
    // According to the type(s) of message(s) expected evaluates
    // the message type field
    //

    if ( type & FILE_SND_B ) {          
      // Checks type field
      if ( memcmp(&header[PCOL_NAME_LEN+1+VERSION_LEN+1], FILE_SEND, MSG_TYPE_LEN) == 0 ) {          
        // Executes the valid message routine and exits function
        VALID_MESSAGE_ROUTINE                        
        return FILE_SND_B;          
      }          
    }
    if ( type & FILE_RCV_B ) {          
      // Checks type field
      if ( memcmp(&header[PCOL_NAME_LEN+1+VERSION_LEN+1], FILE_RECEIVE, MSG_TYPE_LEN) == 0 ) {          
        // Executes the valid message routine and exits function
        VALID_MESSAGE_ROUTINE                        
        return FILE_RCV_B;          
      }          
    }
    if ( type & FILE_DEL_B ) {          
      // Checks type field
      if ( memcmp(&header[PCOL_NAME_LEN+1+VERSION_LEN+1], FILE_DELETE, MSG_TYPE_LEN) == 0 ) {          
        // Executes the valid message routine and exits function
        VALID_MESSAGE_ROUTINE                        
        return FILE_DEL_B;          
      }          
    }        
  }
  
  return FALSE;
      
}

/**
 * Returns the corresponding string for a given result code
 *
 * @param result_code       result code
 * @param buffer            buffer for copying the string
 *
 * @return                  result string
 */
char * message_result_code_to_string(int result_code, char * result_string) {

  sprintf(result_string, "%s", STR_RESULT_UNDEFINED);

  switch (result_code) {

    case RESULT_SUCCESS:

      sprintf(result_string, "%s", STR_RESULT_SUCCESS);
      break;

    case RESULT_INVALID_REQUEST:

      sprintf(result_string, "%s", STR_RESULT_INVALID_REQUEST);
      break;

    case RESULT_INVALID_RESPONSE:

      sprintf(result_string, "%s", STR_RESULT_INVALID_RESPONSE);
      break;

    case RESULT_CONFIG_ERROR:

      sprintf(result_string, "%s", STR_RESULT_CONFIG_ERROR);
      break;

    case RESULT_FILE_ACCESS_ERROR:
    
      sprintf(result_string, "%s", STR_RESULT_FILE_ACCESS_ERROR);
      break;

    case RESULT_FILE_NOT_FOUND:
    
      sprintf(result_string, "%s", STR_RESULT_FILE_NOT_FOUND);
      break;

    case RESULT_FILE_WRITE_ERROR:
    
      sprintf(result_string, "%s", STR_RESULT_FILE_WRITE_ERROR);
      break;

    case RESULT_FILE_READ_ERROR:

      sprintf(result_string, "%s", STR_RESULT_FILE_READ_ERROR);
      break;

    case RESULT_FILE_COMPRESS_ERROR:
    
      sprintf(result_string, "%s", STR_RESULT_FILE_COMPRESS_ERROR);
      break;

    case RESULT_FILE_DECOMPRESS_ERROR:
    
      sprintf(result_string, "%s", STR_RESULT_FILE_DECOMPRESS_ERROR);
      break;

    case RESULT_FILE_ENCODE_ERROR:
    
      sprintf(result_string, "%s", STR_RESULT_FILE_ENCODE_ERROR);
      break;

    case RESULT_FILE_DECODE_ERROR:

      sprintf(result_string, "%s", STR_RESULT_FILE_DECODE_ERROR);
      break;

    case RESULT_FILE_DELETE_ERROR:

      sprintf(result_string, "%s", STR_RESULT_FILE_DELETE_ERROR);
      break;

    case RESULT_INVALID_DESTINATION_DIRECTORY:
    
      sprintf(result_string, "%s", STR_RESULT_INVALID_DESTINATION_DIRECTORY);
      break;

    case RESULT_COULD_NOT_CREATE_DESTINATION_DIRECTORY:

      sprintf(result_string, "%s", STR_RESULT_COULD_NOT_CREATE_DESTINATION_DIRECTORY);
      break;

  }

  return result_string;
}

/**
 * Returns the corresponding result code from a result string
 *
 * @param                   result string
 * @return                  result code
 */
int message_result_string_to_code(char * result_string) {

  int code = RESULT_UNDEFINED;

  if (strcmp(result_string, STR_RESULT_SUCCESS) == 0) {

    code = RESULT_SUCCESS;
  }
  else if (strcmp(result_string, STR_RESULT_CONNECTION_ERROR) == 0) {

    code = RESULT_CONNECTION_ERROR;
  }
  else if (strcmp(result_string, STR_RESULT_INVALID_REQUEST) == 0) {

    code = RESULT_INVALID_REQUEST;
  }
  else if (strcmp(result_string, STR_RESULT_INVALID_RESPONSE) == 0) {

    code = RESULT_INVALID_RESPONSE;
  }
  else if (strcmp(result_string, STR_RESULT_CONFIG_ERROR) == 0) {

    code = RESULT_CONFIG_ERROR;
  }
  else if (strcmp(result_string, STR_RESULT_FILE_ACCESS_ERROR) == 0) {

    code = RESULT_FILE_ACCESS_ERROR;
  }
  else if (strcmp(result_string, STR_RESULT_FILE_NOT_FOUND) == 0) {

    code = RESULT_FILE_NOT_FOUND;
  }
  else if (strcmp(result_string, STR_RESULT_FILE_WRITE_ERROR) == 0) {

    code = RESULT_FILE_WRITE_ERROR;
  }
  else if (strcmp(result_string, STR_RESULT_FILE_READ_ERROR) == 0) {

    code = RESULT_FILE_READ_ERROR;
  }
  else if (strcmp(result_string, STR_RESULT_FILE_COMPRESS_ERROR) == 0) {

    code = RESULT_FILE_COMPRESS_ERROR;
  }
  else if (strcmp(result_string, STR_RESULT_FILE_DECOMPRESS_ERROR) == 0) {

    code = RESULT_FILE_DECOMPRESS_ERROR;
  }
  else if (strcmp(result_string, STR_RESULT_FILE_ENCODE_ERROR) == 0) {

    code = RESULT_FILE_ENCODE_ERROR;
  }
  else if (strcmp(result_string, STR_RESULT_FILE_DECODE_ERROR) == 0) {

    code = RESULT_FILE_DECODE_ERROR;
  }
  else if (strcmp(result_string, STR_RESULT_FILE_DELETE_ERROR) == 0) {

    code = RESULT_FILE_DELETE_ERROR;
  }
  else if (strcmp(result_string, STR_RESULT_INVALID_DESTINATION_DIRECTORY) == 0) {

    code = RESULT_INVALID_DESTINATION_DIRECTORY;
  }
  else if (strcmp(result_string, STR_RESULT_COULD_NOT_CREATE_DESTINATION_DIRECTORY) == 0) {

    code = RESULT_COULD_NOT_CREATE_DESTINATION_DIRECTORY;
  }
  
  return code;
}