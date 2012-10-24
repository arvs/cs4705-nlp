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
#include "prob.h"

#define START_NT 43

/*dependency parameters: D1 = generating nt,tag, D2 = generating word*/

#define D1PROBTYPE 0
#define D1OLEN 4
int D1BACKOFFS[] = {3,9,7,6};

#define D2PROBTYPE 1
#define D2OLEN 2
int D2BACKOFFS[] = {3,13,11,1};
int D2BACKOFFS_NOTAG[] = {2,13,11};

/*unary parameters */

#define UPROBTYPE 2
#define UOLEN 1
int UBACKOFFS[] = {3,4,2,1};

/*start parameters: S1 = generating nt,tag, S2 = generating word*/

#define S1PROBTYPE 3
#define S1OLEN 2
int S1BACKOFFS[] = {1,1};

#define S2PROBTYPE 4
#define S2OLEN 2
int S2BACKOFFS[] = {2,3,1};
int S2BACKOFFS_NOTAG[] = {1,3};

/*subcategorisation parameters */

#define SCPROBTYPE 5
#define SCOLEN 3
int SCBACKOFFS[] = {3,6,4,3};

/*prior parameters: P1 = generating nt,tag, P2 = generating word*/

#define P1PROBTYPE 6
#define P1OLEN 1
int P1BACKOFFS[] = {3,4,2,1};

#define P2PROBTYPE 7
#define P2OLEN 3
int P2BACKOFFS[] = {1,1};

/*coordination/punc parameters: CP1 generates tag, CP2 generates the word*/

#define CP1PROBTYPE 8
#define CP1OLEN 1
int CP1BACKOFFS[] = {3,9,6,1};

#define CP2PROBTYPE 9
#define CP2OLEN 2
int CP2BACKOFFS[] = {3,10,7,1};
int CP2BACKOFFS_NOTAG[] = {2,10,7};

/*gap parameters */

#define GPROBTYPE 10
#define GOLEN 1
int GBACKOFFS[] = {3,5,3,2};

/*make string for P(cm,t | context) */
void make_dep1_string(unsigned char *string,int wm,int tm,int cm,int wh,int th,int p,int ch,int dist,int subcat,int cc,int punc);

/*make string for P(wm | cm,t, context) */
void make_dep2_string(unsigned char *string,int wm,int tm,int cm,int wh,int th,int p,int ch,int dist,int subcat,int cc,int punc);

/*make string for P(ch | p,th,wh) */
void make_unary_string(unsigned char *string,int ch,int wh,int th,int p);

/*make string for P(ch,th | p==TOP) */
void make_s1_string(unsigned char *string,int ch,int wh,int th,int p);

/*make string for P(wh | ch,th,p==TOP) */
void make_s2_string(unsigned char *string,int ch,int wh,int th,int p);


/*make string for P(subcat | ch,th,p,dir) */
void make_subcat_string(unsigned char *string,int subcat,int ch,int wh,int th,int p,int dir);

/*make strings for P(ch,th | anything) P(wh | ch,th,anything)*/
void make_prior1_string(unsigned char *string,int ch,int wh,int th);
void make_prior2_string(unsigned char *string,int ch,int wh,int th);

/*make strings for P(ccword,cctag | ...) or P(pword,ptag | ...)
  type is 0 for coordination 1 for punctuation */
void make_pcc1_string(unsigned char *string,int t,int p,int ch,int th,int wh,int cm,int tm,int wm,int type);
void make_pcc2_string(unsigned char *string,int w,int t,int p,int ch,int th,int wh,int cm,int tm,int wm,int type);

/*make string for P(gap | wh,ch,th,p) */
void make_gap_string(unsigned char *string,int gap,int ch,int wh,int th,int p);


