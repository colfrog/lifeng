#include <err.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <CL/cl.h>

#include "engine.h"

bool space[X][Y][Z];
float neighbours[X][Y][Z];
cl_int sizeX = X, sizeY = Y, sizeZ = Z;
cl_float birth_min = BIRTH_MIN,
	birth_max = BIRTH_MAX,
	life_min = LIFE_MIN,
	life_max = LIFE_MAX;

cl_mem space_buffer, neighbours_buffer;
cl_int ngroups;

cl_device_id device;
cl_context context;
cl_program program;
cl_kernel kernel_grow, kernel_neighbours;
cl_command_queue queue;

void clear_space() {
	for (int i = 0; i < X; i++)
		for (int j = 0; j < Y; j++)
			for (int k = 0; k < Z; k++)
				space[i][j][k] = 0;
	clEnqueueWriteBuffer(queue, space_buffer, CL_TRUE, 0, 
			     sizeof(space), space, 0, NULL, NULL);
}

void randomize_space() {
	for (int i = 0; i < X; i++)
		for (int j = 0; j < Y; j++)
			for (int k = 0; k < Z; k++)
				space[i][j][k] = random() % 2;
	clEnqueueWriteBuffer(queue, space_buffer, CL_TRUE, 0, 
			     sizeof(space), space, 0, NULL, NULL);
}

cl_device_id create_device() {
	cl_platform_id platform;
	cl_device_id dev;
	cl_uint num_platforms, num_devices;
	int error;

	char platform_name[4096];

	error = clGetPlatformIDs(1, &platform, &num_platforms);
	printf("%d\n", num_platforms);
	if (error < 0)
		err(1, "Couldn't identify a platform");

	error = clGetPlatformInfo(platform, CL_PLATFORM_NAME, sizeof(platform_name), platform_name, NULL);
	printf("%s\n", platform_name);

	error = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &dev, &num_devices);
	printf("%d\n", num_devices);
	if (error < 0)
		err(1, "Couldn't access any devices");

	return dev;
}

cl_program build_program(cl_context ctx, cl_device_id dev, const char *filename) {
	cl_program program;
	FILE *program_handle;
	char *program_buffer, *program_log;
	size_t program_size, log_size;
	int error;

	/* Read program file and place content into buffer */
	program_handle = fopen(filename, "r");
	if(program_handle == NULL)
		err(1, "Couldn't find the program file");

	fseek(program_handle, 0, SEEK_END);
	program_size = ftell(program_handle);
	rewind(program_handle);
	program_buffer = (char*)malloc(program_size + 1);
	program_buffer[program_size] = '\0';
	fread(program_buffer, sizeof(char), program_size, program_handle);
	fclose(program_handle);

	/* Create program from file 

	   Creates a program from the source code in the add_numbers.cl file. 
	   Specifically, the code reads the file's content into a char array 
	   called program_buffer, and then calls clCreateProgramWithSource.
	*/
	program = clCreateProgramWithSource(ctx, 1, 
					    (const char**)&program_buffer, &program_size, &error);
	if(error < 0)
		err(1, "Couldn't create the program");
	free(program_buffer);

	/* Build program 

	   The fourth parameter accepts options that configure the compilation. 
	   These are similar to the flags used by gcc. For example, you can 
	   define a macro with the option -DMACRO=VALUE and turn off optimization 
	   with -cl-opt-disable.
	*/
	error = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
	if(error < 0) {

		/* Find size of log and print to std output */
		clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG, 
				      0, NULL, &log_size);
		program_log = (char*) malloc(log_size + 1);
		program_log[log_size] = '\0';
		clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG, 
				      log_size + 1, program_log, NULL);
		printf("%s\n", program_log);
		free(program_log);
		exit(1);
	}

	return program;
}

