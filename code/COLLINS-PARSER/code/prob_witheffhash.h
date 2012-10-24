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

#ifndef PROB_WITHEFFHASH_H
#define PROB_WITHEFFHASH_H

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>


#include "hash.h"
#include "effhash.h"
#include "grammar.h"
#include "sentence.h"

#include "prob.h"

/* wm/tm/cm = modifer word/tag/non-terminal
   
   p=parent nt
   ch=head-child nt
   wh/th = head word/tag
   dist = distance variable
   subcat = subcat variable
*/

double get_dependency_prob_witheffhash(int wm,int tm,int cm,int wh,int th,int p,int ch,int dist,int subcat,int cc,int wcc,int tcc,int punc,int wpunc,int tpunc,hash_table *hash,effhash_table *effhash);


double get_unary_prob_witheffhash(int ch,int wh,int th,int p,hash_table *hash,effhash_table *effhash);

/*subcat: dir=0 means left, dir=1 means right*/
double get_subcat_prob_witheffhash(int subcat,int ch,int wh,int th,int p,int dir,hash_table *hash,effhash_table *effhash);

double get_gap_prob_witheffhash(int gap,int ch,int wh,int th,int p,hash_table *hash,effhash_table *effhash);


/*hashes prior prob for word,tag,ch (where word,tag are indexes into the
  _sentence_)
*/
double get_prior_prob_witheffhash(int ch,int wh,int th,hash_table *hash,int word,int tag);

void reset_prior_hashprobs();

#endif
