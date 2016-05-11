/*
 * base64.h
 *
 * $Id: $
 * $HeadURL: $
 * $LastChangedRevision: $
 * $LastChangedDate: $
 * $LastChangedBy: $
 *
 */

#ifndef BASE64_H
#define BASE64_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>

#include "memory.h"

#define BASE64_DEF_LINE_SIZE            72
#define BASE64_MIN_LINE_SIZE            4

// Macros
#define BASE64_ENCODE(d, s)             base64_encode(d, s)
#define BASE64_DECODE(d, s)             base64_decode(d, s)
#define BASE64_ENCODE_SIZE(s)           base64_encode_size(s)              
#define BASE64_DECODE_SIZE(s)           base64_decode_size(s)
#define BASE64_ENCODE_FILE(o, i, p, s)  base64_process_file(o, i, p, s)

/**
 * Encodes three 8-bit binary characters as four 6-bit characters
 *
 * @param input_block            input characters
 * @param output_block           output characters
 * @param size                   input characters length
 *
 */
void base64_encode_block(unsigned char* in, unsigned char* out, int len);

/**
 * Decodes four 6-bit characters in three 8-bit binary characters
 *
 * @param input_block            input characters
 * @param output_block           output characters
 *
 */
void base64_decode_block(unsigned char *in, unsigned char *out);

/**
 * Calculates size of decode operation output
 *
 * @param size                   input size
 *
 * @return                       output size
 *
 */
int base64_decode_size(int size);

/**
 * Calculates size of encode operation output
 *
 * @param size                   input size
 *
 * @return                       output size
 *
 */
int base64_encode_size(int size);

/**
 * Allocates necessary memory for an encode/decode operation's buffer
 *
 * @param size                   lnegth of string to encode/decode
 * @param operation_type         type of operation to perform ( e || d )
 *
 * @return                       buffer allocated in memory
 *
 */
char* base64_buffer(int size, int operation_type);

/**
 * Encodes a character array in base64
 *
 * @param data                   input data not encoded
 * @param data_size              output data encoded
 *
 * @return                       output data encoded,
 *                               must call free() after usage.
 *
 */
unsigned char* base64_encode(unsigned char* data, int data_size);

/**
 * Decodes a base64 encoded string. Discards padding and newline characters.
 *
 * @param string_encoded         encoded input string
 * @param string_encoded_size    encoded input string size
 *
 * @return                       decoded output string,
 *                               must call free() after usage.
 *
 */
char* base64_decode(char * string_encoded, int string_encoded_size);

/**
 * Encodes a file in base64
 *
 * @param in_file_handler        input file stream
 * @param out_file_handler       output file stream
 * @param line_size              line length
 *
 */
void base64_encode_file(FILE *in_file_handler, FILE *out_file_handler, int line_size);

/**
 * Decodes a base64-encoded file. Discards padding and newline characters.
 *
 * @param in_file_handler        file stream de entrada
 * @param out_file_handler       file stream de salida
 *
 */
void base64_decode_file(FILE *in_file_handler, FILE *out_file_handler);

/**
 * Performs an encoding/decoding operation on a file
 *
 * @param operation_type         type of operation to perform ( e || d )
 * @param input_path             input file path
 * @param output_path            output file path
 * @param line_size              line length for encoding
 *
 * @return                       TRUE o FALSE
 *
 */
int base64_process_file(int operation_type, char *input_path, char *output_path, int line_size);

#ifdef __cplusplus
}
#endif

#endif // BASE64_H

