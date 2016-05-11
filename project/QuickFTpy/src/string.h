/*
 * string.h
 *
 * $Id: $
 * $HeadURL: $
 * $LastChangedRevision: $
 * $LastChangedDate: $
 * $LastChangedBy: $
 *
 */

#ifndef STRING_H
#define STRING_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

#include "macros.h"
#include "memory.h"

// Macros:
#define STR_REPLACE(s,f,r)         string_find_char_and_replace(s,f,r)
#define STR_TRIM(s)                string_trim(s)
#define STR_CMP_IC(s1,s2)          string_cmp_ignore_case(s1,s2)
#define STR_NCMP_IC(s1,s2,n)       string_ncmp_ignore_case(s1,s2,n)
#define STR_FMT_LENGTH(f,a)        string_va_list_length(f,a)
#define STR_DUP(s)                 string_dup(s,__FILE__,__LINE__)
#define STR_NDUP(s,n)              string_ndup(s,n,__FILE__,__LINE__)
#define STR_TOK_R(s,d,l)           string_token_r(s,d,l)
#define STR_TOLOWER(s)             string_tolower(s)
#define STR_TOUPPER(s)             string_toupper(s)
//
#define STR_CENTER_PADDING(t,s,b)  string_center_padding(t,s,0x20,b)
#define STR_LEFT_PADDING(t,s,b)    string_left_padding(t,s,0x20,b)
#define STR_RIGHT_PADDING(t,s,b)   string_right_padding(t,s,0x20,b)
//
#define STR_ITOA                   string_itoa
//
#define STR_APPEND(d,s)            string_append(d,s,__FILE__,__LINE__)
#define STR_FIND_CHAR              string_find_char
#define STR_COUNT_CHAR             string_count_char
#define STR_SEARCH                 string_search
#define STR_PARSE_LONG             string_parse_long
#define STR_HEX_TO_BIN             string_hex_to_binary
//
#define STR_ATOI_DEFAULT           string_atoi_default
#define STR_ATOL_DEFAULT           string_atol_default
#define STR_ATOB_DEFAULT           string_atob_default
#define STR_ATOF_DEFAULT           string_atof_default

/**
 * Extracts a binary number from a hex. buffer
 *
 * @param buffer                
 * @param buffer_len            
 *
 * @return                      TRUE o FALSE
 *
 */
int string_hex_to_binary(char* buffer, int buffer_len);

/**
 * Extracts long from a position in the string
 *
 * @param parsed_number
 * @param string
 * @param pos
 * @param base
 *
 * @return                           number of characters read or -1 on error
 */
int string_parse_long(long *parsed_number, char* string, size_t pos, int base);

/**
 * Finds first occurrence of a character in a string
 *
 * @param string      string to search
 * @param find        character to search for
 *
 * @return            position of the character in the string or -1
 */
int string_find_char(char* string, char find);

/**
 * Counts number of occurrences of a specified character
 *
 * @param string      string to search
 * @param find        char to count
 *
 * @return            number of occurrences or -1
 */
int string_count_char(char* string, char find);

/**
 * Replaces a character on the string
 *
 * @param string      string to work on
 * @param find        character to look for
 * @param replace     character to replace the previous
 *
 * @return            modified string
 */
char* string_find_char_and_replace(char* string, char find, char replace);

/**
 * Removes spaces and newlines at the beggining and end of a string
 *
 * @param s      string to trim
 *
 * @return       string with trim applied or NULL
 */
char* string_trim(char* s);

/**
 * Compares 2 strings and ignores cases
 *
 * @param s1     String 1
 * @param s2     String 2
 *
 * @return       0 if equal or -/+
 */
int string_cmp_ignore_case(const char* s1, const char* s2);

/**
 * Compares 2 strings and ignores cases
 * for a number of characters
 * 
 * @param s1     String 1
 * @param s2     String 2
 * @param n      Number of characters to compare
 *
 * @return       0 if equal or -/+
 */
int string_ncmp_ignore_case(const char* s1, const char* s2, size_t n);

