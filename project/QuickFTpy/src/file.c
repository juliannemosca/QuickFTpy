/*
 * file.c
 *
 * $Id: $
 * $HeadURL: $
 * $LastChangedRevision: $
 * $LastChangedDate: $
 * $LastChangedBy: $
 *
 */

#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/sendfile.h>

#include "file.h"
#include "macros.h"
#include "logger.h"

/**
 * Checks and returns TRUE if file exists
 *
 * @param filepath      route to check
 *
 * @return              TRUE or FALSE
 */
int file_exists( char* filepath ) {
  
  struct stat results;
  int ret = TRUE;
  
  if ( filepath == NULL ) {
    ret = FALSE;
  } else {
    if ( stat( filepath, &results ) ) {
      ret = FALSE;
    }
  }

  return ret;
  
}

/**
 * Checks and returns TRUE if directory exists
 *
 * @param filepath      route to check
 *
 * @return              TRUE or FALSE
 */
int file_directory_exists( char* directory ) {

  struct stat results;
  char tmp[1024];
  size_t len;
  int ret = FALSE;

  if (directory != NULL) {

    // Makes a temporary copy of directory
    strncpy(tmp, directory, sizeof(tmp));
    len = strlen(tmp);

    // Removes slach or backslash from the end
    if(tmp[len - 1] == L'/') tmp[len - 1] = L'\0';
    if(tmp[len - 1] == L'\\') tmp[len - 1] = L'\0';

    // If directory exists
    if ( stat( tmp, &results ) == 0 ) {

      // If it is a directory
      if ( (results.st_mode & S_IFDIR) != 0 ) {
        ret = TRUE;
      }

    }

  }

  return ret;

}

/**
 * Puts a slash or backslash at the end of path if missing
 *
 * @param directory     string with path to check, requires an additional byte for added character
 * @param option        'S' for slash, 'B' for backslash, any other option is ignored
 * 
 * @return              path with added slash or backslash
 */
char* file_directory_put_backslash( char* directory, char option ) {

  if ( directory != NULL ) {
    if ( strlen(directory) > 0 ) {
      if ( directory[strlen(directory)-1] != '\\' && directory[strlen(directory)-1] != '/' ) {
        
          if (option == 'S')
            strcat ( directory, "/" );
          else if (option == 'B')
            strcat ( directory, "\\" );
      }
    }
  }

  return directory;

}

/**
 * Creates a directory and all the parents indicated in it's route
 *
 * @param directory    path of directory to be created
 *
 * @return             TRUE or FALSE
 */
int file_mkdir_parent( char* directory ) {

  char tmp[1024];
  char *p;
  size_t len;
  int success = FALSE;

  if (directory != NULL) {

    // Makes a temporary copy of directory
    strncpy(tmp, directory, sizeof(tmp));
    len = strlen(tmp);
  
    // Removes ending slash or backslash
    if(tmp[len - 1] == L'/') tmp[len - 1] = L'\0';
    if(tmp[len - 1] == L'\\') tmp[len - 1] = L'\0';
  
    // Goes through complete path and looks for directories
    for(p = tmp; *p; p++) {

      // If it finds a slash or backslash
      if(*p == L'/' || *p == L'\\') {
        *p = L'\0';
        
        // Check for empty first path
        if (strlen(tmp) > 0) {

          // Creates if directory or subdirectory does not exists
          if( !file_directory_exists(tmp) ) {
            mkdir(tmp,0700);
          }
        
        }

        *p = L'\\';
      
      }

    }

    if( !file_directory_exists(tmp) ) {
      mkdir(tmp,0700);
    }

    // Verifies that directory was succesfully created
    if (file_directory_exists(directory)) {
      success = TRUE;
    }

  }

  return success;
  
}

/**
 * Creates a directory
 *
 * @param directory    path of directory to be created
 *
 * @return             TRUE or FALSE
 */
