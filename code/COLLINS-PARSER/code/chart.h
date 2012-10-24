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

#ifndef CHART_H
#define CHART_H

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>


#include "edges.h"
#include "prob.h"
#include "prob_witheffhash.h"
#include "grammar.h"

/*ALLTAGS =1 means all tags in the tag dictionary are considered for each
  word, =0 means first best tag from the tagger is the only one considered*/
#define ALLTAGS 1


int PUNC_FLAG; /*1 if punctuation 
		 rule (see section 2.7 of Collins 96) is used*/

double BEAMPROB; /*size of the beam if pbest = highest prob of all edges for
		   a particular span, then all edges with 
		   logprob < log(pbest) - BEAMPROB are discarded*/

/* hash tables used by the probability models*/

hash_table new_hash;
effhash_table eff_hash;


/* a threshold: all edges must have prob greater than this threshold to be
   added to the chart 
   */
double pthresh;

/*print the entire chart*/
void print_chart();

/*print the highest prob tree spanning the entire sentence, rooted in the
  top symbol*/
int print_best_parse();

/*parse a sentence, print the output to stdout*/
void parse_sentence(sentence_type *sentence);

void set_treebankoutputflag(int flag);


#endif
