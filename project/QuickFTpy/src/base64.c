/*
 * base64.c
 *
 * $Id: $
 * $HeadURL: $
 * $LastChangedRevision: $
 * $LastChangedDate: $
 * $LastChangedBy: $
 *
 */

#include "base64.h"
#include "macros.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define base64_blocks(l) (((l) + 2) / 3 * 4 + 1)
#define base64_octets(l)  ((l) / 4  * 3 + 1)

// Translation table as described in RFC1113
static const char cb64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// Translation table for decoding
static const char cd64[] = "|$$$}rstuvwxyz{$$$$$$$>?@ABCDEFGHIJKLMNOPQRSTUVW$$$$$$XYZ[\\]^_`abcdefghijklmnopq";

/**
 * Encodes three 8-bit binary characters as four 6-bit characters
 *
 * @param input_block            input characters
 * @param output_block           output characters
 * @param size                   input characters length
 *
 */
void base64_encode_block(unsigned char* in, unsigned char* out, int len) {
  out[0] = (unsigned char)cb64[(int)(in[0] >> 2)];
  out[1] = (unsigned char)cb64[(int)(((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4))];
  out[2] = (unsigned char)(len > 1 ? cb64[(int)(((in[1] & 0x0f) << 2) | ((in[2] & 0xc0) >> 6))] : '=');
  out[3] = (unsigned char)(len > 2 ? cb64[(int)(in[2] & 0x3f)] : '=');
}

/**
 * Decodes four 6-bit characters in three 8-bit binary characters
 *
 * @param input_block            input characters
 * @param output_block           output characters
 *
 */
void base64_decode_block(unsigned char *in, unsigned char *out) {
  out[0] = (unsigned char)(in[0] << 2 | in[1] >> 4);
  out[1] = (unsigned char)(in[1] << 4 | in[2] >> 2);
  out[2] = (unsigned char)(((in[2] << 6) & 0xc0) | in[3]);
}

/**
 * Calculates size of decode operation output
 *
 * @param size                   input size
 *
 * @return                       output size
 *
 */
int base64_decode_size(int size) {

  if (size == 0) return 0;
  return base64_octets(size);

}

/**
 * Calculates size of encode operation output
 *
 * @param size                   input size
 *
 * @return                       output size
 *
 */
int base64_encode_size(int size) {

  if (size == 0) return 0;
  return base64_blocks(size);

}

/**
 * Allocates necessary memory for an encode/decode operation's buffer
 *
 * @param size                   lnegth of string to encode/decode
 * @param operation_type         type of operation to perform ( e || d )
 *
 * @return                       buffer allocated in memory
 *
 */
char* base64_buffer(int size, int operation_type) {

  char* buffer;

  // Checks for valid string size
  if (!size) {
    buffer = NULL;
  }
  else {

    // Allocates space in memory 
    if (operation_type == 'e') {
      buffer = (char*)calloc(base64_blocks(size), sizeof(char));
    }
    else if (operation_type == 'd') {
      buffer = (char*)calloc(base64_octets(size), sizeof(char));
    }
    else {
      buffer = NULL;
    }

  }

  return buffer;

}

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
unsigned char* base64_encode(unsigned char* data, int data_size) {

  unsigned char* encoded = NULL;
  unsigned char in[3];
  unsigned char out[4];
  int d, i, len, blocksout = 0;

  *in = (unsigned char)0;
  *out = (unsigned char)0;

  encoded = (unsigned char*)calloc(base64_blocks(data_size), sizeof(char));
  if (encoded == NULL) return NULL;

  for (d = 0; d < data_size; d += 3) {

    len = 0;

    for (i = 0; i < 3; i++) {

      in[i] = data[d + i];

      if (d < data_size) {
        len++;
      }
      else {
        in[i] = (unsigned char)0;
      }

    }

    if (len > 0) {

      base64_encode_block(in, out, len);

      for (i = 0; i < 4; i++) {
        encoded[(blocksout * 4) + i] = (int)(out[i]);
      }

      blocksout++;

    }

  }

  return encoded;

}

/**
 * Decodes a Base64 encoded string. Discards padding and newline characters.
 *
 * @param string_encoded         encoded input string
 * @param string_encoded_size    encoded input string size
 *
 * @return                       decoded output string,
 *                               must call free() after usage.
 *
 */
char* base64_decode(char * string_encoded, int string_encoded_size) {

  char* string;
  unsigned char input_block[4];
  unsigned char output_block[3];

  unsigned char v;
  int i;
  int size;

  string = base64_buffer(string_encoded_size, 'd');
  if (string != NULL) {

    char * s = string_encoded;
    char * d = string;

    while (*s != '\0') {

      for (size = 0, i = 0; (i < 4) && (*s != '\0'); i++) {

        v = 0;

        while ((*s != '\0') && (v == 0)) {
          v = (unsigned char)*s++;
          v = (unsigned char)((v < 43 || v > 122) ? 0 : cd64[v - 43]);
          if (v) {
            v = (unsigned char)((v == '$') ? 0 : (v - 61));
          }
        }

        if (*s != '\0' || v != 0) {
          size++;
          if (v) {
            input_block[i] = (unsigned char)(v - 1);
          }
        }
        else {
          input_block[i] = 0;
        }

      }

      if (size) {
        base64_decode_block(input_block, output_block);
        for (i = 0; i < size - 1; i++) {
          *d++ = output_block[i];
        }
      }

    }

  }

  return string;

}

/**
 * Encodes a file in base64
 *
 * @param in_file_handler        input file stream
 * @param out_file_handler       output file stream
 * @param line_size              line length
 *
 */
void base64_encode_file(FILE *in_file_handler, FILE *out_file_handler, int line_size) {

  unsigned char input_block[3];
  unsigned char output_block[4];
  int i;
  int size;
  int blocksout = 0;

  while (!feof(in_file_handler)) {

    size = 0;

    for (i = 0; i < 3; i++) {

      input_block[i] = (unsigned char)getc(in_file_handler);

      if (!feof(in_file_handler)) {
        size++;
      }
      else {
        input_block[i] = 0;
      }

    }

    if (size) {

      base64_encode_block(input_block, output_block, size);

      for (i = 0; i < 4; i++) {
        putc(output_block[i], out_file_handler);
      }

      blocksout++;

    }

    if (blocksout >= (line_size / 4) || feof(in_file_handler)) {

      if (blocksout) {
        fprintf(out_file_handler, "\r\n");
      }

      blocksout = 0;

    }

  }

}

/**
 * Decodes a base64-encoded file. Discards padding and newline characters.
 *
 * @param in_file_handler        file stream de entrada
 * @param out_file_handler       file stream de salida
 *
 */
void base64_decode_file(FILE *in_file_handler, FILE *out_file_handler) {

  unsigned char input_block[4];
  unsigned char output_block[3];
  unsigned char v;
  int i;
  int size;

  while (!feof(in_file_handler)) {

    for (size = 0, i = 0; i < 4 && !feof(in_file_handler); i++) {

      v = 0;

      while (!feof(in_file_handler) && v == 0) {
        v = (unsigned char)getc(in_file_handler);
        v = (unsigned char)((v < 43 || v > 122) ? 0 : cd64[v - 43]);
        if (v) {
          v = (unsigned char)((v == '$') ? 0 : v - 61);
        }
      }

      if (!feof(in_file_handler)) {
        size++;
        if (v) {
          input_block[i] = (unsigned char)(v - 1);
        }
      }
      else {
        input_block[i] = 0;
      }

    }

    if (size) {

      base64_decode_block(input_block, output_block);


      for (i = 0; i < size - 1; i++) {
        putc(output_block[i], out_file_handler);
      }

    }

  }

}

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
int base64_process_file(int operation_type, char *input_path, char *output_path, int line_size) {

  FILE* in_file_handler;
  FILE* out_file_handler;
  int bSuccess = FALSE;

  if (input_path) {

    in_file_handler = fopen(input_path, "rb");
    if (in_file_handler) {

      if (output_path) {

        out_file_handler = fopen(output_path, "wb");
        if (out_file_handler) {

          // Encoding operation
          if (operation_type == 'e') {

            // Verifies line size
            if (line_size < BASE64_MIN_LINE_SIZE) {
              line_size = BASE64_MIN_LINE_SIZE;
            }

            // Encodes file
            base64_encode_file(in_file_handler, out_file_handler, line_size);

            // Checks for errors
            if (ferror(in_file_handler) || ferror(out_file_handler)) {
              bSuccess = FALSE;
            }
            else {
              bSuccess = TRUE;
            }

          }
          else if (operation_type == 'd') {

            // Decode
            base64_decode_file(in_file_handler, out_file_handler);

            // Checks for errors
            if (ferror(in_file_handler) || ferror(out_file_handler)) {
              bSuccess = FALSE;
            }
            else {
              bSuccess = TRUE;
            }

          }
          else {
            bSuccess = FALSE;
          }

          // Closes streams
          fclose(in_file_handler);
          if (fclose(out_file_handler) != 0) {
            bSuccess = FALSE;
          }

        }

      }

    }

  } 

  return(bSuccess);

}
