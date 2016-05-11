/*
 * logger.h
 *
 * $Id: $
 * $HeadURL: $
 * $LastChangedRevision: $
 * $LastChangedDate: $
 * $LastChangedBy: $
 *
 */

#ifndef _LOGGER_H
#define _LOGGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <python2.7/Python.h>

#define LOGGER_INIT         logger_init()
#define LOGGER_DEINIT       logger_deinit()

#define LOGGER              logger_write
#define LOGGER_DEFAULT_NAME "quickft"

// Global pointer to the log writer function
PyObject * gl_py_log_writer;

/**
 * Initializes access to log functions
 * 
 * @param function
 * @param message
 */
void logger_init();

/**
 * Finalizes access to log functions
 * 
 * @param function
 * @param message
 */
void logger_deinit();

/**
 * Writes a line to the log file
 *
 * @param function               function where the line is being added
 * @param message                message to write to the log file
 *
 */
void logger_write(const char* function, char* message);

#ifdef __cplusplus
}
#endif

#endif //_LOGGER_H
