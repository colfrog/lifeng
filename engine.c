#include <stdlib.h>
#include <math.h>

#include "engine.h"

bool space[X][Y][Z];
double N[X][Y][Z];

void clear_space() {
	for (int i = 0; i < X; i++)
		for (int j = 0; j < Y; j++)
			for (int k = 0; k < Z; k++)
				space[i][j][k] = 0;
}

void randomize_space() {
	for (int i = 0; i < X; i++)
		for (int j = 0; j < Y; j++)
			for (int k = 0; k < Z; k++)
				space[i][j][k] = random() % 2;
}

double count_neighbour(int i, int j, int k) {
	double dist, nb = 0;
	int a, b, c;
	for (int m = -1; m <= 1; m++)
		for (int n = -1; n <= 1; n++)
			for (int o = -1; o <= 1; o++) {
				if (m == 0 && n == 0 && o == 0)
					continue;

				dist = sqrt(m*m + n*n + o*o);

				a = i+m;
				b = j+n;
				c = k+o;

				if (a >= 0 && a < X && b >= 0 &&
				    b < Y && c >= 0 && c < Z &&
				    space[a][b][c])
					nb += dist;
			}

	return nb;
}

void count_neighbours() {
	for (int i = 0; i < X; i++)
		for (int j = 0; j < Y; j++)
			for (int k = 0; k < Z; k++)
				N[i][j][k] = count_neighbour(i, j, k);
}

void update_space() {
	double nb;

	count_neighbours();

	for (int i = 0; i < X; i++)
		for (int j = 0; j < Y; j++)
			for (int k = 0; k < Z; k++) {
				nb = N[i][j][k];

				if (space[i][j][k]) {
					if (nb < LIFE_MIN || nb > LIFE_MAX)
						space[i][j][k] = false;
				} else {
					if (nb > BIRTH_MIN && nb < BIRTH_MAX)
						space[i][j][k] = true;
				}
			}
}

void init_opencl() {}
void free_opencl() {}
