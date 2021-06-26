# Assignment 2

Requirements:
- Minimum hard disk space: 16MB
- OpenCL Library
- Must include a platform that can detect a CPU.
- Must include a platform that can detect a GPU.
- Must have Ubuntu 20.02 or higher on Virtual Machine to run `PthreadsExponents.c`

Problem Statement:

Create a menu based program that computes c[i] = a[i] * b[i] using
- Windows threads or Pthreads
- An Entire array on CPU using OpenCL
- An Entire array on GPU using OpenCL
- 25% of the array on CPU and 75% of the array on GPU
- 50% of the array on CPU and 50% of the array on GPU
- 75% of the array on CPU and 25% of the array on GPU

Record and graph average execution times.