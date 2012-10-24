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

#ifndef EDGES_H
#define EDGES_H

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#include "grammar.h"

/* the basic context type

   nvs - number of verbs
   adj - 0/1 if head isn't/is at the end of the edge

   np/s/sbar/vp/other/gap are number of np/s/sbar/vp/other complements/gaps
   in the subcat bag
*/

typedef struct {
  char nvs;
  char adj;

  char np;
  char s;
  char sbar;
  char vp;
  char other;
  char gap;
} ctxt_type;


/* basic data structure for an edge


   head - an index into the sentence being parsed, identifying the head word

   headtag - the tag of the head word (the actual value, _not_ a pointer into
             the sentence)

   headch - an index into the edges,identifying edge which is the head. 
            if type=0, headch=-1

   prob - the probability of the edge
   prob2 - prob with added 'prior' prob cost

   child1 - index into childsarray, the first child of the non-terminal
   numchild - number of children

   next - next edge of same nt, start,end in the chart; we store a linked list
          of edges with the same non-terminal for efficiency

   label - non-terminal label of the edge
   headlabel - label of the head child (needed for dynamic programming)

   lc/rc - contexts to the left and right of the head

   hasverb - 1 if the edge contains a verb somewhere in its surface string
             (but not counting verbs within basenps)

   type - 0=POS tag, 1=non-terminal which is not a POS, 3 for a unary

   valid - 0/1 for false/true. Only a valid edge can be built on, a non-valid
           edge can be extended though

   stop - 0/1 if edge doesn't/does have the stop probability added

   inbeam - 2 means it's not yet calculate whether the edge is in the beam or
            not, 1/0 means it is/isn't in the beam (2 value is for efficiency
	    reasons, effectively we make sure the value of inbeam is only 
	    calculated once)
*/

typedef struct {
  int head;

  char headtag;

  int headch;

  int child1;
  char numchild;

  int next;

  double prob;
  double prob2;

  char label;
  ctxt_type lc;
  ctxt_type rc;

  int hasverb;

  char type;
  char valid;
  char stop;

  char headlabel;
  char inbeam;
} edge_type;

/*returns 1 if contexts are equal as far as dynamic programming is concerned*/
int equal_ctxts(ctxt_type *c1,ctxt_type *c2);

/*returns 1 if edges are equal as far as dynamic programming is concerned*/
int equal_edges(edge_type *e1,edge_type *e2);



/* calc_contexts is used when joining two edges in the chart, 
   e1 e2 -> e3
   to calculate the left and right contexts of the new edge e3

   l1,r1 l2,r2 are left and right contexts of the two edges being joined
   
   l3,r3 will be the left and right contexts of the edge resulting from the
         join
   
   head==1 if edge1 contributes the head, 2 if edge2 contributes the head

   verb==1 if the modifying edge has a verb as its head 

   modnt is the label of the modifying non-terminal, used to calculate the
   subcat frame of the new edge (any complements must be removed from the
   subcat requirements)

 */


void calc_contexts(ctxt_type *l1,ctxt_type *r1,ctxt_type *l2,ctxt_type *r2,ctxt_type *l3,ctxt_type *r3,char head,char verb,int modnt);

/* onectxt_to_dist_subcat converts a context to a distance and subcat value
   which can be used in probability calculations

   l is the subcat frame
   drc is 1 if direction is preceding, 0 for following

   this function converts l into the distance "dist" and "subcat" value
   given the context and direction

*/


void onectxt_to_dist_subcat(ctxt_type *l,int *dist,int *subcat,char drc);



/* puts all the subcat values to 0 for c*/
void clear_args(ctxt_type *c);

/* converts an integer subcat value "sub" to a context "c" */
void find_args(ctxt_type *c,int sub);


#endif
