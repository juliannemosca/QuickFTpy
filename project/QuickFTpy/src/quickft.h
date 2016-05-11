/*
 * quickft.h
 *
 * $Id: $
 * $HeadURL: $
 * $LastChangedRevision: $
 * $LastChangedDate: $
 * $LastChangedBy: $
 *
 */

#ifndef QUICKFT_H
#define QUICKFT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#include "server.h"

#define DEFAULT_PORT      29765
#define DEFAULT_PORT_STR  "29765"
#define DEFAULT_MAX_CONN  256

#define DEFAULT_TIMEOUT           30     // timeout in seconds
#define DEFAULT_TIMEOUT_ACK       8      // timeout in seconds

unsigned long gl_timeout;

// Global variables:
char * gl_msg_param;

#ifdef __cplusplus
}
#endif

#endif // QUICKFT_H
