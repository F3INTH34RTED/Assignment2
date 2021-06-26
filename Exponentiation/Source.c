#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#ifdef __APPLE__
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#define __CRT__SECURE__NO__WARNINGS
#endif // __APPLE__
#pragma comment (lib, "OpenCL.lib")
#define ARRAY_SIZE 49
const unsigned int C_ARRAY_SIZE = ARRAY_SIZE;

enum { bool_false, bool_true };
enum { FIRST_HALF_OF_ARRAY = 1, SECOND_HALF_OF_ARRAY, FULL_ARRAY };

void Execute_On_Cpu_or_Gpu(int cpu_or_gpu,int* input_array_a, int* input_array_b, int* result_array_c);


void Execute_On_Cpu_and_Gpu(unsigned int array_size_percentage_CPU, int processing_part_CPU, unsigned int array_size_percentage_GPU, int processing_part_GPU, int* input_array_a, int* input_array_b, int* result_array_c);

int PercentageSize_ProcessingPart_Valid(unsigned int array_size_percentage, int processing_part);
int FullPercentageSize_Equal_FullProcessingPart(unsigned int* array_size_percentage, int* processing_part);

int checkForCPU();
int checkForGPU();



//device and platform id for cpu
cl_device_id cpu_id;
cl_platform_id cpu_platform;

// device and platform id for gpu
cl_device_id gpu_id;
cl_platform_id gpu_platform;

cl_uint Get_Num_Platforms();
cl_platform_id* Get_Platforms();

void Display_Specific_Platform_Info(const cl_platform_id* platforms, const cl_uint index);
void Display_Specific_Device_Info(const cl_device_id* devices, const cl_uint device_index);
char* Get_Kernel_File(char* file_name);

int main()
{
    // INITIALIZE ARRAYS
    int* input_array_a = (int*)malloc(sizeof(int) * ARRAY_SIZE);
    int* input_array_b = (int*)malloc(sizeof(int) * ARRAY_SIZE);
   int* result_array_c = (int*)malloc(sizeof(int) * ARRAY_SIZE);
 
   int counter = 0;
    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        if (input_array_a && input_array_b && result_array_c)
        {
            input_array_a[i] = i;
            if (i % 5 == 0) {   
                counter = 0;
            }
            
            input_array_b[i] =counter;

           
            result_array_c[i] = 1;
            counter++;
        }
    }

    int choice;
    printf("Press 1 to exponentiate arrays using threads\n");
    printf("Press 2 to exponentiate arrays using GPU only\n");
    printf("Press 3 to exponentiate arrays using CPU only\n");
    printf("Press 4 to exponentiate arrays using 25%% of CPU and 75%% of GPU\n");
    printf("Press 5 to exponentiate arrays using 50%% of CPU and 50%% of GPU\n");
    printf("Press 6 to exponentiate arrays using 75%% of CPU and 25%% of GPU\n");
    printf("Enter your choice:\n");
    scanf("%d", &choice);

    switch (choice) {
    case 1:
    {
        //thread func 
        break;
    }
    case 2:
    {
        Execute_On_Cpu_or_Gpu(1, input_array_a, input_array_b, result_array_c);
        break;
    }
    case 3:
    {
        Execute_On_Cpu_or_Gpu(0, input_array_a, input_array_b, result_array_c);
        break;
    }
    case 4:
    {
        Execute_On_Cpu_and_Gpu(25,FIRST_HALF_OF_ARRAY,75,SECOND_HALF_OF_ARRAY, input_array_a, input_array_b, result_array_c);
        break;
    }
    case 5:
    {
        Execute_On_Cpu_and_Gpu(50, FIRST_HALF_OF_ARRAY, 50, SECOND_HALF_OF_ARRAY, input_array_a, input_array_b, result_array_c);
        break;
    }
    case 6:
    {
        Execute_On_Cpu_and_Gpu(75, FIRST_HALF_OF_ARRAY, 25, SECOND_HALF_OF_ARRAY, input_array_a, input_array_b, result_array_c);
        break;
    }
    default:
        printf("wrong Input\n");
   
    }
    free(input_array_a);
    free(input_array_b);
    free(result_array_c);

    system("pause");
    return 0;
}

/*
* Requirements: Array size percentage (0% - 100%), FIRST_HALF(1) or SECOND_HALF(2) or FULL_ARRAY(3), input array 1, input array 2, result array
* Usage: Execute_On_Cpu(int 0-100, int 1-3)
* ------------------------------------------
* Description: Calculates the total number of elements to be executed in the cpu/gpu (array_size_percentage)
and then executes the elements based on the processing_part (first half, second half, or the full array).
*/

