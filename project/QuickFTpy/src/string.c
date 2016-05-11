/*
 * string.c
 *
 * $Id: $
 * $HeadURL: $
 * $LastChangedRevision: $
 * $LastChangedDate: $
 * $LastChangedBy: $
 *
 */

#include "string.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>

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
int string_parse_long(long *parsed_number, char* string, size_t pos, int base) {

  char *endptr;
  char *endpos;
  long number;

  if (string && parsed_number && (base == 0 || (base >= 2 && base <= 36))) {

    if (pos >= strlen(string)) {
      return -1;
    }

    errno = 0;
    number = strtol(string + pos, &endptr, base);
    endpos = endptr;

    if (errno == ERANGE) {
      return -1;
    }
    if (endpos == string + pos) {
      errno = EINVAL;
      return -1;
    }

    *parsed_number = number;
    return endpos - string;

  }

  return -1;

}

/**
 * Extracts a binary number from a hex. buffer
 *
 * @param buffer                
 * @param buffer_len            
 *
 * @return                      TRUE o FALSE
 *
 */
int string_hex_to_binary(char* buffer, int buffer_len) {

  long i;
  unsigned char *p;

  if (buffer && buffer_len > 0) {

    for (i = 0; i < buffer_len; i++) {
      if (!isxdigit(buffer[i])) return FALSE;
    }

    for (i = 0, p = (unsigned char*)buffer; i < buffer_len; i++, p++) {

      if (*p >= '0' && *p <= '9') {
        *p -= '0';
      }
      else if (*p >= 'a' && *p <= 'f') {
        *p = *p - 'a' + 10;
      }
      else if (*p >= 'A' && *p <= 'F') {
        *p = *p - 'A' + 10;
      }
      else {
        *p = 0;
      }

    }

    // De-hexing will compress data by factor of 2
    for (i = 0; i < (buffer_len/2); i++) {
      buffer[i] = buffer[i * 2] * 16 | buffer[i * 2 + 1];
    }

    buffer[(buffer_len/2)] = '\0';

    return TRUE;

  }

  return FALSE;

}

/**
 * Finds first occurrence of a character in a string
 *
 * @param string      string to search
 * @param find        character to search for
 *
 * @return            position of the character in the string or -1
 */
int string_find_char(char* string, char find) {

  char* temp;
  int pos = 0;
  int ret = -1;

  temp = string;
  while (*temp) {
    if (*temp == find) {
      ret = pos;
      break;
    }
    temp++;
    pos++;
  }

  return ret;

}

/**
 * Counts number of occurrences of a specified character
 *
 * @param string      string to search
 * @param find        char to count
 *
 * @return            number of occurrences or -1
 */
int string_count_char(char* string, char find) {

  char* temp;
  int count = 0;

  temp = string;
  while (*temp) {
    if (*temp == find) {
      count++;
    } else {
      break;
    }
    temp++;
  }

  return count;

}

/**
 * Replaces a character on the string
 *
 * @param string      string to work on
 * @param find        character to look for
 * @param replace     character to replace the previous
 *
 * @return            modified string
 */
char* string_find_char_and_replace(char* string, char find, char replace) {

  char* temp;

  temp = string;
  while (*temp) {
    if (*temp == find)
      *temp = replace;
    temp++;
  }

  return string;

}

/**
 * Removes spaces and newlines at the beggining and end of a string
 *
 * @param s      string to trim
 *
 * @return       string with trim applied or NULL
 */
char* string_trim(char* s) {

  unsigned int len = strlen(s);
  unsigned int i = 0;

  if (len == 0) {
    return s;
  }

  for (i = 0; i < len && isspace(s[i]); i++); // nothing
  if (i > 0) {
    memmove(s, &s[i], strlen(&s[i]) + 1);
  }

  if ((len = strlen(s)) == 0) {
    return s;
  }

  i = len;
  while (s[i - 1] == '\r' || s[i - 1] == '\n' || isspace(s[i - 1])) {
    s[i - 1] = '\0';
    i--;
  }

  return s;

}

/**
 * Compares 2 strings and ignores cases
 *
 * @param s1     String 1
 * @param s2     String 2
 *
 * @return       0 if equal or -/+
 */
