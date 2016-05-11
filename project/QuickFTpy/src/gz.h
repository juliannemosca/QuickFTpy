/**
 * gz.h
 *
 * $Id: $
 * $Source: $
 * $Revision: $
 * $Date: $
 * $Author: $
 *
 */

#ifndef _GZ_H
#define _GZ_H

#ifdef __cplusplus
extern "C" {
#endif

#include <zlib.h>

/**
 * gzip library wrapper
 *
 * @param in          file to unpack
 * @param out         output file
 *
 * @return            TRUE or FALSE
 */
int gz_unpack_file_ex(gzFile in, FILE * out);

/**
 * gzip library wrapper
 *
 * @param in        file to pack
 * @param out       output gz file
 *
 * @return          TRUE or FALSE
 */
int gz_pack_file_ex(FILE *in, gzFile out);

/**
 * Unpacks a gzip file
 *
 * @param path            file to unpack path
 * @param output_path     output file path
 *
 * @return                TRUE or FALSE
 */
int gz_unpack_file(char* path, char* output_path);

/**
 * Packs a file to a gzip file
 *
 * @param path            file to be compressed path
 * @param output_path     compressed file path or NULL
 *
 * @return                TRUE or FALSE
 */
int gz_pack_file(char* path, char* output_path);

/*
 * Packs a string in memory
 *
 * @param out             Pointer by reference to compressed output
 * @param out_len         Pointer by reference to compressed output length
 * @param in              Input string to pack
 * @param in_len          Input string length
 *
 * @return                TRUE or FALSE
 */
int gz_pack_string(unsigned char** out, size_t* out_len, const char* in, size_t in_len);

/*
 * Unpacks a string in memory
 *
 * @param out             Pointer by reference to compressed output
 * @param out_len         Pointer by reference to compressed output length
 * @param in              Input string to unpack
 * @param in_len          Input string length
 *
 * @return                TRUE or FALSE
 */
int gz_unpack_string(unsigned char** out, size_t* out_len, const unsigned char* in, size_t in_len);

#ifdef __cplusplus
}
#endif

#endif  // _GZ_H