void Execute_On_Cpu_and_Gpu(unsigned int array_size_percentage_CPU, int processing_part_CPU, unsigned int array_size_percentage_GPU, int processing_part_GPU, int* input_array_a, int* input_array_b, int* result_array_c) {
    //check if both CPU and GPU are enabled
    //check func stores platform and device id in global params if detected
    if (checkForCPU() == 0) {
        printf("NO CPU DETECTED");
        return;
    }
    if (checkForGPU() == 0) {
        printf("NO GPU DETECTED");
        return;
    }
    
    
    // Check validity of parameters for CPU
    if (!PercentageSize_ProcessingPart_Valid(array_size_percentage_CPU, processing_part_CPU)) return;
    if (!FullPercentageSize_Equal_FullProcessingPart(&array_size_percentage_CPU, &processing_part_CPU)) return;
    
    // Check validity of parameters for GPU
    if (!PercentageSize_ProcessingPart_Valid(array_size_percentage_GPU, processing_part_GPU)) return;
    if (!FullPercentageSize_Equal_FullProcessingPart(&array_size_percentage_GPU, &processing_part_GPU)) return;
    
    printf("EXECUTING %d%% on CPU and %d%% on GPU \n",array_size_percentage_CPU,array_size_percentage_GPU);
    printf("Arrays:\n");
    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        if (input_array_a && input_array_b && result_array_c)
            printf("a[%04d] = %04d\tb[%04d] = %04d\tc[%04d] = %04d\n", i, input_array_a[i], i, input_array_b[i], i, result_array_c[i]);
    }
    system("pause");
    system("cls");


    // Calculate number of elements to process based on percentage provided for CPU


    int array_size_CPU = (int)((C_ARRAY_SIZE * array_size_percentage_CPU) / 100.0); 
    int start_CPU; int end_CPU;

    switch (processing_part_CPU)
    {

    case FIRST_HALF_OF_ARRAY:
        start_CPU = 0;
        end_CPU = array_size_CPU - 1;
        break;
    case SECOND_HALF_OF_ARRAY:
        start_CPU = C_ARRAY_SIZE - array_size_CPU;
        end_CPU = C_ARRAY_SIZE - 1;
        array_size_CPU = C_ARRAY_SIZE - start_CPU;
        break;

    case FULL_ARRAY:
        start_CPU = 0;
        end_CPU = C_ARRAY_SIZE - 1;
        break;
    }
 

    // Calculate number of elements to process based on percentage provided for GPU
    int array_size_GPU = (int)(ceil)((C_ARRAY_SIZE * array_size_percentage_GPU) / 100.0);

   
    int start_GPU; int end_GPU;
    switch (processing_part_GPU)
    {

    case FIRST_HALF_OF_ARRAY:
        start_GPU = 0;
        end_GPU = array_size_GPU - 1;
        break;
    case SECOND_HALF_OF_ARRAY:
        start_GPU = C_ARRAY_SIZE - array_size_GPU;
        end_GPU = C_ARRAY_SIZE - 1;
        array_size_GPU = C_ARRAY_SIZE - start_GPU;
        break;

    case FULL_ARRAY:
        start_GPU = 0;
        end_GPU = C_ARRAY_SIZE - 1;
        break;
    }


    printf("CPU exponentiates %d elements while GPU exponentiates %d \n\n", array_size_CPU, array_size_GPU);

    //initialse tings GPU
    cl_context context_GPU;
    cl_command_queue commandQueue_GPU;
    cl_mem buffer_GPU1;
    cl_mem buffer_GPU2;
    cl_mem result_buffer_GPU;

    // initialise things CPU
    cl_context context_CPU;
    cl_command_queue commandQueue_CPU;
    cl_mem buffer_CPU1;
    cl_mem buffer_CPU2;
    cl_mem result_buffer_CPU;

    cl_int error;


    //create context (interface for device)
    // 1 device, give the device id and set error code
    context_GPU = clCreateContext(NULL, 1, &gpu_id, NULL, NULL, &error);

    if (error != 0) {
        printf("Error in getting context. Code %d\n", error);
    }

    //create a command queue to queue in commands for GPU
    //1 context and deviceID for each queue and set error code
    commandQueue_GPU = clCreateCommandQueue(context_GPU, gpu_id, CL_QUEUE_PROFILING_ENABLE, &error);

    if (error != 0) {
        printf("Error in getting command queue. Code %d\n", error);
    }


    // allocate memory in device for input arrays for GPU
    //like malloc, allocates space in device
    buffer_GPU1 = clCreateBuffer(context_GPU, CL_MEM_READ_WRITE, sizeof(int) * (array_size_GPU), NULL, &error);

    if (error != 0) {
        printf("Error in creating buffer. Code %d\n", error);
    }
    buffer_GPU2 = clCreateBuffer(context_GPU, CL_MEM_READ_WRITE, sizeof(int) * (array_size_GPU), NULL, &error);

    if (error != 0) {
        printf("Error in creating buffer. Code %d\n", error);
    }
    result_buffer_GPU = clCreateBuffer(context_GPU, CL_MEM_READ_WRITE, sizeof(int) * (array_size_GPU), NULL, &error);

    if (error != 0) {
        printf("Error in creating buffer. Code %d\n", error);
    }
   

  

    //create context (interface for device)for CPU
    // 1 device, give the device id and set error code
    context_CPU = clCreateContext(NULL, 1, &cpu_id, NULL, NULL, &error);

    if (error != 0) {
        printf("Error in getting context. Code %d\n", error);
    }

    //create a command queue to queue in commands for CPU
    //1 context and deviceID for each queue and set error code
    commandQueue_CPU = clCreateCommandQueue(context_CPU, cpu_id, CL_QUEUE_PROFILING_ENABLE, &error);

    if (error != 0) {
        printf("Error in getting command queue. Code %d\n", error);
    }


    // allocate memory in device for input arrays in CPU
    //like malloc, allocates space in device
    buffer_CPU1 = clCreateBuffer(context_CPU, CL_MEM_READ_WRITE, sizeof(int) * (array_size_CPU), NULL, &error);

    if (error != 0) {
        printf("Error in creating buffer. Code %d\n", error);
    }

    buffer_CPU2 = clCreateBuffer(context_CPU, CL_MEM_READ_WRITE, sizeof(int) * (array_size_CPU), NULL, &error);

    if (error != 0) {
        printf("Error in creating buffer. Code %d\n", error);
    }
    result_buffer_CPU = clCreateBuffer(context_CPU, CL_MEM_READ_WRITE, sizeof(int) * (array_size_CPU), NULL, &error);

    if (error != 0) {
        printf("Error in creating buffer. Code %d\n", error);
    }

    // 4 events for writing
    cl_event events_for_writing_CPU[2];
    cl_event events_for_writing_GPU[2];
    //transfer input arrays from ram to vram by copying array to buffer in GPU
    error = clEnqueueWriteBuffer(commandQueue_GPU, buffer_GPU1, CL_FALSE, 0, sizeof(int) * (array_size_GPU), input_array_a+start_GPU, 0, NULL, &events_for_writing_GPU[0]);

    if (error != 0) {
        printf("Error in copying array into buffer GPU. Code %d\n", error);
    }

    error = clEnqueueWriteBuffer(commandQueue_GPU, buffer_GPU2, CL_FALSE, 0, sizeof(int) * (array_size_GPU), input_array_b+start_GPU, 0, NULL, &events_for_writing_GPU[1]);

    if (error != 0) {
        printf("Error in copying array into buffer GPU. Code %d\n", error);
    }


    //transfer input array from ram to vram by copying array to buffer in CPU
    error = clEnqueueWriteBuffer(commandQueue_CPU, buffer_CPU1, CL_TRUE, 0, sizeof(int) * (array_size_CPU), input_array_a + start_CPU, 0, NULL, &events_for_writing_CPU[0]);

    if (error != 0) {
        printf("Error in copying array into buffer CPU. Code %d\n", error);
    }

    error = clEnqueueWriteBuffer(commandQueue_CPU, buffer_CPU2, CL_TRUE, 0, sizeof(int) * (array_size_CPU), input_array_b + start_CPU, 0, NULL, &events_for_writing_CPU[1]);

    if (error != 0) {
        printf("Error in copying array into buffer CPU. Code %d\n", error);
    }

    //guarantees that all enqueued commands from CPU and GPU are submitted
    clFlush(commandQueue_GPU);
    clFlush(commandQueue_CPU);


    //.......
     /*  create program from source */
    char* exponentiate_array_program = Get_Kernel_File("Kernels.cl");
    /*  first, check if the file exists, otherwise, the function returns NULL */
    if (exponentiate_array_program == NULL)
    {
        printf("Could not read an OpenCL from the specified file \n");
    }


    cl_kernel kernel_GPU;
    cl_program program_GPU;

    program_GPU = clCreateProgramWithSource(context_GPU, 1, (const char**)&exponentiate_array_program, NULL, &error);

    if (error != 0) {
        printf("Error in creating program. Code %d\n", error);
    }

    //build program
    clBuildProgram(program_GPU, 0, NULL, NULL, NULL, NULL);

    //create kernel in GPU for func
    kernel_GPU = clCreateKernel(program_GPU, "exponentiateArray", &error);

    if (error != 0) {
        printf("Error in creating kernel. Code %d\n", error);
    }

   
    cl_kernel kernel_CPU;
    cl_program program_CPU;

   

    /*  second, try to create a program from the content of the file */
   program_CPU = clCreateProgramWithSource(context_CPU, 1, (const char**)&exponentiate_array_program, NULL, &error);
    if (error != 0)
    {
        printf("Error in creating program. Code %d\n", error);
    }


    //build program
    clBuildProgram(program_CPU, 0, NULL, NULL, NULL, NULL);

    //create kernel in CPU for your func
    kernel_CPU = clCreateKernel(program_CPU, "exponentiateArray", &error);

    if (error != 0) {
        printf("Error in creating kernel. Code %d\n", error);
    }



    //set kernel arguments
    clSetKernelArg(kernel_GPU, 0, sizeof(cl_mem), &buffer_GPU1);
    clSetKernelArg(kernel_GPU, 1, sizeof(cl_mem), &buffer_GPU2);
    clSetKernelArg(kernel_GPU, 2, sizeof(cl_mem), &result_buffer_GPU);
    clSetKernelArg(kernel_GPU, 3, sizeof(int), &array_size_GPU);

    clSetKernelArg(kernel_CPU, 0, sizeof(cl_mem), &buffer_CPU1);
    clSetKernelArg(kernel_CPU, 1, sizeof(cl_mem), &buffer_CPU2);
    clSetKernelArg(kernel_CPU, 2, sizeof(cl_mem), &result_buffer_CPU);
    clSetKernelArg(kernel_CPU, 3, sizeof(int), &array_size_CPU);

    size_t local_CPU = 10;
    //global has to be multiple of local so need to divide and multiply
    size_t global_CPU = ceil(array_size_CPU) * local_CPU;

    size_t local_GPU = 10;
    //global has to be multiple of local so need to divide and multiply
    size_t global_GPU = ceil(array_size_GPU) * local_GPU;


    // 2 events for computing
    cl_event events_for_computing[2];

    //start work at this point, numbr of dimensions 1, waits for 1 event that is writing to buffer, starts event for computing
    error = clEnqueueNDRangeKernel(commandQueue_GPU, kernel_GPU, 1, NULL, &global_GPU, &local_GPU, 2, events_for_writing_GPU, &events_for_computing[0]);

    if (error != 0) {
        printf("Error in executing kernel. Code %d\n", error);
    }


    //start work at this point, numbr of dimensions 1, waits for 1 event that is writing to buffer, starts event for computing
    error = clEnqueueNDRangeKernel(commandQueue_CPU, kernel_CPU, 1, NULL, &global_CPU, &local_CPU, 2, events_for_writing_CPU, &events_for_computing[1]);

    if (error != 0) {
        printf("Error in executing kernel CPU. Code %d\n", error);
    }


    //wait for gpu to finish work before actually reading the result buffer
    error = clFinish(commandQueue_GPU);

    if (error != 0) {
        printf("Error in finishing work from GPU. Code %d\n", error);
    }
    //wait for cpu to finish work before actually reading the result buffer
    error = clFinish(commandQueue_CPU);

    if (error != 0) {
        printf("Error in finishing work from CPU. Code %d\n", error);
    }

    //calculate the execution time for GPU
    cl_ulong start, end;

    clGetEventProfilingInfo(events_for_computing[0], CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &start, NULL);
    clGetEventProfilingInfo(events_for_computing[0], CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &end, NULL);

    printf("Time taken to exponentiate array in GPU %f ms\n", (end - start) / 1000000.0);


    //calculate the execution time for CPU

    cl_ulong start2, end2;

    clGetEventProfilingInfo(events_for_computing[1], CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &start2, NULL);
    clGetEventProfilingInfo(events_for_computing[1], CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &end2, NULL);

    printf("Time taken to exponentiate array in CPU %f ms\n", (end2 - start2) / 1000000.0);


    //read output from vram to ram

    //read contents of buffer GPU into result array, 0 events, 0 offset
    clEnqueueReadBuffer(commandQueue_GPU, result_buffer_GPU, CL_FALSE, 0, sizeof(int) * (array_size_GPU), result_array_c+start_GPU, 1, &events_for_computing[0], NULL);

    //read contents of buffer CPU into result array, 0 events, 0 offset
    clEnqueueReadBuffer(commandQueue_CPU, result_buffer_CPU, CL_FALSE, 0, sizeof(int) * (array_size_CPU), result_array_c + start_CPU, 1, &events_for_computing[1], NULL);

    //make sure we are done reading before printing array
    clFinish(commandQueue_GPU);
    clFinish(commandQueue_CPU);


    //DISPLAY RESULTS
    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        if (input_array_a && input_array_b && result_array_c)
            printf("%02d ^ %02d = %d\n", input_array_a[i], input_array_b[i], result_array_c[i]);
    }

    clReleaseMemObject(buffer_GPU1);
    clReleaseMemObject(buffer_GPU2);
    clReleaseMemObject(result_buffer_GPU);
    clReleaseProgram(program_GPU);
    clReleaseKernel(kernel_GPU);
    clReleaseCommandQueue(commandQueue_GPU);
    clReleaseContext(context_GPU);

    clReleaseMemObject(buffer_CPU1);
    clReleaseMemObject(buffer_CPU2);
    clReleaseMemObject(result_buffer_CPU);
    clReleaseProgram(program_CPU);
    clReleaseKernel(kernel_CPU);
    clReleaseCommandQueue(commandQueue_CPU);
    clReleaseContext(context_CPU);
    free(exponentiate_array_program);
    

}


