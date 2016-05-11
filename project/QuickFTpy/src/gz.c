/**
 * gz.c
 *
 * $Id: $
 * $Source: $
 * $Revision: $
 * $Date: $
 * $Author: $
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>

#include "gz.h"
#include "macros.h"
#include "logger.h"

#include <zlib.h>
#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif

#define CHUNK 16384

/**
 * gzip library wrapper
 *
 * @param in          file to unpack
 * @param out         output file
 *
 * @return            TRUE or FALSE
 */
int gz_unpack_file_ex(gzFile in, FILE * out) {

  char	buffer[1<<14];
  int		length;

  for(;;)
  {
    length = gzread( in, buffer, sizeof(buffer) );
    if ( length < 0 ) return FALSE;
    if ( length == 0 ) break;
    if ( (int)fwrite( buffer, 1, (unsigned)length, out ) != length ) return FALSE;
  }

  if( fclose(out) ) return FALSE;
  if( gzclose(in) != Z_OK ) return FALSE;
  
  return TRUE;

}

/**
 * gzip library wrapper
 *
 * @param in        file to pack
 * @param out       output gz file
 *
 * @return          TRUE or FALSE
 */
int gz_pack_file_ex(FILE *in, gzFile out) {

    unsigned char buf[4096];
    int len;

    for (;;) {
        len = fread( buf, 1, sizeof(buf), in );
        if ( ferror(in) ) {
          return FALSE;
        }
        if (len == 0) break;
        if ( gzwrite( out, buf, (unsigned)len ) != len ) {
          return FALSE;
        }
    }
    fclose(in);
    
    if ( gzclose(out) != Z_OK ) {
      return FALSE;
    }
    return TRUE;

}

/**
 * Unpacks a gzip file
 *
 * @param path            file to unpack path
 * @param output_path     output file path
 *
 * @return                TRUE or FALSE
 */
int gz_unpack_file(char* path, char* output_path) {

  gzFile inFile = NULL;
  FILE* outFile = NULL;
  char buffer[1024];
  int ret = FALSE;

  if ((inFile  = gzopen(path, "rb")) == NULL) {
    sprintf(buffer, "WARNING: cannot open file (%s) for unpacking.", path);
    LOGGER(__FUNCTION__, buffer);
    goto GZUNPACK_END;
  }

  if( (outFile = fopen(output_path, "wb")) == NULL) {
    sprintf(buffer, "WARNING: cannot open output file (%s).", output_path);
    LOGGER(__FUNCTION__, buffer);
    goto GZUNPACK_END;
  }

  if (gz_unpack_file_ex(inFile, outFile) == FALSE) {
    sprintf(buffer, "ERROR: cannot perform unpack operation (%s)", path);
    LOGGER(__FUNCTION__, buffer);
    goto GZUNPACK_END;
  }

  outFile = 0;
  ret = TRUE;

GZUNPACK_END:

  if (outFile) {
    fclose(outFile);
  }

  return ret;

}

/**
 * Packs a file to a gzip file
 *
 * @param path            file to be compressed path
 * @param output_path     compressed file path or NULL
 *
 * @return                TRUE or FALSE
 */
