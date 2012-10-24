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

#include "grammar.h"

#define GDEBUG 1

void init_grammar();
void init_lexicons();

void read_lexicon(FILE *file);
void read_grm(FILE *file);
void read_nonterminals(FILE *file);
void read_nonterminals2(FILE *file);

void read_grammar(char *filename)
{
  char buffer[1000];
  FILE *file;


  init_lexicons();
  if(GDEBUG) fprintf(stderr,"Initialised lexicons\n");

  init_grammar();
  if(GDEBUG) fprintf(stderr,"Initialised grammar\n");

  strcpy(buffer,filename);
  strcat(buffer,".nts");
  file = fopen(buffer,"r");
  read_nonterminals(file);

  file = fopen(buffer,"r");
  read_nonterminals2(file);
  if(GDEBUG) fprintf(stderr,"Loaded non-terminals\n");

  strcpy(buffer,filename);
  strcat(buffer,".lexicon");
  file = fopen(buffer,"r");
  read_lexicon(file);
  if(GDEBUG) fprintf(stderr,"Loaded lexicon\n");

  strcpy(buffer,filename);
  strcat(buffer,".grm");
  file = fopen(buffer,"r");
  read_grm(file);
  if(GDEBUG) fprintf(stderr,"Loaded grammar\n");
}


void init_grammar()
{
  int i,j,k;

  for(i=0;i<GMAXNTS;i++)
    for(j=0;j<GMAXNTS;j++)
      for(k=0;k<GMAXNTS;k++)
	tablep[i][j][k] = tablef[i][j][k] = 0;

  for(i=0;i<GMAXNTS;i++)
    unary_nums[i] = 0;

  for(i=0;i<GMAXNTS;i++)
    for(j=0;j<GMAXNTS;j++)
      lsubcats_counts[i][j] = rsubcats_counts[i][j] = 0;
}

void init_lexicons()
{
  int i,j;

  for(i=0;i<GMAXWORDS;i++)
    fwords[i] = GUNKNOWN;

  for(i=0;i<GMAXWORDS;i++)
    for(j=0;j<GMAXNTS;j++)
      tagdict[i][j] = 0;

  make_lex(200003,&wordlex);
  make_lex(1003,&nt_lex);

  TRACEWORD = add_word("*TRACE*",&wordlex);
  fwords[TRACEWORD] = 1;

}

void read_nonterminals(FILE *file)
{
  char buffer[1000];
  int n;
  
  while(fscanf(file,"%s",buffer)!=EOF)
    {
      n=add_word(buffer,&nt_lex);
      assert(n>=0);
    }
  TRACENT = find_word("NP-A-g",&nt_lex);
  assert(TRACENT>0);
  TRACETAG = find_word("NN",&nt_lex);
  assert(TRACETAG>0);
}

/*this reads the non-terminal strings, and sets up the argmap files*/

void read_nonterminals2(FILE *file)
{
  int i,j,flag;
  int len;
  char buffer[1200];
  char nt_line[100];

  numnts=0;

  while(fgets(nt_line,100,file)!= NULL)
    {
      sscanf(nt_line,"%s",&nts[numnts]);
      numnts++;
    }

  /*now set up the various argmaps*/
  for(i=0;i<GMAXNTS;i++)
    argmap[i]=i;

  for(i=0;i<GMAXNTS;i++)
    hasarg[i]=0;

  for(i=0;i<GMAXNTS;i++)
    gapmap[i]=i;

  for(i=0;i<GMAXNTS;i++)
    hasgap[i]=0;

  for(i=0;i<numnts;i++)
    {
      len=strlen(nts[i]);
      strcpy(buffer,nts[i]);
      flag=0;
      for(j=len;j>=0;j--)
        if(buffer[j]=='-') 
          {
            buffer[j]='\0';
            flag=1;
            if(buffer[j+1]=='A')
              hasarg[i]=1;
          }

      if(flag==1)
        for(j=0;j<numnts;j++)
          if(strcmp(buffer,nts[j])==0)
            argmap[i]=j;
    }

  for(i=0;i<numnts;i++)
    {
      len=strlen(nts[i]);
      if(nts[i][len-1]=='g' &&
         nts[i][len-2]=='-')
        {
          hasgap[i]=1;
          strcpy(buffer,nts[i]);
          buffer[len-2]='\0';
          for(j=0;j<numnts;j++)
            if(strcmp(buffer,nts[j])==0)
              gapmap[i]=j;
        }
    }

  for(i=0;i<numnts;i++)
    if(strcmp(nts[i],"Ss-A-g")==0)
      {
        for(j=0;j<numnts;j++)
          if(strcmp("S-A",nts[j])==0)
            gapmap[i]=j;                

        for(j=0;j<numnts;j++)
          if(strcmp("S",nts[j])==0)
            argmap[i]=j;                
      } 

}