void Execute_On_Cpu_or_Gpu(int cpu_or_gpu, int* input_array_a, int* input_array_b, int* result_array_c) {
    cl_context context;
    cl_int error;
    cl_command_queue commandQueue;
    
    
    if (cpu_or_gpu == 0) {
        //executing only on CPU
        if (checkForCPU() == 0) {
            printf("NO CPU DETECTED\n");
            return;
        }
        context = clCreateContext(NULL, 1, &cpu_id, NULL, NULL, &error);

        if (error != 0) {
            printf("Error in getting context. Code %d\n", error);
        }

        //create a command queue to queue in commands for CPU
        //1 context and deviceID for each queue and set error code
        commandQueue = clCreateCommandQueue(context, cpu_id, CL_QUEUE_PROFILING_ENABLE, &error);

        if (error != 0) {
            printf("Error in getting command queue. Code %d\n", error);
        }

    }
    else {
        //executing only on GPU
        if (checkForGPU() == 0) {
            printf("NO GPU DETECTED\n");
            return;
        }
        context = clCreateContext(NULL, 1, &gpu_id, NULL, NULL, &error);

        if (error != 0) {
            printf("Error in getting context. Code %d\n", error);
        }

        //create a command queue to queue in commands for CPU
        //1 context and deviceID for each queue and set error code
        commandQueue = clCreateCommandQueue(context, gpu_id, CL_QUEUE_PROFILING_ENABLE, &error);

        if (error != 0) {
            printf("Error in getting command queue. Code %d\n", error);
        }

    }


    /*  create program from source */
    char* exponentiate_array_program = Get_Kernel_File("Kernels.cl");
    /*  first, check if the file exists, otherwise, the function returns NULL */
    if (exponentiate_array_program == NULL)
    {
        printf("Could not read an OpenCL from the specified file \n");
    }

    /*  second, try to create a program from the content of the file */
    cl_program program = clCreateProgramWithSource(context, 1, (const char**)&exponentiate_array_program, NULL, &error);
    if (error != 0)
    {
        printf("Error in creating program. Code %d\n", error);
    }



    //build program
    clBuildProgram(program, 0, NULL, NULL, NULL, NULL);

    //create kernel for your func
    cl_kernel kernel = clCreateKernel(program, "exponentiateArray", &error);

    if (error != 0) {
        printf("Error in creating kernel. Code %d\n", error);
    }
    // allocate memory in device for input array
    //like malloc, allocates space in device
    cl_mem buffer1 = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(int) * ARRAY_SIZE, NULL, &error);

    if (error != 0) {
        printf("Error in creating buffer. Code %d\n", error);
    }
    // allocate memory in device for input array
    //like malloc, allocates space in device
    cl_mem buffer2 = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(int) * ARRAY_SIZE, NULL, &error);

    if (error != 0) {
        printf("Error in creating buffer. Code %d\n", error);
    }

    // allocate memory in device for input array
    //like malloc, allocates space in device
    cl_mem result_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(int) * ARRAY_SIZE, NULL, &error);

    if (error != 0) {
        printf("Error in creating buffer. Code %d\n", error);
    }
  
    //transfer input array from ram to vram by copying array to buffer
    // 2 events for writing
    cl_event events_for_writing[2];



    error = clEnqueueWriteBuffer(commandQueue, buffer1, CL_FALSE, 0, sizeof(int) * ARRAY_SIZE, input_array_a, 0, NULL, &events_for_writing[0]);
    if (error != 0) {
        printf("Error in copying array 1 into buffer. Code %d\n", error);
    }
    error = clEnqueueWriteBuffer(commandQueue, buffer2, CL_FALSE, 0, sizeof(int) * ARRAY_SIZE, input_array_b, 0, NULL, &events_for_writing[1]);

    if (error != 0) {
        printf("Error in copying array 2 into buffer. Code %d\n", error);
    }

    clFlush(commandQueue);

    //set kernel arguments
    int sizeOfArray = ARRAY_SIZE;
    clSetKernelArg(kernel, 0, sizeof(cl_mem), &buffer1);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), &buffer2);
    clSetKernelArg(kernel, 2, sizeof(cl_mem), &result_buffer);
    clSetKernelArg(kernel, 3, sizeof(int), &sizeOfArray);


    size_t local = 10;
    //global has to be multiple of local so need to divide and multiply
    size_t global = ceil(sizeOfArray / (float)local) * local;

    cl_event event_for_computing;
    //start work at this point, number of event 0, numbr of dimensions 1 
    error = clEnqueueNDRangeKernel(commandQueue, kernel, 1, NULL, &global, &local, 2, events_for_writing, &event_for_computing);

    if (error != 0) {
        printf("Error in executing kernel. Code %d\n", error);
    }

    //wait for gpu to finish work
    error = clFinish(commandQueue);

    if (error != 0) {
        printf("Error in finishing work. Code %d\n", error);
    }
    //calculate the execution time for GPU
    cl_ulong start, end;

    clGetEventProfilingInfo(event_for_computing, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &start, NULL);
    clGetEventProfilingInfo(event_for_computing, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &end, NULL);

    printf("Time taken to exponentiate array %f ms\n", (end - start) / 1000000.0);
    
    //read contents of buffer into result array, 0 events, 0 offset
    clEnqueueReadBuffer(commandQueue, result_buffer, CL_TRUE, 0, sizeof(int) * ARRAY_SIZE,result_array_c, 0, NULL, NULL);


    printf("Final array\n");
    for (int i = 0; i < ARRAY_SIZE; i++) {
        printf("%d ^ %d is %d \n",input_array_a[i], input_array_b[i], result_array_c[i]);
    }

    clReleaseMemObject(buffer1);
    clReleaseMemObject(buffer2);
    clReleaseMemObject(result_buffer);
    free(exponentiate_array_program);
    clReleaseProgram(program);
    clReleaseKernel(kernel);
    clReleaseCommandQueue(commandQueue);
    clReleaseContext(context);
 

}




