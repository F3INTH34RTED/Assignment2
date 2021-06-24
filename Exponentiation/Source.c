#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#ifdef __APPLE__
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif // __APPLE__

char *Get_Kernel_File(char *file_name);

int main()
{
	system("pause");
	return 0;
}

char *Get_Kernel_File(char *file_name)
{
    FILE *programFile;
    char *programSource;
    size_t program_source_size;
    programFile = fopen(file_name, "rb"); /*  VERY IMPORTANT: "rb" on Windows */
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