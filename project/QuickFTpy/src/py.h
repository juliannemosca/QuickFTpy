/*
 * py.h
 *
 * $Id: $
 * $HeadURL: $
 * $LastChangedRevision: $
 * $LastChangedDate: $
 * $LastChangedBy: $
 *
 */

#ifndef PY_H
#define PY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <python2.7/Python.h>

/**
 * Python module server initialization function
 *
 */
static PyObject * py_server_initialize(PyObject *obj, PyObject *args);

/**
 * Python module server finalization function
 *
 */
static PyObject * py_server_finalize (PyObject* self);

/**
 * Python module 'File Receive' operation for the client
 *
 */
static PyObject * py_client_file_receive( PyObject * self, PyObject * args );

/**
 * Python module 'File Send' operation for the client
 *
 */
static PyObject * py_client_file_send( PyObject * self, PyObject * args );

/**
 * Python module 'File Delete' operation for the client on the server
 *
 */
static PyObject * py_client_file_delete( PyObject * self, PyObject * args );


/*
 * Initialization function for the python module
 * 
 */
void initquickftpy(void);

#ifdef __cplusplus
}
#endif

#endif /* PY_H */

