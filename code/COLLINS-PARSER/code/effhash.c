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

#include "effhash.h"

#define MAXEFFKEYLEN 25

int effhash_find_element(key_type *key,effhash_table *effhash);

void effhash_make_table(int size,effhash_table *effhash)
{
  int i;
  effhash->table=(effhash_node *) malloc(size*sizeof(effhash_node));
  effhash->num=0;
  effhash_clear_table(effhash);
  effhash->size=size;

  for(i=0;i<size;i++)
    {
      effhash->table[i].key.key = (unsigned char *) mymalloc_char(MAXEFFKEYLEN);
/*      effhash->table[i].key.key = (char *) malloc(MAXEFFKEYLEN*sizeof(char));*/
      effhash->table[i].key.klen = 0;
    }
}

int effhash_find_element(key_type *key,effhash_table *effhash)
{
  int pos;

  pos=hashval(key,effhash->size);

  while(effhash->s==effhash->table[pos].s&&!key_equal(&(effhash->table[pos].key),key))
    {
      pos++;
      if(pos>=effhash->size)
	pos=0;
    }

  return pos;
}

void effhash_clear_table(effhash_table *effhash)
{
  int i;
  for(i=0;i<effhash->size;i++)
    effhash->table[i].s=0;
  effhash->s=1;
}

void effhash_newsent(effhash_table *effhash)
{
  effhash->num=0;
  if(effhash->s==MAXEFF_S)
    effhash_clear_table(effhash);
  else
    (effhash->s)++;
}

double eff_findprob(key_type *key,effhash_table *effhash,int *flag)
{
  int pos;

  pos=effhash_find_element(key,effhash);

  if(effhash->table[pos].s!=effhash->s)
    {
      *flag = 0;
      return 0;
    }
 
  *flag = 1;
  return effhash->table[pos].prob;
}


void eff_addprob(key_type *key,effhash_table *effhash,double prob)
{
  int pos;

  pos=effhash_find_element(key,effhash);

  if(effhash->table[pos].s!=effhash->s)
    {
      if(effhash->num<(effhash->size/2))
	{	
	  effhash->table[pos].s=effhash->s;
	  key_copy2(&effhash->table[pos].key,key);
	  effhash->table[pos].prob=prob;
	  (effhash->num)++;
	}
    }
}





