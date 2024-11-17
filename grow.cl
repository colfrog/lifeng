kernel void
grow(global bool *space, global float *neighbours, float life_min, float life_max, float birth_min, float birth_max, int sizeX, int sizeY, int sizeZ) {
	int x = get_global_id(0);
	int y = get_global_id(1);
	int z = get_global_id(2);
	int index = x + y*sizeX + z*sizeX*sizeY;

	float nb = neighbours[index];
	if (space[index]) {
		if (nb < life_min || nb > life_max) {
			space[index] = false;
		}
	} else  {
		if (nb >= birth_min && nb <= birth_max) {
			space[index] = true;
		}
	}
}

kernel void
neighbours(global bool *space, global float *neighbours, int sizeX, int sizeY, int sizeZ) {
	int x = get_global_id(0);
	int y = get_global_id(1);
	int z = get_global_id(2);
	int index = x + y*sizeX + z*sizeX*sizeY;

	float dist = 0;
	int nb = 0;
	int a, b, c;

	for (int i = -1; i <= 1; i++) {
		for (int j = -1; j <= 1; j++) {
			for (int k = -1; k <= 1; k++) {
				if (i == 0 && j == 0 && k == 0)
					continue;
				dist = 1.0/sqrt((float) (i*i + j*j + k*k));

				a = x+i;
				if (a < 0) a = sizeX - 1;
				else if (a == sizeX) a = 0;

				b = y+j;
				if (b < 0) b = sizeX - 1;
				else if (b == sizeX) b = 0;

				c = z+k;
				if (c < 0) c = sizeX - 1;
				else if (c == sizeX) c = 0;

				int rindex = a + b*sizeX + c*sizeX*sizeY;

				if (space[rindex])
					nb += dist;
			}
		}
	}

	neighbours[index] = nb;
}
