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

#ifndef EFFHASH_H
#define EFFHASH_H

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#include "key.h"

#define MAXEFF_S 250

typedef struct effhash_list {
  double prob;
  key_type key;
  unsigned char s;
} effhash_node;

typedef struct {
  int num;
  int size;
  effhash_node *table;
  unsigned char s;
} effhash_table;


void effhash_make_table(int size,effhash_table *effhash);
void effhash_clear_table(effhash_table *effhash);
void effhash_newsent(effhash_table *effhash);

/*looks up a probability in the hash table, 
  flag=1 if it finds it, 0 otherwise*/
double eff_findprob(key_type *key,effhash_table *effhash,int *flag);

/*adds prob for key to the hash table*/
void eff_addprob(key_type *key,effhash_table *effhash,double prob);

#endif