/*
* Requirements: Array size percentage (0% - 100%), FIRST_HALF(1) or SECOND_HALF(2) or FULL_ARRAY(3)
* Usage: int boolVariable = PercentageSize_ProcessingPart_Valid(int 0-100, int 1-3)
* ----------------------------------------------------------------------------------
* Description: Checks to see whether the array percentage (0-100) and processing part(1-3) are within their respective ranges.
*/
int PercentageSize_ProcessingPart_Valid(unsigned int array_size_percentage, int processing_part)
{
    if (array_size_percentage > 100 || array_size_percentage < 0)
    {
        printf("Error: Array size percentage not within the range 0 - 100.\n");
        return bool_false;
    }

    if (processing_part > FULL_ARRAY || processing_part < FIRST_HALF_OF_ARRAY)
    {
        printf("Error: Processing part not within the range 1 - 3.\n");
        return bool_false;
    }

    return bool_true;
}

/*
* Requirements: Array size percentage (0% - 100%), FIRST_HALF(1) or SECOND_HALF(2) or FULL_ARRAY(3)
* Usage: int boolVariable = FullPercentageSize_Equal_FullProcessingPart(int 0-100, int 1-3)
* ------------------------------------------------------------------------------------------
* Description: The array percentage size must be 100 and processing part's value must be 3 (FULL_ARRAY).
It is not possible to ask 40% array size percentage as the first parameter and then ask to process FULL_ARRAY as the second parameter.
*/
int FullPercentageSize_Equal_FullProcessingPart(unsigned int* array_size_percentage, int* processing_part)
{
    if (*array_size_percentage != 100 && *processing_part == FULL_ARRAY)
    {
        printf("Error: Array size percentage is not 100, cannot process full array.\n");
        printf("Would you like to continue processing the whole array (Y/N)?\n");
        char choice = scanf("%c", &choice);
        if (choice == 'n' || choice == 'N') return bool_false;
        *array_size_percentage = 100;
    }

    if (*array_size_percentage == 100 && *processing_part != FULL_ARRAY)
    {
        printf("Error: Processing part of the array chosen is not full, cannot process full array.\n");
        printf("Would you like to continue processing the whole array (Y/N)?\n");
        char choice = scanf("%c", &choice);
        if (choice == 'n' || choice == 'N') return bool_false;
        *processing_part = FULL_ARRAY;
    }
    return bool_true;
}

