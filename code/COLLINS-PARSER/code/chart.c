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

#include "chart.h"

#define CCPROBSMALL 0.0000000000000000001

/* pointer to the current sentence being parsed*/
sentence_type *current;

/* initialise the chart */
void init_chart();

/* add a sentence to the chart (starts the parsing process) */
void add_sentence_to_chart(sentence_type *sentence);

/*complete the chart for words spanning s..e inclusive*/
void complete(int s,int e);

/*
   edges is an array of edges in the chart

   childs is an array used for the children of each node in the chart:

   childs[edge[i].child1] to childs[edge[i].child1+edge[i].numchild-1] are
   the children of the i'th edge in left to right order (being indexes into
   the edges array themselves)


 */

#define PMAXEDGES 200000
#define PMAXCHILDS 3000000

edge_type edges[PMAXEDGES];
int numedges;

int childs[PMAXCHILDS];
int numchilds;

/*
   The edges in the chart spanning words s to e inclusive are contained
   in edges[sindex[s][e]] to edges[eindex[s][e]] inclusive

   if no edges have been added to the chart for the span s..e,
   sindex[s][e]==-1, eindex[s][e]==-2

 */

int sindex[PMAXWORDS][PMAXWORDS];
int eindex[PMAXWORDS][PMAXWORDS];
void init_index();

/*
   add_edge(s,e) adds an edge spanning words s...e inclusive

   the edge is at edges[numedges], rather than being passed in as an argument
   
   returns an index into edges array which points to where the edge has been
   added

   returns -1 if the edge isn't added, i.e. because it fails the dynamic
   programming/beam conditions
 */

int add_edge(int s,int e);


/*
   saments is a data structure that supports add_edge

   saments allows edges in the chart with the same non-terminal label to be
   stored in a linked list, by pointing to the first element of the linked
   list. This helps efficiency of the dynamic programming algorithm (when
   checking to see if there's an edge in the chart with the same label,
   head word etc. it's sufficient to look down the linked list for that
   non-terminal)

   saments also facilitates keeping only the top PMAXHEADS edges with label i
   for each i (another kind of beam which turns out not to be all that useful,
   but I left the code in anyway)

   saments contains information for non-terminals in the current span being
   worked on in the chart: at each point when a new span is started saments
   is re-initialised 

   saments[i] contains the following info for non-terminal i:

     edge1 is the first edge in the chart with label i. edges[i].next points
     to the next element in the linked list, and so on

     minedge is the edge in the list with the lowest probability
     minprob is the prob of the lowest prob edge

     numedges is the number of edges with the i'th non-terminal

     (so once numedges==PMAXHEADS, when adding an edge first test if its prob
     is greater than minprob, if so replace minedge and update minedge/minprob)

*/

typedef struct {
  int edge1;
  int minedge;
  double minprob;
  char numedges;
} sament_type;

sament_type saments[GMAXNTS];
void init_saments();
void find_sament_minprob(int nt);
#define PMAXHEADS 100


/*
   join_2_edges_follow joins two edges where the modifier follow the head:
   (the new edge is added using add_edge)

         P                     P
	 |            =>       | 
	 H ...   R             H ...  R

	 edge1   edge2         newedge

prob =   P1      P2            P1*P2*Prob_r(R | P,H)

   where edge1 spans words s..m inclusive
         edge2 spans words m+1..e inclusive

   edge1 is the e1'th element of the edges array

   edge2 is the i'th element of the edges array for all i=e2s[0] .. e2s[ne2s-1]

   so there are ne2s joining operations: they are all done in one call for
   efficiency reasons
 */

void join_2_edges_follow(int e1,int s,int m,int e,int *e2s,int ne2s);

/*
   join_2_edges_precede joins two edges but where the modifier precedes the 
   head:
   (the new edge is added using add_edge)

          P                   P
          |       =>          | 
   L ...  H             L ... H

   edge1   edge2         newedge

   edge2 is the e1'th element of the edges array

   edge1 is the i'th element of the edges array for all i=e1s[0] .. e1s[ne1-1]

   so there are ne1 joining operations: they are all done in one call for
   efficiency reasons

*/

void join_2_edges_precede(int e2,int s,int m,int e,int *e1s,int ne1);

/*
   join_2_edges_cc joins two edges in a coordination relationship:
   (the new edge is added using add_edge)

   P                                   P
   |                   =>              | 
   H ...   CC     R                    H ...  CC R

   edge1          edge2                newedge

   where edge1 spans words s..m inclusive
         edge2 spans words m+2..e inclusive
	 word m+1 is tagged as a CC by the POS tagger

   edge1 is the e1'th element of the edges array
   edge1 is the e2'th element of the edges array

 */

void join_2_edges_cc(int e1,int e2,int s,int m,int e);

/* add_singles add all the unary rules for words spanning s..e:

                        P
                        |
       H          =>    H 

       edge1            newedge

       prob = P1        prob = P1*Prob_h(H | P)

       where edge1 = i'th edge in the edges array for si<=i<=ei
*/

void add_singles(int s,int e,int si,int ei);

/* add_stops adds all the stop probabilities for edges which do not yet
   have these added (i.e. for all s<=i<=e and edges[i].stop==0 a new edge is
   added with edges[newedge].stop==1 and the two stop probabilities multiplied
   in

*/

void add_stops(int s,int e,int si,int ei);

/* add_traces adds all traces for edges that have both a gap and an NP
   requirement to either their left or right

*/

void add_traces(int s,int e);

/* add_singles_stops(s,e) adds all stop probabilities for edges spanning
   s..e, then builds unary rules on top these, then adds stop probs for
   these unary rules, then builds unary rules on top of these, and so on
   and so on...
*/

void add_singles_stops(int s,int e);

/* adds are data-structures are used by add_singles_stops */

int ADDFLAG;
int adds1[100000];
int numadds1;
int adds2[100000];
int numadds2;

int *adds;
int *numadds;

/* calc_prob2 calculates prob2 for edges[edge]

   i.e. it sets edges[edge].prob2 = edges[edge].prob + prior

   where prior is an estimate of Prob(word,non-terminal,tag) for the edge,
   an estimate of how likely a non-terminal Prob(word,non-terminal,tag) is
   to appear in the correct tree
   */

void calc_prob2(int edge);


/* prints an edge tabbed in by offset spaces */

void print_edge(int e,int offset);

/*bestprobs[s][e] holds the highest probability for all edges added spanning
  words s..e 

  it's used for pruning edges from the chart

  note that it is continually updated as edges are added for section s..e
  the thresholding is used at two stages: first while the edges for s..e are
  being constructed -- at this stage some edges will make it through the beam
  in spite of being outside the beam once all the edges have been added (a 
  high probability edge may be added later than a low probability edge, and
  make it invalid); second, when the edges are being built on an initial check
  is made to see if they fall in the beam 

*/

