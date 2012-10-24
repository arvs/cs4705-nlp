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
#include "sentence.h"

void convert_sentence(sentence_type *sentence);

int read_sentence(FILE *file,sentence_type *sentence)
{
  int i;
  char wbuf[100],tbuf[100];
  int lw,lt;
  
  if(fscanf(file,"%d",&sentence->nws)==EOF)
      return 0;

  assert(sentence->nws<PMAXWORDS);

  for(i=0;i<sentence->nws;i++)
    {
      fscanf(file,"%s %s ",wbuf,tbuf);
      
      sentence->words[i]=strdup(wbuf);
      sentence->tags[i]=strdup(tbuf);
    }

  convert_sentence(sentence);

  return 1;
}

int read_sentences(FILE *file,sentence_type *s,int max)
{
  int num=0;

  while(num<max&&read_sentence(file,&s[num])!=0)
    num++;

  return num;
}

void print_noparse(sentence_type *orig)
{
  int i;

  printf("(TOP ");

  for(i=0;i<orig->nws;i++)
    printf("%s/%s ",orig->words[i],orig->tags[i]);  

  printf(")\n");
}

int ispunc(char *tag);
int iscomma(char *tag,char *word);

/* calculates wordnos, tagnos, nws_np, commaats and commaats2 from
   words, tags, and nws (see sentence.h for the details of these
   variables)
   */

void convert_sentence(sentence_type *sentence)
{
  int i;
  int n;
  int w,t;
  int lflag;

  n = 0;

  for(i=0;i<PMAXWORDS;i++)
    sentence->commaats[i]=0;

  for(i=0;i<sentence->nws;i++)
    {
      if(!ispunc(sentence->tags[i]))
	{
	  w = find_word(sentence->words[i],&wordlex);
	  if(w==-1)
	    sentence->wordnos[n] = GUNKNOWN;
	  else
	    sentence->wordnos[n] = w;

	  sentence->wordpos[n] = i;

	  t = find_word(sentence->tags[i],&nt_lex);

	  if(!(t>=0))
	    {
	      printf("TAG %s not found\n",sentence->tags[i]);
	      assert(0);
	    }
	  
	  sentence->tagnos[n] = t;
	  n++;
	}
      else
	{
	  if(iscomma(sentence->tags[i],sentence->words[i])&&n>0)
	    {
	      sentence->commaats[n-1] =1;

	      w = find_word(sentence->words[i],&wordlex);
	      if(w==-1)
		sentence->commawords[n-1] = GUNKNOWN;
	      else
		sentence->commawords[n-1] = w;
	      
	      t = find_word(sentence->tags[i],&nt_lex);
	      
	      if(!(t>=0))
		{
		  printf("TAG %s not found\n",sentence->tags[i]);
		  assert(0);
		}	      
	      sentence->commatags[n-1] = t;
	    }
	}
    }
  sentence->nws_np = n;
  sentence->commaats[n-1] = 0;
  
  lflag=0;
  for(i=0;i<sentence->nws_np+1;i++)
    {
      if(sentence->tagnos[i]==NT_LRB) lflag=1;
      if(sentence->tagnos[i]==NT_RRB) lflag=0;
      if(lflag==0)
	sentence->commaats2[i]=sentence->commaats[i];
      else
	sentence->commaats2[i]=0;
    }

}

int ispunc(char *tag)
{
  if(strcmp(tag,",")==0) return 1;
  if(strcmp(tag,".")==0) return 1;
  if(strcmp(tag,"``")==0) return 1;
  if(strcmp(tag,"''")==0) return 1;
  if(strcmp(tag,":")==0) return 1;

  return 0;
}


int iscomma(char *tag,char *word)
{
  if(strcmp(word,"...")==0) return 0;

  if(strcmp(tag,",")==0) return 1;
  if(strcmp(tag,":")==0) return 1;



  return 0;
}