void read_lexicon(FILE *file)
{
  char word[1000],tag[1000];
  int fw;
  int wn,tn;

  while(fscanf(file,"%s %s %d",word,tag,&fw)!=EOF)
    {
      wn = add_word(word,&wordlex);
      assert(wn>=0);

/*      fprintf(stderr,"%s\n",tag);*/
      tn = find_word(tag,&nt_lex);
      assert(tn>=0);
	
      if(fw)
	fwords[wn] = wn;
      else
	fwords[wn] = GUNKNOWN;
      tagdict[wn][tn] = 1;
    }

}

void read_grm(FILE *file)
{
  char buffer[1000];
  char b1[1000],b2[1000],b3[1000];
  int n1,n2,n3;

  while(fscanf(file,"%s",buffer)!=EOF)
    {
      if(strcmp(buffer,"L")==0)
	{
	  fscanf(file,"%s %s %s",b1,b2,b3);

	  n1 = find_word(b1,&nt_lex);
/*	  fprintf(stderr,"%s\n",b1);*/
	  assert(n1>=0);

	  n2 = find_word(b2,&nt_lex);
	  assert(n2>=0);

	  n3 = find_word(b3,&nt_lex);
	  assert(n3>=0);

	  tablep[n1][n2][n3] = 1;
	}
      else if(strcmp(buffer,"R")==0)
	{
	  fscanf(file,"%s %s %s",b1,b2,b3);

	  n1 = find_word(b1,&nt_lex);
	  assert(n1>=0);

	  n2 = find_word(b2,&nt_lex);
	  assert(n2>=0);

	  n3 = find_word(b3,&nt_lex);
	  assert(n3>=0);

	  tablef[n1][n2][n3] = 1;

	}
      else if(strcmp(buffer,"U")==0)
	{
	  fscanf(file,"%s %s",b1,b2);

	  n1 = find_word(b1,&nt_lex);
	  assert(n1>=0);

	  n2 = find_word(b2,&nt_lex);
	  assert(n2>=0);

	  unaries[n2][unary_nums[n2]] = n1;
	  unary_nums[n2]++;
	}
      else if(strcmp(buffer,"X")==0)
	{
	  fscanf(file,"%s %s %d",b1,b2,&n3);

	  n1 = find_word(b1,&nt_lex);
	  assert(n1>=0);

	  n2 = find_word(b2,&nt_lex);
	  assert(n2>=0);

	  lsubcats[n1][n2][ lsubcats_counts[n1][n2] ] = n3;
	  lsubcats_counts[n1][n2]++;
	}
      else if(strcmp(buffer,"Y")==0)
	{
	  fscanf(file,"%s %s %d",b1,b2,&n3);

	  n1 = find_word(b1,&nt_lex);
	  assert(n1>=0);

	  n2 = find_word(b2,&nt_lex);
	  assert(n2>=0);

	  rsubcats[n1][n2][ rsubcats_counts[n1][n2] ] = n3;
	  rsubcats_counts[n1][n2]++;

	}
      else assert(0);
    }
}

int isverb(int tag)
{

  if(tag>=PVB&&tag<=PVBZ)
    return 1;

  return 0;
}
