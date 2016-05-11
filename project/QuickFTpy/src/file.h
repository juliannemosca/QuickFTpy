/*
 * file.h
 *
 * $Id: $
 * $HeadURL: $
 * $LastChangedRevision: $
 * $LastChangedDate: $
 * $LastChangedBy: $
 *
 */

#ifndef _FILE_H
#define _FILE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

/**
 * Checks and returns TRUE if file exists
 *
 * @param filepath      route to check
 *
 * @return              TRUE or FALSE
 */
int file_exists( char* filepath );

/**
 * Checks and returns TRUE if directory exists
 *
 * @param filepath      route to check
 *
 * @return              TRUE or FALSE
 */
int file_directory_exists( char* directory );

/**
 * Puts a slash or backslash at the end of path if missing
 *
 * @param directory     string with path to check, requires an additional byte for added character
 * @param option        'S' for slash, 'B' for backslash, any other option is ignored
 * 
 * @return              path with added slash or backslash
 */
char* file_directory_put_backslash( char* directory, char option );

/**
 * Creates a directory and all the parents indicated in it's route
 *
 * @param directory    path of directory to be created
 *
 * @return             TRUE or FALSE
 */
int file_mkdir_parent( char* directory );

/**
 * Creates a directory
 *
 * @param directory    path of directory to be created
 *
 * @return             TRUE or FALSE
 */
int file_mkdir( char* directory );

/**
 * Returns the size of a file
 *
 * @param filepath     string with a filepath
 *
 * @return             size of the file in bytes
 */
long long file_size( char* filepath );

/**
 * Moves a file to a new path
 *
 * @param filepath       path of the file to be moved
 * @param new_filepath   new path for the file
 *
 * @returh               TRUE or FALSE
 */
int file_move( char* filepath, char* new_filepath );

/**
 * Makes a copy of a file
 *
 * @param filepath       path of source file to be copied
 * @param new_filepath   path of new file to be created
 * @param do_overwrite   indicates if file should be overwritten if needed
 *
 * @returh               TRUE or FALSE
 */
int file_copy( char* filepath, char* new_filepath, int do_overwrite );

/**
 * Deletes a file
 *
 * @param filepath       path of file to be deleted
 *
 * @return               TRUE or FALSE
 */
int file_delete( char* filepath );

/**
 * Deletes a directory
 *
 * @param dir_path       path of directory to be deleted
 *
 * @return               TRUE o FALSE
 */
int file_directory_delete( char* dir_path );

/**
 * Returns the base path of a file
 *
 * @param filepath        full path of file to extract base path
 * @param directory       buffer for result
 *
 * @return                base path or empty if not found
 */
char * file_get_base_path(char *filepath, char *directory);

#ifdef __cplusplus
}
#endif

#endif  // _FILE_H
