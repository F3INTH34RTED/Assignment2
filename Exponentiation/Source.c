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
#define ARRAY_SIZE 50

const unsigned int C_ARRAY_SIZE = ARRAY_SIZE;

enum { bool_false, bool_true };
enum { FIRST_HALF_OF_ARRAY = 1, SECOND_HALF_OF_ARRAY, FULL_ARRAY };
enum { EXEC_CPU = 0, EXEC_GPU };

void Execute_On_Cpu_or_Gpu(int cpu_or_gpu);

void Execute_On_Cpu_and_Gpu(unsigned int array_size_percentage_CPU, int processing_part_CPU, unsigned int array_size_percentage_GPU, int processing_part_GPU);

int PercentageSize_ProcessingPart_Valid(unsigned int array_size_percentage, int processing_part);
int FullPercentageSize_Equal_FullProcessingPart(unsigned int *array_size_percentage, int *processing_part);

int Get_Cpus();
int Get_Gpus();

//device and platform id for cpu
cl_device_id cpu_devices_id;
cl_platform_id cpu_platform;

// device and platform id for gpu
cl_device_id gpu_devices_id;
cl_platform_id gpu_platform;

cl_uint Get_Num_Platforms();
cl_platform_id *Get_Platforms();

void Display_Specific_Platform_Info(const cl_platform_id *platforms, const cl_uint index);
void Display_Specific_Device_Info(const cl_device_id *devices, const cl_uint device_index);
char *Get_Kernel_File(char *file_name);

int *input_array_a;
int *input_array_b;
int *result_array_c;

int main()
{
    // INITIALIZE ARRAYS
    input_array_a = (int *)malloc(sizeof(int) * ARRAY_SIZE);
    input_array_b = (int *)malloc(sizeof(int) * ARRAY_SIZE);
    result_array_c = (int *)malloc(sizeof(int) * ARRAY_SIZE);

    int counter = 0;
    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        if (input_array_a && input_array_b && result_array_c)
        {
            input_array_a[i] = i;
            
            if (i % 5 == 0) {
                counter = 0;
            }
            input_array_b[i] = counter++;
            
            result_array_c[i] = 1;
        }
    }

    int choice = 0;
    do
    {
        printf("Press 1 to quit.\n");
        printf("Press 2 to exponentiate arrays using GPU only\n");
        printf("Press 3 to exponentiate arrays using CPU only\n");
        printf("Press 4 to exponentiate arrays using 25%% of CPU and 75%% of GPU\n");
        printf("Press 5 to exponentiate arrays using 50%% of CPU and 50%% of GPU\n");
        printf("Press 6 to exponentiate arrays using 75%% of CPU and 25%% of GPU\n");
        printf("Enter your choice:\n");
        scanf("%d", &choice);
        system("cls");
        switch (choice) {
        case 1:
            free(input_array_a);
            free(input_array_b);
            free(result_array_c);
            return 0;
            break;
        case 2:
        {
            Execute_On_Cpu_or_Gpu(EXEC_GPU);
            break;
        }
        case 3:
        {
            Execute_On_Cpu_or_Gpu(EXEC_CPU);
            break;
        }
        case 4:
        {
            Execute_On_Cpu_and_Gpu(25, FIRST_HALF_OF_ARRAY, 75, SECOND_HALF_OF_ARRAY);
            break;
        }
        case 5:
        {
            Execute_On_Cpu_and_Gpu(50, FIRST_HALF_OF_ARRAY, 50, SECOND_HALF_OF_ARRAY);
            break;
        }
        case 6:
        {
            Execute_On_Cpu_and_Gpu(75, FIRST_HALF_OF_ARRAY, 25, SECOND_HALF_OF_ARRAY);
            break;
        }
        default:
            printf("Wrong Input\n");
            break;
        }
        system("pause");
        system("cls");
    } while (choice != 1);

    return 0;
}

