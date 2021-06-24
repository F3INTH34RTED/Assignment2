#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#ifdef __APPLE__
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif // __APPLE__

#define ARRAY_SIZE 1024
const unsigned int C_ARRAY_SIZE = ARRAY_SIZE;
enum { FIRST_ARRAY_PART = 1, SECOND_ARRAY_PART, FULL_ARRAY_PART };

char *Get_Kernel_File(char *file_name);
void Execute_On_Cpu(int array_size_percentage, int processing_part);


int main()
{
    Execute_On_Cpu(50, FIRST_ARRAY_PART);
	system("pause");
	return 0;
}

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

/*
* Usage: Execute_On_Cpu(int 0-100, int 1-3)
* ------------------------------------------
* Description: This method takes the array size in percentage (1 - 100) as the first parameter and the part of array to execute on
first part, second part, or full part of the array. The first part of the array will execute the first array_size_percentage elements of the array, the second
part of the array will execute second array_size_percentage elements of the array, and full part will execute the whole array
*/
void Execute_On_Cpu(int array_size_percentage, int processing_part)
{
    if (array_size_percentage != 100 && processing_part == FULL_ARRAY_PART)
    {
        printf("Array size percentage is not 100, cannot process full array.\n");
        printf("Would you like to continue processing the whole array (Y/N)?\n");
        char choice = '\0';
        scanf("%c", &choice);
        if (choice == 'n' || choice == 'N') return;
        else array_size_percentage = 100;
    }
    if (array_size_percentage == 100 && processing_part != FULL_ARRAY_PART)
    {
        printf("Processing part of the array chosen is not full, cannot process full array.\n");
        printf("Would you like to continue processing the whole array (Y/N)?\n");
        char choice = '\0';
        scanf("%c", &choice);
        if (choice == 'n' || choice == 'N') return;
        else processing_part = FULL_ARRAY_PART;
        return;
    }

    double a_s = array_size_percentage / 100.0;
    printf("%lf\n", a_s);
}