int gz_pack_file(char* path, char* output_path) {

  FILE* inFile = NULL;
  gzFile outFile = NULL;
  char destination_path[1024];
  char buffer[1024];
  int ret = FALSE;

  if ((inFile = fopen (path, "rb")) == NULL) {
    sprintf(buffer, "WARNING: cannot open file (%s) for packing.", path);
    LOGGER(__FUNCTION__, buffer);
    goto GZPACK_END;
  }

  if (output_path == NULL || strlen(output_path) == 0) {
    strcpy(destination_path, path);
    strcat(destination_path, ".gz" );
  } else {
    strcpy(destination_path, output_path);
  }

  if ((outFile = gzopen(destination_path, "wb")) == NULL) {
    sprintf(buffer, "WARNING: cannot open destination gzip file (%s).", path);
    LOGGER(__FUNCTION__, buffer);
    goto GZPACK_END;
  }

  if (gz_pack_file_ex(inFile, outFile) == FALSE) {
    LOGGER(__FUNCTION__, "ERROR: cannot perform pack operation");
    goto GZPACK_END;
  }
  
  inFile = 0;
  ret = TRUE;

GZPACK_END:

  if (inFile) {
	  fclose( inFile );
  }

  return ret;

}

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
int gz_pack_string(unsigned char **out, size_t *out_len, const char *in, size_t in_len ) {

  struct z_stream_s *stream = NULL;
  size_t out_size;
  off_t offset;
  char buffer[1024];

  if ( out && out_len && in) {

    *out = NULL;

    stream = calloc(1,sizeof(struct z_stream_s));
    stream->zalloc = Z_NULL;
    stream->zfree = Z_NULL;
    stream->opaque = NULL;
    stream->next_in = (unsigned char*) in;
    stream->avail_in = in_len;

    // Initializes compression structure (add 16 to MAX_WBITS to enforce gzip format)
    if ( deflateInit2( stream, Z_BEST_COMPRESSION, Z_DEFLATED, MAX_WBITS + 16, 8, Z_DEFAULT_STRATEGY) != Z_OK ) {
      sprintf(buffer, "ERROR: in deflateInit2 (%s)", stream->msg?stream->msg:"<no message>");
      LOGGER(__FUNCTION__, buffer);
      goto err;
    }

    // Guess 50% compression
    out_size = in_len / 2;
    if (out_size < 1024) out_size = 1024;
    *out = calloc(out_size,sizeof(char));
    stream->next_out = *out;
    stream->avail_out = out_size;

    while (1) {
      switch (deflate(stream, Z_FINISH))
      {
        case Z_STREAM_END:
          goto done;
        case Z_OK:
          if (stream->avail_out >= stream->avail_in+16)
          break;
        case Z_BUF_ERROR:
          offset = stream->next_out - ((unsigned char*)*out);
          out_size *= 2;
          *out = realloc(*out, out_size);
          stream->next_out = *out + offset;
          stream->avail_out = out_size - offset;
          break;
        default:
          sprintf(buffer, "ERROR: compression didn't finish (%s)", stream->msg ? stream->msg : "<no message>");
          LOGGER(__FUNCTION__, buffer);
          goto err;
      }
    }

    done:
      *out_len = stream->total_out;
      (*out)[*out_len] = '\0';
      if (deflateEnd(stream)!=Z_OK) {
        sprintf(buffer, "ERROR: freeing gzip structures" );
        LOGGER(__FUNCTION__, buffer);
        goto err;
      }
      free(stream);
      return TRUE;
  
    err:
      if (stream) {
        deflateEnd(stream);
        free(stream);
      }
      if (*out) {
        free(*out);
      }
      return FALSE;

  }
  
  return FALSE;

}

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
int gz_unpack_string(unsigned char **out, size_t *out_len, const unsigned char *in, size_t in_len) {

  struct z_stream_s *stream = NULL;
  size_t out_size;
  off_t offset;
  char buffer[1024];

  if ( out && out_len && in) {

    *out = NULL;

    stream = calloc(1,sizeof(struct z_stream_s));
    stream->zalloc = Z_NULL;
    stream->zfree = Z_NULL;
    stream->opaque = NULL;
    stream->next_in = (unsigned char*) in;
    stream->avail_in = in_len;

    // Initializes compression structure (add 16 to MAX_WBITS to enforce gzip format)
    if ( inflateInit2( stream, MAX_WBITS + 16 ) != Z_OK ) {
      sprintf(buffer, "ERROR: in inflateInit2 (%s)", stream->msg?stream->msg:"<no message>");
      LOGGER(__FUNCTION__, buffer);
      goto err;
    }

    // Guess 50% compression
    out_size = in_len * 2;
    if (out_size < 1024) out_size = 1024;

    *out = calloc(out_size,sizeof(char));
    stream->next_out = *out;
    stream->avail_out = out_size;

    while (1) {
      switch(inflate(stream, Z_FINISH))
      {
        case Z_STREAM_END:
          goto done;
        case Z_OK:
          if (stream->avail_out >= stream->avail_in+16)
          break;
        case Z_BUF_ERROR:
          offset = stream->next_out - ((unsigned char*)*out);
          out_size *= 2;
          *out = realloc(*out, out_size);
          stream->next_out = *out + offset;
          stream->avail_out = out_size - offset;
          break;
        default:
          sprintf(buffer, "ERROR: decompression returned an error (%s)", stream->msg ? stream->msg : "<no message>");
          LOGGER(__FUNCTION__, buffer);
          goto err;
      }
    }

    done:
      *out_len = stream->total_out;
      (*out)[*out_len] = '\0';
      if (inflateEnd(stream)!=Z_OK) {
        sprintf(buffer, "ERROR: freeing gzip structures");
        LOGGER(__FUNCTION__, buffer);
        goto err;
      }
      free(stream);
      return TRUE;

    err:
      if (stream) {
        inflateEnd(stream);
        free(stream);
      }
      if (*out) {
        free(*out);
      }
      return FALSE;
  
  }
  
  return FALSE;

}
