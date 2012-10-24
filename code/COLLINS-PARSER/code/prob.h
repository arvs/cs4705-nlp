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

#ifndef PROB_H
#define PROB_H

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#include "hash.h"
#include "genprob.h"
#include "grammar.h"

/* wm/tm/cm = modifer word/tag/non-terminal
   
   p=parent nt
   ch=head-child nt
   wh/th = head word/tag
   dist = distance variable
   subcat = subcat variable
*/

#define STOPWORD 50002
#define STOPNT 253

int DISTAFLAG; /*1 if adjacency condition used, 0 otherwise*/
int DISTVFLAG; /*1 if verb condition used, 0 otherwise*/


void add_dependency_counts(int wm,int tm,int cm,int wh,int th,int p,int ch,int dist,int subcat,int cc,int wcc,int tcc,int punc,int wpunc,int tpunc,hash_table *hash);

void add_tagword_counts(int wm,int tm,hash_table *hash);

double get_dependency_prob(int wm,int tm,int cm,int wh,int th,int p,int ch,int dist,int subcat,int cc,int wcc,int tcc,int punc,int wpunc,int tpunc,hash_table *hash);

void add_unary_counts(int ch,int wh,int th,int p,hash_table *hash);

double get_unary_prob(int ch,int wh,int th,int p,hash_table *hash);

/*subcat: dir=0 means left, dir=1 means right*/
void add_subcat_counts(int subcat,int ch,int wh,int th,int p,int dir,hash_table *hash);
double get_subcat_prob(int subcat,int ch,int wh,int th,int p,int dir,hash_table *hash);


/*prior probabilitiy of a ch,th,wh triple -- used in the search, nothing
  else

  counts for this are added when unary counts and dependency counts are added
*/
void add_prior_counts(int ch,int wh,int th,hash_table *hash);
double get_prior_prob(int ch,int wh,int th,hash_table *hash);

/*gap: dir=0 means left, dir=1 means right*/
void add_gap_counts(int gap,int ch,int wh,int th,int p,hash_table *hash);
double get_gap_prob(int gap,int ch,int wh,int th,int p,hash_table *hash);


/*make string for P(cm,t | context) */
void make_dep1_string(unsigned char *string,int wm,int tm,int cm,int wh,int th,int p,int ch,int dist,int subcat,int cc,int punc);

/*make string for P(wm | cm,t, context) */
void make_dep2_string(unsigned char *string,int wm,int tm,int cm,int wh,int th,int p,int ch,int dist,int subcat,int cc,int punc);

/*make string for P(ch | p,th,wh) */
void make_unary_string(unsigned char *string,int ch,int wh,int th,int p);

/*make string for P(ch,th | p==TOP) */
void make_s1_string(unsigned char *string,int ch,int wh,int th,int p);

/*make string for P(wh | ch,th,p==TOP) */
void make_s2_string(unsigned char *string,int ch,int wh,int th,int p);

/*make string for P(subcat | ch,th,p,dir) */
void make_subcat_string(unsigned char *string,int subcat,int ch,int wh,int th,int p,int dir);

/*make strings for P(ch,th | anything) P(wh | ch,th,anything)*/
void make_prior1_string(unsigned char *string,int ch,int wh,int th);
void make_prior2_string(unsigned char *string,int ch,int wh,int th);

void byte2_to_char(unsigned char *string,int n);
void byte3_to_char(unsigned char *string,int n);


/*make string for P(gap | wh,ch,th,p) */
void make_gap_string(unsigned char *string,int gap,int ch,int wh,int th,int p);

#endif