double bestprobs[GMAXNTS][GMAXNTS];


/* returns 1 if the edge is within the current beam. Used while the edges for
   s..e in the chart are being added */
int inbeam(int edge,int s,int e);

/* returns 1 if the edge is within the current beam. Also sets
   edges[edge].inbeam indicating whether or not the edge makes the
   beam. Used when s..e is being built upon, at which point edges can
   definitely be classified as falling in/out of the beam */

int inbeam2(int edge,int s,int e);

int TREEBANKOUTPUTFLAG;

void set_treebankoutputflag(int flag)
{
  TREEBANKOUTPUTFLAG = flag;
}

/*==========================================================================*/

/*parse a sentence, print the output to stdout*/
void parse_sentence(sentence_type *sentence)
{
  int i,j;

  reset_prior_hashprobs();
  effhash_newsent(&eff_hash);           
  init_chart(GMAXNTS);
  current = sentence;
  	 
  add_sentence_to_chart(current);
  
  for(i=0;i<current->nws_np;i++)
    for(j=i-1;j>=0;j--)
      {
	complete(j,i);
      }
  
  if(print_best_parse()!=-1) return;
  
  printf("PROB 0 0 0\n");
  print_noparse(current);

}

void init_index()
{
  int i,j;

  for(i=0;i<PMAXWORDS;i++)
    for(j=0;j<PMAXWORDS;j++)
      {
	sindex[i][j]=-1;
	eindex[i][j]=-2;
      }
}

void init_saments()
{
  int i;

  for(i=0;i<GMAXNTS;i++)
    saments[i].edge1=-1;
}

void init_chart()
{
  init_index();

  numadds = &numadds1;
  adds = adds1;
  numadds1 = 0;

  init_saments();
  
  numedges=0;
  numchilds=0;
}

/* note the following bug: for singles, an edge can be replaced when another
   edge is built on top of it. For example, SBAR -> S is added, then the S
   is replaced */

int add_edge(int s,int e)
{
  int i,j;
  int nt;

  /* if type!=0, i.e. not a part of speech tag, return -1 if the edge
     doesn't make the threshold pthresh even before its prior prob is
     added
     */
  if(edges[numedges].type!=0) 
    {
      if(edges[numedges].prob<pthresh)
	return -1;
    }

  /*inbeam=2 means we haven't checked whether the edge falls within the beam
    or not yet, as not all edges have been added for span s...e*/
  edges[numedges].inbeam=2;

  calc_prob2(numedges);

  if(sindex[s][e]==-1) /*first edge to be added for this span*/
    {
      if(edges[numedges].type==0)		
	bestprobs[s][e]=-10000; /*don't count POS tags in beam comparisons*/
      else
	bestprobs[s][e]=edges[numedges].prob2;
        /*bestprobs is set to prob2, currently the highest probability edge
	  spanning words s..e*/
    }
  else if(edges[numedges].label==NT_TOP)
    {
      /*NT_TOP can only span the whole sentence (it's the start symbol)*/
      if(s!=0 || e!=current->nws_np-1)
	return -1;
    }
  else
    {
      /*return if not a POS tag and not in the beam
	(note: POS tags are always kept)*/

      if(edges[numedges].type!=0&&!inbeam(numedges,s,e))
	return -1;

	/*update bestprobs if the prob of this new edge beats the current
	  high for span s..e*/

      else if(edges[numedges].prob2 > bestprobs[s][e]
	      && edges[numedges].type!=0)
	bestprobs[s][e] = edges[numedges].prob2;	  
    }

  nt=edges[numedges].label;


  /*case 1: saments[nt].edge== -1, indicating that this is the first edge
    with label nt to be added to span s..e of the chart*/

  if(saments[nt].edge1==-1)
    {
      /*initialise saments to point to this edge*/
      saments[nt].edge1=numedges;
      saments[nt].minedge=numedges;
      saments[nt].minprob=edges[numedges].prob;
      saments[nt].numedges=1;

      /*there are no other edges with label nt in this span, so the next
	element in the linked list of edges with label nt is null */
      edges[numedges].next=-1;

      /*if it's the first edge in the chart of *any* label then initialise
	sindex[s][e]*/
      if(sindex[s][e]==-1)
	sindex[s][e]=numedges;

      eindex[s][e]=numedges;

      /*note that an edge has been added*/
      adds[*numadds] = numedges;
      (*numadds)++;
      ADDFLAG=1;

      numedges++;

      /*return a pointer to the edge*/
      return numedges-1;
    }

  /*Case 2: it's not the first edge with label nt to be added, but 
    saments[nt].numedges<PMAXHEADS indicates that the beam for label
    nt is not yet full (only the top PMAXHEADS non-terminals with this
    label are kept)*/

  if(saments[nt].numedges<PMAXHEADS)
    {
      i=saments[nt].edge1;

      /*go down the linked list, if a matching edge is found then the
	dynamic programming step kicks in: either replace the old edge if
	it has lower probability, otherwise return without adding anything
	*/
      while(i!=-1)
	{
	  if(equal_edges(&edges[i],&edges[numedges]))
	    if(edges[numedges].prob>edges[i].prob)
	      {
		j=edges[i].next;
		edges[i]=edges[numedges];
		edges[i].next=j;
		find_sament_minprob(nt);

		adds[*numadds] = i;
		(*numadds)++;
		
		ADDFLAG=1;
		return i;
	      }
	    else return -1;      

	  j=i;
	  i=edges[i].next;
	}
      
      /* at this stage no matching non-terminals have been found.
	 add the new edge at position edges[numedges]

	 edges[j] was the last non-terminal with label nt: 
	 edges[j].next=numedges adds the new edge to the end of the linked
	 list
	 */

      edges[j].next=numedges;
      edges[numedges].next=-1;
      saments[nt].numedges++;
      if(edges[numedges].prob<saments[nt].minprob)
	{saments[nt].minprob=edges[numedges].prob;saments[nt].minedge=numedges;}

      adds[*numadds] = numedges;
      (*numadds)++;

      eindex[s][e]=numedges;
      numedges++;

      ADDFLAG=1;
      return numedges-1;
    }

  /*Case 3: there are already PMAXHEADS edges with label nt*/

  /*if the new edge has lower probability than the lowest in the linked
    list, don't add it*/
  if(edges[numedges].prob<saments[nt].minprob) return -1;

  /*otherwise check the dynamic programming case again*/
  i=saments[nt].edge1;

  while(i!=-1)
    {
      if(equal_edges(&edges[i],&edges[numedges]))
	if(edges[numedges].prob>edges[i].prob)
	  {
	    j=edges[i].next;
	    edges[i]=edges[numedges];
	    edges[i].next=j;
	    find_sament_minprob(nt);

	    adds[*numadds] = i;
	    (*numadds)++;

	    ADDFLAG=1;
	    return i;
	  }
	else return -1;      
      
      i=edges[i].next;
    }

  /*if dynamic programming does not reject/accept the new edge, replace
    the lowest probability edge in the linked list with the new edge*/

  i=saments[nt].minedge;
  j=edges[i].next;
  edges[i]=edges[numedges];
  edges[i].next=j;
  find_sament_minprob(nt);

  adds[*numadds] = i;
  (*numadds)++;

  ADDFLAG=1;
  return i;

}

