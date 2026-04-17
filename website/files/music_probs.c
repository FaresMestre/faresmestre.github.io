#include"music_probs.h"

long long **pascal_row;

void print_pascal() {
	for (int i = 0 ; i <= 60 ; ++i) {
		printf("%d : ", i);
		for (int j = 0 ; j <= i ; ++j) printf("%lld ", (long long) pascal_row[i][j]);
		printf("\n");
	}
}

//Computation functions implementation

int factorial(int n) {
	if (n <= 0) return 1;
	return n*factorial(n-1);
}

double power(double x, unsigned n) {
	if (n == 0) return 1;
	if (n == 1) return n;
	if (n % 2 == 0) return power(x * x, n/2);
	else return x * power(x * x, n/2);
}

long long comb(int n, int k) {
	if (pascal_row[n][k] != 0) return pascal_row[n][k];
	pascal_row[n][k] = comb(n-1, k-1) + comb(n-1, k);
	return pascal_row[n][k];
}

double binomial_of_k(binomial b, int k) {
	long long c = comb(b.n, k);
	return c * power(b.p, k) * power(1 - b.p, b.n - k);
}

//Random pick functions implementation

key pick_next_key(note *n) {
	unsigned x = 389;	//Random variable to be picked between 0 and 100
	int k = -30;		//Random variable : offset between n -> pitch and res
	key res = n -> pitch;
	if (n -> pitch != Silent) while (n -> pitch + k < 0 || n -> pitch + k > 27) {
		x = rand()%101;
		if (x < 5) k = -8;
		else if (x <  8) k = -7; else if (x < 10) k = -6; else if (x < 25) k = -5;
		else if (x < 35) k = -4; else if (x < 37) k = -3; else if (x < 40) k = -2;
		else if (x < 45) k = -1; else if (x < 50) k =  0; else if (x < 55) k =  1;
		else if (x < 58) k =  2; else if (x < 60) k =  3; else if (x < 70) k =  4;
		else if (x < 85) k =  5; else if (x < 87) k =  6; else if (x < 90) k =  7;
		else if (x < 95) k =  8; else return Silent;
	}
	else return rand()%28;
	return res + k;
}

rythm pick_next_rythm(channel *c) {
	unsigned duration = sum_durations(c);
	if (duration % 2 == 1) return sixteenth;
	if (duration <= 8) switch (rand()%4) {
		case 0 : return sixteenth;
		case 1 : return eighth;
		case 2 : return quarter;
		case 3 : return half;
	}
	else if (duration <= 12) switch(rand()%3) {
		case 0 : return sixteenth;
		case 1 : return eighth;
		case 2 : return quarter;
	}
	else switch (rand()%2) {
		case 0 : return sixteenth;
		case 1 : return eighth;
	}
	return sixteenth;
}

unsigned pick_next_velocity(note *n) {	//Has to use a cutoff for computing reasons : 0.5^60 is too small and uncomputable
	int x = 30;	//Random variable to be picked between 16 and 44
	int k = rand()%9986;
	if (k < 1) x = 16; 
	else if (k <    4) x = 17; else if (k <   12) x = 18; else if (k <   29) x = 19; else if (k <   65) x = 20;
	else if (k <  134) x = 21; else if (k <  256) x = 22; else if (k <  458) x = 23; else if (k <  770) x = 24;
	else if (k < 1220) x = 25; else if (k < 1826) x = 26; else if (k < 2589) x = 27; else if (k < 3488) x = 28;
	else if (k < 4480) x = 29; else if (k < 5505) x = 30; else if (k < 6497) x = 31; else if (k < 7396) x = 32;
	else if (k < 8159) x = 33; else if (k < 8765) x = 34; else if (k < 9215) x = 35; else if (k < 9527) x = 36;
	else if (k < 9729) x = 37; else if (k < 9851) x = 38; else if (k < 9920) x = 39; else if (k < 9956) x = 40;
	else if (k < 9973) x = 41; else if (k < 9981) x = 42; else if (k < 9984) x = 43;
	else x = 44;
	int res = n -> velocity + x - 10*n -> duration;
	if (res > 127) res = 127;
	if (res < 0) res = 0;
	return (unsigned) res;
}
	
chord *pick_progression() {
	chord *res = malloc(4 * sizeof(chord));
	int k = 0; 	//Random intermediate
	for (int i = 0 ; i < 4 ; ++i) {
		k = rand()%101;
		switch (i) {
		case 0 : if (k < 34) res[i] = C; else if (k < 67) res[i] = F; else res[i] = G; break;
		case 1 : if (k < 30) res[i] = d; else if (k < 60) res[i] = e; else if (k < 90) res[i] = a; else res[i] = bdim;
		break;
		case 2 : if (k < 20) res[i] = C; else if (k < 32) res[i] = d; else if (k < 44) res[i] = e;
			 else if (k < 64) res[i] = F; else if (k < 84) res[i] = G; else if (k < 96) res[i] = a;
			 else res[i] = bdim;
		break;
		case 3 : if (k < 33) res[i] = C; else if (k > 66) res[i] = F; else res[i] = G; break;
		default : res[i] = C;
		}
	}
	return res;
}

//Generation implementation

void generate_melody(channel *c) {
	if (sum_durations(c) == 0) append_note(c, new_note(rand()%28, rand()%5, rand()%64 + 64));
	while (!validate_channel(c)) {
		note *n = get_last_note(c);
		key next_key = pick_next_key(n);
		rythm next_rythm = pick_next_rythm(c);
		unsigned next_velocity = pick_next_velocity(n);
		note *next = new_note(next_key, next_rythm, next_velocity);
		append_note(c, next);
	}
}
