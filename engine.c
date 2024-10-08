#include <stdlib.h>
#include <math.h>

#include "engine.h"

bool space[X*Y*Z];
double N[X*Y*Z];

void write_space() {} // Already in RAM

void clear_space() {
	for (int i = 0; i < X; i++)
		for (int j = 0; j < Y; j++)
			for (int k = 0; k < Z; k++)
				space[TABLE_INDEX(i, j, k)] = 0;
}

void randomize_space() {
	for (int i = 0; i < X; i++)
		for (int j = 0; j < Y; j++)
			for (int k = 0; k < Z; k++)
				space[TABLE_INDEX(i, j, k)] = random() % 2;
}

double count_neighbour(int i, int j, int k) {
	double dist = 0;
	int nb = 0;
	int a, b, c;
	for (int m = -1; m <= 1; m++)
		for (int n = -1; n <= 1; n++)
			for (int o = -1; o <= 1; o++) {
				if (m == 0 && n == 0 && o == 0)
					continue;

				dist = 1.0/sqrt(m*m + n*n + o*o);
				a = i+m;
				b = j+n;
				c = k+o;

				if (a >= 0 && a < X && b >= 0 &&
				    b < Y && c >= 0 && c < Z &&
				    space[TABLE_INDEX(a, b, c)])
					nb += dist;
			}

	return nb;
}

void count_neighbours() {
	for (int i = 0; i < X; i++)
		for (int j = 0; j < Y; j++)
			for (int k = 0; k < Z; k++)
				N[TABLE_INDEX(i, j, k)] = count_neighbour(i, j, k);
}

void update_space() {
	double nb;

	count_neighbours();

	for (int i = 0; i < X; i++)
		for (int j = 0; j < Y; j++)
			for (int k = 0; k < Z; k++) {
				nb = N[TABLE_INDEX(i, j, k)];

				if (space[TABLE_INDEX(i, j, k)]) {
					if (nb < LIFE_MIN || nb > LIFE_MAX)
						space[TABLE_INDEX(i, j, k)] = false;
				} else {
					if (nb >= BIRTH_MIN && nb <= BIRTH_MAX)
						space[TABLE_INDEX(i, j, k)] = true;
				}
			}
}

void init_opencl() {}
void free_opencl() {}