/*update the values for sament*/

void find_sament_minprob(int nt)
{
  int i;

  i=saments[nt].edge1;

  saments[nt].minprob=edges[i].prob;
  saments[nt].minedge=i;

  i=edges[i].next;
  while(i!=-1)
    {
      if(edges[i].prob<saments[nt].minprob)
	{saments[nt].minprob=edges[i].prob;saments[nt].minedge=i;}
      i=edges[i].next;
    }
}

void add_sentence_to_chart(sentence_type *sentence)
{
  int i,j,k;
  char *td;
  int word;
  char temptd[GMAXNTS];
  int flag;

  for(i=0;i<sentence->nws_np;i++)
    {
      init_saments();

      word=sentence->wordnos[i];
      flag=0;
      if(fwords[word]!=GUNKNOWN)
	{
	  for(j=0;j<GMAXNTS;j++)
	    if(tagdict[fwords[word]][j]==1)
	      flag=1;
	}

      if(flag==0||!ALLTAGS)
	{
	  td=temptd;
	  for(j=0;j<GMAXNTS;j++)
	    temptd[j]=0;
	  temptd[sentence->tagnos[i]]=1;
	}
      else
	td=tagdict[fwords[word]];
      
      for(j=0;j<100;j++)
	if(td[j]==1)
	  {
	    edges[numedges].head=i;
	    edges[numedges].headtag=j;
	    edges[numedges].headch=-1;
	    edges[numedges].prob=0;
	    edges[numedges].prob2=0;
	    edges[numedges].label=j;
	    edges[numedges].type=0;
	    edges[numedges].valid=1;
	    edges[numedges].stop=1;
	    edges[numedges].headlabel=0;      

	    clear_args(&edges[numedges].lc);
	    clear_args(&edges[numedges].rc);

	    edges[numedges].lc.nvs=0;
	    edges[numedges].lc.adj=1;

	    edges[numedges].rc.nvs=0;
	    edges[numedges].rc.adj=1;

	    edges[numedges].hasverb = isverb(j);

	    k=add_edge(i,i);

	  }

      add_singles_stops(i,i);

    }
}
void add_singles(int s,int e,int si,int ei)
{
  int i,j,k;
  edge_type etemp;
  double prob,probl,probr;
  int nl,nr;
  int il,ir;
  int flaggap;
  int wh,th,p,ch;

  for(i=si;i<=ei;i++)
    if(edges[i].valid==1&&inbeam(i,s,e)&&edges[i].stop==1)
      for(j=0;j<unary_nums[edges[i].label];j++)
	if(unaries[edges[i].label][j]!=-1)
	{
	  etemp=edges[i];
	  etemp.stop=0;
	  etemp.type=1;
	  etemp.label=unaries[edges[i].label][j];
	  etemp.headch=i;

	  etemp.headlabel=edges[i].label;

	  if(etemp.label == NT_NPB)
	    etemp.hasverb = 0;

	  clear_args(&etemp.lc);
	  clear_args(&etemp.rc);
	  
	  etemp.lc.nvs=0;
	  etemp.lc.adj=1;
	  
	  etemp.rc.nvs=0;
	  etemp.rc.adj=1;

	  wh=current->wordnos[edges[i].head];
	  th=edges[i].headtag;
	  p=etemp.label;
	  ch=edges[i].label;

	  prob=get_unary_prob_witheffhash(ch,wh,th,p,&new_hash,&eff_hash)+edges[i].prob;

	  nl=lsubcats_counts[p][ch];
	  nr=rsubcats_counts[p][ch];

	  flaggap=(hasgap[etemp.label] && !hasgap[edges[i].label]);

	  for(il=0;il<nl;il++)
	    {
	      if(nl==1)
		probl=0;
	      else
		probl=get_subcat_prob_witheffhash(lsubcats[p][ch][il],ch,wh,th,p,0,&new_hash,&eff_hash);


	      find_args(&etemp.lc,lsubcats[p][ch][il]);
	      for(ir=0;ir<nr;ir++)
		{
		  if(nr==1)
		    probr=0;
		  else
		    probr=get_subcat_prob_witheffhash(rsubcats[p][ch][ir],ch,wh,th,p,1,&new_hash,&eff_hash);

		  find_args(&etemp.rc,rsubcats[p][ch][ir]);
		  etemp.prob=prob+probl+probr;

		  if(flaggap==0)
		    {
		      edges[numedges]=etemp;
		      if((k=add_edge(s,e))!=-1)
			{
			  edges[k].child1=numchilds;
			  edges[k].numchild=1;
			  childs[numchilds]=i;
			  numchilds++;
			}
		    }
		  else
		    {
		      edges[numedges].prob=prob+probl+probr;

		      /*case3 -- from child to left*/
		      edges[numedges]=etemp;
		      edges[numedges].lc.gap=1;
		      edges[numedges].prob+=get_gap_prob_witheffhash(3,ch,wh,th,p,&new_hash,&eff_hash);

		      if((k=add_edge(s,e))!=-1)
			{
			  edges[k].child1=numchilds;
			  edges[k].numchild=1;
			  childs[numchilds]=i;
			  numchilds++;
			}

		      /*case4 -- from child to right*/
		      edges[numedges]=etemp;
		      edges[numedges].rc.gap=1;
		      edges[numedges].prob+=get_gap_prob_witheffhash(4,ch,wh,th,p,&new_hash,&eff_hash);
/*		      printf("CHECK %g %g\n",edges[i].prob,edges[numedges].prob);
		      print_edge(i,0);*/
		      if((k=add_edge(s,e))!=-1)
			{
			  edges[k].child1=numchilds;
			  edges[k].numchild=1;
			  childs[numchilds]=i;
			  numchilds++;
			}
		    }
		}
	    }
	}
}


