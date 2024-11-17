#include <stdbool.h>

#define X 100
#define Y 100
#define Z 20
#define PIXEL_SIZE 4

#define BIRTH_MIN 2
#define BIRTH_MAX 3
#define LIFE_MIN 2
#define LIFE_MAX 4

extern bool space[X*Y*Z];

void clear_space();
void randomize_space();
void update_space();
void write_space();

#define TABLE_INDEX(i, j, k) (i + j*X + k*X*Y)
