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

void Execute_On_Cpu(unsigned int array_size_percentage, int processing_part, int* input_array_a, int* input_array_b, int* result_array_c);
void Execute_On_Gpu(unsigned int array_size_percentage, int processing_part, int* input_array_a, int* input_array_b, int* result_array_c);

int PercentageSize_ProcessingPart_Valid(unsigned int array_size_percentage, int processing_part);
int FullPercentageSize_Equal_FullProcessingPart(unsigned int* array_size_percentage, int* processing_part);

cl_uint Get_Num_Platforms();
cl_platform_id* Get_Platforms();
void Display_All_Platform_Info(const cl_platform_id* platforms);
void Display_Specific_Platform_Info(const cl_platform_id* platforms, const cl_uint index);



cl_uint Get_Num_Devices(const cl_platform_id* platforms, const cl_uint platform_index);
cl_device_id* Get_Devices(cl_platform_id* platforms, const cl_uint platform_index);
void Display_All_Device_Info(const cl_device_id* devices, const cl_platform_id* platforms, const cl_uint platform_index);
void Display_Specific_Device_Info(const cl_device_id* devices, const cl_uint device_index);

//void Create_Context(cl_context* context, const cl_platform_id* platforms, const cl_device_id* devices, const cl_uint platform_index);

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

    Execute_On_Cpu(25, FIRST_HALF_OF_ARRAY, input_array_a, input_array_b, result_array_c);
    system("pause");
    system("cls");
    Execute_On_Gpu(75, SECOND_HALF_OF_ARRAY, input_array_a, input_array_b, result_array_c);

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
void Execute_On_Cpu(unsigned int array_size_percentage, int processing_part, int* input_array_a, int* input_array_b, int* result_array_c)
{
    printf("EXECUTING ON CPU\n");
    printf("Arrays:\n");
    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        if (input_array_a && input_array_b && result_array_c)
            printf("a[%04d] = %04d\tb[%04d] = %04d\tc[%04d] = %04d\n", i, input_array_a[i], i, input_array_b[i], i, result_array_c[i]);
    }
    system("pause");
    system("cls");

    // Check validity of parameters
    if (!PercentageSize_ProcessingPart_Valid(array_size_percentage, processing_part)) return;
    if (!FullPercentageSize_Equal_FullProcessingPart(&array_size_percentage, &processing_part)) return;

    int array_size = (int)((C_ARRAY_SIZE * array_size_percentage) / 100.0); // Calculate number of elements to process based on percentage provided
    printf("Array size to calc is %d\n ", array_size);
    int start; int end;
    switch (processing_part)
    {
        
    case FIRST_HALF_OF_ARRAY:
        start = 0;
        end = array_size-1;
        break;
    case SECOND_HALF_OF_ARRAY:
        start = C_ARRAY_SIZE-array_size ;
        end = C_ARRAY_SIZE-1;
        array_size = C_ARRAY_SIZE - start;
        break;

    case FULL_ARRAY:
        start = 0;
        end = C_ARRAY_SIZE - 1;
        break;
    }
    printf("Starts at %d and ends at %d", start, end);


    // PLATFORMS
    cl_platform_id* platforms = Get_Platforms();
    cl_uint platform_index = 1; // The appropriate platform for CPU must be detected accordingly.
    Display_All_Platform_Info(platforms);

    // DEVICES
    cl_device_id* devices = Get_Devices(platforms, platform_index);
    /*
    * If there is no integrated gpu, then there will only be 1 cpu device and hence, device_index will be 0.
    * If there are n integrated gpus, total devices will be (n+1) devices. Hence, cpu will be at index (n-1)
    */
    cl_uint device_index = Get_Num_Devices(platforms, platform_index) == 1 ? 0 : Get_Num_Devices(platforms, platform_index) - 1;
    Display_All_Device_Info(devices, platforms, platform_index);

    // CREATE CONTEXT
    cl_int status = 0;
    cl_context context = clCreateContext(0, Get_Num_Devices(platforms, platform_index), devices, NULL, NULL, &status);
    if (status != CL_SUCCESS)
    {
        printf("Could not create a context.\n");
        free(devices);
        free(platforms);
        exit(-1);
    }
    else printf("Context created successfuly\n");

    // READ KERNEL FILE
    char* kernel_file = Get_Kernel_File("Kernels.cl");
    if (kernel_file == NULL)
    {
        printf("Could not find the OpenCL kernel file.\n");
        clReleaseContext(context);
        free(devices);
        free(platforms);
        free(input_array_a);
        free(input_array_b);
        free(result_array_c);
        exit(-1);
    }
    else printf("Kernel file successfully read!\n");

    //CREATE AND BUILD PROGRAM
    cl_program program = clCreateProgramWithSource(context, 1, (const char**)&kernel_file, NULL, &status);
    if (status != CL_SUCCESS)
    {
        printf("Could not create the program.\n");
        clReleaseContext(context);
        free(devices);
        free(platforms);
        free(input_array_a);
        free(input_array_b);
        free(result_array_c);
        exit(-1);
    }

    status = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    if (status != CL_SUCCESS)
    {
        printf("Could not create the program.\n");
        clReleaseProgram(program);
        clReleaseContext(context);
        free(devices);
        free(platforms);
        free(input_array_a);
        free(input_array_b);
        free(result_array_c);
        exit(-1);
    }
    else printf("Program created and built successfully!\n");

    // CREATE KERNEL
    cl_kernel kernel = clCreateKernel(program, "exponentiateArray", &status);
    if (status != CL_SUCCESS)
    {
        printf("Could not create a kernel.\n");
        clReleaseProgram(program);
        clReleaseContext(context);
        free(devices);
        free(platforms);
        free(input_array_a);
        free(input_array_b);
        free(result_array_c);
        exit(-1);
    }
    else printf("Kernel created successfully!\n");

    // CREATE QUEUE
    cl_command_queue commandQ = clCreateCommandQueue(context, devices[device_index], 0, &status);
    if (status != CL_SUCCESS)
    {
        printf("Could not create a command queue.\n");
        clReleaseKernel(kernel);
        clReleaseProgram(program);
        clReleaseContext(context);
        free(devices);
        free(platforms);
        free(input_array_a);
        free(input_array_b);
        free(result_array_c);
        exit(-1);
    }
    else printf("Queue created successfully!\n");

    size_t local_size = 2, global_size = (size_t)ceil(array_size / (float)local_size) * local_size;
    printf("Local size: %zu\nGlobal size: %zu\n", local_size, global_size);
    system("pause");
    system("cls");

    // CREATE BUFFER
    cl_mem clmem_input_array_a = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(int) * (array_size), NULL, &status);
    if (status != CL_SUCCESS)
    {
        printf("Could not create buffer for result array.\n");
        clReleaseCommandQueue(commandQ);
        clReleaseKernel(kernel);
        clReleaseProgram(program);
        clReleaseContext(context);
        free(devices);
        free(platforms);
        free(input_array_a);
        free(input_array_b);
        free(result_array_c);
        exit(-1);
    }
    else printf("Buffer a created successfully!\n");

    cl_mem clmem_input_array_b = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(int) * (array_size), NULL, &status);
    if (status != CL_SUCCESS)
    {
        printf("Could not create buffer for result array.\n");
        clReleaseMemObject(clmem_input_array_a);
        clReleaseCommandQueue(commandQ);
        clReleaseKernel(kernel);
        clReleaseProgram(program);
        clReleaseContext(context);
        free(devices);
        free(platforms);
        free(input_array_a);
        free(input_array_b);
        free(result_array_c);
        exit(-1);
    }
    else printf("Buffer b created successfully!\n");

    cl_mem clmem_result_array_c = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(int) * (array_size), NULL, &status);
    if (status != CL_SUCCESS)
    {
        printf("Could not create buffer for result array.\n");
        clReleaseMemObject(clmem_input_array_a);
        clReleaseMemObject(clmem_input_array_b);
        clReleaseCommandQueue(commandQ);
        clReleaseKernel(kernel);
        clReleaseProgram(program);
        clReleaseContext(context);
        free(devices);
        free(platforms);
        free(input_array_a);
        free(input_array_b);
        free(result_array_c);
        exit(-1);
    }
    else printf("Buffer c created successfully!\n");

    // WRITE BUFFER TO DEVICE MEMORY
    status = clEnqueueWriteBuffer(commandQ, clmem_input_array_a, CL_TRUE, 0,(array_size) * sizeof(int), input_array_a+start, 0, NULL, NULL);
    if (status != CL_SUCCESS)
        printf("Could not write input array a buffer to the device.\n");
    else printf("Buffer a written successfully!\n");

    status = clEnqueueWriteBuffer(commandQ, clmem_input_array_b, CL_TRUE, 0, (array_size) * sizeof(int), input_array_b+start, 0, NULL, NULL);
    if (status != CL_SUCCESS)
        printf("Could not write input array b buffer to the device.\n");
    else printf("Buffer a written successfully!\n");

    status = clEnqueueWriteBuffer(commandQ, clmem_result_array_c, CL_TRUE, 0, (array_size) * sizeof(int), result_array_c+start, 0, NULL, NULL);
    if (status != CL_SUCCESS)
        printf("Could not write result array buffer to the device.\n");
    else printf("Buffer a written successfully!\n");

    // SET KERNEL ARGUMENTS
    status = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void*)&clmem_input_array_a);
    if (status != CL_SUCCESS)
        printf("Could not set kernel argument 0.\n");
    else printf("Kernel argument 0 set succesfully!\n");

    status = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void*)&clmem_input_array_b);
    if (status != CL_SUCCESS)
        printf("Could not set kernel argument 1.\n");
    else printf("Kernel argument 1 set succesfully!\n");

    status = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void*)&clmem_result_array_c);
    if (status != CL_SUCCESS)
        printf("Could not set kernel argument 2.\n");
    else printf("Kernel argument 2 set succesfully!\n");

    status = clSetKernelArg(kernel, 3, sizeof(int), (void*)&array_size);
    if (status != CL_SUCCESS)
        printf("Could not set kernel argument 3.\n");
    else printf("Kernel argument 3 set succesfully!\n");

    system("pause");
    system("cls");

    // EXECUTE TASK
    status = clEnqueueNDRangeKernel(commandQ, kernel, 1, NULL, &global_size, &local_size, 0, NULL, NULL);
    if (status != CL_SUCCESS)
        printf("Could not enqeue task to the kernel.\n");
    else printf("Task enqueued succesfully!\n");

    // FINISH TASK
    status = clFinish(commandQ);
    if (status != CL_SUCCESS)
        printf("Could not finish executing the task.\n");
    else printf("Task finished succesfully!\n");

    // READ FROM DEVICE
    status = clEnqueueReadBuffer(commandQ, clmem_result_array_c, CL_TRUE, 0, (array_size) * sizeof(int), result_array_c+start, 0, NULL, NULL);
    if (status != CL_SUCCESS)
        printf("Could not read from buffer.\n");
    else printf("Read to result array c succesfully!\n");

     //DISPLAY RESULTS
    for (int i = start; i < array_size; i++)
    {
        if (input_array_a && input_array_b && result_array_c)
            printf("%02d ^ %02d = %d\n", input_array_a[i], input_array_b[i], result_array_c[i]);
    }

    clReleaseMemObject(clmem_input_array_a);
    clReleaseMemObject(clmem_input_array_b);
    clReleaseMemObject(clmem_result_array_c);
    clReleaseCommandQueue(commandQ);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseContext(context);
    free(devices);
    free(platforms);
}