int string_cmp_ignore_case(const char* s1, const char* s2) {

  for (;; s1++, s2++) {
    register char c1 = tolower(*s1);
    register char c2 = tolower(*s2);
    if (c1 != c2 || c1 == 0) {
      return (c1 - c2);
    }

  }
  return 0;

}

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
int string_ncmp_ignore_case(const char* s1, const char* s2, size_t n) {

  for (; n-- > 0; s1++, s2++) {
    register char c1 = tolower(*s1);
    register char c2 = tolower(*s2);
    if (c1 != c2 || c1 == 0) {
      return (c1 - c2);
    }
  }
  return 0;

}

/**
 * Returns the estimated message length for a specified
 * format/arg. list
 * 
 * @param fmt      
 * @param args     
 *
 * @return        
 */
int string_va_list_length(const char* fmt, va_list args) {

  char *parameter_buffer;
  int i,n, width;

  if (fmt == NULL) {
    return 0;
  }

  n = strlen(fmt);
  for(i = 0; fmt[i]; i++) {

    if (fmt[i]=='%') {

      i++;

      if (fmt[i]=='%') {
        continue;
      }

      width = atoi(fmt + i);
      if (width < 0) {
        width=-width;
      }

      while(!isalpha(fmt[i])) {
        i++;
      }

      switch (fmt[i]) {

        case 's':

          parameter_buffer = va_arg(args, char*);
          if (width && width > (int)strlen(parameter_buffer)) {
            n+=width;
          } else {
            n+=strlen(parameter_buffer);
          }
          break;

        default:

          (void)va_arg(args, void*);
          if (width && width > 16) {
            n += width;
          } else {
            n += 16;
          }

        }

    }

  }

  return n+1;

}

/**
 * Duplicates a string and allocates space for the new string
 * 
 * @param s      string
 * @param file   (debug)
 * @param line   (debug)
 *
 * @return       duplicated string or NULL
 */
char* string_dup(const char* s, char *file, long line) {

  char* r = NULL;

  if (s != NULL) {

    r = (char*)malloc(strlen(s)+1);
    strcpy(r, s);

  }

  return r;

}

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
char* string_ndup(const char* s, size_t n, char *file, long line) {

  char* r = NULL;

  if (s != NULL) {

    r = (char*)malloc(n + 1);
    strncpy(r, s, n);
    r[n] = 0x00;

  }

  return r;

}


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
char* string_token_r(char *s, const char *delim, char **last) {

  char* p;

  if (s == NULL)
    s = *last;

  if (s == NULL)
    return NULL;

  if (*s == '\0') {
    *last = NULL;
    return NULL;
  }

  p = s;
  while (*p && !strchr(delim, *p))
    p++;

  if (*p == '\0')
    *last = NULL;
  else {
    *p = '\0';
    p++;
    *last = p;
  }

  return s;

}

/**
 * Turns a string to lowercase
 *
 * @param s      string to convert
 *
 * @return       converted string
 */
char* string_tolower(char* s) {

  char* temp;

  temp = s;
  while (*temp) {
    *temp = tolower(*temp);
    temp++;
  }
  *temp = 0x00;
  return s;

}

/**
 * Turns a string to uppercase
 *
 * @param s      string to convert
 *
 * @return       converted string
 */
char* string_toupper(char* s) {

  char* temp;

  temp = s;
  while (*temp) {
    *temp = toupper(*temp);
    temp++;
  }
  *temp = 0x00;
  return s;

}

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
char* string_center_padding(const char* s, int size, char fill, char* buffer) {

  buffer[0] = 0x00;

  if (strlen(s) < (size_t)size) {

    // L=(W-S)/2 padding spaces on the left, and R=(W-S)-L padding spaces
    int pad = (int) ((size - strlen(s)) / 2);
    if (pad*2 != (size + (int)strlen(s))) {
      pad++; // fix decimals
    }
    memset(buffer, fill, size);
    memcpy(&buffer[pad-1], s, strlen(s));

  } else {
    strcpy(buffer, s);
  }

  buffer[size] = 0x00;

  return buffer;

}

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
char* string_left_padding(const char* s, int size, char fill, char* buffer) {

  buffer[0] = 0x00;

  if (strlen(s) < (size_t)size) {
    memset(buffer, fill, size);
    memcpy(&buffer[size-strlen(s)], s, strlen(s));
  } else {
    strcpy(buffer, s);
  }

  buffer[size] = 0x00;

  return buffer;

}

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
char* string_right_padding(const char* s, int size, char fill, char* buffer) {

  buffer[0] = 0x00;
  strcpy(buffer, s);

  if (strlen(buffer) < (size_t)size) {
    memset(&buffer[strlen(buffer)], fill, (size-strlen(buffer)));
  }

  buffer[size] = 0x00;

  return buffer;

}