/**
 * Returns the estimated message length for a specified
 * format/arg. list
 * 
 * @param fmt      
 * @param args     
 *
 * @return        
 */
int string_va_list_length(const char* fmt, va_list args);

/**
 * Duplicates a string and allocates space for the new string
 * 
 * @param s      string
 * @param file   (debug)
 * @param line   (debug)
 *
 * @return       duplicated string or NULL
 */
char* string_dup(const char* s, char *file, long line);

/**
 * Duplicates the indicated number of bytes and saves data to a new string
 * 
 * @param s      string
 * @param n      number of bytes to copy
 * @param file   (debug)
 * @param line   (debug)
 *
 * @return       string duplicado o NULL si s es nulo
 */
char* string_ndup(const char* s, size_t n, char *file, long line);

/**
 * strtok implementation with progress
 *
 * @param s        Points to the string from which to extract tokens.
 * @param delim    Points to a null-terminated set of delimiter characters.
 * @param last     Is a value-return parameter used by strtok_r() to record
 *                 its progress through s1.
 *
 * @return         a pointer to the next token in s1. If there are no
 *                 remaining tokens, it returns a null pointer.
 */
char* string_token_r(char *s, const char *delim, char **last);

/**
 * Turns a string to lowercase
 *
 * @param s      string to convert
 *
 * @return       converted string
 */
char* string_tolower(char* s);

/**
 * Turns a string to uppercase
 *
 * @param s      string to convert
 *
 * @return       converted string
 */
char* string_toupper(char* s);

/**
 * Completes a string with a given character and centers content
 *
 * @param s          String to work on
 * @param size       Final string size expected
 * @param fill       Character to fill spaces with
 * @param buffer     Return buffer
 *
 * @return           Return buffer
 */
char* string_center_padding(const char* s, int size, char fill, char* buffer);

/**
 * Completes a string with a given character adjust content to the left
 *
 * @param s          String to work on
 * @param size       Final string size expected
 * @param fill       Character to fill spaces with
 * @param buffer     Return buffer
 *
 * @return           Return buffer
 */
char* string_left_padding(const char* s, int size, char fill, char* buffer);

/**
 * Completes a string with a given character adjust content to the right
 *
 * @param s          String to work on
 * @param size       Final string size expected
 * @param fill       Character to fill spaces with
 * @param buffer     Return buffer
 *
 * @return           Return buffer
 */
char* string_right_padding(const char* s, int size, char fill, char* buffer);

/**
 * reverses a string
 *
 * @param begin
 * @param end
 */
void string_reverse(char* begin, char* end);

/**
 * Converts an integer value to a string
 *
 * @param value
 * @param buf
 * @param base
 *
 * @return
 */
char* string_itoa(int value, char* buffer, int base);

/**
 * Appends a string to another, re allocating as necessary
 * 
 * @param dest            string to append to
 * @param src             string to be appended
 * @param file            (debug)
 * @param line            (debug)
 *
 * @return                new string size or -1 on error
 *
 */
int string_append(char **dest, const char *src, char *file, long line);

/**
 * Searches a string within another string starting from a specified position
 *
 * @param string          string to search on
 * @param search          string to search for
 * @param index           start position on the string
 *
 * @return                position where the string was found or -1 on error
 */
int string_search(char *string, const char *search, size_t index);

/**
 * Converts ascii string to integer
 *
 * @param string          
 * @param value           default value
 *
 * @return                integer value
 */
int string_atoi_default(char* string, int value);

/**
 * Converts from ascii string to long
 * 
 * @param string          
 * @param value           return value
 *
 * @return                long long value
 */
long string_atol_default(char* string, long value);

/**
 * Converts ascii string to boolean numeric value
 * 
 * @param string          string
 * @param value           default value
 *
 * @return                TRUE or FALSE
 */
int string_atob_default(char* string, int value);

/**
 * Converts an ascii string to double
 *
 * @param string          
 * @param value           default value
 *
 * @return                double
 */
double string_atof_default(char* string, double value);

#ifdef __cplusplus
}
#endif

#endif  // STRING_H