/*
FUNCTION: Execute On Cpu and Gpu
PRE-REQ:  Array size percentage (0% - 100%), FIRST_HALF(1) or SECOND_HALF(2) or FULL_ARRAY(3)
USAGE:    Execute_On_Cpu_and_Gpu(int 0-100, int 1-3)
-----------------------------------------------------------------------------------------------------------
DESC: Calculates the total number of elements to be executed in the cpu/gpu (array_size_percentage)
and then executes the elements based on the processing_part (first half, second half, or the full array).
*/
void Execute_On_Cpu_and_Gpu(unsigned int array_size_percentage_CPU, int processing_part_CPU, unsigned int array_size_percentage_GPU, int processing_part_GPU) {
    if (Get_Cpus() == 0) {
        printf("NO CPU DETECTED");
        return;
    }
    if (Get_Gpus() == 0) {
        printf("NO GPU DETECTED");
        return;
    }

    // Check validity of parameters for CPU
    if (!PercentageSize_ProcessingPart_Valid(array_size_percentage_CPU, processing_part_CPU)) return;
    if (!FullPercentageSize_Equal_FullProcessingPart(&array_size_percentage_CPU, &processing_part_CPU)) return;

    // Check validity of parameters for GPU
    if (!PercentageSize_ProcessingPart_Valid(array_size_percentage_GPU, processing_part_GPU)) return;
    if (!FullPercentageSize_Equal_FullProcessingPart(&array_size_percentage_GPU, &processing_part_GPU)) return;

    printf("EXECUTING %d%% on CPU and %d%% on GPU \n", array_size_percentage_CPU, array_size_percentage_GPU);
    printf("Arrays:\n");
    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        if (input_array_a && input_array_b && result_array_c)
            printf("a[%04d] = %04d\tb[%04d] = %04d\tc[%04d] = %04d\n", i, input_array_a[i], i, input_array_b[i], i, result_array_c[i]);
    }

    // Calculate number of elements to process based on percentage provided for CPU
    int array_size_cpu = (int)((C_ARRAY_SIZE * array_size_percentage_CPU) / 100.0);
    int start_index_cpu; int end_index_cpu;

    switch (processing_part_CPU)
    {
    case FIRST_HALF_OF_ARRAY:
        start_index_cpu = 0;
        end_index_cpu = array_size_cpu - 1;
        break;
    case SECOND_HALF_OF_ARRAY:
        start_index_cpu = C_ARRAY_SIZE - array_size_cpu;
        end_index_cpu = C_ARRAY_SIZE - 1;
        array_size_cpu = C_ARRAY_SIZE - start_index_cpu;
        break;

    case FULL_ARRAY:
        start_index_cpu = 0;
        end_index_cpu = C_ARRAY_SIZE - 1;
        break;
    }

    // Calculate number of elements to process based on percentage provided for GPU
    unsigned int array_size_gpu = (int)(ceil)((C_ARRAY_SIZE * array_size_percentage_GPU) / 100.0);
    int start_index_gpu; int end_index_gpu;

    switch (processing_part_GPU)
    {
    case FIRST_HALF_OF_ARRAY:
        start_index_gpu = 0;
        end_index_gpu = array_size_gpu - 1;
        break;
    case SECOND_HALF_OF_ARRAY:
        start_index_gpu = C_ARRAY_SIZE - array_size_gpu;
        end_index_gpu = C_ARRAY_SIZE - 1;
        array_size_gpu = C_ARRAY_SIZE - start_index_gpu;
        break;

    case FULL_ARRAY:
        start_index_gpu = 0;
        end_index_gpu = C_ARRAY_SIZE - 1;
        break;
    }

    printf("CPU exponentiates %d elements while GPU exponentiates %d \n\n", array_size_cpu, array_size_gpu);
    system("pause");
    system("cls");

    // Init gpu environment
    cl_context context_gpu;
    cl_command_queue commandQ_gpu;
    cl_mem clmem_input_array_a_gpu;
    cl_mem clmem_input_array_b_gpu;
    cl_mem clmem_result_array_c_gpu;

    // Init cpu environment
    cl_context context_cpu;
    cl_command_queue commandQ_cpu;
    cl_mem clmem_input_array_a_cpu;
    cl_mem clmem_input_array_b_cpu;
    cl_mem clmem_result_array_c_cpu;
    
    cl_int status;

    // Creating gpu enviroment
    context_gpu = clCreateContext(NULL, 1, &gpu_devices_id, NULL, NULL, &status);
    if (status != 0) printf("Could not create context for GPU. Error %d\n", status);
    else printf("GPU Context successfuly created!\n");

    // for older version change clCreateCommandQueueWithProperties to clCreateCommandQueue
    commandQ_gpu = clCreateCommandQueue(context_gpu, gpu_devices_id, CL_QUEUE_PROFILING_ENABLE, &status);
    if (status != 0) printf("Could not create command queue for GPU. Error %d\n", status);
    else printf("GPU command queue successfuly created!\n");

    clmem_input_array_a_gpu = clCreateBuffer(context_gpu, CL_MEM_READ_WRITE, sizeof(int) * (array_size_gpu), NULL, &status);
    if (status != 0) printf("Could not create input array a buffer for GPU. Error %d\n", status);
    else printf("Input array a buffer successfully created for the GPU\n");

    clmem_input_array_b_gpu = clCreateBuffer(context_gpu, CL_MEM_READ_WRITE, sizeof(int) * (array_size_gpu), NULL, &status);
    if (status != 0) printf("Could not create input array b buffer for GPU. Error %d\n", status);
    else printf("Input array b buffer successfully created for the GPU\n");

    clmem_result_array_c_gpu = clCreateBuffer(context_gpu, CL_MEM_READ_WRITE, sizeof(int) * (array_size_gpu), NULL, &status);
    if (status != 0) printf("Could not create result array c buffer for GPU. Error %d\n\n", status);
    else printf("Result array c buffer successfully created for the GPU\n\n");

    // Creating cpu enviroment
    context_cpu = clCreateContext(NULL, 1, &cpu_devices_id, NULL, NULL, &status);
    if (status != 0) printf("Could not create context for CPU. Error %d\n", status);
    else printf("CPU Context successfuly created!\n");

    commandQ_cpu = clCreateCommandQueue(context_cpu, cpu_devices_id, CL_QUEUE_PROFILING_ENABLE, &status);
    if (status != 0) printf("Could not create command queue for CPU. Error %d\n", status);
    else printf("CPU command queue successfuly created!\n");

    clmem_input_array_a_cpu = clCreateBuffer(context_cpu, CL_MEM_READ_WRITE, sizeof(int) * (array_size_cpu), NULL, &status);
    if (status != 0) printf("Could not create input array a buffer for CPU. Error %d\n", status);
    else printf("Input array a buffer successfully created for the CPU\n");

    clmem_input_array_b_cpu = clCreateBuffer(context_cpu, CL_MEM_READ_WRITE, sizeof(int) * (array_size_cpu), NULL, &status);
    if (status != 0) printf("Could not create input array b buffer for CPU. Error %d\n", status);
    else printf("Input array b buffer successfully created for the CPU\n");

    clmem_result_array_c_cpu = clCreateBuffer(context_cpu, CL_MEM_READ_WRITE, sizeof(int) * (array_size_cpu), NULL, &status);
    if (status != 0) printf("Could not create result array c buffer for CPU. Error %d\n", status);
    else printf("Result array c buffer successfully created for the CPU\n");

    system("pause");
    system("cls");

    // 4 events for writing
    cl_event events_for_writing_CPU[2];
    cl_event events_for_writing_GPU[2];
    
    // Writing buffers from host to gpu device.
    status = clEnqueueWriteBuffer(commandQ_gpu, clmem_input_array_a_gpu, CL_FALSE, 0, sizeof(int) * (array_size_gpu), input_array_a + start_index_gpu, 0, NULL, &events_for_writing_GPU[0]);
    if (status != 0) printf("Could not write input array a buffer to GPU. Error %d\n", status);
    else printf("Input array a successfuly written to GPU!\n");

    status = clEnqueueWriteBuffer(commandQ_gpu, clmem_input_array_b_gpu, CL_FALSE, 0, sizeof(int) * (array_size_gpu), input_array_b + start_index_gpu, 0, NULL, &events_for_writing_GPU[1]);
    if (status != 0) printf("Could not write input array b buffer to GPU. Error %d\n", status);
    else printf("Input array b successfuly written to GPU!\n\n");

    // Writing buffers from host to cpu device.
    status = clEnqueueWriteBuffer(commandQ_cpu, clmem_input_array_a_cpu, CL_TRUE, 0, sizeof(int) * (array_size_cpu), input_array_a + start_index_cpu, 0, NULL, &events_for_writing_CPU[0]);
    if (status != 0) printf("Could not write input array a buffer to CPU. Error %d\n", status);
    else printf("Input array a successfuly written to CPU!\n");

    status = clEnqueueWriteBuffer(commandQ_cpu, clmem_input_array_b_cpu, CL_TRUE, 0, sizeof(int) * (array_size_cpu), input_array_b + start_index_cpu, 0, NULL, &events_for_writing_CPU[1]);
    if (status != 0) printf("Could not write input array b buffer to CPU. Error %d\n", status);
    else printf("Input array b successfuly written to CPU!\n");

    clFlush(commandQ_gpu);
    clFlush(commandQ_cpu);

    system("pause");
    system("cls");

    // Reading kernel file
    char *exponentiate_array_program = Get_Kernel_File("Kernels.cl");
    if (exponentiate_array_program == NULL) printf("Could not read the kernel file \n");
    else printf("Successfully read Kernel file.\n");

    // Creating GPU program and kernel.
    cl_program program_GPU;
    program_GPU = clCreateProgramWithSource(context_gpu, 1, (const char **)&exponentiate_array_program, NULL, &status);
    if (status != 0) printf("Error in creating program for GPU. Code %d\n", status);
    else printf("Program successfully created for GPU.\n");

    clBuildProgram(program_GPU, 0, NULL, NULL, NULL, NULL);

    cl_kernel kernel_GPU;
    kernel_GPU = clCreateKernel(program_GPU, "exponentiateArray", &status);
    if (status != 0) printf("Error in creating kernel for GPU. Code %d\n", status);
    else printf("Kernel successfully created for GPU.\n");

    // Creating CPU program and kernel.
    cl_program program_CPU;
    program_CPU = clCreateProgramWithSource(context_cpu, 1, (const char **)&exponentiate_array_program, NULL, &status);
    if (status != 0) printf("Error in creating program for CPU. Code %d\n", status);
    else printf("Program successfully created for CPU.\n");

    clBuildProgram(program_CPU, 0, NULL, NULL, NULL, NULL);

    cl_kernel kernel_CPU;
    kernel_CPU = clCreateKernel(program_CPU, "exponentiateArray", &status);
    if (status != 0) printf("Error in creating kernel for CPU. Code %d\n", status);
    else printf("Kernel successfully created for CPU.\n");

    //Gpu kernel arguments
    status = clSetKernelArg(kernel_GPU, 0, sizeof(cl_mem), &clmem_input_array_a_gpu);
    if (status != 0) printf("Could not set argument 0 for GPU kernel. Code %d\n", status);
    else printf("Argument 0 for GPU kernel successfully set.\n");
    status = clSetKernelArg(kernel_GPU, 1, sizeof(cl_mem), &clmem_input_array_b_gpu);
    if (status != 0) printf("Could not set argument 1 for GPU kernel. Code %d\n", status);
    else printf("Argument 1 for GPU kernel successfully set.\n");
    status = clSetKernelArg(kernel_GPU, 2, sizeof(cl_mem), &clmem_result_array_c_gpu);
    if (status != 0) printf("Could not set argument 2 for GPU kernel. Code %d\n", status);
    else printf("Argument 2 for GPU kernel successfully set.\n");
    status = clSetKernelArg(kernel_GPU, 3, sizeof(int), &array_size_gpu);
    if (status != 0) printf("Could not set argument 3 for GPU kernel. Code %d\n", status);
    else printf("Argument 3 for GPU kernel successfully set.\n\n");

    // Cpu kernel arguments
    status = clSetKernelArg(kernel_CPU, 0, sizeof(cl_mem), &clmem_input_array_a_cpu);
    if (status != 0) printf("Could not set argument 0 for CPU kernel. Code %d\n", status);
    else printf("Argument 0 for CPU kernel successfully set.\n");
    status = clSetKernelArg(kernel_CPU, 1, sizeof(cl_mem), &clmem_input_array_b_cpu);
    if (status != 0) printf("Could not set argument 1 for CPU kernel. Code %d\n", status);
    else printf("Argument 1 for CPU kernel successfully set.\n");
    status = clSetKernelArg(kernel_CPU, 2, sizeof(cl_mem), &clmem_result_array_c_cpu);
    if (status != 0) printf("Could not set argument 2 for CPU kernel. Code %d\n", status);
    else printf("Argument 2 for CPU kernel successfully set.\n");
    status = clSetKernelArg(kernel_CPU, 3, sizeof(int), &array_size_cpu);
    if (status != 0) printf("Could not set argument 3 for CPU kernel. Code %d\n", status);
    else printf("Argument 3 for CPU kernel successfully set.\n");

    size_t local_CPU = 10;
    size_t global_CPU = ceil(array_size_cpu) * local_CPU;

    size_t local_GPU = 10;
    size_t global_GPU = ceil(array_size_gpu) * local_GPU;

    system("pause");
    system("cls");

    // 2 events for computing
    cl_event events_for_computing[2];
    status = clEnqueueNDRangeKernel(commandQ_gpu, kernel_GPU, 1, NULL, &global_GPU, &local_GPU, 2, events_for_writing_GPU, &events_for_computing[0]);
    if (status != 0) printf("Kernel could not be executed in GPU. Code %d\n", status);
    else printf("Kernel successfully executed in GPU.\n");

    status = clEnqueueNDRangeKernel(commandQ_cpu, kernel_CPU, 1, NULL, &global_CPU, &local_CPU, 2, events_for_writing_CPU, &events_for_computing[1]);
    if (status != 0) printf("Kernel could not be executed in CPU. Code %d\n", status);
    else printf("Kernel successfully executed in CPU.\n");

    status = clFinish(commandQ_gpu);
    if (status != 0) printf("Error in waiting for GPU kernel. Code %d\n", status);
    else printf("GPU task successfully completed.\n");

    status = clFinish(commandQ_cpu);
    if (status != 0) printf("Error in waiting for CPU kernel. Code %d\n", status);
    else printf("CPU task successfully completed.\n");

    //Calculate the execution time for GPU
    cl_ulong start, end;
    clGetEventProfilingInfo(events_for_computing[0], CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &start, NULL);
    clGetEventProfilingInfo(events_for_computing[0], CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &end, NULL);

    //Calculate the execution time for CPU
    cl_ulong start2, end2;
    clGetEventProfilingInfo(events_for_computing[1], CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &start2, NULL);
    clGetEventProfilingInfo(events_for_computing[1], CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &end2, NULL);

    // Read result buffer from gpu and cpu
    status = clEnqueueReadBuffer(commandQ_gpu, clmem_result_array_c_gpu, CL_FALSE, 0, sizeof(int) * (array_size_gpu), result_array_c + start_index_gpu, 1, &events_for_computing[0], NULL);
    if (status != 0) printf("Could not read result buffer from GPU to host. Code %d\n", status);
    else printf("Successfuly read result buffer from GPU to host.\n");
    
    status = clEnqueueReadBuffer(commandQ_cpu, clmem_result_array_c_cpu, CL_FALSE, 0, sizeof(int) * (array_size_cpu), result_array_c + start_index_cpu, 1, &events_for_computing[1], NULL);
    if (status != 0) printf("Could not read result buffer from CPU to host. Code %d\n", status);
    else printf("Successfuly read result buffer from CPU to host.\n");

    clFinish(commandQ_gpu);
    clFinish(commandQ_cpu);

    system("pause");
    system("cls");

    //Display Results
    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        if (input_array_a && input_array_b && result_array_c)
            printf("%02d ^ %02d = %d\n", input_array_a[i], input_array_b[i], result_array_c[i]);
    }
    printf("Time taken to exponentiate array in GPU %f ms\n", (end - start) / 1000000.0);
    printf("Time taken to exponentiate array in CPU %f ms\n", (end2 - start2) / 1000000.0);

    clReleaseMemObject(clmem_input_array_a_gpu);
    clReleaseMemObject(clmem_input_array_b_gpu);
    clReleaseMemObject(clmem_result_array_c_gpu);
    clReleaseProgram(program_GPU);
    clReleaseKernel(kernel_GPU);
    clReleaseCommandQueue(commandQ_gpu);
    clReleaseContext(context_gpu);

    clReleaseMemObject(clmem_input_array_a_cpu);
    clReleaseMemObject(clmem_input_array_b_cpu);
    clReleaseMemObject(clmem_result_array_c_cpu);
    clReleaseProgram(program_CPU);
    clReleaseKernel(kernel_CPU);
    clReleaseCommandQueue(commandQ_cpu);
    clReleaseContext(context_cpu);
    free(exponentiate_array_program);
}