void add_dependency_counts(int wm,int tm,int cm,int wh,int th,int p,int ch,int dist,int subcat,int cc,int wcc,int tcc,int punc,int wpunc,int tpunc,hash_table *hash)
{
  unsigned char buffer[1000];

  if(cm != STOPNT)
    add_prior_counts(cm,wm,tm,hash);

  wm = fwords[wm];
  p  = argmap[p];
  ch = gapmap[ch];

  make_dep1_string(buffer,wm,tm,cm,wh,th,p,ch,dist,subcat,cc,punc);
  add_counts(buffer,D1OLEN,D1BACKOFFS,D1PROBTYPE,hash);

  make_dep2_string(buffer,wm,tm,cm,wh,th,p,ch,dist,subcat,cc,punc);
  add_counts(buffer,D2OLEN,D2BACKOFFS_NOTAG,D2PROBTYPE,hash);

  if(cc==1)
    {
      make_pcc1_string(buffer,tcc,p,ch,th,wh,cm,tm,wm,0);
      add_counts(buffer,CP1OLEN,CP1BACKOFFS,CP1PROBTYPE,hash);

      make_pcc2_string(buffer,wcc,tcc,p,ch,th,wh,cm,tm,wm,0);
      add_counts(buffer,CP2OLEN,CP2BACKOFFS_NOTAG,CP2PROBTYPE,hash);
    }

  if(punc==1)
    {
      make_pcc1_string(buffer,tpunc,p,ch,th,wh,cm,tm,wm,1);
      add_counts(buffer,CP1OLEN,CP1BACKOFFS,CP1PROBTYPE,hash);

      make_pcc2_string(buffer,wpunc,tpunc,p,ch,th,wh,cm,tm,wm,1);
      add_counts(buffer,CP2OLEN,CP2BACKOFFS_NOTAG,CP2PROBTYPE,hash);
    }

}

double get_dependency_prob(int wm,int tm,int cm,int wh,int th,int p,int ch,int dist,int subcat,int cc,int wcc,int tcc,int punc,int wpunc,int tpunc,hash_table *hash)
{
  unsigned char buffer[1000];
  double p1,p2,p3,p4;

  wm = fwords[wm];
  p  = argmap[p];
  ch = gapmap[ch];

  make_dep1_string(buffer,wm,tm,cm,wh,th,p,ch,dist,subcat,cc,punc);
  p1 = get_prob(buffer,D1OLEN,D1BACKOFFS,D1PROBTYPE,0,5,hash);

  if(cm != STOPNT)
    {
      make_dep2_string(buffer,wm,tm,cm,wh,th,p,ch,dist,subcat,cc,punc);
      p2 = get_prob(buffer,D2OLEN,D2BACKOFFS,D2PROBTYPE,0,5,hash);
    }
  else
    p2 = 1;

  if(cc==1)
    {
      make_pcc1_string(buffer,tcc,p,ch,th,wh,cm,tm,wm,0);
      p3 = get_prob(buffer,CP1OLEN,CP1BACKOFFS,CP1PROBTYPE,0,5,hash);

/*      printf("P3 %g ",p3);*/

      make_pcc2_string(buffer,wcc,tcc,p,ch,th,wh,cm,tm,wm,0);
      p3 *= get_prob(buffer,CP2OLEN,CP2BACKOFFS,CP2PROBTYPE,0,5,hash);

/*      printf("%g\n",p3);*/
    }
  else
    p3 = 1;

  if(punc==1)
    {
      make_pcc1_string(buffer,tpunc,p,ch,th,wh,cm,tm,wm,1);
      p4 = get_prob(buffer,CP1OLEN,CP1BACKOFFS,CP1PROBTYPE,0,5,hash);

/*      printf("P4 %g ",p4);*/

      make_pcc2_string(buffer,wpunc,tpunc,p,ch,th,wh,cm,tm,wm,1);
      p4 *= get_prob(buffer,CP2OLEN,CP2BACKOFFS,CP2PROBTYPE,0,5,hash);

/*      printf("%g\n",p4);*/
    }
  else
    p4 = 1;

/*  printf("DEP %d %d %d %d %d %d %d %d %d %d %d %g %g %g\n",
	 wm,tm,cm,
	 wh,th,p,
	 ch,dist,subcat,cc,punc,p1,p2,log(p1*p2));*/

  return log(p1*p2*p3*p4);
}

