#include <stdbool.h>

#define X 480
#define Y 420
#define Z 211
#define PIXEL_SIZE 8

extern bool space[X][Y][Z];
void clear_space();
void randomize_space();
void update_space();
