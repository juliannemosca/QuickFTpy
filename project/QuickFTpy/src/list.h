/*
 * list.h
 *
 * $Id: $
 * $HeadURL: $
 * $LastChangedRevision: $
 * $LastChangedDate: $
 * $LastChangedBy: $
 *
 */

#ifndef LIST_H
#define LIST_H

#ifdef __cplusplus
extern "C" {
#endif

#include "macros.h"
#include "mutex.h"

// Macros
#define LIST_NEW(t)                   list_new(t, __FILE__, __LINE__)
#define LIST_FREE(l)                  do { list_free(l); l = NULL; } while (0)
#define LIST_IS_EMPTY                 list_is_empty
#define LIST_ADD(l, c, s)             list_add(l, c, s, __FILE__, __LINE__)
#define LIST_ADD_UNIQUE(l, c, s)      list_add_unique(l, c, s, __FILE__, __LINE__)
#define LIST_ADD_ORDERED(l, c, s, f)  list_add_ordered(l, c, s, f, __FILE__, __LINE__)
#define LIST_REMOVE                   list_remove_node
#define LIST_SIZE                     list_get_size
#define LIST_DELETE                   list_delete
#define LIST_SEARCH_CONTENT           list_search_by_content
#define LIST_SEARCH                   list_search_by
#define LIST_SEARCH_INDEX             list_search_by_index

/**
 * Returns the value of the node content
 *
 * @param node            pointer to node
 * @param type            data type
 *
 * @return                value of node content casted as the specified data type
 */
#define GETLISTNODECONTENT(node, type) (type*)((node)->content)

/**
 * Defines a list node type
 */
typedef struct _list_node_t {

  void* content;
  size_t size;

  struct _list_node_t* next;

} list_node_t;

/**
 * Defines a list type
 */
typedef struct _list_t {

  struct _list_node_t* first;
  struct _list_node_t* last;
  struct _mutex_t* mutex;

} list_t;

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
list_t * list_new(int thread_safe, char * file, long line);

/**
 * Frees a previously initialized list
 *
 * @param list            pointer to a list structure
 */
void list_free(list_t * list);

/**
 * Checks if a list is empty
 *
 * @param list            list to check on
 *
 * @return                TRUE or FALSE
 *
 */
int list_is_empty(list_t* list);

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
void list_add(list_t* list, const void* content, size_t size, char* file, long line);

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
void list_add_unique(list_t* list, const void* content, size_t size, char* file, long line);

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
void list_add_ordered(list_t* list, const void* content, size_t size, int(*greater_than)(void*, size_t, void*, size_t), char* file, long line);

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
int list_remove_node(list_t* list, list_node_t* node, void(*free_content)(void**));

/**
 * Returns the number of nodes in a list
 *
 * @param list            list for counting nodes
 *
 * @return                number of nodes in the list
 *
 */
int list_get_size(list_t* list);

/**
 * Free()s the memory used by the list 
 *
 * @param list            list to free the memory of
 * @param free_content    function for freeing a node's content
 *
 */
void list_delete(list_t* list, void(*free_content)(void**));

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
list_node_t* list_search_by_content(list_t* list, const void* content, size_t size);

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
list_node_t* list_search_by(list_t* list, void* content, size_t size, int(*are_equal)(list_node_t*, void*, size_t));

/**
 * Fetches a node from the list by its index
 *
 * @param list            list to get node from
 * @param index           index of desired node
 *
 * @return                pointer to the desired node or NULL
 *
 */
list_node_t* list_search_by_index(list_t* list, int index);

#ifdef __cplusplus
}
#endif

#endif  // LIST_H
