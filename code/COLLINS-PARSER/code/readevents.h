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

#ifndef READEVENTS_H
#define READEVENTS_H

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#include "prob.h"
#include "hash.h"
#include "grammar.h"


/* 

reads events from "file", stores them in hash table "hash"
reads at most max lines, or all lines from the file if max==-1

assumes word_lex, nt_lex hold word->integer and non-terminal -> integer 
mappings

assumes the following formats:

[1] sentence of word tag pairs   

6 n word_1 tag_1 word_2 tag_2 ... word_n tag_n 

e.g.
6 13 Takeover NN experts NNS said VBD they PRP doubted VBD the DT financier NN would MD make VB a DT bid NN by IN himself PRP

[2] unary events

3 word tag parent-nonterminal headchild-nonterminal left-subcat right-subcat

e.g.
3 said VBD TOP S 00000  00000 
3 said VBD S VP 10000  00000

[3] dependency events

2 mod-word mod-tag head-word head-tag mod-nt parent-nt head-nt subcat distance <coordination> <punctuation>

 where: distance = direction adjacency verb 
        e.g. 100 means to the left,  not adjacent, no verb
             001 means to the right, not adjacent, has a verb

        <coordination> = "0" or "1 word tag"
        <punctuation>  = "0" or "1 word tag"
*/

void read_events(FILE *file,hash_table *hash,int max);

#endif
