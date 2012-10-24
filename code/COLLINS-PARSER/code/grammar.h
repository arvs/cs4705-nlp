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

#ifndef GRAMMAR_H
#define GRAMMAR_H

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#include "lexicon.h"

/* must include:

   tablep/tablef (triples of non-terminals possible)
   subcats
   unaries

   nt lexicon (nt -> integer)

   word lexicon (word -> integer)
   tag dictionary
   fwords: indication of which words appear infrequently enough to be replaced
           by "UNKNOWN"
   
   takes as input a file name, assumes:

   .grm = grammar (L,R,U,X,Y = left/right triples, unaries, left/right subcats)
   .lexicon = word tag fword
   .nts = list of non-terminals
*/

#define GMAXNTS 130      /*max number of non-terminals*/
#define GMAXWORDS 60000  /*max vocabulary size*/
#define GUNKNOWN 49999   /*special word reserved for "unknown" words*/

#define GMAXNTLETTERS 30 /*max number of letters in a non-terminal*/

/* now some specific non-terminals: this needs to be generalized but for now
   we'll list them
*/
#define PVB 45
#define PVBZ 50

#define NT_CC 5
#define NT_NP 22
#define NT_NPB 70
#define NT_TOP 43

#define NT_NPA 74
#define NT_SBARA 71
#define NT_SA 72
#define NT_VPA 73
#define NT_SGA 87

int TRACEWORD;
int TRACENT;
int TRACETAG;

/*a function that returns 1 if the tag is a verb*/
int isverb(int tag);


/* tablep[parent][head][modifier] is 1 if (parent, head, left-modifier) is a 
   valid triple

   tablef is the same but for the right modifiers
*/   

char tablep[GMAXNTS][GMAXNTS][GMAXNTS];
char tablef[GMAXNTS][GMAXNTS][GMAXNTS];


/* unary_nums[child] is number of non-terminals which can dominate child in a
   parent-head relationship,

   unaries[child] is the list of non-terminals which can dominate child
*/

int unary_nums[GMAXNTS];
char unaries[GMAXNTS][GMAXNTS];


/* subcat frames:
   a list of the subcat frames to the left and right,

   lsubcats_counts[parent][head] is number of left subcats for this parent/head
   lsubcats[parent][head][i] is the i'th left subcat frame

   rsubcats is the same but for right subcats
*/

#define MAXSUBCATS 50

int lsubcats[GMAXNTS][GMAXNTS][MAXSUBCATS];
int lsubcats_counts[GMAXNTS][GMAXNTS];

int rsubcats[GMAXNTS][GMAXNTS][MAXSUBCATS];
int rsubcats_counts[GMAXNTS][GMAXNTS];



/* lexicons for words and non-terminals */

lex_type wordlex;
lex_type nt_lex;

/*stores the non-terminal strings*/
char nts[GMAXNTS][GMAXNTLETTERS];
int numnts;

/* maps nt to nt without -g or -A flags */
int argmap[GMAXNTS];

/*1 if the NT has a -A flag*/
char hasarg[GMAXNTS];

/* maps nt to nt without -g flag */
int gapmap[GMAXNTS];

/*1 if the NT has a -g flag*/
char hasgap[GMAXNTS];


/* fwords[i] is 1 if the i'th word is a frequent word, 0 otherwise

   tagdict[word][tag] is 1 if tag is valid for word, 0 otherwise
*/

int fwords[GMAXWORDS];
char tagdict[GMAXWORDS][GMAXNTS];


/* reads a grammar from a file: assumes filename.grm, filename.lexicon, 
   filename.nts hold the grammar, lexicon and non-terminals respectively
*/

void read_grammar(char *filename);

#endif

