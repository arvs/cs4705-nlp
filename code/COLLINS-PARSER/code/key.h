/* This code is the statistical natural language parser described in

   M. Collins. 1999.  Head-Driven
   Statistical Models for Natural Language Parsing. PhD Dissertation,
   University of Pennsylvania.

   Copyright (C) 1999 Michael Collins

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef KEY_H
#define KEY_H

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#include "mymalloc.h"
#include "mymalloc_char.h"

typedef struct {
  unsigned char *key;
  int klen;
} key_type;

int hashval(key_type *key,int size);

/*copies k2 to k1, allocates space for k1*/
void key_copy(key_type *k1,key_type *k2);

/*copies k2 to k1, _does not_ allocate space for k1*/
void key_copy2(key_type *k1,key_type *k2);

int key_equal(key_type *k1,key_type *k2);


#endif
