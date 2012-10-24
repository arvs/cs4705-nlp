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

#ifndef HASH_H
#define HASH_H

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>


#include "key.h"

typedef struct hash_list {
  struct hash_list *next;
  int count;
  key_type key;
} hash_node;

typedef struct {
  int num;
  int size;
  hash_node **table;
} hash_table;

/*assumes the following functions on key_type:

  hashval(key_type *key,int size) function from key -> value 0<=value<size
  key_copy(key_type *k1,key_type *k2) copies k2 to k1
  key_equal(key_type *k1,key_type *k2) returns 1 if the keys are equal, 0
                                       otherwise
*/


/*adds count to the count for key (creating new element if not key is not
  already in the hash table)

  returns the count for key after the increment
*/
int hash_add_element(key_type *key,hash_table *hash,int count);


/*returns the count for key in hash table (0 if not there)*/
int hash_find_element(key_type *key,hash_table *hash);

/*makes a hash table with size elements*/
void hash_make_table(int size,hash_table *hash);

#endif
