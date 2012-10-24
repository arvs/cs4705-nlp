#
# Compilation and link flags
#
CC = gcc
CFLAGS = -O3 -Wimplicit
LIBS=  -lm 

#
# My targets to build
#

OBJ = main.o lexicon.o grammar.o mymalloc.o mymalloc_char.o key.o hash.o genprob.o prob.o effhash.o prob_witheffhash.o readevents.o sentence.o edges.o chart.o
HDR =  lexicon.h grammar.h mymalloc.h mymalloc_char.h key.h hash.h genprob.h prob.h effhash.h prob_witheffhash.h readevents.h sentence.h edges.h chart.h

all: sfs

$(OBJ) : $(HDR)

ofiles: 
	gcc -c main.c lexicon.c sentence.c edges.c chart.c mymalloc.c grammar.c key.c hash.c genprob.c prob.c effhash.c prob_witheffhash.c readevents.c mymalloc_char.c $(CFLAGS) $(LIBS)

sfs : main.o lexicon.o grammar.o mymalloc.o mymalloc_char.o key.o hash.o genprob.o prob.o effhash.o prob_witheffhash.o readevents.o sentence.o edges.o chart.o
	gcc main.o lexicon.o grammar.o mymalloc.o mymalloc_char.o key.o hash.o genprob.o prob.o effhash.o prob_witheffhash.o readevents.o sentence.o edges.o chart.o -o parser $(CFLAGS) $(LIBS)

cleanup:
	rmz *.o

