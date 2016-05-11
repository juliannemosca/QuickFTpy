/*
 * list.c
 *
 * $Id: $
 * $HeadURL: $
 * $LastChangedRevision: $
 * $LastChangedDate: $
 * $LastChangedBy: $
 *
 */

#include <string.h>
#include <stdlib.h>

#include "list.h"
#include "mutex.h"

/**
 * Creates a structure for a new list
 *
 * @param thread_safe     TRUE for thread-safe lists
 * @param file            (debug) file where function is being called
 * @param line            (debug) line where function is being called
 *
 * @return                pointer to a new list structure
 *
 * @TODO: add debug logging with params file and line.
 */
list_t * list_new(int thread_safe, char* file, long line) {

  list_t * list;
  
  list = malloc(sizeof(list_t));
  list->first = NULL;
  list->last = NULL;

  if (thread_safe == TRUE) {
    
    // Allocates memory for a thread-safe structure
    list->mutex = (MUTEX_T*)malloc(sizeof(struct _mutex_t));
    memset(list->mutex, 0x00, sizeof(struct _mutex_t));

    // Creates mutex
    MUTEX_CREATE(&list->mutex);
  
  } else {

    // If it is not thread safe sets mutex to NULL
    list->mutex = NULL;

  }

  return list;

}

/**
 * Frees a previously initialized list
 *
 * @param list            pointer to a list structure
 */
void list_free(list_t * list) {
  
  if (list && list->mutex) {
    MUTEX_DESTROY(&list->mutex);
    free(list->mutex);
  }

  free(list);
}

/**
 * Checks if a list is empty
 *
 * @param list            list to check on
 *
 * @return                TRUE or FALSE
 *
 */
int list_is_empty(list_t * list) {

  int ret = FALSE;
  
  // locks the mutex for thread-safe lists
  if (list && list->mutex) MUTEX_LOCK(list->mutex);
  
  if(list == NULL || list->first == NULL) {
    ret = TRUE;
  }

  // unlocks the mutex for thread-safe lists
  if (list && list->mutex) MUTEX_UNLOCK(list->mutex);

  return ret;

}

/**
 * Adds a new node to a list
 *
 * @param list            list to add node to
 * @param content         pointer to the content of the new node
 * @param size            size of content of the new node
 * @param file            (debug) file where function is being called
 * @param line            (debug) line where function is being called
 * 
 * @TODO: add debug logging with params file and line.
 *
 */
void list_add(list_t * list, const void * content, size_t size, char * file, long line) {

  list_node_t* new_node = NULL;
  
  if (list != NULL && content != NULL) {

    // locks the mutex for thread-safe lists
    if (list->mutex) MUTEX_LOCK(list->mutex);
    
    new_node = (list_node_t*)malloc(sizeof(list_node_t));

    new_node->content = (void*)content;
    new_node->size = size;
    new_node->next = NULL;

    if (list->first == NULL) {

      list->first = new_node;

    } else {

      if (list->last != NULL) {
        list->last->next = new_node;
      }

    }

    list->last = new_node;

    // unlocks the mutex for thread-safe lists
    if (list->mutex) MUTEX_UNLOCK(list->mutex);
    
  }

}

/**
 * Adds a new node to a list only if content does not exists in another node
 *
 * @param list            list to add node to
 * @param content         pointer to the content of the new node
 * @param size            size of content of the new node
 * @param file            (debug) file where function is being called
 * @param line            (debug) line where function is being called
 * 
 * @TODO: add debug logging with params file and line.
 *
 */
void list_add_unique(list_t * list, const void * content, size_t size, char * file, long line) {

  list_node_t*  match_node = NULL;

  if (list != NULL && content != NULL) {

    // Searches for duplicate content in list
    match_node = list_search_by_content(list, content, size);
    if (match_node == NULL) {

      // Adds the new node
      list_add(list, content, size, file, line);

    }
  }
}

/**
 * Insert node with an order function
 *
 * @param list            list to add node to
 * @param content         pointer to the content of the new node
 * @param size            size of content of the new node
 * @param greater_than    pointer to a comparison function
 * @param file            (debug) file where function is being called
 * @param line            (debug) line where function is being called
 * 
 * @TODO: add debug logging with params file and line.
 *
 */
void list_add_ordered(list_t * list, const void * content, size_t size, int(*greater_than)(void*, size_t, void*, size_t), char * file, long line) {

  list_node_t * node;
  list_node_t * prev_node;
  list_node_t * new_node;

  if (list != NULL && content != NULL) {

    // If it is a thred-safe list locks the mutex
    if (list->mutex) MUTEX_LOCK(list->mutex);
    
      // Allocates memory for a new node
      new_node = (list_node_t*)malloc(sizeof(list_node_t));

      // Defines content of the new node
      new_node->content = (void*)content;
      new_node->size = size;
      new_node->next = NULL;

      // Looks for the position where the new node will be inserted
      node = list->first;
      prev_node = node;
      
      // Looks for previous node
      while (node != NULL && greater_than(node->content, node->size, new_node->content, new_node->size) == FALSE) {
        prev_node = node;
        node = node->next;
      }

      if (node == prev_node) {

        // Must be inserted at the beggining of the list
        new_node->next = node;
        list->first = new_node;
        
      } else {
        
        // Inserts the node somewhere in the middle
        prev_node->next = new_node;
        new_node->next = node;
      }
      
      // Updates the last node
      if (node == NULL) {
        list->last = new_node;
      }

      // If using a thred-safe list unlocks mutex
      if (list->mutex) MUTEX_UNLOCK(list->mutex);

  }

}

/**
 * Removes a node from a list
 *
 * @param list            list where the node will be removed
 * @param node            node to remove from the list
 * @param free_content    pointer to a function for freeing the node content
 *
 * @return                TRUE or FALSE
 *
 */
