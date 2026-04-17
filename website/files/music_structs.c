#include"music_structs.h"

//Display functions implementation

void print_note(note *n) {
	printf("pitch : ");
	switch (n -> pitch) {
		case C3 : printf("C3, "); break; case D3 : printf("D3, "); break; case E3 : printf("E3, "); break;
		case F3 : printf("F3, "); break; case G3 : printf("G3, "); break; case A3 : printf("A3, "); break;
		case B3 : printf("B3, "); break; case C4 : printf("C4, "); break; case D4 : printf("D4, "); break;
		case E4 : printf("E4, "); break; case F4 : printf("F4, "); break; case G4 : printf("G4, "); break;
		case A4 : printf("A4, "); break; case B4 : printf("B4, "); break; case C5 : printf("C5, "); break;
		case D5 : printf("D5, "); break; case E5 : printf("E5, "); break; case F5 : printf("F5, "); break;
		case G5 : printf("G5, "); break; case A5 : printf("A5, "); break; case B5 : printf("B5, "); break;
		case C6 : printf("C6, "); break; case D6 : printf("D6, "); break; case E6 : printf("E6, "); break;
		case F6 : printf("F6, "); break; case G6 : printf("G6, "); break; case A6 : printf("A6, "); break;
		case B6 : printf("B6, "); break; case Silent : printf("Silent, "); break;
	}
	printf("rythm : ");
	switch (n -> duration) {
		case full : printf("full, "); break;
		case half : printf("half, "); break;
		case quarter : printf("quarter, "); break;
		case eighth : printf("eighth, "); break;
		case sixteenth : printf("sixteenth, "); break;
		default : printf("error ? : %d,",n -> duration);	//Résout un bug qui ne devrait pas arriver ?????
	}
	printf("velocity : %d\n", n -> velocity);
}

void print_channel(channel *c) {
	for (note *n = c -> head ; n != NULL ; n = n -> next) print_note(n);
}

void print_measure(measure *m) {
	for (int i = 0 ; i < 10 ; ++i) {
		printf("--Channel %d--\n", i);
		print_channel(m -> channels[i]);
	}
}

void print_staff(staff *s) {
	for (int i = 0 ; i < 4 ; ++i) {
		printf("----Measure %d----\n", i);
		print_measure(s -> measures[i]);
	}
}

//Validation functions implementation

int sum_durations(channel *c) {
	int duration_sum = 0;
	for (note *n = c -> head ; n != NULL ; n = n -> next) switch (n -> duration) {
		case full : duration_sum += 16; break;
		case half : duration_sum += 8; break;
		case quarter : duration_sum += 4; break;
		case eighth : duration_sum += 2; break;
		case sixteenth : duration_sum += 1; break;
		default : return -1;
	}
	return duration_sum;
}

bool validate_channel(channel *c) { return sum_durations(c) == 16; }

bool validate_measure(measure *m) {
	for (int i = 0 ; i < 10 ; ++i) if (m -> channels[i] -> head != NULL && !validate_channel(m -> channels[i])) return false;
	return true;
}

//Initializers implementation

note *new_note(key pitch, rythm duration, unsigned velocity) {
	velocity = velocity%128;
	note *res = malloc(sizeof(note));
	res -> pitch = pitch; res -> duration = duration; res -> velocity = velocity; res -> next = NULL;
	return res;
}

channel *new_channel() {
	channel *res = malloc(sizeof(channel));
	res -> head = NULL;
	return res;
}

measure *new_measure() {
	measure *res = malloc(sizeof(measure));
	for (int i = 0 ; i < 10 ; ++i) res -> channels[i] = new_channel();
	return res;
}

staff *new_staff() {
	staff *res = malloc(sizeof(staff));
	for (int i = 0 ; i < 4 ; ++i) res -> measures[i] = new_measure();
	return res;
}


//Freeing functions implementation

void free_channel(channel *c) {
	note *curr = c -> head;
	if (curr != NULL) for (note *n = c -> head -> next ; n != NULL ; n = n -> next) {
		free(curr);
		curr = n;
	}
	free(curr);
}

void free_measure(measure *m) {
	for (int i = 0 ; i < 10 ; ++i) free_channel(m -> channels[i]);
}

void free_staff(staff *s) {
	for (int i = 0 ; i < 4 ; ++i) free_measure(s -> measures[i]);
}

//Structure manipulations implementation

note *get_last_note(channel *c) {
	note *res = c -> head;
	if (res != NULL) while (res -> next != NULL) res = res -> next;
	return res;
}

bool append_note(channel *c, note *n) {
	int nd = 0;
	switch (n -> duration) {
	case full : nd = 16; break;
	case half : nd = 8; break;
	case quarter : nd = 4; break;
	case eighth : nd = 2; break;
	case sixteenth : nd = 1; break;
	default : return false;
	}
	if (nd + sum_durations(c) > 16) return false;
	if (c -> head == NULL) c -> head = n;
	else {
		note *tail = c -> head;
		while (tail -> next != NULL) tail = tail -> next;
		tail -> next = n;
	}
	return true;
}

void set_measure_channel(measure *m, channel *c, size_t index) {
	index = index%10;
	m -> channels[index] = c;
}

void set_staff_measure(staff *s, measure *m, size_t index) {
	index = index%4;
	s -> measures[index] = m;
}

void append_chord(measure *m, chord c) {
	note* notes[3];
	switch (c) {
	case C : notes[0] = new_note(C3, quarter, 127); 
		 notes[1] = new_note(E3, quarter, 127); 
		 notes[2] = new_note(G3, quarter, 127);
		 break;
	case d : notes[0] = new_note(D3, quarter, 127); 
		 notes[1] = new_note(F3, quarter, 127); 
		 notes[2] = new_note(A3, quarter, 127);
		 break;
	case e : notes[0] = new_note(E3, quarter, 127); 
		 notes[1] = new_note(G3, quarter, 127); 
		 notes[2] = new_note(B3, quarter, 127);
		 break;
	case F : notes[0] = new_note(F3, quarter, 127); 
		 notes[1] = new_note(A3, quarter, 127); 
		 notes[2] = new_note(C4, quarter, 127);
		 break;
	case G : notes[0] = new_note(G3, quarter, 127); 
		 notes[1] = new_note(B3, quarter, 127); 
		 notes[2] = new_note(D4, quarter, 127);
		 break;
	case a : notes[0] = new_note(A3, quarter, 127); 
		 notes[1] = new_note(C4, quarter, 127); 
		 notes[2] = new_note(E4, quarter, 127);
		 break;
	case bdim : notes[0] = new_note(B3, quarter, 127); 
		    notes[1] = new_note(D4, quarter, 127); 
		    notes[2] = new_note(F4, quarter, 127);
		 break;
	}
	for (int i = 5 ; i < 8 ; ++i) append_note(m -> channels[i], notes[i-5]);
}
