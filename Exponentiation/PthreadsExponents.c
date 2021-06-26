#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/sysinfo.h>

struct Exponent_Array_Parameters
{
    int *input_array_a;
    int *input_array_b;
    int *result_array_c;
    int start;
    int end;
    int struct_array_size;
};

void *Exponentiation(void *params);
int Power(const unsigned int base, const unsigned int exponent);

int main(void)
{
    int threads_number = 10;
    // printf("Please enter the number of threads: ");
    // scanf("%d", &threads_number);

    int array_size = 25;
    // printf("Please enter the number of elements: ");
    // scanf("%d", &array_size);

    fflush(stdin);

    int input_array_a[array_size], input_array_b[array_size], result_array_c[array_size], counter = 0;
    for (int i = 0; i < array_size; ++i)
    {
        input_array_a[i] = i;
        if (i % 5 == 0) counter = 0;
        input_array_b[i] = counter++;
        result_array_c[i] = 1;
    }

    pthread_t *myThreadArray = malloc(sizeof(pthread_t) * threads_number);

    struct Exponent_Array_Parameters array_of_structs[threads_number];
    for (int i = 0; i < threads_number; i++)
    {
        array_of_structs[i].input_array_a = input_array_a;
        array_of_structs[i].input_array_b = input_array_b;
        array_of_structs[i].result_array_c = result_array_c;
        array_of_structs[i].start = (i * array_size) / threads_number;
        array_of_structs[i].end = ((i + 1) * array_size) / threads_number;
        printf("Thread number: %d\t[%d - %d)\n", (i + 1), array_of_structs[i].start, array_of_structs[i].end);
        array_of_structs[i].struct_array_size = array_size;
    }
    printf("-----------------------------------------------------\n\n");

    printf("Array A:\n");
    for (int i = 0; i < array_size; i++)
        printf("a[%02d] = %02d\n", i, input_array_a[i]);
    printf("------------------\n\n");
    printf("Array B:\n");
    for (int i = 0; i < array_size; i++)
        printf("b[%02d] = %02d\n", i, input_array_b[i]);
    printf("------------------\n\n");
    printf("Array C:\n");
    for (int i = 0; i < array_size; i++)
        printf("c[%02d] = %02d\n", i, result_array_c[i]);
    printf("------------------\n\n");

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < threads_number; i++)
        pthread_create(&myThreadArray[i], NULL, &Exponentiation, (void *)&array_of_structs[i]);

    for (int i = 0; i < threads_number; i++)
        pthread_join(myThreadArray[i], NULL);

    clock_gettime(CLOCK_MONOTONIC, &end);

    printf("Result:\n");
    for (int i = 0; i < array_size; i++)
        printf("%02d ^ %02d = %d\n", input_array_a[i], input_array_b[i], result_array_c[i]);
    printf("------------------\n\n");

    long long elapsed = (end.tv_sec - start.tv_sec) * 1000000000ll + end.tv_nsec - start.tv_nsec;
    //microseconds+seconds
    printf("Execution time = %f seconds\nNumber of cores: %02d\n\n\n", (elapsed / 1000000000.0), get_nprocs());

    return EXIT_SUCCESS;
}

/*
* Function: Exponentiation
* Pre-requisites: void *parameter
* Usage: void *x = Exponentiation(*parameter);
* --------------------------------------------
* Description: Thread function to compute exponentiation of base array with that of an exponent array.
*/
void *Exponentiation(void *params)
{
    struct Exponent_Array_Parameters *info = (struct Exponent_Array_Parameters *)params;

    for (int i = info->start; i < info->end; i++)
    {
        //Swap(&info->input_array_a[i], &info->input_array_a[info->struct_array_size - i - 1]);
        info->result_array_c[i] = Power(info->input_array_a[i], info->input_array_b[i]);
    }
}

/*
* Function: Power
* Pre-requisites: int x, int y
* Usage: int z = Power(x,y);
* --------------------------------------------
* Description: Function to calucate exponent of the base.
*/
int Power(const unsigned int base, const unsigned int exponent)
{
    int result = 1;
    if (base == 0) return 0;
    for (int i = 0; i < exponent; ++i)
        result *= base;
    return result;
}