void add_prior_counts(int ch,int wh,int th,hash_table *hash)
{
  unsigned char buffer[1000];

  ch = gapmap[ch];

  wh = fwords[wh];
  
  make_prior1_string(buffer,ch,wh,th);
  add_counts(buffer,P1OLEN,P1BACKOFFS,P1PROBTYPE,hash);
  
  make_prior2_string(buffer,ch,wh,th);
  add_counts(buffer,P2OLEN,P2BACKOFFS,P2PROBTYPE,hash);

}

double get_prior_prob(int ch,int wh,int th,hash_table *hash)
{
  unsigned char buffer[1000];
  double p1,p2;

/*  printf("PRIOR %d %d %d\n",ch,wh,th);*/

  ch = gapmap[ch];

  wh = fwords[wh];
  
  make_prior1_string(buffer,ch,wh,th);
  p1=get_prob(buffer,P1OLEN,P1BACKOFFS,P1PROBTYPE,0,5,hash);
  
  make_prior2_string(buffer,ch,wh,th);
  p2=get_prob(buffer,P2OLEN,P2BACKOFFS,P2PROBTYPE,1,0,hash);

  return log(p1*p2);
}

void add_unary_counts(int ch,int wh,int th,int p,hash_table *hash)
{
  unsigned char buffer[1000];

  add_prior_counts(ch,wh,th,hash);

  if(p==START_NT)
    {
      wh = fwords[wh];

      make_s1_string(buffer,ch,wh,th,p);
      add_counts(buffer,S1OLEN,S1BACKOFFS,S1PROBTYPE,hash);

      make_s2_string(buffer,ch,wh,th,p);
      add_counts(buffer,S2OLEN,S2BACKOFFS_NOTAG,S2PROBTYPE,hash);
    }
  else
    {
      make_unary_string(buffer,ch,wh,th,p);
      add_counts(buffer,UOLEN,UBACKOFFS,UPROBTYPE,hash);
    }
}

double get_unary_prob(int ch,int wh,int th,int p,hash_table *hash)
{
  unsigned char buffer[1000];
  double p1,p2;

/*  printf("UNARY %d %d %d %d\n",ch,wh,th,p);*/

  if(p==START_NT)
    {
      wh = fwords[wh];

      make_s1_string(buffer,ch,wh,th,p);
      p1=get_prob(buffer,S1OLEN,S1BACKOFFS,S1PROBTYPE,0,5,hash);

      make_s2_string(buffer,ch,wh,th,p);
      p2=get_prob(buffer,S2OLEN,S2BACKOFFS,S2PROBTYPE,0,5,hash);

      return log(p1*p2);
    }
  else
    {
      make_unary_string(buffer,ch,wh,th,p);
      return log(get_prob(buffer,UOLEN,UBACKOFFS,UPROBTYPE,0,5,hash));
    }
}

/*subcat: dir=0 means left, dir=1 means right*/
void add_subcat_counts(int subcat,int ch,int wh,int th,int p,int dir,hash_table *hash)
{
  unsigned char buffer[1000];

  p=argmap[p];
  ch=argmap[ch];

  make_subcat_string(buffer,subcat,ch,wh,th,p,dir);
  add_counts(buffer,SCOLEN,SCBACKOFFS,SCPROBTYPE,hash);

}

double get_subcat_prob(int subcat,int ch,int wh,int th,int p,int dir,hash_table *hash)
{
  unsigned char buffer[1000];

  p=argmap[p];
  ch=argmap[ch];

  make_subcat_string(buffer,subcat,ch,wh,th,p,dir);
  return log(get_prob(buffer,SCOLEN,SCBACKOFFS,SCPROBTYPE,5,0,hash));

}