/*
* Usage: cl_uint number_platforms = Get_Num_Platforms();
* ----------------------------------------------------
* Description: Returns an integer for total number of platforms detected.
*/
cl_uint Get_Num_Platforms()
{
    cl_int status;
    cl_uint num_platforms;
    status = clGetPlatformIDs(0, NULL, &num_platforms);
    if (status != CL_SUCCESS)
    {
        printf("Could not find any platforms.\n");
        system("pause");
        exit(-1);
    }
    return num_platforms;
}

/*
* Usage: cl_platform_id platforms = Get_Platforms();
* ----------------------------------------------------
* Description: Returns an array of all platforms that were deteected by OpenCL.
*/
cl_platform_id* Get_Platforms()
{
    cl_uint num_platforms = Get_Num_Platforms();

    cl_platform_id* platforms;
    platforms = (cl_platform_id*)malloc(sizeof(cl_platform_id) * num_platforms);
    clGetPlatformIDs(num_platforms, platforms, NULL);

    return platforms;
}



/*
* Requirements: cl_platform_id *platforms, cl_uint platform_index
* Usage: Display_Specific_Info(platforms, platform_index)
* -----------------------------------------------------------------
* Description: Display details of specific platform based on index.
*/
void Display_Specific_Platform_Info(const cl_platform_id* platforms, const cl_uint platform_index)
{
    if (platform_index >= Get_Num_Platforms())
    {
        printf("Invalid Index.\n");
        return;
    }

    const char* attributes_names[5] = { "Name", "Vendor", "Version", "Profile", "Extensions" };
    const cl_platform_info attributes_types[5] = { CL_PLATFORM_NAME, CL_PLATFORM_VENDOR, CL_PLATFORM_VERSION, CL_PLATFORM_PROFILE, CL_PLATFORM_EXTENSIONS };

    size_t info_size = 0;
    char* info;
    printf("Using Platform: \n");
    for (int i = 0; i < 5; i++)
    {
        clGetPlatformInfo(platforms[platform_index], attributes_types[i], 0, NULL, &info_size);
        info = (char*)malloc(info_size);
        clGetPlatformInfo(platforms[platform_index], attributes_types[i], info_size, info, NULL);

        printf("%d.%d %-11s: %s\n", platform_index, i + 1, attributes_names[i], info);

        free(info);
    }
    printf("\n");
    system("pause");
}


