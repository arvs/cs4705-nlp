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

#ifndef SENTENCE_H
#define SENTENCE_H

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#include "grammar.h"

/*max number of words in a sentence*/
#define PMAXWORDS 120


#define NT_LRB 1
#define NT_RRB 2

/* the data structure for a sentence, the input to the parsing algorthm

   words = words in the sentence
   tags  = tags in the sentence
   nws   = number of words in the sentence

   wordnos/tagnos = input to the chart parser. Details are:

   1) wordnos/tagnos are integers corresponding to the words/tags,
      as found in the dictionaries wordlex and nt_lex

   2) wordnos/tagnos is sentence *without* punctuation

   3) nws_np is the number of wordnos/tagnos (stands for nws_no-puncutation)

   4) wordpos[i] maps wordnos/tagnos to words/tags i.e. wordpos[i] is the 
      position of the i'th word in wordnos in the words array

   5) commaats[i] == 1 if wordnos[i] has a word tagged as "," or ":" following
      it (in the original words array). So this is the only piece of 
      information that the parsing algorithm uses about punctuation

   6) commaats2[i] == 1 means that the comma at position i is used to
      discard some constituents from the chart (see section 2.7 of Collins 96) 
      Usually, commaats2[i] == commaats[i]. In this implementation there is
      one exception: if the comma occurs between parantheses it is not used
      to prune constituents from the chart

 */

typedef struct {  
  int wordnos[PMAXWORDS];
  int tagnos[PMAXWORDS];
  int wordpos[PMAXWORDS];
  int nws_np;

  char *words[PMAXWORDS];
  char *tags[PMAXWORDS];
  int nws;

  char commaats[PMAXWORDS];

  int commatags[PMAXWORDS];
  int commawords[PMAXWORDS];

  char commaats2[PMAXWORDS];

} sentence_type;

/* reads a sentence from the file. Assumes the format is

   n word_1 tag_1 ... word_n tag_n

   e.g.

   18 Pierre NNP Vinken NNP , , 61 CD years NNS old JJ , , will MD join VB 
   the DT board NN as IN a DT nonexecutive JJ director NN Nov. NNP 29 CD . .

*/

int read_sentence(FILE *file,sentence_type *sentence);

/* reads up to max sentences into an array starting at s. 
   Returns the number of sentences which have been read
   */

int read_sentences(FILE *file,sentence_type *s,int max);

/* used when the parser fails to return a parse, prints 

   (TOP word_1/tag_1 ... word_n/tag_n )*/

void print_noparse(sentence_type *s);

#endif
