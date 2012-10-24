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

#include "edges.h"

int equal_ctxts(ctxt_type *c1,ctxt_type *c2)
{
  if( ( ( !(c1->nvs)&&c2->nvs ) ||
        (!(c2->nvs)&&c1->nvs) ) ||
     c1->adj!=c2->adj||

     c1->np!=c2->np||
     c1->s!=c2->s||
     c1->sbar!=c2->sbar||
     c1->vp!=c2->vp||
     c1->other!=c2->other||
     c1->gap!=c2->gap
     )
    return 0;

  return 1;
}

int equal_edges(edge_type *e1,edge_type *e2)
{
  if(!equal_ctxts(&e1->lc,&e2->lc)||
     !equal_ctxts(&e1->rc,&e2->rc)||
     e1->head!=e2->head||
     e1->headtag!=e2->headtag||
     e1->type!=e2->type||
     e1->valid!=e2->valid||
     e1->stop!=e2->stop||
     e1->headlabel!=e2->headlabel||
     e1->hasverb!=e2->hasverb||
     e1->label!=e2->label)
    return 0;

  return 1;
}

void calc_contexts(ctxt_type *l1,ctxt_type *r1,ctxt_type *l2,ctxt_type *r2,ctxt_type *l3,ctxt_type *r3,char head,char verb,int modnt)
{
  int modnt2;

  if(head==1)
    {
      (*l3)=(*l1);
      (*r3)=(*r1);

      r3->nvs=verb||r1->nvs||l2->nvs||r2->nvs;
      r3->adj=0;

      modnt2=gapmap[modnt];

      if(modnt2==NT_NPA)
	{
	  if(r3->np>0)
	    (r3->np)--;
	}
      else if(modnt2==NT_SBARA)
	{
	  if(r3->sbar>0)
	    (r3->sbar)--;
	}	
      else if(modnt2==NT_SA||modnt==NT_SGA)
	{
	  if(r3->s>0)
	    (r3->s)--;
	}	
      else if(modnt2==NT_VPA)
	  {
	    if(r3->vp>0)
	      (r3->vp)--;
	  }	
      else if(hasarg[modnt2])
	{
	  if(r3->other>0)
	    (r3->other)--;
	}	
      modnt2=gapmap[modnt];
      if(hasgap[modnt])
	{
	  if(r3->gap>0)
	    (r3->gap)--;
	}	
	
      return;
    }
  else if(head==2)
    {
      (*r3)=(*r2);
      (*l3)=(*l2);

      l3->nvs=verb||l1->nvs||r1->nvs||l2->nvs;
      l3->adj=0;

      modnt2=gapmap[modnt];

      if(modnt2==NT_NPA)
	{
	  if(l3->np>0)
	    (l3->np)--;
	}
      else if(modnt2==NT_SBARA)
	{
	  if(l3->sbar>0)
	    (l3->sbar)--;
	}	
      else if(modnt2==NT_SA||modnt==NT_SGA)
	{
	  if(l3->s>0)
	    (l3->s)--;
	}	
      else if(modnt2==NT_VPA)
	  {
	    if(l3->vp>0)
	      (l3->vp)--;
	  }	
      else if(hasarg[modnt2])
	{
	  if(l3->other>0)
	    (l3->other)--;
	}	
	
      if(hasgap[modnt])
	{
	  if(l3->gap>0)
	    (l3->gap)--;
	}	

      return;
    }
  else 
    assert(0);

}


void onectxt_to_dist_subcat(ctxt_type *l,int *dist,int *subcat,char drc)
{
  *dist = 0;
  if(drc==1)
    (*dist)+=100;

  if(l->adj == 1)
    (*dist)+=10;

  if(l->nvs >= 1)
    (*dist)+=1;

  *subcat = 100000 * l->np +
            10000 * l->s +
            1000 * l->sbar +
            100 * l->vp +
            10 * l->other +
            l->gap;
}

void clear_args(ctxt_type *c)
{
  c->np=0;
  c->s=0;
  c->sbar=0;
  c->vp=0;
  c->other=0;
  c->gap=0;
}

void find_args(ctxt_type *c,int sub)
{
  if(sub==0)
    {
      clear_args(c);
      return;
    }

  c->np=sub/10000;
  sub=sub - (c->np*10000);

  c->s=sub/1000;
  sub=sub - (c->s*1000);

  c->sbar=sub/100;
  sub=sub - (c->sbar*100);

  c->vp=sub/10;
  sub=sub - (c->vp*10);

  c->other=sub;

  c->gap=0;
}
