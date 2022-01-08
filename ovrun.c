#include <objectview.h>
#include "engine.h"

void copy_space(ovobject *obj) {
	for (int i = 0; i < X; i++)
		for (int j = 0; j < Y; j++)
			for (int k = 0; k < Z; k++)
				ovobject_set(obj, space[i][j][k], i, j, k);

}

void step(ovobject *obj) {
	update_space();
	copy_space(obj);
}

int main() {
	randomize_space();

	ovobject *obj = malloc(sizeof(ovobject));
	memset(obj, 0, sizeof(ovobject));
	obj->data = malloc(X*Y*Z*sizeof(double));
	obj->x = X;
	obj->y = Y;
	obj->z = Z;
	obj->data_size = X*Y*Z;
	obj->time_index = 0;

	copy_space(obj);
	
	objectview *ov = ov_create(obj);
	ovwindow *window = ovwindow_create("lifeng", ov);
	window->context->step_function = step;
	ovwindow_sdl_loop(NULL);

	ovobject_save(obj, "lifeng.ovo");
}