/*
* Requirements: cl_device_id *devices, cl_uint device_index
* Usage: Display_All_Device_Info(devices, device_index);
* ---------------------------------------------------------------------------------------
* Description: Displays specific device information at index `device_index`.
*/
void Display_Specific_Device_Info(const cl_device_id* devices, const cl_uint device_index)
{
    size_t info_size = 0;
    char* info;

    printf("Using Device:\n");
    // display device name 
    clGetDeviceInfo(devices[device_index], CL_DEVICE_NAME, 0, NULL, &info_size);
    info = (char*)malloc(info_size);
    clGetDeviceInfo(devices[device_index], CL_DEVICE_NAME, info_size, info, NULL);
    printf("%d. Device: %s\n", device_index + 1, info);
    free(info);

    // display hardware device version 
    clGetDeviceInfo(devices[device_index], CL_DEVICE_VERSION, 0, NULL, &info_size);
    info = (char*)malloc(info_size);
    clGetDeviceInfo(devices[device_index], CL_DEVICE_VERSION, info_size, info, NULL);
    printf("   %d.%d Hardware version      : %s\n", device_index + 1, 1, info);
    free(info);

    // display software driver version 
    clGetDeviceInfo(devices[device_index], CL_DRIVER_VERSION, 0, NULL, &info_size);
    info = (char*)malloc(info_size);
    clGetDeviceInfo(devices[device_index], CL_DRIVER_VERSION, info_size, info, NULL);
    printf("   %d.%d Software version      : %s\n", device_index + 1, 2, info);
    free(info);

    // print c version supported by compiler for device 
    clGetDeviceInfo(devices[device_index], CL_DEVICE_OPENCL_C_VERSION, 0, NULL, &info_size);
    info = (char*)malloc(info_size);
    clGetDeviceInfo(devices[device_index], CL_DEVICE_OPENCL_C_VERSION, info_size, info, NULL);
    printf("   %d.%d Software version      : %s\n", device_index + 1, 2, info);
    free(info);

    // display parallel compute units 
    cl_uint number_of_compute_units;
    clGetDeviceInfo(devices[device_index], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(number_of_compute_units), &number_of_compute_units, NULL);
    printf("   %d.%d Parallel compute units: %d\n", device_index + 1, 4, number_of_compute_units);
    printf("\n");
    system("pause");
    system("cls");
}
int checkForCPU() {

    cl_uint number_of_available_platforms = Get_Num_Platforms();
    cl_platform_id* platforms = Get_Platforms();

    cl_uint number_of_available_cpus = 0;
    cl_device_id* cpu_devices;

    //search for cpu device and platform
    for (int i = 0; i < number_of_available_platforms; i++)
    {
        // get all devices
        clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_CPU, 0, NULL, &number_of_available_cpus);
        cpu_devices = (cl_device_id*)malloc(sizeof(cl_device_id) * number_of_available_cpus);
        clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_CPU, number_of_available_cpus, cpu_devices, NULL);

        if (number_of_available_cpus > 0) {
            //selects first available CPU
            cpu_platform = platforms[i];
            cpu_id = cpu_devices[0];
            printf("CPU found on platform:\n");
            Display_Specific_Platform_Info(platforms, i);
            printf("Device Info for CPU\n");
            Display_Specific_Device_Info(cpu_devices, 0);
            free(cpu_devices);
            break;
        }

    }
    return number_of_available_cpus;


}