int file_mkdir( char* directory ) {

  int ret = FALSE;

  if (directory != NULL) {

    // If directory already exists returns TRUE
    if ( file_directory_exists(directory) ) {
      ret = TRUE;
    }
    // Attempts to create the directory and all of it's required parents
    else {
      ret = file_mkdir_parent(directory);
    }

  }

  return ret;

}

/**
 * Returns the size of a file
 *
 * @param filepath     string with a filepath
 *
 * @return             size of the file in bytes
 */
long long file_size( char* filepath ) {

  struct stat results;

  if (filepath != NULL) {

    if( !stat( filepath, &results ) ) {
      return results.st_size;
    }

  }

  return 0;

}

/**
 * Moves a file to a new path
 *
 * @param filepath       path of the file to be moved
 * @param new_filepath   new path for the file
 *
 * @returh               TRUE or FALSE
 */
int file_move( char* filepath, char* new_filepath ) {

  int success = FALSE;
  int ret = 0;

  if (filepath != NULL && new_filepath != NULL) {

    ret = rename(filepath, new_filepath);
    if ( ret == 0 ) {
      success = TRUE;
    } else {
  
      char err_message[1024];
      sprintf(err_message, "File operation failed with code [%d]", errno);
      LOGGER(__FUNCTION__, err_message);
      
    }

  }

  return success;

}

/**
 * Makes a copy of a file
 *
 * @param filepath       path of source file to be copied
 * @param new_filepath   path of new file to be created
 * @param do_overwrite   indicates if file should be overwritten if needed
 *
 * @returh               TRUE or FALSE
 */
int file_copy( char* filepath, char* new_filepath, int do_overwrite ) {

  int success = FALSE;
  int input, output;    

  if (!do_overwrite) {
    if (file_exists(new_filepath)) {
      return FALSE;
    }
  }

  if ((input = open(filepath, O_RDONLY)) == -1)
  {
    return -1;
  }    
  if ((output = open(new_filepath, O_RDWR | O_CREAT, 0600)) == -1)
  {
    close(input);
    return -1;
  }

  off_t bytes_copied = 0;
  struct stat fileinfo = {0};
  fstat(input, &fileinfo);
  int result = sendfile(output, input, &bytes_copied, fileinfo.st_size);

  close(input);
  close(output);

  if (result > 0) success = TRUE; 

  return success;

}

/**
 * Deletes a file
 *
 * @param filepath       path of file to be deleted
 *
 * @return               TRUE or FALSE
 */
int file_delete( char* filepath ) {

  int success = FALSE;

  if (filepath != NULL) {

    if ( unlink(filepath) == -1 ) {

      char err_message[1024];
      sprintf(err_message, "File operation failed with code [%d]", errno);
      LOGGER(__FUNCTION__, err_message);

    } else {
      success = TRUE;
    }

  }

  return success;

}

/**
 * Deletes a directory
 *
 * @param dir_path       path of directory to be deleted
 *
 * @return               TRUE o FALSE
 */
int file_directory_delete( char* dir_path ) {

  int success = FALSE;

  if (dir_path != NULL) {

    if ( rmdir(dir_path) == -1 ) {

      char err_message[1024];
      sprintf(err_message, "File operation failed with code [%d]", errno);
      LOGGER(__FUNCTION__, err_message);

    } else {
      success = TRUE;
    }

  }

  return success;

}

/**
 * Returns the base path of a file
 *
 * @param filepath        full path of file to extract base path
 * @param directory       buffer for result
 *
 * @return                base path or empty if not found
 */
char * file_get_base_path(char *filepath, char *directory) {

  int i;
  int length;

  length = strlen( filepath );

  for ( i = length; i > 0; i-- ) {

    if ( filepath[i] == '\\' || filepath[i] == '/' ) {
      strncpy( directory, filepath, i );
      directory[i++] = '\0';
      return directory;
    }
  
  }

  // No directory found in path, returns empty
  directory[0] = '\0';
  return directory;

}
