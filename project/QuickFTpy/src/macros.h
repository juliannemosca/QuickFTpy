/*
 * macros.h
 *
 * $Id: $
 * $HeadURL: $
 * $LastChangedRevision: $
 * $LastChangedDate: $
 * $LastChangedBy: $
 *
 */

#ifndef MACROS_H
#define	MACROS_H

#ifdef	__cplusplus
extern "C" {
#endif

// Defines boolean values
#define TRUE  1
#define FALSE 0

// Buffer sizes for static memory allocation
#ifndef _BUFFER_SIZE_XL
#define	_BUFFER_SIZE_XL 32768
#endif

#ifndef _BUFFER_SIZE_L
#define	_BUFFER_SIZE_L 16384
#endif

#ifndef _BUFFER_SIZE_M
#define	_BUFFER_SIZE_M 4192
#endif

#ifndef _BUFFER_SIZE_S
#define	_BUFFER_SIZE_S 1024
#endif

#ifndef _BUFFER_SIZE_XS
#define	_BUFFER_SIZE_XS 256
#endif

// For printing a debug line
#ifdef _DEBUG
#define DBG_LINE(s) printf("DBG >> [%s %d] %s\n", __FILE__,__LINE__, s);
#else
#define DBG_LINE
#endif

// Para funciones de debug evita errores por nulos
#ifndef TEXT
#define TEXT(t) ((t == NULL)?"":t)
#endif

#ifdef	__cplusplus
}
#endif

#endif	// MACROS_H
