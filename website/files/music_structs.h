#include<stdlib.h>
#include<stdio.h>
#include<stdbool.h>
#include<string.h>

enum key {C3, D3, E3, F3, G3, A3, B3, 
	C4, D4, E4, F4, G4, A4, B4, 
	C5, D5, E5, F5, G5, A5, B5,
	C6, D6, E6, F6, G6, A6, B6,
	Silent
}; //On est sur la gamme C/a
typedef enum key key;

enum rythm {full, half, quarter, eighth, sixteenth};
typedef enum rythm rythm;

enum chord {C, d, e, F, G, a, bdim};
typedef enum chord chord;

struct note {
	key pitch;
	rythm duration;
	unsigned velocity; //De 0 à 127
	struct note *next;
};
typedef struct note note;

struct channel {
	note *head;
};
typedef struct channel channel;

struct measure {
	channel* channels[10]; //On permet jusqu'à 10 canaux sonores simultanés par mesure : 5 pour la mélodie, 5 pour les accords
};
typedef struct measure measure;

struct staff {
	measure* measures[4];
};
typedef struct staff staff;


//Display functions

void print_note(note *n);
void print_channel(channel *c);
void print_measure(measure *m);
void print_staff(staff *s);

//Validation functions

int sum_durations(channel *c);
bool validate_channel(channel *c); //Checks if channel is filled
bool validate_measure(measure *m);

//Initializers

note *new_note(key pitch, rythm duration, unsigned velocity);
channel *new_channel();
measure *new_measure();
staff *new_staff();

//Freeing functions

void free_channel(channel *c);
void free_measure(measure *m);
void free_staff(staff *s);

//Structure manipulations

note *get_last_note(channel *c);
bool append_note(channel *c, note *n);
void set_measure_channel(measure *m, channel *c, size_t index);
void set_staff_measure(staff *s, measure *m, size_t index);
void append_chord(measure *m, chord c);
