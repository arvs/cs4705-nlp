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

#ifndef GENPROBS_H
#define GENPROBS_H

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#include "hash.h"

/* add_counts:
   
   event is a string representing the event, in format

   <3 blank chars> output context

   olen is the number of chars in the output

   backoffs e.g. {4,6,4,3,2} means there are 4 levels, with 6,4,3 and 2
                 characters of the context respectively

   type is the type (e.g. dependency, subcat, unary etc.)
   different counts are added for each type
*/		 

void add_counts(unsigned char *event,int olen,int *backoffs,char type,hash_table *hash);

/*add_counts_level is slightly different, in that it adds all the counts at
  level "level"
*/

void add_counts_level(unsigned char *event,int olen,int *backoffs,int level,char type,hash_table *hash);

/*get_prob has same inputs, w1 and w2 are weight giving held-out mass for
  the lower level of backoff, backoff weight is

  lambda = d / (d + w1 + w2 *u)

  where d = denominator of more specific estimate, u is the number of unique
  outcomes in the more specific estimate (witten-bell smoothing)
*/

double get_prob(unsigned char *event,int olen,int *backoffs,char type,int w1,int w2,hash_table *hash);


#endif