/*
* Requirements: Array size percentage (0% - 100%), FIRST_HALF(1) or SECOND_HALF(2) or FULL_ARRAY(3), input array 1, input array 2, result array
* Usage: Execute_On_Cpu(int 0-100, int 1-3)
* ------------------------------------------
* Description: Calculates the total number of elements to be executed in the cpu/gpu (array_size_percentage)
and then executes the elements based on the processing_part (first half, second half, or the full array).
*/
void Execute_On_Gpu(unsigned int array_size_percentage, int processing_part, int* input_array_a, int* input_array_b, int* result_array_c)
{
    printf("EXECUTING ON GPU\n");
    printf("Arrays:\n");
    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        if (input_array_a && input_array_b && result_array_c)
            printf("a[%04d] = %04d\tb[%04d] = %04d\tc[%04d] = %04d\n", i, input_array_a[i], i, input_array_b[i], i, result_array_c[i]);
    }
    system("pause");
    system("cls");

    // Check validity of parameters
    if (!PercentageSize_ProcessingPart_Valid(array_size_percentage, processing_part)) return;
    if (!FullPercentageSize_Equal_FullProcessingPart(&array_size_percentage, &processing_part)) return;

    int array_size = (ceil)((C_ARRAY_SIZE * array_size_percentage) / 100.0); // Calculate number of elements to process based on percentage provided
    printf("Array size to calc is %d\n ", array_size);
    int start; int end;
    switch (processing_part)
    {

    case FIRST_HALF_OF_ARRAY:
        start = 0;
        end = array_size - 1;
        break;
    case SECOND_HALF_OF_ARRAY:
        start = C_ARRAY_SIZE - array_size;
        end = C_ARRAY_SIZE - 1;
        array_size = C_ARRAY_SIZE - start;
        break;

    case FULL_ARRAY:
        start = 0;
        end = C_ARRAY_SIZE - 1;
        break;
    }
    printf("Starts at %d and ends at %d", start, end);


    // PLATFORMS
    cl_platform_id* platforms = Get_Platforms();
    cl_uint platform_index = 0; // The appropriate platform for CPU must be detected accordingly.
    Display_All_Platform_Info(platforms);

    // DEVICES
    cl_device_id* devices = Get_Devices(platforms, platform_index);
    /*
    * If there is no integrated gpu, then there will only be 1 cpu device and hence, device_index will be 0.
    * If there are n integrated gpus, total devices will be (n+1) devices. Hence, cpu will be at index (n-1)
    */
    cl_uint device_index = Get_Num_Devices(platforms, platform_index) == 1 ? 0 : Get_Num_Devices(platforms, platform_index) - 1;
    Display_All_Device_Info(devices, platforms, platform_index);

    // CREATE CONTEXT
    cl_int status = 0;
    cl_context context = clCreateContext(0, Get_Num_Devices(platforms, platform_index), devices, NULL, NULL, &status);
    if (status != CL_SUCCESS)
    {
        printf("Could not create a context.\n");
        free(devices);
        free(platforms);
        exit(-1);
    }
    else printf("Context created successfuly\n");

    // READ KERNEL FILE
    char* kernel_file = Get_Kernel_File("Kernels.cl");
    if (kernel_file == NULL)
    {
        printf("Could not find the OpenCL kernel file.\n");
        clReleaseContext(context);
        free(devices);
        free(platforms);
        free(input_array_a);
        free(input_array_b);
        free(result_array_c);
        exit(-1);
    }
    else printf("Kernel file successfully read!\n");

    //CREATE AND BUILD PROGRAM
    cl_program program = clCreateProgramWithSource(context, 1, (const char**)&kernel_file, NULL, &status);
    if (status != CL_SUCCESS)
    {
        printf("Could not create the program.\n");
        clReleaseContext(context);
        free(devices);
        free(platforms);
        free(input_array_a);
        free(input_array_b);
        free(result_array_c);
        exit(-1);
    }

    status = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    if (status != CL_SUCCESS)
    {
        printf("Could not create the program.\n");
        clReleaseProgram(program);
        clReleaseContext(context);
        free(devices);
        free(platforms);
        free(input_array_a);
        free(input_array_b);
        free(result_array_c);
        exit(-1);
    }
    else printf("Program created and built successfully!\n");

    // CREATE KERNEL
    cl_kernel kernel = clCreateKernel(program, "exponentiateArray", &status);
    if (status != CL_SUCCESS)
    {
        printf("Could not create a kernel.\n");
        clReleaseProgram(program);
        clReleaseContext(context);
        free(devices);
        free(platforms);
        free(input_array_a);
        free(input_array_b);
        free(result_array_c);
        exit(-1);
    }
    else printf("Kernel created successfully!\n");

    // CREATE QUEUE
    cl_command_queue commandQ = clCreateCommandQueue(context, devices[device_index], 0, &status);
    if (status != CL_SUCCESS)
    {
        printf("Could not create a command queue.\n");
        clReleaseKernel(kernel);
        clReleaseProgram(program);
        clReleaseContext(context);
        free(devices);
        free(platforms);
        free(input_array_a);
        free(input_array_b);
        free(result_array_c);
        exit(-1);
    }
    else printf("Queue created successfully!\n");

    size_t local_size = 2, global_size = (size_t)ceil(array_size / (float)local_size) * local_size;
    printf("Local size: %zu\nGlobal size: %zu\n", local_size, global_size);
    system("pause");
    system("cls");

    // CREATE BUFFER
    cl_mem clmem_input_array_a = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(int) * (array_size), NULL, &status);
    if (status != CL_SUCCESS)
    {
        printf("Could not create buffer for result array.\n");
        clReleaseCommandQueue(commandQ);
        clReleaseKernel(kernel);
        clReleaseProgram(program);
        clReleaseContext(context);
        free(devices);
        free(platforms);
        free(input_array_a);
        free(input_array_b);
        free(result_array_c);
        exit(-1);
    }
    else printf("Buffer a created successfully!\n");

    cl_mem clmem_input_array_b = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(int) * (array_size), NULL, &status);
    if (status != CL_SUCCESS)
    {
        printf("Could not create buffer for result array.\n");
        clReleaseMemObject(clmem_input_array_a);
        clReleaseCommandQueue(commandQ);
        clReleaseKernel(kernel);
        clReleaseProgram(program);
        clReleaseContext(context);
        free(devices);
        free(platforms);
        free(input_array_a);
        free(input_array_b);
        free(result_array_c);
        exit(-1);
    }
    else printf("Buffer b created successfully!\n");

    cl_mem clmem_result_array_c = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(int) * (array_size), NULL, &status);
    if (status != CL_SUCCESS)
    {
        printf("Could not create buffer for result array.\n");
        clReleaseMemObject(clmem_input_array_a);
        clReleaseMemObject(clmem_input_array_b);
        clReleaseCommandQueue(commandQ);
        clReleaseKernel(kernel);
        clReleaseProgram(program);
        clReleaseContext(context);
        free(devices);
        free(platforms);
        free(input_array_a);
        free(input_array_b);
        free(result_array_c);
        exit(-1);
    }
    else printf("Buffer c created successfully!\n");

    // WRITE BUFFER TO DEVICE MEMORY
    status = clEnqueueWriteBuffer(commandQ, clmem_input_array_a, CL_TRUE, 0, (array_size) * sizeof(int), input_array_a + start, 0, NULL, NULL);
    if (status != CL_SUCCESS)
        printf("Could not write input array a buffer to the device.\n");
    else printf("Buffer a written successfully!\n");

    status = clEnqueueWriteBuffer(commandQ, clmem_input_array_b, CL_TRUE, 0, (array_size) * sizeof(int), input_array_b + start, 0, NULL, NULL);
    if (status != CL_SUCCESS)
        printf("Could not write input array b buffer to the device.\n");
    else printf("Buffer a written successfully!\n");

    status = clEnqueueWriteBuffer(commandQ, clmem_result_array_c, CL_TRUE, 0, (array_size) * sizeof(int), result_array_c + start, 0, NULL, NULL);
    if (status != CL_SUCCESS)
        printf("Could not write result array buffer to the device.\n");
    else printf("Buffer a written successfully!\n");

    // SET KERNEL ARGUMENTS
    status = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void*)&clmem_input_array_a);
    if (status != CL_SUCCESS)
        printf("Could not set kernel argument 0.\n");
    else printf("Kernel argument 0 set succesfully!\n");

    status = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void*)&clmem_input_array_b);
    if (status != CL_SUCCESS)
        printf("Could not set kernel argument 1.\n");
    else printf("Kernel argument 1 set succesfully!\n");

    status = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void*)&clmem_result_array_c);
    if (status != CL_SUCCESS)
        printf("Could not set kernel argument 2.\n");
    else printf("Kernel argument 2 set succesfully!\n");

    status = clSetKernelArg(kernel, 3, sizeof(int), (void*)&array_size);
    if (status != CL_SUCCESS)
        printf("Could not set kernel argument 3.\n");
    else printf("Kernel argument 3 set succesfully!\n");

    system("pause");
    system("cls");

    // EXECUTE TASK
    status = clEnqueueNDRangeKernel(commandQ, kernel, 1, NULL, &global_size, &local_size, 0, NULL, NULL);
    if (status != CL_SUCCESS)
        printf("Could not enqeue task to the kernel.\n");
    else printf("Task enqueued succesfully!\n");

    // FINISH TASK
    status = clFinish(commandQ);
    if (status != CL_SUCCESS)
        printf("Could not finish executing the task.\n");
    else printf("Task finished succesfully!\n");

    // READ FROM DEVICE
    status = clEnqueueReadBuffer(commandQ, clmem_result_array_c, CL_TRUE, 0, (array_size) * sizeof(int), result_array_c + start, 0, NULL, NULL);
    if (status != CL_SUCCESS)
        printf("Could not read from buffer.\n");
    else printf("Read to result array c succesfully!\n");

    //DISPLAY RESULTS
    for (int i = start; i < array_size; i++)
    {
        if (input_array_a && input_array_b && result_array_c)
            printf("%02d ^ %02d = %d\n", input_array_a[i], input_array_b[i], result_array_c[i]);
    }

    clReleaseMemObject(clmem_input_array_a);
    clReleaseMemObject(clmem_input_array_b);
    clReleaseMemObject(clmem_result_array_c);
    clReleaseCommandQueue(commandQ);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseContext(context);
    free(devices);
    free(platforms);
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
* Requirements: cl_platform_id *platforms;
* Usage: Display_Platform_Info(platforms);
* ----------------------------------------
* Description: Prints all detected platform's information
*/
void Display_All_Platform_Info(const cl_platform_id* platforms)
{
    cl_uint num_platforms = Get_Num_Platforms();

    const char* attributes_names[5] = { "Name", "Vendor", "Version", "Profile", "Extensions" };
    const cl_platform_info attributes_types[5] = { CL_PLATFORM_NAME, CL_PLATFORM_VENDOR, CL_PLATFORM_VERSION, CL_PLATFORM_PROFILE, CL_PLATFORM_EXTENSIONS };

    size_t info_size = 0;
    char* info;

    for (cl_uint i = 0; i < num_platforms; i++)
    {
        printf("\n %d. Platform \n", i + 1);

        for (int j = 0; j < 5; j++)
        {
            clGetPlatformInfo(platforms[i], attributes_types[j], 0, NULL, &info_size);
            info = (char*)malloc(info_size);
            clGetPlatformInfo(platforms[i], attributes_types[j], info_size, info, NULL);

            printf("%d.%d %-11s: %s\n", i + 1, j + 1, attributes_names[j], info);

            free(info);
        }
        printf("\n");
    }
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
* Requirements: cl_platform_id *platforms, cl_uint platform_index
* Usage: cl_uint num_devices = Get_Num_Devices(platforms, platform_index);
* ------------------------------------------------------------------------------
* Description: Returns total number of devices detected in a platform.
*/
cl_uint Get_Num_Devices(const cl_platform_id* platforms, const cl_uint platform_index)
{
    cl_int status;
    cl_uint num_devices;
    status = clGetDeviceIDs(platforms[platform_index], CL_DEVICE_TYPE_ALL, 0, NULL, &num_devices);
    if (status != CL_SUCCESS)
    {
        printf("Could not find any devices.\n");
        free((void*)platforms);
        exit(-1);
    }
    return num_devices;
}

/*
* Requirements: cl_platform_id *platforms, cl_uint platform_index
* Usage: cl_device_id devices = Get_Devices(platforms, platform_index)
* ---------------------------------------------------------------------
* Description: Returns an array of devices detected under the `platforms` array at index `platform_index`.
*/
cl_device_id* Get_Devices(cl_platform_id* platforms, const cl_uint platform_index)
{
    if (platform_index >= Get_Num_Platforms())
    {
        printf("Invalid Index.\n");
        cl_device_id* devices = (cl_device_id*)malloc(sizeof(cl_device_id)); // return device with no value, as it will be freed in the caller function
        return devices;
    }

    cl_uint num_devices = Get_Num_Devices(platforms, platform_index);

    cl_device_id* devices = (cl_device_id*)malloc(sizeof(cl_device_id) * C_ARRAY_SIZE);
    clGetDeviceIDs(platforms[platform_index], CL_DEVICE_TYPE_ALL, num_devices, devices, NULL);
    return devices;
}

/*
* Requirements: cl_device_id *devices, cl_platform_id *platforms, cl_uint platform_index
* Usage: Display_All_Device_Info(devices, platforms, platform_index);
* ---------------------------------------------------------------------------------------
* Description: Displays all device information under the `platforms` array.
*/
void Display_All_Device_Info(const cl_device_id* devices, const cl_platform_id* platforms, const cl_uint platform_index)
{
    if (platform_index >= Get_Num_Platforms())
    {
        printf("Invalid Index.\n");
        return;
    }

    cl_uint num_devices = Get_Num_Devices(platforms, platform_index);
    clGetDeviceIDs(platforms[platform_index], CL_DEVICE_TYPE_ALL, 0, NULL, &num_devices);

    printf("Number of availble devices in this platform: %d \n", num_devices);

    size_t info_size = 0;
    char* info;

    cl_uint number_of_compute_units;
    for (cl_uint i = 0; i < num_devices; ++i)
    {
        // display device name 
        clGetDeviceInfo(devices[i], CL_DEVICE_NAME, 0, NULL, &info_size);
        info = (char*)malloc(info_size);
        clGetDeviceInfo(devices[i], CL_DEVICE_NAME, info_size, info, NULL);
        printf("%d. Device: %s\n", i + 1, info);
        free(info);

        // display hardware device version 
        clGetDeviceInfo(devices[i], CL_DEVICE_VERSION, 0, NULL, &info_size);
        info = (char*)malloc(info_size);
        clGetDeviceInfo(devices[i], CL_DEVICE_VERSION, info_size, info, NULL);
        printf("   %d.%d Hardware version      : %s\n", i + 1, 1, info);
        free(info);

        // display software driver version 
        clGetDeviceInfo(devices[i], CL_DRIVER_VERSION, 0, NULL, &info_size);
        info = (char*)malloc(info_size);
        clGetDeviceInfo(devices[i], CL_DRIVER_VERSION, info_size, info, NULL);
        printf("   %d.%d Software version      : %s\n", i + 1, 2, info);
        free(info);

        // print c version supported by compiler for device 
        clGetDeviceInfo(devices[i], CL_DEVICE_OPENCL_C_VERSION, 0, NULL, &info_size);
        info = (char*)malloc(info_size);
        clGetDeviceInfo(devices[i], CL_DEVICE_OPENCL_C_VERSION, info_size, info, NULL);
        printf("   %d.%d Software version      : %s\n", i + 1, 2, info);
        free(info);

        // display parallel compute units 
        clGetDeviceInfo(devices[i], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(number_of_compute_units), &number_of_compute_units, NULL);
        printf("   %d.%d Parallel compute units: %d\n", i + 1, 4, number_of_compute_units);
        printf("\n");
    }
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