void init_opencl() {
	/* OpenCL structures */
	cl_int error;

	/* Create device and context 

	   Creates a context containing only one device — the device structure 
	   created earlier.
	*/
	device = create_device();
	context = clCreateContext(NULL, 1, &device, NULL, NULL, &error);
	if (error < 0)
		err(1, "Couldn't create a context");

	/* Build program */
	program = build_program(context, device, "grow.cl");

	/* Create data buffer 

	   • `global_size`: total number of work items that will be 
	   executed on the GPU (e.g. total size of your array)
	   • `local_size`: size of local workgroup. Each workgroup contains 
	   several work items and goes to a compute unit 

	   In this example, the kernel is executed by eight work-items divided into 
	   two work-groups of four work-items each. Returning to my analogy, 
	   this corresponds to a school containing eight students divided into 
	   two classrooms of four students each.   

	   Notes: 
	   • Intel recommends workgroup size of 64-128. Often 128 is minimum to 
	   get good performance on GPU
	   • On NVIDIA Fermi, workgroup size must be at least 192 for full 
	   utilization of cores
	   • Optimal workgroup size differs across applications
	*/
	ngroups = 128;
	puts("allocating space");
        space_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE |
				      CL_MEM_COPY_HOST_PTR,
				      X*Y*Z * sizeof(bool),
				      space,
				      &error);
	puts("allocating neighbours");
        neighbours_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE |
				      CL_MEM_COPY_HOST_PTR,
				      X*Y*Z * sizeof(cl_float),
				      neighbours,
				      &error);
	if (error < 0) {
		perror("Couldn't create a buffer");
		exit(1);   
	};

	/* Create a command queue 

	   Does not support profiling or out-of-order-execution
	*/
	puts("Creating command queue");
	queue = clCreateCommandQueue(context, device, 0, &error);
	if (error < 0)
		err(1, "Couldn't create a command queue");

	/* Create a kernel */
	kernel_grow = clCreateKernel(program, "grow", &error);
	if (error < 0)
	        err(1, "Couldn't create a kernel");

	/* Create kernel arguments */
	error = clSetKernelArg(kernel_grow, 0, sizeof(cl_mem), &space_buffer);
	error |= clSetKernelArg(kernel_grow, 1, sizeof(cl_mem), &neighbours_buffer);
	error |= clSetKernelArg(kernel_grow, 2, sizeof(cl_float), &life_min);
	error |= clSetKernelArg(kernel_grow, 3, sizeof(cl_float), &life_max);
	error |= clSetKernelArg(kernel_grow, 4, sizeof(cl_float), &birth_min);
	error |= clSetKernelArg(kernel_grow, 5, sizeof(cl_float), &birth_max);
	error |= clSetKernelArg(kernel_grow, 6, sizeof(cl_int), &sizeX);
	error |= clSetKernelArg(kernel_grow, 7, sizeof(cl_int), &sizeY);
	error |= clSetKernelArg(kernel_grow, 8, sizeof(cl_int), &sizeZ);
	if(error < 0)
		err(1, "Couldn't create a kernel argument %d", error);

	/* Create a kernel */
	kernel_neighbours = clCreateKernel(program, "neighbours", &error);
	if (error < 0)
	        err(1, "Couldn't create a kernel");

	/* Create kernel arguments */
	error = clSetKernelArg(kernel_neighbours, 0, sizeof(cl_mem), &space_buffer);
	error |= clSetKernelArg(kernel_neighbours, 1, sizeof(cl_mem), &neighbours_buffer);
	error |= clSetKernelArg(kernel_neighbours, 2, sizeof(cl_int), &sizeX);
	error |= clSetKernelArg(kernel_neighbours, 3, sizeof(cl_int), &sizeY);
	error |= clSetKernelArg(kernel_neighbours, 4, sizeof(cl_int), &sizeZ);
	if(error < 0)
		err(1, "Couldn't create a kernel argument");
}

void update_space() {
	clEnqueueWriteBuffer(queue, neighbours_buffer, CL_TRUE, 0, 
			     sizeof(neighbours), neighbours, 0, NULL, NULL);
	clEnqueueWriteBuffer(queue, space_buffer, CL_TRUE, 0, 
			     sizeof(space), space, 0, NULL, NULL);
	/* Enqueue kernel 

	   At this point, the application has created all the data structures 
	   (device, kernel, program, command queue, and context) needed by an 
	   OpenCL host application. Now, it deploys the kernel to a device.

	   Of the OpenCL functions that run on the host, clEnqueueNDRangeKernel 
	   is probably the most important to understand. Not only does it deploy 
	   kernels to devices, it also identifies how many work-items should 
	   be generated to execute the kernel (global_size) and the number of 
	   work-items in each work-group (local_size).
	*/
	size_t global_size[3] = {X, Y, Z};
	size_t local_size[3] = {1, 1, 1};
	int error;

	error = clSetKernelArg(kernel_neighbours, 0, sizeof(cl_mem), &space_buffer);
	error |= clSetKernelArg(kernel_neighbours, 1, sizeof(cl_mem), &neighbours_buffer);
	error |= clSetKernelArg(kernel_neighbours, 2, sizeof(cl_int), &sizeX);
	error |= clSetKernelArg(kernel_neighbours, 3, sizeof(cl_int), &sizeY);
	error |= clSetKernelArg(kernel_neighbours, 4, sizeof(cl_int), &sizeZ);
	error = clEnqueueNDRangeKernel(queue, kernel_neighbours, 3, NULL,
				       global_size, local_size, 0, NULL, NULL); 
	if (error < 0)
		err(1, "Couldn't enqueue the kernel");

	error = clSetKernelArg(kernel_grow, 0, sizeof(cl_mem), &space_buffer);
	error |= clSetKernelArg(kernel_grow, 1, sizeof(cl_mem), &neighbours_buffer);
	error |= clSetKernelArg(kernel_grow, 2, sizeof(cl_float), &life_min);
	error |= clSetKernelArg(kernel_grow, 3, sizeof(cl_float), &life_max);
	error |= clSetKernelArg(kernel_grow, 4, sizeof(cl_float), &birth_min);
	error |= clSetKernelArg(kernel_grow, 5, sizeof(cl_float), &birth_max);
	error |= clSetKernelArg(kernel_grow, 6, sizeof(cl_int), &sizeX);
	error |= clSetKernelArg(kernel_grow, 7, sizeof(cl_int), &sizeY);
	error |= clSetKernelArg(kernel_grow, 8, sizeof(cl_int), &sizeZ);
	error = clEnqueueNDRangeKernel(queue, kernel_grow, 3, NULL,
				       global_size, local_size, 0, NULL, NULL); 
	if (error < 0)
		err(1, "Couldn't enqueue the kernel");

	/* Read the kernel's output    */
	error = clEnqueueReadBuffer(queue, neighbours_buffer, CL_TRUE, 0, 
				    sizeof(neighbours), neighbours, 0, NULL, NULL);
	error = clEnqueueReadBuffer(queue, space_buffer, CL_TRUE, 0, 
				    sizeof(space), space, 0, NULL, NULL);
	if (error < 0)
		err(1, "Couldn't read the buffer");
}

void free_opencl() {
	/* Deallocate resources */
	clReleaseKernel(kernel_grow);
	clReleaseKernel(kernel_neighbours);
	clReleaseMemObject(space_buffer);
	clReleaseCommandQueue(queue);
	clReleaseProgram(program);
	clReleaseContext(context);
}
