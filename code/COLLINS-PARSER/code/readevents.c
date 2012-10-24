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
#include "readevents.h"

#define U_CODE 3
#define D_CODE 2
#define F_CODE 6
#define G_CODE 4

void read_events_s(FILE *file,hash_table *hash);
void read_events_d(FILE *file,hash_table *hash);
void read_events_u(FILE *file,hash_table *hash);
void read_events_g(FILE *file,hash_table *hash);

int read_events_word(FILE *file);
int read_events_nt(FILE *file);

int read_events_word2(FILE *file);
int read_events_nt2(FILE *file);

void read_events(FILE *file,hash_table *hash,int max)
{
  int m=0;
  int code;

  while(fscanf(file,"%d",&code)!=EOF&&(m<max||max==-1))
    {
      m++;

/*      printf("MMM %d\n",m);*/

      if(((double) (m/100000))==(((double) m)/100000.0))
	fprintf(stderr,"Hash table: %d lines read\n",m);

      if(code==F_CODE)
	{
	  read_events_s(file,hash);
	}
      else if(code==D_CODE)
	{
	  read_events_d(file,hash);
	}
      else if(code==U_CODE)
	{
	  read_events_u(file,hash);
	}
      else if(code==G_CODE)
	{
	  read_events_g(file,hash);
	}
      else assert(0);
      
    }
}


void read_events_s(FILE *file,hash_table *hash)
{
  int i,n;
  char word[1000],tag[1000];
  int wn,tn;
  
  fscanf(file,"%d",&n);

  for(i=0;i<n;i++)
    {
      fscanf(file,"%s %s",word,tag);
      wn = find_word(word,&wordlex);

      if(!(wn>=0))
	{
	  fprintf(stderr,"ERROR: %s not found in lexicon\n",word);
	  assert(0);
	}
      tn = find_word(tag,&nt_lex);
      if(!(tn>=0))
	{
	  fprintf(stderr,"ERROR: %s not found in lexicon\n",tag);
	  assert(0);
	}

      /*finally add counts for the word/tag pair to the hash table
      add_tagword_entries2(wn,tn,hash);*/
      add_tagword_counts(wn,tn,hash);
    }
}

void read_events_d(FILE *file,hash_table *hash)
{
  /* read in the following values:

     wm/tm, wh/th are modifer word/tag, head word/tag 
     p,ch,cm are parent, head and modifier non-terminals
     cc = 1 if coordination, 0 otherwise
     punc = 1 if punctuation, 0 otherwise
     
     wcc/tcc, wpunc/tpunc are coordinator word/tag, punctuation word/tag

     subcat and dist are the subcat and distance strings
  */

  int wm,tm,wh,th,p,ch,cm,cc,wcc,tcc,punc,wpunc,tpunc;
  int subcat,dist;

  wm = read_events_word(file);
  tm = read_events_nt(file);

  wh = read_events_word(file);
  th = read_events_nt(file);

  cm = read_events_nt(file);
  p = read_events_nt(file);
  ch = read_events_nt(file);

  assert(fscanf(file,"%d",&subcat)!=EOF);

  assert(fscanf(file,"%d",&dist)!=EOF);

  assert(fscanf(file,"%d",&cc)!=EOF);

  if(cc)
    {
      wcc = read_events_word(file);
      tcc = read_events_nt(file);
    }

  assert(fscanf(file,"%d",&punc)!=EOF);

  if(punc)
    {
      wpunc = read_events_word(file);
      tpunc = read_events_nt(file);
    }

  if(cm == STOPNT) punc = 0;

  add_dependency_counts(wm,tm,cm,
			wh,th,
			p,ch,
			dist,subcat,
			cc,wcc,tcc,
			punc,wpunc,tpunc,
			hash);

}

void read_events_u(FILE *file,hash_table *hash)
{
  int wh,th,p,ch,lsubcat,rsubcat;

  wh=read_events_word(file);
  th=read_events_nt(file);

  p=read_events_nt(file);
  ch=read_events_nt(file);

  assert(fscanf(file,"%d",&lsubcat)!=EOF);
  assert(fscanf(file,"%d",&rsubcat)!=EOF);

  /*now add the counts*/

  add_unary_counts(ch,wh,th,p,hash);
  add_subcat_counts(lsubcat,ch,wh,th,p,0,hash);
  add_subcat_counts(rsubcat,ch,wh,th,p,1,hash);

/*  u.p = p;
  u.ch = ch;
  u.wh = wh;
  u.th = th;

  add_entries_U(&u,hash,HUR);
  add_entries_S(&u,hash,lsubcat,rsubcat);*/
}

void read_events_g(FILE *file,hash_table *hash)
{
  int wh,th,p,ch,gap;

  assert(fscanf(file,"%d",&gap)!=EOF);
  wh=read_events_word(file);
  th=read_events_nt(file);

  p=read_events_nt(file);
  ch=read_events_nt(file);

  /*now add the counts*/

  add_gap_counts(gap,ch,wh,th,p,hash);
}



int read_events_word(FILE *file)
{
  char buffer[1000];
  int w;

  assert(fscanf(file,"%s",buffer)!=EOF);
  if(strcmp(buffer,"#STOP#")==0)
    w = STOPWORD;
  else
    w = find_word(buffer,&wordlex);
  if(!(w>=0))
    {
      fprintf(stderr,"ERROR: %s not found in lexicon\n",buffer);
      assert(0);
    }

  return w;
}

int read_events_nt(FILE *file)
{
  char buffer[1000];
  int w;

  assert(fscanf(file,"%s",buffer)!=EOF);
  if(strcmp(buffer,"#STOP#")==0)
    w = STOPNT;
  else
    w = find_word(buffer,&nt_lex);

  if(!(w>=0))
    {
      fprintf(stderr,"ERROR: %s not found in lexicon\n",buffer);
      assert(0);
    }

  assert(w>=0);
  return w;

}

int read_events_word2(FILE *file)
{
  char buffer[1000];

  assert(fscanf(file,"%s",buffer)!=EOF);

  return 0;
}

int read_events_nt2(FILE *file)
{
  char buffer[1000];

  assert(fscanf(file,"%s",buffer)!=EOF);

  return 0;

}


