__kernel void kernel_exponentiation(__global unsigned long int *a, __global unsigned long int *b, __global unsigned long int *c, const int array_size)
{
	int index = get_global_id(0);
	if (index < array_size)
	{
		printf("%lu\n", a[index]);
	}
}