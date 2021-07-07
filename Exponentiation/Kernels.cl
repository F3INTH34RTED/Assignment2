int power(int base, unsigned int exp) {
    int i;
    int result = 1;
    for (i = 0; i < exp; i++)
        result =result* base;
    return result;
}

//the q says its an array of ints so we dont need to use unsigned longs 
__kernel void exponentiateArray(__global int* array1, __global int* array2, __global int* resultArray, const unsigned int sizeOfArray) {
    int i = get_global_id(0);
    if (i < sizeOfArray) { 
        resultArray[i] = power(array1[i] , array2[i]);
    }
};
