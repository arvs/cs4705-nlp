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

#include <assert.h>

#include "key.h"

int key_equal(key_type *k1,key_type *k2)
{
  int i;

  if(k1->klen!=k2->klen)
    return 0;

  for(i=0;i<k1->klen;i++)
    if(k1->key[i] != k2->key[i])
      return 0;
  
  return 1;
}

void key_copy(key_type *k1,key_type *k2)
{
  int i;

  k1->klen = k2->klen;

  k1->key = (unsigned char *) mymalloc_char(k2->klen);

  for(i=0;i<k1->klen;i++)
    k1->key[i] = k2->key[i];
}

void key_copy2(key_type *k1,key_type *k2)
{
  int i;

  k1->klen = k2->klen;

  for(i=0;i<k1->klen;i++)
    k1->key[i] = k2->key[i];
}

int hashval(key_type *key,int size)
{
  int i;
  unsigned int val;

  val = key->key[0];

  for(i=1;i<key->klen;i++)
    val = (val*256 + key->key[i]) % size;

  return val;
}








