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
#include "prob_witheffhash.h"

#define DEFFTYPE 0
#define DEFFLEN 21

#define UEFFTYPE 1
#define UEFFLEN 8

#define SCEFFTYPE 2
#define SCEFFLEN 12

#define GEFFTYPE 3
#define GEFFLEN 9

/*array for caching prior probabilities*/
double prior_hashprobs[PMAXWORDS][GMAXNTS][GMAXNTS];


/* wm/tm/cm = modifer word/tag/non-terminal
   
   p=parent nt
   ch=head-child nt
   wh/th = head word/tag
   dist = distance variable
   subcat = subcat variable
*/

void make_alldep_string(char *string,int wm,int tm,int cm,int wh,int th,int p,int ch,int dist,int subcat,int cc,int wcc,int tcc,int punc,int wpunc,int tpunc);

double get_dependency_prob_witheffhash(int wm,int tm,int cm,int wh,int th,int p,int ch,int dist,int subcat,int cc,int wcc,int tcc,int punc,int wpunc,int tpunc,hash_table *hash,effhash_table *effhash)
{
  key_type key;
  unsigned char buffer[1000];
  int flag;
  double prob;

  key.key = buffer;
  key.klen = DEFFLEN;

  make_alldep_string(buffer,wm,tm,cm,wh,th,p,ch,dist,subcat,cc,wcc,tcc,punc,wpunc,tpunc);
  buffer[0] = DEFFTYPE;
  buffer[1] = 0;
  buffer[2] = 0;

  prob = eff_findprob(&key,effhash,&flag);

/*  printf("DEP %d %d %d %d %d %d %d %d %d %d %d ",
         wm,tm,cm,
         wh,th,p,
         ch,dist,subcat,cc,punc);

  if(flag) printf("%g\n",prob);*/

  if(flag) return prob;

  prob = get_dependency_prob(wm,tm,cm,wh,th,p,ch,dist,subcat,cc,wcc,tcc,punc,wpunc,tpunc,hash);
  eff_addprob(&key,effhash,prob);
/*  printf("%g\n",prob);*/
  return prob;
}


double get_unary_prob_witheffhash(int ch,int wh,int th,int p,hash_table *hash,effhash_table *effhash)
{
  key_type key;
  unsigned char buffer[1000];
  int flag;
  double prob;

  key.key = buffer;
  key.klen = UEFFLEN;

  make_unary_string(buffer,ch,wh,th,p);
  buffer[0] = UEFFTYPE;
  buffer[1] = 0;
  buffer[2] = 0;

  prob = eff_findprob(&key,effhash,&flag);

  if(flag) return prob;

  prob = get_unary_prob(ch,wh,th,p,hash);

  eff_addprob(&key,effhash,prob);

  return prob;
}

/*subcat: dir=0 means left, dir=1 means right*/
double get_subcat_prob_witheffhash(int subcat,int ch,int wh,int th,int p,int dir,hash_table *hash,effhash_table *effhash)
{
  key_type key;
  unsigned char buffer[1000];
  int flag;
  double prob;

  key.key = buffer;
  key.klen = SCEFFLEN;

  make_subcat_string(buffer,subcat,ch,wh,th,p,dir);
  buffer[0] = SCEFFTYPE;
  buffer[1] = 0;
  buffer[2] = 0;

  prob = eff_findprob(&key,effhash,&flag);

  if(flag) return prob;

  prob = get_subcat_prob(subcat,ch,wh,th,p,dir,hash);
  eff_addprob(&key,effhash,prob);
  return prob;
}

double get_gap_prob_witheffhash(int gap,int ch,int wh,int th,int p,hash_table *hash,effhash_table *effhash)
{
  key_type key;
  unsigned char buffer[1000];
  int flag;
  double prob;

  key.key = buffer;
  key.klen = GEFFLEN;

  make_gap_string(buffer,gap,ch,wh,th,p);
  buffer[0] = GEFFTYPE;
  buffer[1] = 0;
  buffer[2] = 0;

  prob = eff_findprob(&key,effhash,&flag);

  if(flag) return prob;

  prob = get_gap_prob(gap,ch,wh,th,p,hash);
  eff_addprob(&key,effhash,prob);
  return prob;
}

double get_prior_prob_witheffhash(int ch,int wh,int th,hash_table *hash,int word,int tag)
{
  double prob;

  prob=prior_hashprobs[word][tag][ch];

  if(prob<0.1) return prob;

  prob = get_prior_prob(ch,wh,th,hash);

  prior_hashprobs[word][tag][ch] = prob;

  return (prob);
}

void reset_prior_hashprobs()
{
  int i,j,k;

  for(i=0;i<PMAXWORDS;i++)
    for(j=0;j<GMAXNTS;j++)
      for(k=0;k<GMAXNTS;k++)
	prior_hashprobs[i][j][k]=10;
}

void make_alldep_string(char *string,int wm,int tm,int cm,int wh,int th,int p,int ch,int dist,int subcat,int cc,int wcc,int tcc,int punc,int wpunc,int tpunc)
{

  
  byte2_to_char(&string[3],wm);

  string[5] = (char) tm;
  string[6] = (char) cm;
  string[7] = (char) punc;
  string[8] = (char) cc;

  string[9] = (char) p;
  string[10] = (char) ch;
  string[11] = (char) dist;
  byte3_to_char(&string[12],subcat);
  string[15] = (char) th;
  byte2_to_char(&string[16],wh);

  if(cc)
    {
      byte2_to_char(&string[18],wcc);
      string[20] = (char) tcc;
    }
  else
    {
      string[18]=string[19]=string[20]=0;
    }

  if(punc)
    {
      byte2_to_char(&string[18],wpunc);
      string[20] = (char) tpunc;
    }
  else
    {
      string[18]=string[19]=string[20]=0;
    }

}