void print_edge(int e,int off)
{
  int i,j,newoff;
  int flag;

  flag=1;

  if(e==-1)
    {
      printf("TRACE T\n");
      return;
    }

  if( (edges[e].label == NT_NP || edges[e].label == NT_NPA)
     && edges[e].numchild ==1)
    {
      j= childs[edges[e].child1];
/*      if(edges[j].label == NT_NPB)
	flag=0;*/
    }
    

  if(flag)
    {
      printf("%s %g ",nts[edges[e].label],edges[e].prob);
      newoff=off+strlen(nts[edges[e].label])+1;
    }
  else
    newoff=off;

  /*POS tag case*/
  if(edges[e].type==0)
    {
      printf("%s\n",current->words[current->wordpos[edges[e].head]]);
      return;
    }

  print_edge(childs[edges[e].child1],newoff);

  for(i=edges[e].child1+1;i<edges[e].child1+edges[e].numchild;i++)
    {    
      for(j=0;j<newoff;j++)
	printf(" ");
      print_edge(childs[i],newoff);
    }

}

int find_childno(int e,int c)
{
  int i;

  for(i=0;i<edges[e].numchild;i++)
    {
      if(childs[edges[e].child1+i] == c)
	return i+1;
    }

  return -1;
}

void print_wholent(int e)
{

  printf("%s",nts[edges[e].label]);

  if(edges[e].type==4)
    {
      printf("_NA~%d",edges[e].numchild);
      return;
    }
  
  if(1)
    {
      printf("~%s",current->words[current->wordpos[edges[e].head]]);
      printf("~%d",edges[e].numchild);
      printf("~%d",find_childno(e,edges[e].headch));
    }

}

void print_edges_flat(int e)
{
  int i,j,w,next;
  int flag;

  flag=1;

  if(e==-1)
    {
      printf("T/TRACE ");
      return;
    }

if(TREEBANKOUTPUTFLAG &&
   (edges[e].label == NT_NP || edges[e].label == NT_NPA)
     && edges[e].numchild ==1)
    {
      j= childs[edges[e].child1];
      if(edges[j].label == NT_NPB)
	flag=0;
    }


  /*POS tag case*/

  /*POS tag - print preceding punctuation*/
  if(edges[e].type==0)
    {
      if(edges[e].head==0)
	for(i=0;i<current->wordpos[edges[e].head];i++)
	  printf("%s/PUNC%s ",current->words[i],current->tags[i]);

      printf("%s/%s ",current->words[current->wordpos[edges[e].head]]
	     ,nts[edges[e].headtag]);
      w=current->wordpos[edges[e].head];
      if(edges[e].head==current->nws_np-1)
	next=current->nws;
      else
	next=current->wordpos[edges[e].head+1];
      for(i=w+1;i<next;i++)
	printf("%s/PUNC%s ",current->words[i],current->tags[i]);
      return;
    }

  if(flag)
    {
      printf("(");
      print_wholent(e);
      printf(" ");
    }

  print_edges_flat(childs[edges[e].child1]);

  for(i=edges[e].child1+1;i<edges[e].child1+edges[e].numchild;i++)
    {    
      print_edges_flat(childs[i]);
    }

  if(flag)
    printf(") ");
}

void print_chart()
{
  int i;
  int dist,subcat;

  for(i=0;i<numedges;i++)
    {
      printf("EDGE %d %g %g %d %d %d ",i,edges[i].prob,edges[i].prob2,edges[i].stop,edges[i].type,edges[i].label);

      printf(" L ");
      onectxt_to_dist_subcat(&edges[i].lc,&dist,&subcat,1);
      printf("%d %d HV%d",dist,subcat,edges[i].hasverb);

      printf(" R ");
      onectxt_to_dist_subcat(&edges[i].rc,&dist,&subcat,0);
      printf("%d %d\n",dist,subcat);

      print_edge(i,0);
      printf("\n\n");
    }

}

void join_2_edges_follow(int e1,int s,int m,int e,int *e2s,int ne2s)
{
  int h2_is_verb,e2,i2;

  int e1hlabel;

  /*e3,e4,d3,d4 are variables for new edges/distances created when
    e1/e2 are respectively the heads

    e3flag,e4flag = 0/1 if e3/e4 haven't/have been created
  */

  edge_type e3;
  int e3flag=0;
  int e3flag2=0;

  int dist,subcat,cc,punc;
  int wcc,tcc,wpunc,tpunc;
  int wh,ch,th;
  int wm,cm,tm;
  int p;
  int i,k;
  double prob,combineprob;
  int e1c;

/*ccprob is used to prefer wide scope premodification of CCed phrases,
    and narrow scope post modification*/

  double ccprob_post;

  /* d3 is e1 as head, e2 as modifier
     d4 is e2 as head, e1 as modifier
     
     calulate the distance variables and heads - the parent depends on 
     later calls in this routine

  */

  e1c = childs[edges[e1].child1+edges[e1].numchild-1];

  if(edges[e1].type==0)
    e1hlabel=edges[e1].headtag;
  else
    e1hlabel=edges[edges[e1].headch].label;

  if(edges[e1].type==1&&edges[e1].label==NT_NPB)
    e1hlabel=edges[e1c].label;

  if(edges[e1].stop==0 &&
     (edges[e1].type==3||edges[e1].type==1))
/*     &&tablef[edges[e1].label][e1hlabel][edges[e2].label])*/
    {

      for(i2=0;i2<ne2s;i2++)
	{
	  e2 = e2s[i2];

	  combineprob=edges[e1].prob+edges[e2].prob;
  
	  if(combineprob>=pthresh &&
	     (sindex[s][e]==-1 || combineprob>=(bestprobs[s][e]-BEAMPROB)) &&
	     edges[e2].stop ==1 &&
	     tablef[edges[e1].label][e1hlabel][edges[e2].label])
	    {
	      h2_is_verb=edges[e2].hasverb;
	      if(e3flag2==0)
		{
		  if(edges[e1].label!=NT_NPB)
		    {
		      onectxt_to_dist_subcat(&edges[e1].rc,&dist,&subcat,0);
		      punc = current->commaats[m];
		      if(punc)
			{
			  wpunc=current->commawords[m];
			  tpunc=current->commatags[m];
			}
		      else
			wpunc=tpunc=0;
		      wh=current->wordnos[edges[e1].head];
		      th=edges[e1].headtag;
		      ch=edges[e1].label;
		      ch=e1hlabel;
		    }
		  else
		    {
		      wh=current->wordnos[edges[e1c].head];
		      th=edges[e1c].headtag;
		      ch=edges[e1c].label;
		      
		      wm=current->wordnos[edges[e2].head];
		      tm=edges[e2].headtag;
		      cm=edges[e2].label;
		      dist = 10;
		      subcat = 0;
		      punc=current->commaats[m];
		      if(punc)
			{
			  wpunc=current->commawords[m];
			  tpunc=current->commatags[m];
			}
		      else
			wpunc=tpunc=0;
		    }
		  

		  wcc=tcc=cc=0;
		  
		  p=edges[e1].label;
		  e3flag2 = 1;
		}	  

	      wm=current->wordnos[edges[e2].head];
	      tm=edges[e2].headtag;
	      cm=edges[e2].label;

	      if(edges[e1].type!=0 && argmap[edges[e1].label]==argmap[edges[edges[e1].headch].label])
		ccprob_post=-CCPROBSMALL;
	      else
		ccprob_post=0;
	      
	      prob=get_dependency_prob_witheffhash(wm,tm,cm,wh,th,p,ch,dist,subcat,cc,wcc,tcc,punc,wpunc,tpunc,&new_hash,&eff_hash)+combineprob+ccprob_post;



	      if(prob>pthresh)
		{
		  calc_contexts(&edges[e1].lc,&edges[e1].rc,&edges[e2].lc,&edges[e2].rc,&e3.lc,&e3.rc,1,h2_is_verb,edges[e2].label);
		  e3.hasverb = edges[e1].hasverb || edges[e2].hasverb;

		  if(edges[e1].label==NT_NPB)
		    {
		      e3.lc.nvs=0;
		      e3.lc.adj=1;
		      e3.rc.nvs=0;		      
		      e3.rc.adj=1;
		      e3.hasverb=0;
		    }
		  
		  if(e3flag==0)
		    {
		      e3.head=edges[e1].head;
		      e3.headtag=edges[e1].headtag;
		      e3.type=3;
		      e3flag=1;
		      e3.headch=edges[e1].headch;
		      e3.label=edges[e1].label;
		      e3.headlabel=edges[e1].headlabel;
		      e3.valid=1;
		      e3.stop=0;
		      e3flag = 1;
		    }

		  e3.prob=prob;
		  
		  edges[numedges]=e3;
		  
		  if((k=add_edge(s,e))!=-1)
		    {
		      edges[k].numchild=edges[e1].numchild+1;
		      edges[k].child1=numchilds;
		      for(i=0;i<edges[e1].numchild;i++)
			childs[numchilds+i]=childs[edges[e1].child1+i];
		      childs[numchilds+edges[k].numchild-1]=e2;
		      /*	      printf("AAAA\n");
				      print_edge(k,0);*/
		      numchilds+=edges[k].numchild;
		    }
		}
	    }
	}
    }
}

