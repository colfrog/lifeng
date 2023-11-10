#include <stdbool.h>

#define X 180
#define Y 120
#define Z 211
#define PIXEL_SIZE 2

#define BIRTH_MIN 4
#define BIRTH_MAX 7
#define LIFE_MIN 4
#define LIFE_MAX 9

extern bool space[X][Y][Z];

void clear_space();
void randomize_space();
void update_space();
