#include"music_probs.h"

int main() {
	srand(SEED);
	staff *s = new_staff();

	for (int i = 0 ; i < 4 ; ++i) {
		measure *m = s -> measures[i];

		chord *prog = pick_progression();
		for (int j = 0 ; j < 4 ; ++j) append_chord(m, prog[j]);
		free(prog);
		
		generate_melody(m -> channels[0]);
	}

	print_staff(s);
	free_staff(s);
}