void join_2_edges_precede(int e2,int s,int m,int e,int *e1s,int ne1)
{
  int h1_is_verb,e1;

  int e2hlabel;

  /*e3,e4,d3,d4 are variables for new edges/distances created when
    e1/e2 are respectively the heads

    e3flag,e4flag = 0/1 if e3/e4 haven't/have been created
  */

  edge_type e4;
  int e4flag=0;
  int e4flag2=0;

  int dist,subcat,cc,punc;
  int wh,ch,th;
  int wm,cm,tm;
  int p;
  int i,i2,k;

  int wcc,tcc,wpunc,tpunc;

  double prob,combineprob;
  int e2c;

/*ccprob is used to prefer wide scope premodification of CCed phrases,
    and narrow scope post modification*/

  double ccprob_pre;


/*  if(edges[e1].label==5) return;
  if(edges[e2].label==5) return;*/

  /* d3 is e1 as head, e2 as modifier
     d4 is e2 as head, e1 as modifier
     
     calulate the distance variables and heads - the parent depends on 
     later calls in this routine

  */

  /*now for the extend case*/

  e2c = childs[edges[e2].child1];
  
  if(edges[e2].type==0)
    e2hlabel=edges[e2].headtag;
  else
    e2hlabel=edges[edges[e2].headch].label;

  if(edges[e2].label == NT_NPB)
    e2hlabel=edges[e2c].label;

  /*now the extend case again*/

  if(edges[e2].stop==0 && 
     (edges[e2].type==3||edges[e2].type==1))
/*      &&tablep[edges[e2].label][e2hlabel][edges[e1].label])*/
    {
      for(i2=0;i2<ne1;i2++)
	{
	  e1 = e1s[i2];
	  
	  combineprob=edges[e1].prob+edges[e2].prob;

/*	  printf("BBB %g\n",combineprob);
	  
	  print_edge(e1,0);
	  print_edge(e2,0);*/

	  
	  if(tablep[edges[e2].label][e2hlabel][edges[e1].label] &&
	     combineprob>=pthresh &&
	     edges[e1].stop ==1 &&
	     (sindex[s][e]==-1 || combineprob>=(bestprobs[s][e]-BEAMPROB)))
	    {

	      h1_is_verb=edges[e1].hasverb;

	      if(e4flag2==0)
		{
		  if(edges[e2].type!=0 &&
		     argmap[edges[e2].label]==argmap[edges[edges[e2].headch].label])
		    ccprob_pre=0;
		  else
		    ccprob_pre=-CCPROBSMALL;

		  if(edges[e2].label!=NT_NPB)
		    {
		      onectxt_to_dist_subcat(&edges[e2].lc,&dist,&subcat,1);
		      punc = current->commaats[m];
		      if(punc)
			{
			  wpunc = current->commawords[m];
			  tpunc = current->commatags[m];
			}
		      else
			wpunc=tpunc=0;
		      
		      wh=current->wordnos[edges[e2].head];
		      th=edges[e2].headtag;
		      ch=edges[e2].label;
		      ch=e2hlabel;
		    }
		  else
		    {
		      wh=current->wordnos[edges[e2c].head];
		      th=edges[e2c].headtag;
		      ch=th;
		      cc=0;
		      
		      dist = 110;
		      subcat = 0;
		      punc=current->commaats[m];
		      if(punc)
			{
			  wpunc = current->commawords[m];
			  tpunc = current->commatags[m];
			}
		      else
			wpunc=tpunc=0;
		    }
		  wcc=tcc=cc=0;
		  
		  p=edges[e2].label;
		  		  
		  e4flag2 = 1;
		}


	      wm=current->wordnos[edges[e1].head];
	      tm=edges[e1].headtag;
	      cm=edges[e1].label;
	      	      
	      prob=get_dependency_prob_witheffhash(wm,tm,cm,wh,th,p,ch,dist,subcat,cc,wcc,tcc,punc,wpunc,tpunc,&new_hash,&eff_hash)+combineprob+ccprob_pre;

	      if(prob>pthresh)
		{
		  calc_contexts(&edges[e1].lc,&edges[e1].rc,&edges[e2].lc,&edges[e2].rc,&e4.lc,&e4.rc,2,h1_is_verb,edges[e1].label);
		  e4.hasverb=edges[e1].hasverb || edges[e2].hasverb;

		  if(edges[e2].label==NT_NPB)
		    {
		      e4.lc.nvs=0;
		      e4.lc.adj=1;
		      e4.rc.nvs=0;
		      e4.rc.adj=1;
		      e4.hasverb=0;
		    }

		  if(e4flag==0)
		    {
		      e4.head=edges[e2].head;
		      e4.headtag=edges[e2].headtag;
		      e4.type=3;
		      e4.headch=edges[e2].headch;
		      e4.label=edges[e2].label;
		      e4.headlabel=edges[e2].headlabel;
		      e4.valid=1;
		      e4.stop=0;
		      
		      e4flag=1;
		    }

		  e4.prob=prob;

		  edges[numedges]=e4;
		  
		  if((k=add_edge(s,e))!=-1)
		    {
		      edges[k].numchild=edges[e2].numchild+1;
		      edges[k].child1=numchilds;
		      for(i=0;i<edges[e2].numchild;i++)
			childs[numchilds+i+1]=childs[edges[e2].child1+i];
		      childs[numchilds]=e1;
		      /*printf("AAAA\n");
			print_edge(k,0);*/
		      numchilds+=edges[k].numchild;
		    }
		}
	    }
	}
    }
}