/*
FUNCTION : Execute On Cpu or Gpu
PRE-REQ  :  int EXEC_CPU(0) or EXEC_GPU(1)
USAGE    :    Execute_On_Cpu_or_Gpu(int 0-1)
-----------------------------------------------------------------------------------------------------------
DESC     : Exponentiates the whole array either on cpu or gpu based on the parameter passed
*/
void Execute_On_Cpu_or_Gpu(int cpu_or_gpu) {
    printf("Arrays:\n");
    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        if (input_array_a && input_array_b && result_array_c)
            printf("a[%04d] = %04d\tb[%04d] = %04d\tc[%04d] = %04d\n", i, input_array_a[i], i, input_array_b[i], i, result_array_c[i]);
    }
    system("pause");
    system("cls");

    cl_context context;
    cl_int status;
    cl_command_queue commandQueue;

    if (cpu_or_gpu == EXEC_CPU) {
        if (Get_Cpus() == 0) {
            printf("NO CPU DETECTED\n");
            return;
        }
        context = clCreateContext(NULL, 1, &cpu_devices_id, NULL, NULL, &status);
        if (status != 0) printf("Could not create context for CPU. Error %d\n", status);
        else printf("CPU Context successfuly created!\n");

        commandQueue = clCreateCommandQueue(context, cpu_devices_id, CL_QUEUE_PROFILING_ENABLE, &status);
        if (status != 0) printf("Could not create command queue for CPU. Error %d\n", status);
        else printf("CPU command queue successfuly created!\n");
    }
    else {
        if (Get_Gpus() == 0) {
            printf("NO GPU DETECTED\n");
            return;
        }
        context = clCreateContext(NULL, 1, &gpu_devices_id, NULL, NULL, &status);
        if (status != 0) printf("Could not create context for GPU. Error %d\n", status);
        else printf("GPU Context successfuly created!\n");

        commandQueue = clCreateCommandQueue(context, gpu_devices_id, CL_QUEUE_PROFILING_ENABLE, &status);
        if (status != 0) printf("Could not create command queue for GPU. Error %d\n", status);
        else printf("GPU command queue successfuly created!\n");
    }

    char *exponentiate_array_program = Get_Kernel_File("Kernels.cl");
    if (status != 0) printf("Could not read kernel file. Error %d\n", status);
    else printf("Kernel file successfuly read!\n");

    cl_program program = clCreateProgramWithSource(context, 1, (const char **)&exponentiate_array_program, NULL, &status);
    if (status != 0) printf("Could not create program. Error %d\n", status);
    else printf("Program successfuly created!\n");

    clBuildProgram(program, 0, NULL, NULL, NULL, NULL);

    cl_kernel kernel = clCreateKernel(program, "exponentiateArray", &status);
    if (status != 0) printf("Could not create kernel. Error %d\n", status);
    else printf("Kernel successfuly created!\n");

    cl_mem clmem_input_array_a = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(int) * ARRAY_SIZE, NULL, &status);
    if (status != 0) printf("Could not create input array a buffer. Error %d\n", status);
    else printf("Input array a buffer successfuly created!\n");

    cl_mem clmem_input_array_b = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(int) * ARRAY_SIZE, NULL, &status);
    if (status != 0) printf("Could not create input array b buffer. Error %d\n", status);
    else printf("Input array b buffer successfuly created!\n");

    cl_mem clmem_result_array_c = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(int) * ARRAY_SIZE, NULL, &status);
    if (status != 0) printf("Could not create result array c buffer. Error %d\n", status);
    else printf("Result array c buffer successfuly created!\n");

    cl_event events_for_writing[2];
    status = clEnqueueWriteBuffer(commandQueue, clmem_input_array_a, CL_FALSE, 0, sizeof(int) * ARRAY_SIZE, input_array_a, 0, NULL, &events_for_writing[0]);
    if (status != 0) printf("Could not write input array a to device. Error %d\n", status);
    else printf("Input array a written successfuly to the device!\n");

    status = clEnqueueWriteBuffer(commandQueue, clmem_input_array_b, CL_FALSE, 0, sizeof(int) * ARRAY_SIZE, input_array_b, 0, NULL, &events_for_writing[1]);
    if (status != 0) printf("Could not write input array b to device. Error %d\n", status);
    else printf("Input array b written successfuly to the device!\n");

    clFlush(commandQueue);

    clSetKernelArg(kernel, 0, sizeof(cl_mem), &clmem_input_array_a);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), &clmem_input_array_b);
    clSetKernelArg(kernel, 2, sizeof(cl_mem), &clmem_result_array_c);
    clSetKernelArg(kernel, 3, sizeof(int), &C_ARRAY_SIZE);

    size_t local = 10;
    size_t global = ceil(C_ARRAY_SIZE / (float)local) * local;

    cl_event event_for_computing;
    status = clEnqueueNDRangeKernel(commandQueue, kernel, 1, NULL, &global, &local, 2, events_for_writing, &event_for_computing);
    if (status != 0) printf("Could not execute kernel. Error %d\n", status);
    else printf("Kernel successfully executed!\n");

    status = clFinish(commandQueue);
    if (status != 0) printf("Could not finish the work. Error %d\n", status);
    else printf("Work successfully finished!\n");

    cl_ulong start, end;
    clGetEventProfilingInfo(event_for_computing, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &start, NULL);
    clGetEventProfilingInfo(event_for_computing, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &end, NULL);

    status = clEnqueueReadBuffer(commandQueue, clmem_result_array_c, CL_TRUE, 0, sizeof(int) * ARRAY_SIZE, result_array_c, 0, NULL, NULL);
    if (status != 0) printf("Could not read the result buffer from device to host. Error %d\n", status);
    else printf("Result buffer successfully read from device to host!\n");

    system("pause");
    system("cls");

    printf("Final array\n");
    for (int i = 0; i < ARRAY_SIZE; i++) {
        if (input_array_a && input_array_b && result_array_c)
            printf("%02d ^ %02d is %d \n", input_array_a[i], input_array_b[i], result_array_c[i]);
    }
    printf("Time taken to exponentiate array %f ms\n", (end - start) / 1000000.0);

    clReleaseMemObject(clmem_input_array_a);
    clReleaseMemObject(clmem_input_array_b);
    clReleaseMemObject(clmem_result_array_c);
    free(exponentiate_array_program);
    clReleaseProgram(program);
    clReleaseKernel(kernel);
    clReleaseCommandQueue(commandQueue);
    clReleaseContext(context);
}

