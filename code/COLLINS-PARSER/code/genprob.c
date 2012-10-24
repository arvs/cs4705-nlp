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

#include "genprob.h"

#define BONTYPE 0 /*numerators sub-type*/
#define BODTYPE 1 /*denominators*/
#define BOUTYPE 2 /*unique outcomes count*/

#define PROBSMALL 0.0000000000000000001

void add_counts(unsigned char *event,int olen,int *backoffs,char type,hash_table *hash)
{
  int i;
  key_type key;
  unsigned char buffer[1000];
  int len; /*total length of the input string*/
  int ns[100];

  len = 3+olen+backoffs[1];
  key.key = buffer;

  for(i=0;i<len;i++)
    buffer[i] = event[i];


  /*first add the numerators*/

  assert(backoffs[0]<100);

  buffer[0] = type;
  buffer[1] = BONTYPE;
  for(i=1;i<=backoffs[0];i++)
    {
      buffer[2] = i;
      key.klen = 3+olen+backoffs[i];
      ns[i] = hash_add_element(&key,hash,1);
    }

  /*now the unique counts*/

  key.key = buffer+olen;
  buffer[olen] = type;
  buffer[olen+1] = BOUTYPE;
  for(i=1;i<=backoffs[0];i++)
    {
      if(ns[i] == 1)
	{
	  buffer[olen+2] = i;
	  key.klen = 3+backoffs[i];
	  hash_add_element(&key,hash,1);
	}
    }
  
  /*now the denominators*/

  key.key = buffer+olen;
  buffer[olen] = type;
  buffer[olen+1] = BODTYPE;
  for(i=1;i<=backoffs[0];i++)
    {
      buffer[olen+2] = i;
      key.klen = 3+backoffs[i];
      hash_add_element(&key,hash,1);
    }
}

void add_counts_level(unsigned char *event,int olen,int *backoffs,int level,char type,hash_table *hash)
{
  int i;
  key_type key;
  unsigned char buffer[1000];
  int len; /*total length of the input string*/
  int ns[100];

  len = 3+olen+backoffs[1];
  key.key = buffer;

  for(i=0;i<len;i++)
    buffer[i] = event[i];


  /*first add the numerators*/

  assert(backoffs[0]<100);

  buffer[0] = type;
  buffer[1] = BONTYPE;
  for(i=1;i<=backoffs[0];i++)
    {
      buffer[2] = level;
      key.klen = 3+olen+backoffs[i];
      ns[i] = hash_add_element(&key,hash,1);
    }

  /*now the unique counts*/

  key.key = buffer+olen;
  buffer[olen] = type;
  buffer[olen+1] = BOUTYPE;
  for(i=1;i<=backoffs[0];i++)
    {
      if(ns[i] == 1)
	{
	  buffer[olen+2] = level;
	  key.klen = 3+backoffs[i];
	  hash_add_element(&key,hash,1);
	}
    }

  /*now the denominators*/

  key.key = buffer+olen;
  buffer[olen] = type;
  buffer[olen+1] = BODTYPE;
  for(i=1;i<=backoffs[0];i++)
    {
      buffer[olen+2] = level;
      key.klen = 3+backoffs[i];
      hash_add_element(&key,hash,1);
    }
}


double get_prob(unsigned char *event,int olen,int *backoffs,char type,int w1,int w2,hash_table *hash)
{
  int i;
  key_type key;
  unsigned char buffer[1000];
  int len; /*total length of the input string*/
  int ns[100],us[100],ds[100]; /*counts for numerators, denominators, uniques at
			     each level. Assumes that level 1 is most specific
			   */
  double prob;
  int bo;

  len = 3+olen+backoffs[1];
  key.key = buffer;

  for(i=0;i<len;i++)
    buffer[i] = event[i];

  /*first get the numerators*/

  assert(backoffs[0]<100);

  buffer[0] = type;
  buffer[1] = BONTYPE;
  for(i=1;i<=backoffs[0];i++)
    {
      buffer[2] = i;
      key.klen = 3+olen+backoffs[i];
      ns[i] = hash_find_element(&key,hash);
    }

  /*now the unique counts*/

  key.key = buffer+olen;
  buffer[olen] = type;
  buffer[olen+1] = BOUTYPE;
  for(i=1;i<=backoffs[0];i++)
    {
      buffer[olen+2] = i;
      key.klen = 3+backoffs[i];
      us[i] = hash_find_element(&key,hash);
    }

  /*now the denominators*/

  key.key = buffer+olen;
  buffer[olen] = type;
  buffer[olen+1] = BODTYPE;
  for(i=1;i<=backoffs[0];i++)
    {
      buffer[olen+2] = i;
      key.klen = 3+backoffs[i];
      ds[i]=hash_find_element(&key,hash);
    }

/*  for(i=backoffs[0];i>=1;i--)
    printf("BB %d %d %d %d %d\n",(int) type,i,ns[i],ds[i],us[i]);*/

/*  if(ds[backoffs[0]] <= 0.1 || ns[backoffs[0]] <= 0.1)
    return 1.0/10000.0;*/

  if(ds[backoffs[0]] <= 0.1)
    return PROBSMALL;

  assert( ds[backoffs[0]] > 0.1);
/*  assert( ns[backoffs[0]] > 0.1);*/
  assert( us[backoffs[0]] > 0.1);

  prob = PROBSMALL;

/*  prob = (double) ns[backoffs[0]] / ds[backoffs[0]];*/

  for(i=backoffs[0];i>=1;i--)
    {
      bo = w1 + w2*us[i];
      if(ds[i] > 0.1)
	prob = (bo*prob + ns[i]) /( (double) (bo + ds[i]));
    }

  return prob;
}