void complete(int s,int e)
{
  int i,j,k,k2;
  int t2[100000],t3[100000],t5[100000];
  int nt2,nt3,nt5;

  if(numedges>(PMAXEDGES*(2.0/3.0))
     || numchilds > (PMAXCHILDS*(2.0/3.0)))
    return;

  init_saments();

  for(i=s;i<e;i++)
    if( !PUNC_FLAG || (!current->commaats2[i]||e==(current->nws_np-1)||current->commaats[e]) )
      {
	nt2=nt3=nt5=0;
	
	for(k=sindex[i+1][e];k<=eindex[i+1][e];k++)
	  {	
	    if(inbeam2(k,i+1,e))
	      if(edges[k].stop==1)		      
		{
		  t2[nt2] = k;
		  nt2++;
		}
	      else
		{
		  t3[nt3] = k;
		  nt3++;
		} 

	  }

	for(j=sindex[s][i];j<=eindex[s][i];j++)
	  if(inbeam2(j,s,i))
	    if(edges[j].stop==0)
	      {
		join_2_edges_follow(j,s,i,e,t2,nt2);
	      }
	    else
	      {
		t5[nt5] = j;
		nt5++;
	      }

	for(k2=0;k2<nt3;k2++)
	  {
	    k = t3[k2];
	    join_2_edges_precede(k,s,i,e,t5,nt5);
	  }

	if(current->tagnos[i+1]==NT_CC&&i<e-1)
	  for(j=sindex[s][i];j<=eindex[s][i];j++)
	    if(inbeam2(j,s,i))
	      for(k=sindex[i+2][e];k<=eindex[i+2][e];k++)
		if(inbeam2(k,i+2,e))
		  join_2_edges_cc(j,k,s,i,e);
      }
    else
      {
	nt2=nt3=nt5=0;
	
	for(k=sindex[i+1][e];k<=eindex[i+1][e];k++)
	  {	
	    if(inbeam2(k,i+1,e))
	      if(edges[k].stop==1)		      
		{
		  t2[nt2] = k;
		  nt2++;
		}
	      else
		{
		  t3[nt3] = k;
		  nt3++;
		} 

	  }

	for(j=sindex[s][i];j<=eindex[s][i];j++)
	  if(inbeam2(j,s,i))
	    if(edges[j].stop==0)
	      {
		;
/*		join_2_edges_follow(j,s,i,e,t2,nt2);*/
	      }
	    else
	      {
		t5[nt5] = j;
		nt5++;
	      }

	for(k2=0;k2<nt3;k2++)
	  {
	    k = t3[k2];
	    if(edges[k].label == NT_NPB)
	      join_2_edges_precede(k,s,i,e,t5,nt5);
	  }
      }
  
  add_singles_stops(s,e);

}

int print_best_parse()
{
  int i,best;

  best=-1;

  for(i=sindex[0][current->nws_np-1];i<=eindex[0][current->nws_np-1];i++)
    if(edges[i].label==43 && edges[i].stop==0)
      {
/*	printf("prob %g\n",edges[i].prob);
	print_edge(i,0);*/
	if(best==-1)
	  best=i;
	else
	  if(edges[i].prob>edges[best].prob)
	    best=i;
      }

  if(best!=-1)
    {
      printf("PROB %d %g %d \n",best,edges[best].prob,edges[best].stop);

      print_edge(best,0);
      print_edges_flat(best);
      printf("\n");
    }

  return best;
}

void join_2_edges_cc(int e1,int e2,int s,int m,int e)
{
  int h2_is_verb;

  int e1hlabel;

  /*e3,d3,are variables for new edges/distances created when
    e1 is the head

    e3flag = 0/1 if e3 hasn't/has been created
  */

  edge_type e3;
  int e3flag=0;

  int dist,subcat,cc,punc;
  int wh,ch,th;
  int wm,cm,tm;
  int p;

  int wcc,tcc,wpunc,tpunc;

  int i,k;
  double prob,combineprob;

  combineprob=edges[e1].prob+edges[e2].prob;

  if(combineprob<pthresh) return;

  if(sindex[s][e]!=-1)
    if( combineprob < (bestprobs[s][e]-BEAMPROB) )
      return;

  h2_is_verb=edges[e2].hasverb;

  /* d3 is e1 as head, e2 as modifier
     
     calulate the distance variables and heads - the parent depends on 
     later calls in this routine

  */

  onectxt_to_dist_subcat(&edges[e1].rc,&dist,&subcat,0);
  punc = current->commaats[m]||current->commaats[m+1];

  if(current->commaats[m])
    {
      wpunc = current->commawords[m];
      tpunc = current->commatags[m];
    }
  else if(current->commaats[m+1])
    {
      wpunc = current->commawords[m+1];
      tpunc = current->commatags[m+1];
    }
  else
    wpunc=tpunc=0;
  
  wh=current->wordnos[edges[e1].head];
  th=edges[e1].headtag;
  wm=current->wordnos[edges[e2].head];
  tm=edges[e2].headtag;
  ch=edges[e1].label;
  cm=edges[e2].label;
  cc=1;
  wcc=current->wordnos[m+1];
  tcc=current->tagnos[m+1];

  /*now for the extend case*/

  if(edges[e1].type==0)
    e1hlabel=edges[e1].headtag;
  else
    e1hlabel=edges[edges[e1].headch].label;


  if((edges[e1].type==3||edges[e1].type==1)
     &&tablef[edges[e1].label][e1hlabel][edges[e2].label]
     &&edges[e1].stop==0 && edges[e2].stop==1)
    {
      p=edges[e1].label;
      ch=e1hlabel;

      prob=get_dependency_prob_witheffhash(wm,tm,cm,wh,th,p,ch,dist,subcat,cc,wcc,tcc,punc,wpunc,tpunc,&new_hash,&eff_hash)+combineprob;

      if(prob>pthresh)
	{
	  calc_contexts(&edges[e1].lc,&edges[e1].rc,&edges[e2].lc,&edges[e2].rc,&e3.lc,&e3.rc,1,h2_is_verb,edges[e2].label);
	  e3.hasverb = edges[e1].hasverb || edges[e2].hasverb;
	      
	  if(e3flag==0)
	    {
	      e3.head=edges[e1].head;
	      e3.headtag=edges[e1].headtag;
	      e3.type=3;
	      
	      e3flag=1;
	    }
	  e3.headch=edges[e1].headch;
	  e3.prob=prob;
	  e3.label=edges[e1].label;
	  e3.headlabel=edges[e2].label;
	  e3.valid=1;
	  e3.stop=0;
	  edges[numedges]=e3;

	  if((k=add_edge(s,e))!=-1)
	    {
	      edges[k].numchild=edges[e1].numchild+2;
	      edges[k].child1=numchilds;
	      for(i=0;i<edges[e1].numchild;i++)
		childs[numchilds+i]=childs[edges[e1].child1+i];
	      childs[numchilds+edges[k].numchild-2]=sindex[m+1][m+1];
	      childs[numchilds+edges[k].numchild-1]=e2;
	      numchilds+=edges[k].numchild;
	    }
	}
    }
}