/*
FUNCITON : Percentage Size and Processing Part Validity
PRE-REQ  : Array size percentage (0% - 100%), FIRST_HALF(1) or SECOND_HALF(2) or FULL_ARRAY(3)
USAGE    : int boolVariable = PercentageSize_ProcessingPart_Valid(int 0-100, int 1-3)
----------------------------------------------------------------------------------
DESC     : Checks to see whether the array percentage (0-100) and processing part(1-3) are within their respective ranges.
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
FUNCTION : Full Percentage Size Equals Full Processing Part
PRE-REQ  : Array size percentage (0% - 100%), FIRST_HALF(1) or SECOND_HALF(2) or FULL_ARRAY(3)
USAGE    : int boolVariable = FullPercentageSize_Equal_FullProcessingPart(int 0-100, int 1-3)
------------------------------------------------------------------------------------------
DESC     : The array percentage size must be 100 and processing part's value must be 3 (FULL_ARRAY).
           It is not possible to ask 40% array size percentage as the first parameter and then ask to process FULL_ARRAY as the
           second parameter.
*/
int FullPercentageSize_Equal_FullProcessingPart(unsigned int *array_size_percentage, int *processing_part)
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
        free(input_array_a);
        free(input_array_b);
        free(result_array_c);
        exit(-1);
    }
    return num_platforms;
}

