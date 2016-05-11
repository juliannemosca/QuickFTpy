/*
 * message.h
 *
 * $Id: $
 * $HeadURL: $
 * $LastChangedRevision: $
 * $LastChangedDate: $
 * $LastChangedBy: $
 *
 */

#ifndef MESSAGE_H
#define MESSAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>

// Defines protocol name
#define PCOL_NAME             "QUIFT_MSG"

// Defines protocol version
#define VERSION               "V1.0"

// Defines message codes for headers
#define FILE_SEND             "FILE_SND"
#define FILE_RECEIVE          "FILE_RCV"
#define FILE_DELETE           "FILE_DEL"

// Defines length of fields
#define PCOL_NAME_LEN         9
#define VERSION_LEN           4
#define HEADER_LEN            32
#define SIZE_LEN              8
#define MSG_TYPE_LEN          8
#define VAR_PART_MINIMUM_LEN  256
#define RESULT_VALUE_LEN      19

// Defines length of chunks for send/receive loops
#define CHUNK_SIZE    1024

// Defines an ACK message
#define MESSAGE_ACK    "QUIFT_MSG=V1.0=ACK_____=00000000"

// Defines binary values for message codes
#define FILE_SND_B    0x01
#define FILE_RCV_B    0x02
#define FILE_DEL_B    0x04

// Defines parameter names
#define PARAM_PATH      "=path:"
#define PARAM_LENGTH    "=length:"
#define PARAM_CONTENT   "=content:"
#define PARAM_FILENAME  "=filename:"
#define PARAM_RESULT    "=result:"

// Defines message separator between fixed-part and variable-part
#define MSG_SEPARATOR ":"

// Macro for accesing function
#define IS_VALID_HEADER       message_is_valid_header

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
char * message_file_receive_request( int len, char * filename, unsigned long * msg_len ) ;

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
char * message_file_receive_response( int result_code, unsigned long len, char * content, unsigned long * msg_len );

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
char * message_file_send_request( char * path, unsigned long len, char * content, unsigned long * msg_len );

/**
 * Generates a File Send response message
 *
 * @param result_code         operation result code
 * @param msg_len             output parameter returns generated message length
 *
 * @return                    generated message, NOT terminated with NULL,
 *                            must be free()d after usage
 */
char * message_file_send_response( int result_code, unsigned long * msg_len );

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
char * message_file_delete_request( int len, char * filename, unsigned long * msg_len );

/**
 * Generates a File Delete response message
 *
 * @param result_code         operation result code
 * @param msg_len             output parameter returns generated message length
 *
 * @return                    generated message, NOT terminated with NULL,
 *                            must be free()d after usage
 */
char * message_file_delete_response( int result_code, unsigned long * msg_len );

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
int message_is_valid_header ( char * header, long * var_part_size, unsigned long type );

/**
 * Returns the corresponding string for a given result code
 *
 * @param result_code       result code
 * @param buffer            buffer for copying the string
 *
 * @return                  result string
 */
char * message_result_code_to_string(int result_code, char * result_string);

/**
 * Returns the corresponding result code from a result string
 *
 * @param                   result string
 * @return                  result code
 */
int message_result_string_to_code(char * result_string);

#endif // MESSAGE_H