#define MAXSINGLES 5

/* continue to add singles and stops until no more edges are being added */

void add_singles_stops(int s,int e)
{
  int i,n;

  ADDFLAG=0;
  add_singles(s,e,sindex[s][e],eindex[s][e]);
  add_traces(s,e);

  numadds = &numadds1;
  adds = adds1;
  numadds1 = 0;

  add_stops(s,e,sindex[s][e],eindex[s][e]);

/*  printf("ADDS");
  for(i=0;i<numadds;i++)
    printf(" %d",adds[i]);
  printf("\n");*/

  n=1;
  while(ADDFLAG==1&&n<=MAXSINGLES)
    {
      ADDFLAG=0;

      numadds = &numadds2;
      adds = adds2;
      numadds2 = 0;

      for(i=0;i<numadds1;i++)
	add_singles(s,e,adds1[i],adds1[i]);

      add_traces(s,e);

      numadds = &numadds1;
      adds = adds1;
      numadds1 = 0;

      for(i=0;i<numadds2;i++)
	add_stops(s,e,adds2[i],adds2[i]);

      n++;
    }
  numadds1=numadds2=0;
}
int empty_ctxt(ctxt_type *c)
{
  if(c->np > 0 ||
     c->s > 0 ||
     c->sbar > 0 ||
     c->vp > 0 ||
     c->other > 0 ||
     c->gap > 0 )
    return 0;

  return 1;
}


void add_stops(int s,int e,int si,int ei)
{
  int i,j,k;
  int dist,subcat,cc,punc;
  int wh,ch,th;
  int wm,cm,tm;
  int p;

  int ec;

  int ehlabel;

  for(i=si;i<=ei;i++)
    if(edges[i].stop==0&&inbeam(i,s,e)&&edges[i].valid==1 &&
       empty_ctxt(&edges[i].lc) && empty_ctxt(&edges[i].rc))
      {
	edges[numedges]=edges[i];
	edges[numedges].stop=1;
	
	wm=STOPWORD;
	tm=STOPNT;
	cm=STOPNT;
	
	cc=0;
	
	ec = childs[edges[i].child1+edges[i].numchild-1];	    
	
	if(edges[i].type==0)
	  ehlabel=edges[i].headtag;
	else
	  ehlabel=edges[edges[i].headch].label;
	
	if(edges[i].label == NT_NPB)
	  {
	    ehlabel=edges[ec].label;
	    wh=current->wordnos[edges[ec].head];
	    th=edges[ec].headtag;
	    subcat = 0;
	    dist = 10;
	  }
	else
	  {
	    wh=current->wordnos[edges[i].head];
	    th=edges[i].headtag;	
	    onectxt_to_dist_subcat(&edges[i].rc,&dist,&subcat,0);
	  }
	
	punc = 0;
	p=edges[i].label;
	ch=ehlabel;
	
/*	printf("EDGE1 %d %g %g %d %d %d ",i,edges[i].prob,edges[i].prob2,edges[i].stop,edges[i].type,edges[i].label);

	printf(" L ");
	onectxt_to_dist_subcat(&edges[i].lc,&dist2,&subcat2,1);
	printf("%d %d",dist2,subcat2);
       
	printf(" R ");
	onectxt_to_dist_subcat(&edges[i].rc,&dist2,&subcat2,0);
	printf("%d %d\n",dist2,subcat2);
	
	print_edge(i,0);

	printf("\n\n");

	printf("PROB1 %g\n",get_dependency_prob_witheffhash(wm,tm,cm,wh,th,p,ch,dist,subcat,cc,punc,&new_hash,&eff_hash));*/

	
	edges[numedges].prob=get_dependency_prob_witheffhash(wm,tm,cm,wh,th,p,ch,dist,subcat,cc,0,0,punc,0,0,&new_hash,&eff_hash)+edges[i].prob;
	
	ec = childs[edges[i].child1];
	
	if(edges[i].type==0)
	  ehlabel=edges[i].headtag;
	else
	  ehlabel=edges[edges[i].headch].label;
	
	if(edges[i].label == NT_NPB)
	  {
	    ehlabel=edges[ec].label;
	    wh=current->wordnos[edges[ec].head];
	    th=edges[ec].headtag;
	    subcat = 0;
	    dist = 110;
	  }
	else
	  {
	    wh=current->wordnos[edges[i].head];
	    th=edges[i].headtag;	
	    onectxt_to_dist_subcat(&edges[i].lc,&dist,&subcat,1);
	  }
	
	punc = 0;
	p=edges[i].label;
	ch=ehlabel;
	
/*	printf("PROB2 %g\n",get_dependency_prob_witheffhash(wm,tm,cm,wh,th,p,ch,dist,subcat,cc,punc,&new_hash,&eff_hash));*/
	
	edges[numedges].prob+=get_dependency_prob_witheffhash(wm,tm,cm,wh,th,p,ch,dist,subcat,cc,0,0,punc,0,0,&new_hash,&eff_hash);
	
	if((k=add_edge(s,e))!=-1) /* edge is added to the chart, so add */
	  {			/* its children */
	    edges[k].child1=numchilds;
	    edges[k].numchild=edges[i].numchild;
	    for(j=0;j<edges[i].numchild;j++)
	      childs[numchilds+j]=childs[edges[i].child1+j];
	    numchilds+=edges[k].numchild;
	  }	
      }
}

