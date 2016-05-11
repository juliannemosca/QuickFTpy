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

#include <pthread.h>
#include <string.h>

#include "mutex.h"
#include "macros.h"

/**
 * Creates a new mutex
 *
 * @param mutex     mutex data structure
 *
 * @return          TRUE or FALSE
 */
int mutex_create(MUTEX_T** mutex) {

  // If mutex was not created already
  if ( (*mutex) && (*mutex)->created == FALSE ) {

    // Initializes structure
    memset((*mutex), 0x00, sizeof(MUTEX_T));

    (*mutex)->mutex = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));

    // Creates mutex
    if ( pthread_mutex_init( (*mutex)->mutex, NULL) == 0 ) {
    
      (*mutex)->condition = (pthread_cond_t*) malloc(sizeof(pthread_cond_t));

      // Creates condition
      if ( pthread_cond_init( (*mutex)->condition, NULL) == 0 ) {
  
        // Sets result
        (*mutex)->created = TRUE;
        return TRUE;

      }

    }

  } else {
    LOGGER(__FUNCTION__, "mutex has already been created");
  }

  return FALSE;

}

/**
 * Destroys a mutex
 *
 * @param mutex     mutex data structure
 *
 * @return          TRUE or FALSE
 */
int mutex_destroy(MUTEX_T** mutex) {

  // If mutex was properly created
  if ( (*mutex) && (*mutex)->created ) {

    // Locks to wait until the mutex is free for deleting
    pthread_mutex_lock((*mutex)->mutex);
    pthread_mutex_unlock((*mutex)->mutex);

    // Destroys mutex
    pthread_mutex_destroy((*mutex)->mutex);

    // Destroys condition
    pthread_cond_destroy((*mutex)->condition);

    free( (*mutex)->mutex );
    free( (*mutex)->condition );    

    // Initializes structure
    memset((*mutex), 0x00, sizeof(MUTEX_T));

    return TRUE;
    
  } else {

    LOGGER(__FUNCTION__, "mutex has not been created");
    return FALSE;

  }

}

/**
 * Performs a lock on the mutex
 *
 * @param mutex     mutex data structure
 *
 * @return          TRUE or FALSE
 */
int mutex_lock(MUTEX_T* mutex) {

  char buffer[_BUFFER_SIZE_S]; 
  pthread_t id = pthread_self();

  // If mutex was properly created
  if (mutex && mutex->created) {

    // If the owner of the mutex lock is trying to lock again the mutex
    if (mutex->owner == id) {

      sprintf(buffer, "mutex has been locked by the thread (%ld)", (long int)id);
      LOGGER(__FUNCTION__, buffer);

      return FALSE;

    }

    // Performs a lock on the mutex
    pthread_mutex_lock(mutex->mutex);
    
    // Stores the owner id
    mutex->owner = id;

    return TRUE;
  
  } else {

    LOGGER(__FUNCTION__, "mutex has not been created");
    return FALSE;

  }

}

/**
 * Unlocks a mutex
 *
 * @param mutex     mutex data structure
 *
 * @return          TRUE or FALSE
 */
int mutex_unlock(MUTEX_T* mutex) {

  char buffer[_BUFFER_SIZE_S];
  pthread_t id = pthread_self();

  // If mutex was properly created
  if ( mutex->created ) {

    // If the thread trying to free the mutex is not the owner of the lock
    if( mutex->owner != id ) {

      sprintf(buffer, "mutex lock cannot be removed by another thread (%ld) (owner: %ld)", (long int)id, (long int)mutex->owner);
      LOGGER(__FUNCTION__, buffer);

      return FALSE;

    }  

    // Deletes owner id
    mutex->owner = 0;

    // Unlocks the mutex
    pthread_mutex_unlock(mutex->mutex);

    return TRUE;

  } else {

    LOGGER(__FUNCTION__, "mutex has not been created");
    return FALSE;

  }

}

/**
 * Checks if a mutex is currently locked
 *
 * @param mutex     mutex data structure
 *
 * @return          TRUE or FALSE
 */
int mutex_is_locked(MUTEX_T* mutex) {

  // If mutex was created and the lock has an owner
  if ( mutex->created && mutex->owner ) {
    return TRUE;
  }

  return FALSE;

}

/**
 * Signals the condition on a mutex
 *
 * @param mutex     mutex data structure
 *
 * @return          TRUE or FALSE
 */
int mutex_condition_signal(MUTEX_T* mutex) {

  char buffer[_BUFFER_SIZE_S];
  pthread_t id = pthread_self();

  // If mutex was properly created
  if ( mutex->created ) {

    sprintf(buffer, "the thread condition (%ld) is signaled by the thread (%ld)", (long int)mutex->owner, (long int)id);
    LOGGER(__FUNCTION__, buffer);

    // Signals condition
    pthread_cond_init(mutex->condition, NULL);

    return TRUE;

  } else {

    LOGGER(__FUNCTION__, "mutex has not been created");
    return FALSE;

  }

}

/**
 * Waits on the mutex condition
 *
 * @param mutex     mutex data structure
 *
 * @return          TRUE or FALSE
 */
int mutex_condition_wait(MUTEX_T* mutex) {

  char buffer[_BUFFER_SIZE_S];
  pthread_t id = pthread_self();

  // If mutex was properly created
  if ( mutex->created ) {

    // If it isn't the owner putting the mutex on wait
    if( mutex->owner != id ) {

      sprintf(buffer, "condition can only be put on wait by its owner thread (%ld) (owner: %ld)", (long int)id, (long int)mutex->owner);
      LOGGER(__FUNCTION__, buffer);

      return FALSE;

    }

    sprintf(buffer, "thread condition (%ld) set on wait by thread (%ld)", (long int)mutex->owner, (long int)id);
    LOGGER(__FUNCTION__, buffer);

    mutex_unlock(mutex);
    pthread_cond_wait(mutex->condition, mutex->mutex);    
    mutex_lock(mutex);

    return TRUE;

  } else {

    LOGGER(__FUNCTION__, "mutex has not been created");
    return FALSE;

  }

}