int checkForGPU() {
    
    cl_uint number_of_available_platforms = Get_Num_Platforms();
    cl_platform_id* platforms = Get_Platforms();

    cl_uint number_of_available_gpus=0;
    cl_device_id* gpu_devices;

    //search for GPU device and platform
    for (int i = 0; i < number_of_available_platforms; i++)
    {
        // get all devices
        clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, 0, NULL, &number_of_available_gpus);
        gpu_devices = (cl_device_id*)malloc(sizeof(cl_device_id) * number_of_available_gpus);
        clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, number_of_available_gpus, gpu_devices, NULL);

        if (number_of_available_gpus > 0) {
            //selects first available CPU
            cpu_platform = platforms[i];
            gpu_id = gpu_devices[0];
            printf("GPU found on platform:\n");
            Display_Specific_Platform_Info(platforms, i);
            printf("Device Info for GPU\n");
            Display_Specific_Device_Info(gpu_devices, 0);
            free(gpu_devices);
            break;
        }

    }
    return number_of_available_gpus;

}


/*
* Usage: char *filename = Get_Kernel_File("filename")
* -----------------------------------------------------------------------------
* Description: This method takes the filename as input parameter and returns the content of the filename if it exists.
*/
char* Get_Kernel_File(char* file_name)
{
    FILE* programFile;
    char* programSource;
    size_t program_source_size;
    programFile = fopen(file_name, "rb");
    if (!programFile)
        return NULL;
    fseek(programFile, 0, SEEK_END);
    program_source_size = ftell(programFile);
    rewind(programFile);
    programSource = (char*)malloc(program_source_size + 1);
    if (programSource) programSource[program_source_size] = '\0';
    if (programSource != 0) fread(programSource, sizeof(char), program_source_size, programFile);
    fclose(programFile);
    return programSource;
}
