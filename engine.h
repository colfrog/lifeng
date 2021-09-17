#include <stdbool.h>

#define X 400
#define Y 200
#define Z 24
#define PIXEL_SIZE 8

extern bool space[X][Y][Z];
void clear_space();
void randomize_space();
void update_space();
