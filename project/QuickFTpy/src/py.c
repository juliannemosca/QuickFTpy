/*
 * py.c
 *
 * $Id: $
 * $HeadURL: $
 * $LastChangedRevision: $
 * $LastChangedDate: $
 * $LastChangedBy: $
 *
 */

#include "py.h"
#include "server.h"
#include "client.h"

/**
 * Python module server initialization function
 *
 */
static PyObject * py_server_initialize(PyObject * self, PyObject * args) {
  
  return server_initialize(self, args);
}

/**
 * Python module server finalization function
 *
 */
static PyObject * py_server_finalize (PyObject * self) {
  
  return server_finalize(self);
}

/**
 * Python module 'File Receive' operation for the client
 *
 */
static PyObject * py_client_file_receive( PyObject * self, PyObject * args ) {
  
  return client_file_receive(self, args);
}

/**
 * Python module 'File Send' operation for the client
 *
 */
static PyObject * py_client_file_send( PyObject * self, PyObject * args ) {
  
  return client_file_send(self, args);
}

/**
 * Python module 'File Delete' operation for the client on the server
 *
 */
static PyObject * py_client_file_delete( PyObject * self, PyObject * args ) {
  
  return client_file_delete(self, args);
}


// Python method definitions
static PyMethodDef quickFTpyMethods[] = {
    { "servstart",  (PyCFunction)py_server_initialize,    METH_VARARGS, NULL },
    { "servend",    (PyCFunction)py_server_finalize,      METH_NOARGS,  NULL },
    { "clsend",     (PyCFunction)py_client_file_send,     METH_VARARGS, NULL },
    { "clrecv",     (PyCFunction)py_client_file_receive,  METH_VARARGS, NULL },
    { "cldel",      (PyCFunction)py_client_file_delete,   METH_VARARGS, NULL },
    { NULL,         NULL,                                 0,            NULL }
};

void initquickftpy(void)
{
    Py_InitModule("quickftpy", quickFTpyMethods);
}
