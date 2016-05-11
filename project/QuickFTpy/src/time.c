/*
 * time.c
 *
 * $Id: $
 * $HeadURL: $
 * $LastChangedRevision: $
 * $LastChangedDate: $
 * $LastChangedBy: $
 *
 */
 
#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <time.h>

#include "time.h"
#include "logger.h"

 /**
 * For (some) compatibility with Win32
 * @WARNING: not the same output!
 */
unsigned long GetTickCount() {
    
  time_t now;
  time(&now);
  return (unsigned long) now;
}