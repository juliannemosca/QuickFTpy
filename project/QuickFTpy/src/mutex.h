/*
 * mutex.h
 *
 * $Id: $
 * $HeadURL: $
 * $LastChangedRevision: $
 * $LastChangedDate: $
 * $LastChangedBy: $
 *
 */

#ifndef MUTEX_H
#define MUTEX_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>

#include "macros.h"
#include "logger.h"

//
// Macros
//
#ifdef _DEBUG
  #define MUTEX_CREATE(t)       if(!mutex_create(t))LOGGER("MUTEX_DBG", "mutex create fail");
  #define MUTEX_DESTROY(t)      if(!mutex_destroy(t))LOGGER("MUTEX_DBG","mutex destroy fail");
  #define MUTEX_LOCK(t)         if(!mutex_lock(t))LOGGER("MUTEX_DBG","mutex lock fail");
  #define MUTEX_UNLOCK(t)       if(!mutex_unlock(t))LOGGER("MUTEX_DBG","mutex unlock fail");
  #define MUTEX_IS_LOCKED(t)    mutex_is_locked(t)
  #define MUTEX_COND_SIGNAL(t)  if(!mutex_condition_signal(t))LOGGER("MUTEX_DBG","condition signal fail");
  #define MUTEX_COND_WAIT(t)    if(!mutex_condition_wait(t))LOGGER("MUTEX_DBG","condition wait fail");
#else
  #define MUTEX_CREATE(t)       mutex_create(t);
  #define MUTEX_DESTROY(t)      mutex_destroy(t);
  #define MUTEX_LOCK(t)         mutex_lock(t);
  #define MUTEX_UNLOCK(t)       mutex_unlock(t);
  #define MUTEX_IS_LOCKED(t)    mutex_is_locked(t)
  #define MUTEX_COND_SIGNAL(t)  mutex_condition_signal(t);
  #define MUTEX_COND_WAIT(t)    mutex_condition_wait(t);
#endif

//
// Mutex data structure
//
typedef struct _mutex_t {

  int created;
  pthread_mutex_t * mutex;
  pthread_cond_t * condition;
  pthread_t owner;

} MUTEX_T;

/**
 * Creates a new mutex
 *
 * @param mutex     mutex data structure
 *
 * @return          TRUE or FALSE
 */
int mutex_create(MUTEX_T** mutex);

/**
 * Destroys a mutex
 *
 * @param mutex     mutex data structure
 *
 * @return          TRUE or FALSE
 */
int mutex_destroy(MUTEX_T** mutex);

/**
 * Performs a lock on the mutex
 *
 * @param mutex     mutex data structure
 *
 * @return          TRUE or FALSE
 */
int mutex_lock(MUTEX_T* mutex);

/**
 * Unlocks a mutex
 *
 * @param mutex     mutex data structure
 *
 * @return          TRUE or FALSE
 */
int mutex_unlock(MUTEX_T* mutex);

/**
 * Checks if a mutex is currently locked
 *
 * @param mutex     mutex data structure
 *
 * @return          TRUE or FALSE
 */
int mutex_is_locked(MUTEX_T* mutex);

/**
 * Signals the condition on a mutex
 *
 * @param mutex     mutex data structure
 *
 * @return          TRUE or FALSE
 */
int mutex_condition_signal(MUTEX_T* mutex);

/**
 * Waits on the mutex condition
 *
 * @param mutex     mutex data structure
 *
 * @return          TRUE or FALSE
 */
int mutex_condition_wait(MUTEX_T* mutex);

#ifdef __cplusplus
}
#endif

#endif // MUTEX_H