void add_gap_counts(int gap,int ch,int wh,int th,int p,hash_table *hash)
{
  unsigned char buffer[1000];

  p=argmap[p];
  ch=argmap[ch];

  make_gap_string(buffer,gap,ch,wh,th,p);
  add_counts(buffer,GOLEN,GBACKOFFS,GPROBTYPE,hash);

}

double get_gap_prob(int gap,int ch,int wh,int th,int p,hash_table *hash)
{
  unsigned char buffer[1000];

  p=argmap[p];
  ch=argmap[ch];

  make_gap_string(buffer,gap,ch,wh,th,p);
  return log(get_prob(buffer,GOLEN,GBACKOFFS,GPROBTYPE,5,0,hash));

}



/*make string for P(cm,t | context) 

  position    element
  3           tm
  4           cm
  5           punc
  6           cc

  7           p
  8           ch
  9           dist
  10..12      subcat
  13          th
  14..15      wh

*/

void calc_newdist(int *newd,int d)
{
  if(d>=100)
    {
      *newd = 100;
      d-=100;
    }
  else
    *newd=0;

  if(d>=10)
    {
      if(DISTAFLAG)
	*newd += 10;
      d-=10;
    }

  if(d>=1)
    {
      if(DISTVFLAG)
	*newd += 1;
    }
}

void make_dep1_string(unsigned char *string,int wm,int tm,int cm,int wh,int th,int p,int ch,int dist,int subcat,int cc,int punc)
{
  int newdist;

  calc_newdist(&newdist,dist);

  string[3] = (char) tm;
  string[4] = (char) cm;
  string[5] = (char) punc;
  string[6] = (char) cc;

  string[7] = (char) p;
  string[8] = (char) ch;
  string[9] = (char) newdist;
  byte3_to_char(&string[10],subcat);
  string[13] = (char) th;
  byte2_to_char(&string[14],wh);

}

/*make string for P(wm | cm,t, context) 

  position    element
  3..4        wm

  5           tm
  6           cm
  7           punc
  8           cc
  9           p
  10           ch
  11          dist
  12..14      subcat
  15          th
  16..17      wh
*/

void make_dep2_string(unsigned char *string,int wm,int tm,int cm,int wh,int th,int p,int ch,int dist,int subcat,int cc,int punc)
{
  int newdist;

  calc_newdist(&newdist,dist);

  byte2_to_char(&string[3],wm);

  string[5] = (char) tm;
  string[6] = (char) cm;
  string[7] = (char) punc;
  string[8] = (char) cc;

  string[9] = (char) p;
  string[10] = (char) ch;
  string[11] = (char) newdist;
  byte3_to_char(&string[12],subcat);
  string[15] = (char) th;
  byte2_to_char(&string[16],wh);

}

/*make string for P(wm | cm,t, context) 

  position    element
  3..4        wm

  5           tm
*/

void add_tagword_counts(int wm,int tm,hash_table *hash)
{
  unsigned char buffer[1000];
  int bos[] = {1,1};

  wm = fwords[wm];

  byte2_to_char(&buffer[3],wm);

  buffer[5] = (char) tm;

  add_counts_level(buffer,D2OLEN,bos,3,D2PROBTYPE,hash);
  add_counts_level(buffer,S2OLEN,bos,2,S2PROBTYPE,hash);
  add_counts_level(buffer,CP2OLEN,bos,3,CP2PROBTYPE,hash);
}


void byte2_to_char(unsigned char *string,int n)
{
  string[0] = (n & 255);
  string[1] = n/256;
}

void byte3_to_char(unsigned char *string,int n)
{
  string[0] = (n&255);
  string[1] = ((n/256)&255);
  string[2] = n/65536;
}


/*make string for P(ch | p,th,wh)

  position    element
  3           ch

  4           p
  5           th
  6..7        wh
*/

void make_unary_string(unsigned char *string,int ch,int wh,int th,int p)
{

  string[3] = (char) ch;

  string[4] = (char) p;
  string[5] = (char) th;
  byte2_to_char(&string[6],wh);
}

