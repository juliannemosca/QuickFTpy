/*
 * logger.c
 *
 * $Id: $
 * $HeadURL: $
 * $LastChangedRevision: $
 * $LastChangedDate: $
 * $LastChangedBy: $
 *
 */

#include <stdlib.h>
#include "logger.h"
#include "macros.h"
#include "mutex.h"

static PyGILState_STATE gstate;
MUTEX_T * log_mutex = NULL;
static logger_initialized = FALSE;

/**
 * Initializes access to log functions
 * 
 * @param function
 * @param message
 */
void logger_init() {
  
  if (logger_initialized == FALSE) {
    
    log_mutex = (MUTEX_T*)malloc(sizeof(struct _mutex_t) );
    memset(log_mutex, 0x00, sizeof(struct _mutex_t));
    MUTEX_CREATE(&log_mutex);
  
    logger_initialized = TRUE;
    
  }
  
}

/**
 * Finalizes access to log functions
 * 
 * @param function
 * @param message
 */
void logger_deinit() {
  
  if (logger_initialized == TRUE) {
    
    MUTEX_LOCK(log_mutex);
    MUTEX_UNLOCK(log_mutex);
    MUTEX_DESTROY(&log_mutex);
    
    free(log_mutex);
    log_mutex = NULL;
    
    logger_initialized = FALSE;
    
  }
  
}

/**
 * Writes a line to the log file
 *
 * @param function               function where the line is being added
 * @param message                message to write to the log file
 *
 */
void logger_write(const char * function, char * message) {
  
  if (logger_initialized) {
  
    MUTEX_LOCK(log_mutex);
    gstate = PyGILState_Ensure();
    
    // Builds the argument list
    PyObject * arglist = Py_BuildValue("(ss)", function, message);

    if (gl_py_log_writer != NULL) {  
      PyEval_CallObject(gl_py_log_writer,arglist);
    }
    Py_DECREF(arglist);

    PyGILState_Release(gstate);
    MUTEX_UNLOCK(log_mutex);
    
  }
  
}
