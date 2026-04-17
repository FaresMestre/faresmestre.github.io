#include<stdlib.h>
#include<stdio.h>
#include<time.h>
#include"music_structs.h"

#define SEED (unsigned long) time(NULL)

struct binomial {
	unsigned n;
	unsigned p;	//Percentage from 0 to 100%
};
typedef struct binomial binomial;

void print_pascal();

//Computation functions

int factorial(int n);
double power(double x, unsigned n);
long long comb(int n, int k);
double binomial_of_k(binomial b, int k);

//Random pick functions

key pick_next_key(note *n);
rythm pick_next_rythm(channel *c);
unsigned pick_next_velocity(note *n);
chord *pick_progression();

//Generation

void generate_melody(channel *c);

//Global variable

extern long long **pascal_row;
