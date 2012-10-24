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

#include "lexicon.h"

void make_lex(int size, lex_type *lexicon)
{
  lex_hash_entry *table;
  int i;

  table = (lex_hash_entry *) malloc (size*sizeof(lex_hash_entry));

  for(i=0;i<size;i++)
    table[i].entry=NULL;

  lexicon->size=size;
  lexicon->hashtable=table;
  lexicon->numkeys=0;
}

int get_lex_hash(char word[], int size)
{
  int h=1;
  int i;
  
  for(i=0;i<strlen(word);i++)
    {
      h=(h*word[i])%size;
    }

  return h;
}

int add_word(char word[],lex_type *lexicon){
  int h;

  h=get_lex_hash(word,lexicon->size);

  while(lexicon->hashtable[h].entry!=NULL)
    {
      if(strcmp(lexicon->hashtable[h].entry,word)==0)
	return lexicon->hashtable[h].key;
      h++;
      if(h==lexicon->size)
	h=0;
    } 

  lexicon->hashtable[h].entry= (char *) malloc((strlen(word)+1)*sizeof(char));
  strcpy(lexicon->hashtable[h].entry,word);
  lexicon->hashtable[h].key=lexicon->numkeys;
  (lexicon->numkeys)++;
  
  return lexicon->numkeys-1;
}

int add_word_with_key(char word[],lex_type *lexicon,int key){
  int h;

  h=get_lex_hash(word,lexicon->size);

  while(lexicon->hashtable[h].entry!=NULL)
    {
      h++;
      if(h==lexicon->size)
	h=0;
    } 

  lexicon->hashtable[h].entry= (char *) malloc((strlen(word)+1)*sizeof(char));
  strcpy(lexicon->hashtable[h].entry,word);
  lexicon->hashtable[h].key=key;
  (lexicon->numkeys)++;
  
  return key;
}

int find_word(char word[],lex_type *lexicon){
  int h;

  h=get_lex_hash(word,lexicon->size);

  while(lexicon->hashtable[h].entry!=NULL)
    {
      if(strcmp(lexicon->hashtable[h].entry,word)==0)
	return lexicon->hashtable[h].key;
      h++;
      if(h==lexicon->size)
	h=0;
    } 

  return -1;
}

void print_lex_tofile(FILE* file, lex_type *lexicon)
{
  int i;

  for(i=0;i<lexicon->size;i++)
    {
      if(lexicon->hashtable[i].entry!=NULL)
	fprintf(file,"%d %s\n",lexicon->hashtable[i].key,lexicon->hashtable[i].entry);
    }
  return;
}

void read_lex_fromfile(FILE* file, lex_type *lexicon)
{
  int d;
  char buffer[1000];

  while(fscanf(file,"%d %s",&d,buffer)!=EOF)
    add_word_with_key(buffer,lexicon,d);


  return;
}


void read_lex_fromfile2(FILE* file, lex_type *lexicon)
{
  int d;
  char buffer[1000];

  while(fscanf(file,"%s %d",buffer,&d)!=EOF)
    add_word_with_key(buffer,lexicon,d);


  return;
}

int add_wordcount(char word[],lex_type *lexicon)
{
  int h;

  h=get_lex_hash(word,lexicon->size);

  while(lexicon->hashtable[h].entry!=NULL)
    {
      if(strcmp(lexicon->hashtable[h].entry,word)==0)
	{
	  (lexicon->hashtable[h].key)++;
	  return lexicon->hashtable[h].key;
	}
      h++;
      if(h==lexicon->size)
	h=0;
    } 
  
  lexicon->hashtable[h].entry= (char *) malloc((strlen(word)+1)*sizeof(char));
  strcpy(lexicon->hashtable[h].entry,word);
  lexicon->hashtable[h].key=1;
  (lexicon->numkeys)++;
  
  return 1;
}