/**
 * reverses a string
 *
 * @param begin
 * @param end
 */
void string_reverse(char* begin, char* end) {

  char aux;
  while (end > begin)
    aux = *end, *end-- = *begin, *begin++ = aux;

}

/**
 * Converts an integer value to a string
 *
 * @param value
 * @param buf
 * @param base
 *
 * @return
 */
char* string_itoa(int value, char* buffer, int base) {

  static char num[] = "0123456789abcdefghijklmnopqrstuvwxyz";
  char* wstr = buffer;
  int sign;
  div_t res;

  // Validate base
  if (base < 2 || base > 35) {
    *buffer = '\0';
    return buffer;
  }

  // Take care of sign
  if ((sign = value) < 0) value = -value;

  // Conversion. Number is reversed.
  do {

    res = div(value, base);
    *wstr++ = num[res.rem];

  } while ((value = res.quot));

  if (sign < 0) *wstr++ = '-';
  *wstr = '\0';

  // Reverse string
  string_reverse(buffer, wstr - 1);

  return buffer;

}

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
int string_append(char **dest, const char *src, char *file, long line) {

  int ret = -1;
  int dest_len = 0;
  int src_len = 0;

  if (src) {

    src_len = strlen(src);
    if (src_len > 0) {

      if ((*dest) == NULL) {
        (*dest) = (char*)malloc(src_len + 1);
      }
      else {

        dest_len = strlen((*dest));

        (*dest) = (char*)realloc((*dest), src_len + dest_len + 1);

      }

      memcpy((*dest) + dest_len, src, src_len);
      (*dest)[dest_len + src_len] = '\0';

      ret = strlen((*dest));

    }

  }

  return ret;

}

/**
 * Searches a string within another string starting from a specified position
 *
 * @param string          string to search on
 * @param search          string to search for
 * @param index           start position on the string
 *
 * @return                position where the string was found or -1 on error
 */
int string_search(char *string, const char *search, size_t index) {

  int ret = -1;

  if (string && search && strlen(string) > index) {

    int pos;

    while ((pos = string_find_char(string + index, search[0])) != -1) {

      index += pos;

      if (string_ncmp_ignore_case(string + index, search, strlen(search)) == 0) {
        ret = index;
        break;
      }

      index++;

    }

  }

  return ret;

}

/**
 * Converts ascii string to integer
 *
 * @param string          
 * @param value           default value
 *
 * @return                integer value
 */
int string_atoi_default(char* string, int value) {

  int ret = value;
  if (string && strlen(string) > 0) {
    ret = atoi(string);
  }
  return ret;

}

/**
 * Converts from ascii string to long
 * 
 * @param string          
 * @param value           return value
 *
 * @return                long long value
 */
long string_atol_default(char* string, long value) {

  long ret = value;
  if (string && strlen(string) > 0) {
    ret = atol(string);
  }
  return ret;

}

/**
 * Converts ascii string to boolean numeric value
 * 
 * @param string          string
 * @param value           default value
 *
 * @return                TRUE or FALSE
 */
int string_atob_default(char* string, int value) {

  int ret = value;
  if (string && strlen(string) > 0) {
    ret = FALSE;
    if (string_cmp_ignore_case(string, "true") == 0) {
      ret = TRUE;
    }
  }
  return ret;

}

/**
 * Converts an ascii string to double
 *
 * @param string          
 * @param value           default value
 *
 * @return                double
 */
double string_atof_default(char* string, double value) {

  double ret = value;
  if (string && strlen(string) > 0) {
    ret = atof(string);
  }
  return ret;

}