void calc_prob2(int edge)
{
  int ch,wh,th;
  double prob;

  if(edges[edge].type==0)
    {
      edges[edge].prob2 = 0;
      return;
    }

  ch=edges[edge].label;
  wh=current->wordnos[edges[edge].head];
  th=edges[edge].headtag;

  prob= get_prior_prob_witheffhash(ch,wh,th,&new_hash,edges[edge].head,edges[edge].headtag);

  edges[edge].prob2 = edges[edge].prob + prob;
}



int inbeam(int edge,int s,int e)
{
  if(edges[edge].type!=1)
    {
      if(edges[edge].prob2 < (bestprobs[s][e]-BEAMPROB) ) 
        return 0;
    }
  else
    if(edges[edge].label == NT_NP || edges[edge].label == NT_NPA)
      {
        if(edges[edge].prob2 < (bestprobs[s][e]-BEAMPROB - 3) ) 
          return 0;
      }
  else
    if(edges[edge].prob2 < (bestprobs[s][e]-BEAMPROB) ) 
        return 0;

  return 1;
}

int inbeam2(int edge,int s,int e)
{
  if(edges[edge].inbeam<=1) return edges[edge].inbeam;

  if(edges[edge].type!=1)
    {
      if(edges[edge].prob2 < (bestprobs[s][e]-BEAMPROB) ) 
        {edges[edge].inbeam=0;return 0;}
    }
  else
    if(edges[edge].label == NT_NP || edges[edge].label == NT_NPA)
      {
        if(edges[edge].prob2 < (bestprobs[s][e]-BEAMPROB - 3) ) 
          {edges[edge].inbeam=0;return 0;}
      }
  else
    if(edges[edge].prob2 < (bestprobs[s][e]-BEAMPROB) ) 
        {edges[edge].inbeam=0;return 0;}


  edges[edge].inbeam=1;
  return 1;
}


void add_traces(int s,int e)
{
  int i,j,k,si,ei;
  char *tl;
  int ehlabel;

  int dist,subcat,cc,punc;
  int wh,ch,th;
  int wm,cm,tm;
  int p;

  int ec;
  
  si=sindex[s][e];
  ei=eindex[s][e];

  for(i=si;i<=ei;i++)
    if(edges[i].rc.gap>0 && edges[i].rc.np>0 &&
       edges[i].stop==0&&inbeam(i,s,e)&&edges[i].valid==1)
      {
	edges[numedges]=edges[i];
	
/*	onectxt_to_dist_subcat(&edges[i].rc,&dist,0,1,e<current->nws_np-1&&current->commaats[e],0);*/

	edges[numedges].rc.gap--;
	edges[numedges].rc.np--;
	edges[numedges].rc.adj=0;


	wm=TRACEWORD;
	tm=TRACETAG;
	cm=TRACENT;
	
	cc=0;
	
	ec = childs[edges[i].child1+edges[i].numchild-1];	    
	
	if(edges[i].type==0)
	  ehlabel=edges[i].headtag;
	else
	  ehlabel=edges[edges[i].headch].label;
	
	if(edges[i].label == NT_NPB)
	  {
	    ehlabel=edges[ec].label;
	    wh=current->wordnos[edges[ec].head];
	    th=edges[ec].headtag;
	    subcat = 0;
	    dist = 10;
	  }
	else
	  {
	    wh=current->wordnos[edges[i].head];
	    th=edges[i].headtag;	
	    onectxt_to_dist_subcat(&edges[i].rc,&dist,&subcat,0);
	  }
	
	punc = 0;
	p=edges[i].label;
	ch=ehlabel;

	edges[numedges].prob=get_dependency_prob_witheffhash(wm,tm,cm,wh,th,p,ch,dist,subcat,cc,0,0,punc,0,0,&new_hash,&eff_hash)+edges[i].prob;
	    
	if((k=add_edge(s,e))!=-1) /* edge is added to the chart, so add */
	  {			  /* its children */
	    edges[k].child1=numchilds;
	    edges[k].numchild=edges[i].numchild+1;
	    for(j=0;j<edges[i].numchild;j++)
	      childs[numchilds+j]=childs[edges[i].child1+j];
	    childs[numchilds+edges[i].numchild]=-1;
	    numchilds+=edges[k].numchild;
	  }
      }
    else if(edges[i].lc.gap>0 && edges[i].lc.np>0 &&
       edges[i].stop==0&&inbeam(i,s,e)&&edges[i].valid==1)
      {
	edges[numedges]=edges[i];

	edges[numedges].lc.gap--;
	edges[numedges].lc.np--;
	edges[numedges].lc.adj=0;

	wm=TRACEWORD;
	tm=TRACETAG;
	cm=TRACENT;

	cc = 0;

	ec = childs[edges[i].child1];
	
	if(edges[i].type==0)
	  ehlabel=edges[i].headtag;
	else
	  ehlabel=edges[edges[i].headch].label;
	
	if(edges[i].label == NT_NPB)
	  {
	    ehlabel=edges[ec].label;
	    wh=current->wordnos[edges[ec].head];
	    th=edges[ec].headtag;
	    subcat = 0;
	    dist = 110;
	  }
	else
	  {
	    wh=current->wordnos[edges[i].head];
	    th=edges[i].headtag;	
	    onectxt_to_dist_subcat(&edges[i].lc,&dist,&subcat,1);
	  }
	
	punc = 0;
	p=edges[i].label;
	ch=ehlabel;
	
	edges[numedges].prob=get_dependency_prob_witheffhash(wm,tm,cm,wh,th,p,ch,dist,subcat,cc,0,0,punc,0,0,&new_hash,&eff_hash)+edges[i].prob;
	    
	if((k=add_edge(s,e))!=-1) /* edge is added to the chart, so add */
	  {			/* its children */
	    edges[k].child1=numchilds;
	    edges[k].numchild=edges[i].numchild+1;
	    for(j=0;j<edges[i].numchild;j++)
	      childs[numchilds+j+1]=childs[edges[i].child1+j];
/*	    childs[numchilds]=childs[edges[i].child1];*/
	    childs[numchilds]=-1;
	    numchilds+=edges[k].numchild;
	  }
      }
}