/*make string for P(ch,th | p==TOP)

  position    element
  3           ch
  4           th

  5           TOP

*/

void make_s1_string(unsigned char *string,int ch,int wh,int th,int p)
{

  string[3] = (char) ch;
  string[4] = (char) th;

  string[5] = (char) p;
}

/*make string for P(wh | ch,th, p==TOP)

  position    element
  3..4        wh

  5           th
  6           ch
  7           TOP

*/

void make_s2_string(unsigned char *string,int ch,int wh,int th,int p)
{

  byte2_to_char(&string[3],wh);

  string[5] = (char) th;
  string[6] = (char) ch;
  string[7] = (char) p;
}

/*make string for P(subcat | wh,ch,th, p,dir)

  position    element
  3..5        subcat

  6           ch
  7           p
  8           dir
  9           th
  10..11      wh

*/

void make_subcat_string(unsigned char *string,int subcat,int ch,int wh,int th,int p,int dir)
{
  byte3_to_char(&string[3],subcat);

  string[6] = (char) ch;
  string[7] = (char) p;
  string[8] = (char) dir;
  string[9] = (char) th;
  byte2_to_char(&string[10],wh);
}

/*make string for P(ch | wh,th)

  position    element
  3           ch

  4           0
  5           th
  6..7        wh

*/

void make_prior1_string(unsigned char *string,int ch,int wh,int th)
{

  string[3] = (char) ch;

  string[4] = 0;
  string[5] = (char) th;
  byte2_to_char(&string[6],wh);
}

/*make string for P(wh, th |  p==anything)

  position    element
  3..4        wh
  5           th

  6           0

*/

void make_prior2_string(unsigned char *string,int ch,int wh,int th)
{

  byte2_to_char(&string[3],wh);
  string[5] = (char) th;

  string[6] = 0;

}

/* pcc1_string

   position      element
   3             t

   4             type
   5             p
   6             ch
   7             cm
   8             th
   9             tm
   10            wh
   12            wm
*/

void make_pcc1_string(unsigned char *string,int t,int p,int ch,int th,int wh,int cm,int tm,int wm,int type)
{
/*  printf("PCC1 %d %d %d %d %d %d %d %d %d\n",
	 t,p,ch,th,wh,cm,tm,wm,type);*/


  string[3] = (char) t;

  string[4] = (char) type;

  string[5] = (char) p;
  string[6] = (char) ch;
  string[7] = (char) cm;

  string[8] = (char) th;
  string[9] = (char) tm;

  byte2_to_char(&string[10],wh);
  byte2_to_char(&string[12],wm);
}

/* pcc2_string

   position      element
   3..4          w

   5             t
   6             type
   7             p
   8             ch
   9             cm
   10            th
   11            tm
   12            wh
   14            wm
*/

void make_pcc2_string(unsigned char *string,int w,int t,int p,int ch,int th,int wh,int cm,int tm,int wm,int type)
{

/*  printf("PCC1 %d %d %d %d %d %d %d %d %d %d\n",
	 w,t,p,ch,th,wh,cm,tm,wm,type);*/

  byte2_to_char(&string[3],w);
  
  string[5] = (char) t;

  string[6] = (char) type;

  string[7] = (char) p;
  string[8] = (char) ch;
  string[9] = (char) cm;

  string[10] = (char) th;
  string[11] = (char) tm;

  byte2_to_char(&string[12],wh);
  byte2_to_char(&string[14],wm);
}


/*make string for P(gap | wh,ch,th, p)

  position    element
  3           gap

  4           ch
  5           p
  6           th
  7..8      wh

*/

void make_gap_string(unsigned char *string,int gap,int ch,int wh,int th,int p)
{
  string[3] = (char) gap;

  string[4] = (char) ch;
  string[5] = (char) p;
  string[6] = (char) th;
  byte2_to_char(&string[7],wh);
}