int list_remove_node(list_t * list, list_node_t * node, void(*free_content)(void**)) {

  list_node_t * prev_node = NULL;
  list_node_t * cur_node = NULL;
  int match = FALSE;

  if (list != NULL && node != NULL) {

    // locks the mutex for thread-safe lists
    if (list->mutex) MUTEX_LOCK(list->mutex);
    
    cur_node = list->first;
    prev_node = cur_node;

    while (cur_node != NULL && cur_node != node) {
      prev_node = cur_node;
      cur_node = cur_node->next;
    }

    if (cur_node != NULL) {

      // Found the node to delete
      match = TRUE;
      if (prev_node == cur_node) {

        // If it is the first node
        
        list->first = list->first->next;
        
        // Calls the function provided for freeing the content
        if (free_content) free_content(&cur_node->content);
        
        free(cur_node);
        prev_node = NULL;

        // Checks for the last node to set NULL
        if (list->first == NULL) {
          list->last = NULL;
        }
          
      } else {

        // If it is a node in the middle or at the end
        
        // Makes a bridge to skip the removed node
        prev_node->next = cur_node->next;

        // Calls the function provided for freeing the content
        if (free_content) free_content(&cur_node->content);
        
        free(cur_node);
        cur_node = NULL;

        // Checks for the last node to set NULL
        if (prev_node->next == NULL) {
          list->last = prev_node;
        }

      }

    }

    // unlocks the mutex for thread-safe lists
    if (list->mutex) MUTEX_UNLOCK(list->mutex);
    
  }

  return match;
}

/**
 * Returns the number of nodes in a list
 *
 * @param list            list for counting nodes
 *
 * @return                number of nodes in the list
 *
 */
int list_get_size(list_t * list) {

  list_node_t * cur_node = NULL;
  int count = 0;

  if (list != NULL) {
  
    // locks the mutex for thread-safe lists
    if (list->mutex) MUTEX_LOCK(list->mutex);

    cur_node = list->first;
    while (cur_node != NULL) {
      count++;
      cur_node = cur_node->next;
    }

    // unlocks the mutex for thread-safe lists
    if (list->mutex) MUTEX_UNLOCK(list->mutex);

  }
  
  return count;
}

/**
 * Free()s the memory used by the list 
 *
 * @param list            list to free the memory of
 * @param free_content    function for freeing a node's content
 *
 */
void list_delete(list_t * list, void(*free_content)(void**)) {

  list_node_t * cur_node = NULL;
  list_node_t * next_node = NULL;

  if (list != NULL && list->first != NULL) {

    // locks the mutex for thread-safe lists
    if (list->mutex) MUTEX_LOCK(list->mutex);
    
    // Iterates through the list
    cur_node = list->first;
    while (cur_node != NULL) {

      next_node = cur_node->next;

      // Calls provided free content function
      if (free_content) free_content(&cur_node->content);
      free(cur_node);

      cur_node = next_node;

    }

    list->first = NULL;
    list->last = NULL;

    // unlocks the mutex for thread-safe lists
    if (list->mutex) MUTEX_UNLOCK(list->mutex);
  
  }

  return;
}

/**
 * Searches the list and returns the node for a specific content 
 * or NULL if not found
 *
 * @param list            list to search on
 * @param content         pointer to content to be searched
 * @param size            size of content
 *
 * @return                node with the content or NULL if not found
 *
 */
list_node_t * list_search_by_content(list_t * list, const void * content, size_t size) {

  list_node_t * node = NULL;

  if (list != NULL) {
  
    // locks the mutex for thread-safe lists
    if (list->mutex) MUTEX_LOCK(list->mutex);
    
    node = list->first;
    while (node != NULL) {
      if (node->size == size && (memcmp(node->content, content, size) == 0)) break;
      node = node->next;
    }

    // unlocks the mutex for thread-safe lists
    if (list->mutex) MUTEX_UNLOCK(list->mutex);
  
  }
  
  return node;
}

/**
 * Searches the list and returns the node for a specific content 
 * using a provided comparison function, or NULL if not found
 *
 * @param list            list to search on
 * @param content         pointer to content to be searched
 * @param size            size of content
 * @param are_equal       comparison function
 *
 * @return                node with the content or NULL if not found
 *
 */
list_node_t * list_search_by(list_t* list, void* content, size_t size, int(*are_equal)(list_node_t*, void*, size_t)) {

  list_node_t * node = NULL;
  int match = FALSE;
  
  if (list != NULL) {
  
    // locks the mutex for thread-safe lists
    if (list->mutex) MUTEX_LOCK(list->mutex);
    
    node = list->first;
    while (node != NULL && !match) {

      // Calls the provided comparison function
      if (are_equal(node, content, size) == TRUE) {		  
        match = TRUE;
      } else {
        node = node->next;
      }

    }

    // unlocks the mutex for thread-safe lists
    if (list->mutex) MUTEX_UNLOCK(list->mutex);
  
  }

  return node;
}

/**
 * Fetches a node from the list by its index
 *
 * @param list            list to get node from
 * @param index           index of desired node
 *
 * @return                pointer to the desired node or NULL
 *
 */
list_node_t * list_search_by_index(list_t * list, int index) {

  list_node_t * node = NULL;
  list_node_t * result = NULL;
  int count = 0;

  if (list != NULL) {
  
    // locks the mutex for thread-safe lists
    if (list->mutex) MUTEX_LOCK(list->mutex);
    
    node = list->first;
    while (node != NULL) {

      if (count == index) {
        result = node;
        break;
      }

      node = node->next;
      count++;

    }

    // unlocks the mutex for thread-safe lists
    if (list->mutex) MUTEX_UNLOCK(list->mutex);
  
  }
  
  return result;
}
