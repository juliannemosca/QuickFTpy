/*
 * threads.h
 *
 * $Id: $
 * $HeadURL: $
 * $LastChangedRevision: $
 * $LastChangedDate: $
 * $LastChangedBy: $
 *
 */

#ifndef THREAD_H
#define THREAD_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#include <windows.h>
#include <process.h>
#endif

#include <pthread.h>
#include "macros.h"

#include <stdio.h>

//
// Macros:
//


#ifdef _WIN32

#define thread_t                              HANDLE
#define THREAD_FUNCTION                       unsigned (__stdcall *) (void *)
#define THREAD_CREATE(thread,func,arg)        (int)((*thread=(HANDLE)_beginthreadex(NULL,0,(THREAD_FUNCTION)func,arg,0,NULL))==NULL)
#define THREAD_JOIN(thread, result)           while (WaitForSingleObject((thread),INFINITE)!=WAIT_OBJECT_0) Sleep(10); if (result) CloseHandle(thread);
#define THREAD_DETACH(thread)                 if(thread!=NULL)CloseHandle(thread)
#define THREAD_CANCEL(thread)                 TerminateThread(thread,0)
#define THREAD_SELF()                         GetCurrentThreadId()
//#define THREAD_SLEEP(nms)                   Sleep(nms)

#else

#define thread_t                              pthread_t
#define THREAD_CREATE(thread,func,arg)        pthread_create((*thread), NULL, func, arg)
#define THREAD_JOIN(thread, result)           if (result) pthread_cancel(*(thread)); pthread_join(*(thread), NULL);
#define THREAD_DETACH(thread)                 if(thread!=NULL)pthread_detach(thread)
#define THREAD_CANCEL(thread)                 pthread_cancel(thread)
#define THREAD_SELF()                         pthread_self()

#endif

#ifdef __cplusplus
}
#endif

#endif  // THREAD_H
