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

#ifndef LEXICON_H
#define LEXICON_H

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>



typedef struct {
  int key;
  char *entry;
} lex_hash_entry;

typedef struct {
  int numkeys;
  int size;
  lex_hash_entry *hashtable;
} lex_type;

void make_lex(int size, lex_type *lexicon);
int get_lex_hash(char word[], int size);
int add_word(char word[],lex_type *lexicon);
int add_word_with_key(char word[],lex_type *lexicon, int key);
int find_word(char word[],lex_type *lexicon);
void print_lex_tofile(FILE* file, lex_type *lexicon);
void read_lex_fromfile(FILE* file, lex_type *lexicon);

/*same as read_lex_fromfile,but format is "word number"*/
void read_lex_fromfile2(FILE* file, lex_type *lexicon);

int add_wordcount(char word[],lex_type *lexicon);

#endif
