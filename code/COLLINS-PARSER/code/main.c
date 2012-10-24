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

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#include "lexicon.h"

#include "grammar.h"

#include "mymalloc.h"
#include "mymalloc_char.h"

#include "hash.h"

#include "prob.h"

#include "readevents.h"

#include "sentence.h"
#include "chart.h"

sentence_type sentences[2500];

int main(int argc, char *argv[])
{
  int s;
  int numsentences;
  FILE *words;
  char grammar[1000];
  char buffer[1000];
  float temp;
  int npflag;

  time_t g_time;
  time_t s_time;

  if(argc!=8) 
    {
      fprintf(stderr,"ERROR in command line, usage:\n cat countsfile | parser.out sentences-file grammarfile beamsize punctuation-flag distaflag distvflag npflag\n");
      return 0;
    }

  sscanf(argv[1],"%s",buffer);
  words=fopen(buffer,"r");
  assert(words!=NULL);

  sscanf(argv[2],"%s",grammar);

  sscanf(argv[3],"%f",&temp);
  BEAMPROB = log(temp);

  sscanf(argv[4],"%d",&PUNC_FLAG);

  sscanf(argv[5],"%d",&DISTAFLAG);
  sscanf(argv[6],"%d",&DISTVFLAG);
  sscanf(argv[7],"%d",&npflag);
  assert(npflag==0 || npflag==1);
  set_treebankoutputflag(npflag);

  mymalloc_init();
  mymalloc_char_init();

  hash_make_table(8000007,&new_hash);
  effhash_make_table(1000003,&eff_hash);

  read_grammar(grammar);

  numsentences=read_sentences(words,sentences,2500);

  fprintf(stderr,"NUMSENTENCES %d\n",numsentences);

  read_events(stdin,&new_hash,-1);

  for(s=0;s<numsentences;s++)
   {
     time(&g_time);

     pthresh = -5000000;

     parse_sentence(&sentences[s]);
     
/*     print_chart();*/
     time(&s_time);
     printf("TIME %d\n",(int) (s_time-g_time));
   }
  return 1;
}