/*
* Usage: cl_platform_id platforms = Get_Platforms();
* ----------------------------------------------------
* Description: Returns an array of all platforms that were deteected by OpenCL.
*/
cl_platform_id *Get_Platforms()
{
    cl_uint num_platforms = Get_Num_Platforms();

    cl_platform_id *platforms;
    platforms = (cl_platform_id *)malloc(sizeof(cl_platform_id) * num_platforms);
    clGetPlatformIDs(num_platforms, platforms, NULL);

    return platforms;
}



/*
* Requirements: cl_platform_id *platforms, cl_uint platform_index
* Usage: Display_Specific_Info(platforms, platform_index)
* -----------------------------------------------------------------
* Description: Display details of specific platform based on index.
*/
void Display_Specific_Platform_Info(const cl_platform_id *platforms, const cl_uint platform_index)
{
    if (platform_index >= Get_Num_Platforms())
    {
        printf("Invalid Index.\n");
        return;
    }

    const char *attributes_names[5] = { "Name", "Vendor", "Version", "Profile", "Extensions" };
    const cl_platform_info attributes_types[5] = { CL_PLATFORM_NAME, CL_PLATFORM_VENDOR, CL_PLATFORM_VERSION, CL_PLATFORM_PROFILE, CL_PLATFORM_EXTENSIONS };

    size_t info_size = 0;
    char *info;
    printf("Using Platform: \n");
    for (int i = 0; i < 5; i++)
    {
        clGetPlatformInfo(platforms[platform_index], attributes_types[i], 0, NULL, &info_size);
        info = (char *)malloc(info_size);
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
void Display_Specific_Device_Info(const cl_device_id *devices, const cl_uint device_index)
{
    size_t info_size = 0;
    char *info;

    printf("Using Device:\n");
    // display device name 
    clGetDeviceInfo(devices[device_index], CL_DEVICE_NAME, 0, NULL, &info_size);
    info = (char *)malloc(info_size);
    clGetDeviceInfo(devices[device_index], CL_DEVICE_NAME, info_size, info, NULL);
    printf("%d. Device: %s\n", device_index + 1, info);
    free(info);

    // display hardware device version 
    clGetDeviceInfo(devices[device_index], CL_DEVICE_VERSION, 0, NULL, &info_size);
    info = (char *)malloc(info_size);
    clGetDeviceInfo(devices[device_index], CL_DEVICE_VERSION, info_size, info, NULL);
    printf("   %d.%d Hardware version      : %s\n", device_index + 1, 1, info);
    free(info);

    // display software driver version 
    clGetDeviceInfo(devices[device_index], CL_DRIVER_VERSION, 0, NULL, &info_size);
    info = (char *)malloc(info_size);
    clGetDeviceInfo(devices[device_index], CL_DRIVER_VERSION, info_size, info, NULL);
    printf("   %d.%d Software version      : %s\n", device_index + 1, 2, info);
    free(info);

    // print c version supported by compiler for device 
    clGetDeviceInfo(devices[device_index], CL_DEVICE_OPENCL_C_VERSION, 0, NULL, &info_size);
    info = (char *)malloc(info_size);
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

/*
FUNCTION : Get Cpu's
PRE-REQ  : N/A
USAGE    : int get_total_cpus = Get_Cpu()
---------------------------------------
DESC     : Returns total number of Cpu's detected.
*/
int Get_Cpus() {
    cl_uint number_of_available_platforms = Get_Num_Platforms();
    cl_platform_id *platforms = Get_Platforms();

    cl_uint number_of_available_cpus = 0;
    cl_device_id *cpu_devices;

    //search for cpu device and platform
    for (cl_uint i = 0; i < number_of_available_platforms; i++)
    {
        // get all devices
        clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_CPU, 0, NULL, &number_of_available_cpus);
        cpu_devices = (cl_device_id *)malloc(sizeof(cl_device_id) * number_of_available_cpus);
        clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_CPU, number_of_available_cpus, cpu_devices, NULL);

        if (number_of_available_cpus > 0) {
            //selects first available CPU
            cpu_platform = platforms[i];
            if (cpu_devices) cpu_devices_id = cpu_devices[0];
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

/*
FUNCTION : Get Gpu's
PRE-REQ  : N/A
USAGE    : int get_total_gpus = Get_Gpu()
---------------------------------------
DESC     : Returns total number of Gpu's detected.
*/
int Get_Gpus() {

    cl_uint number_of_available_platforms = Get_Num_Platforms();
    cl_platform_id *platforms = Get_Platforms();

    cl_uint number_of_available_gpus = 0;
    cl_device_id *gpu_devices;

    //search for GPU device and platform
    for (cl_uint i = 0; i < number_of_available_platforms; i++)
    {
        // get all devices
        clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, 0, NULL, &number_of_available_gpus);
        gpu_devices = (cl_device_id *)malloc(sizeof(cl_device_id) * number_of_available_gpus);
        clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, number_of_available_gpus, gpu_devices, NULL);

        if (number_of_available_gpus > 0) {
            //selects first available CPU
            cpu_platform = platforms[i];
            if (gpu_devices) gpu_devices_id = gpu_devices[0];
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
char *Get_Kernel_File(char *file_name)
{
    FILE *programFile;
    char *programSource;
    size_t program_source_size;
    programFile = fopen(file_name, "rb");
    if (!programFile)
        return NULL;
    fseek(programFile, 0, SEEK_END);
    program_source_size = ftell(programFile);
    rewind(programFile);
    programSource = (char *)malloc(program_source_size + 1);
    if (programSource) programSource[program_source_size] = '\0';
    if (programSource != 0) fread(programSource, sizeof(char), program_source_size, programFile);
    fclose(programFile);
    return programSource;
}