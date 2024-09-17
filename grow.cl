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

	for (float i = -1; i <= 1; i++) {
		for (float j = -1; j <= 1; j++) {
			for (float k = -1; k <= 1; k++) {
				if (i == 0 && j == 0 && k == 0)
					continue;
				dist = 1.0/sqrt(i*i + j*j + k*k);

				a = x+i;
				b = y+j;
				c = z+k;
				int rindex = a + b*sizeX + c*sizeX*sizeY;

				if (a >= 0 && a < sizeX && b >= 0 && b < sizeY
				    && c >= 0 && c < sizeZ && space[rindex])
					nb += dist;
			}
		}
	}

	neighbours[index] = nb;
}
