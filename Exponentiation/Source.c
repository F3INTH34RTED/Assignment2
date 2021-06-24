#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#ifdef __APPLE__
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#define __CRT__SECURE__NO__WARNINGS
#endif // __APPLE__

#define ARRAY_SIZE 1024
const unsigned int C_ARRAY_SIZE = ARRAY_SIZE;

enum { bool_false, bool_true };

enum { FIRST_ARRAY_PART = 1, SECOND_ARRAY_PART, FULL_ARRAY_PART };

char *Get_Kernel_File(char *file_name);
void Execute_On_Cpu(unsigned int array_size_percentage, int processing_part);
void Execute_On_Gpu(unsigned int array_size_percentage, int processing_part);
int Array_Part_Percentage_Valid(unsigned int array_size_percentage, int processing_part);
int Array_Part_Percentage_Matches(unsigned int *array_size_percentage, int *processing_part);

int main()
{
    Execute_On_Cpu(100, FIRST_ARRAY_PART);
	system("pause");
	return 0;
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

/*
* Usage: int boolVariable = Array_Part_Percentage_Valid(int 0-100, int 1-3)
* ---------------------------------------------------------------------------
* Description: Checks to see whether the array percentage and processing part is within the range 0 - 100 or 1 -3 respectively.
Returns false if condition fails and true otherwise.
*/
int Array_Part_Percentage_Valid(unsigned int array_size_percentage, int processing_part)
{
    if (array_size_percentage > 100 || array_size_percentage < 0)
    {
        printf("Error: Array size percentage not within the range 0 - 100.\n");
        return bool_false;
    }

    if (processing_part > FULL_ARRAY_PART || processing_part < FIRST_ARRAY_PART)
    {
        printf("Error: Processing part not within the range 1 - 3.\n");
        return bool_false;
    }

    return bool_true;
}

/*
* Usage: int boolVariable = Array_Part_Percentage_Matches(int 0-100, int 1-3)
* -----------------------------------------------------------------------------
* Description: Checks to see whether array percentage and processing part is 100 and full array part respectively. Returns false if condition fails and true
otherwise.
*/
int Array_Part_Percentage_Matches(unsigned int *array_size_percentage, int *processing_part)
{
    if (*array_size_percentage != 100 && *processing_part == FULL_ARRAY_PART)
    {
        printf("Error: Array size percentage is not 100, cannot process full array.\n");
        printf("Would you like to continue processing the whole array (Y/N)?\n");
        char choice = scanf("%c", &choice);
        if (choice == 'n' || choice == 'N') return bool_false;
        *array_size_percentage = 100;
    }

    if (*array_size_percentage == 100 && *processing_part != FULL_ARRAY_PART)
    {
        printf("Error: Processing part of the array chosen is not full, cannot process full array.\n");
        printf("Would you like to continue processing the whole array (Y/N)?\n");
        char choice = scanf("%c", &choice);
        if (choice == 'n' || choice == 'N') return bool_false;
        *processing_part = FULL_ARRAY_PART;
    }
    return bool_true;
}

/*
* Usage: Execute_On_Cpu(int 0-100, int 1-3)
* ------------------------------------------
* Description: Executes first, second, or full part based on the percentage of elements of an array that is to be executed on the cpu.
*/
void Execute_On_Cpu(unsigned int array_size_percentage, int processing_part)
{
    if (!Array_Part_Percentage_Valid(array_size_percentage, processing_part)) return;
    if (!Array_Part_Percentage_Matches(&array_size_percentage, &processing_part)) return;
    printf("%d\n%d\n", array_size_percentage, processing_part);
}

void Execute_On_Gpu(unsigned int array_size_percentage, int processing_part)
{
    if (!Array_Part_Percentage_Valid(array_size_percentage, processing_part)) return;
    if (!Array_Part_Percentage_Matches(&array_size_percentage, &processing_part)) return;
    printf("%d\n%d\n", array_size_percentage, processing_part);
}
