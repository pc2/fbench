/** @file mergesorthost.cpp */

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

#include "../common/utility.h"
#include "timer.h"

using namespace std;

template <class T>
void runTest(const string& testName, cl_device_id dev, cl_context ctx,
        cl_command_queue queue, BenchmarkDatabase& resultDB, BenchmarkOptions& op,
        const string& compileFlags);


// ****************************************************************************
// Function: mergesortCPU
//
// Purpose:
//   Simple cpu mergesort routine to verify device results
//
// Arguments:
//   data : the input data
//   reference : space for the cpu solution
//   dev_result : result from the device
//   size : size of the output array
//
// Returns:  nothing, prints relevant info to stdout
//
// Programmer: Kyle Spafford
// Creation: August 13, 2009
//
// Modifier: Farjana Jalil
// Date: August 27, 2020
//
// ****************************************************************************

template <class T>
bool mergesortCPU(T* reference, T* dev_result, const size_t size)
{
	bool passed = true;
	
	sort(reference, reference+size);
	
	for(int i = 0; i < size; i++)
	{
		if(reference[i] != dev_result[i])
		{
			passed = false;
		}
	}
	
	cout << "Test ";
    if (passed)
        cout << "Passed" << endl;
	
    else
        cout << "Failed" << endl;
	
    return passed;
}

// ****************************************************************************
// Function: RunBenchmark
//
// Purpose:
//   Executes the merge sort benchmark
//
// Arguments:
//   dev: the opencl device id to use for the benchmark
//   ctx: the opencl context to use for the benchmark
//   queue: the opencl command queue to issue commands to
//   resultDB: results from the benchmark are stored in this db
//   op: the options parser / parameter database
//
// Returns:  nothing
//
// Programmer: Kyle Spafford
// Creation: August 13, 2009
//
// Modifications:
//   Kyle Spafford, Wed Jun 8 15:20:22 EDT 2011
//   Updating to use non-recursive algorithm
//   Jeremy Meredith, Thu Sep 24 17:30:18 EDT 2009
//   Use implicit include of source file instead of runtime loading.
//
// ****************************************************************************
extern const char *cl_source_mergesort;

void benchmarkMergeSort(cl_device_id dev,
						cl_context ctx,
						cl_command_queue queue,
						BenchmarkDatabase &resultDB,
						BenchmarkOptions  &op)
{
    // Always run single precision test
    // OpenCL doesn't support templated kernels, so we have to use macros
    string spMacros = "-DSINGLE_PRECISION";
	
    runTest<int>("mergesort", dev, ctx, queue, resultDB, op, spMacros);
}

template <class T>
void runTest(const string& testName, cl_device_id dev, cl_context ctx,
			cl_command_queue queue, BenchmarkDatabase& resultDB, 
			BenchmarkOptions& options, const string& compileFlags)
{
    auto iter = options.appsToRun.find(mergesort);

    if (iter == options.appsToRun.end())
    {   
        cerr << "ERROR: Could not find benchmark options";
        return; 
    }

    ApplicationOptions appOptions = iter->second;

    int err = 0;

	cl_program prog =  createProgramFromBitstream(ctx, appOptions.bitstreamFile, dev);

    // Find the kernel
    cl_kernel mergesort = clCreateKernel(prog, "mergesort", &err);
    CL_CHECK_ERROR(err);
	
	int size = 64;
	
	//Create input data on CPU
    unsigned int bytes = size * sizeof(T);
	
	T* reference = new T[size];
	
	// Allocate pinned host memory for input data (buffer_in_data)
    cl_mem buffer_in = clCreateBuffer(ctx, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR,
					bytes, NULL, &err);
    CL_CHECK_ERROR(err);
	
    T* buffer_in_data = (T*)clEnqueueMapBuffer(queue, buffer_in, true,
						CL_MAP_READ|CL_MAP_WRITE, 0, bytes, 0, NULL, NULL, &err);
    CL_CHECK_ERROR(err);

    // Allocate pinned host memory for output data (buffer_out_data)
    cl_mem buffer_out = clCreateBuffer(ctx, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR,
						bytes, NULL, &err);
    CL_CHECK_ERROR(err);
	
    T* buffer_out_data = (T*)clEnqueueMapBuffer(queue, buffer_out, true,
						CL_MAP_READ|CL_MAP_WRITE, 0, bytes, 0, NULL, NULL, &err);
    CL_CHECK_ERROR(err);
	

	int input_data[size];

	
	//Input data
	for (int i = 0; i < size; i++)
    {
        input_data[i] = rand() % 100;
    }
	

	//Sorting Input data chunk-by-chunk
    for(int i = 0; i < size; i += 8)
    {
        for(int j = i; j < (i + 8); j++)
        {
            for (int x = i; x < (i + 8); x++)
            {
                if(input_data[x] > input_data[x+1])
                {
                    int temp = input_data[x];
                    input_data[x] = input_data[x+1];
                    input_data[x+1] = temp;
                }
            }

        }
    }
	

	// Initialize host memory
    cout << "Initializing host memory." << endl; 
	for(int i = 0; i < size; i++)
	{
		buffer_in_data[i] = input_data[i];
	}

	for (int i = 0; i < size; i++)
    {
        buffer_out_data[i] = -1;
    }
	
	// Allocate device memory for input array
    cl_mem device_in_data = clCreateBuffer(ctx, CL_MEM_READ_WRITE, bytes, NULL, &err);
    CL_CHECK_ERROR(err);

    // Allocate device memory for output array
    cl_mem device_out_data = clCreateBuffer(ctx, CL_MEM_READ_WRITE, bytes, NULL, &err);
    CL_CHECK_ERROR(err);
	
	// Set the kernel arguments
    err = clSetKernelArg(mergesort, 0, sizeof(cl_mem), (void*)&device_in_data);
    CL_CHECK_ERROR(err);
	
    err = clSetKernelArg(mergesort, 1, sizeof(cl_mem), (void*)&device_out_data);
    CL_CHECK_ERROR(err);
	
    err = clSetKernelArg(mergesort, 2, sizeof(cl_int), (void*)&size);
    CL_CHECK_ERROR(err);
	
	// Copy data to GPU
    cout << "Copying input data to device." << endl;
    cl_event evTransfer = NULL;

    err = clEnqueueWriteBuffer(queue, device_in_data, true, 0, bytes, buffer_in_data, 0,
            NULL, &evTransfer);
    CL_CHECK_ERROR(err);
	
	err = clFinish(queue);
    CL_CHECK_ERROR(err);
	
	
	cl_ulong    submitTime;
	cl_ulong    endTime;
		
	err = clGetEventProfilingInfo(evTransfer, CL_PROFILING_COMMAND_SUBMIT,
                                     sizeof(cl_ulong), &submitTime, NULL);
	CL_CHECK_ERROR(err);
	       
	err = clGetEventProfilingInfo(evTransfer, CL_PROFILING_COMMAND_END,
                                     sizeof(cl_ulong), &endTime, NULL);
	CL_CHECK_ERROR(err);


    double inTransferTime = endTime - submitTime;

    // Repeat the test multiple times to get a good measurement
    int passes = appOptions.passes;
    int iters  = appOptions.iterations;

    cout << "Running benchmark with size " << size << endl;

    for (int k = 0; k < passes; k++)
    {
        int th = Timer::Start();
        for (int j = 0; j < iters; j++)
        {
			err = clEnqueueTask(queue, mergesort, 0, NULL, NULL);
			CL_CHECK_ERROR(err);
		}
		
        err = clFinish(queue);
        CL_CHECK_ERROR(err);
        double totalMergeSortTime = Timer::Stop(th, "total mergesort time");
		err = clEnqueueReadBuffer(queue, device_out_data, true, 0, bytes, buffer_out_data,
                0, NULL, &evTransfer);
        CL_CHECK_ERROR(err);
		
		
		err = clFinish(queue);
        CL_CHECK_ERROR(err);
		
		cl_ulong    startTime;
		cl_ulong    endTime;

        err = clGetEventProfilingInfo(evTransfer, CL_PROFILING_COMMAND_START,
                                     sizeof(cl_ulong), &startTime, NULL);
        CL_CHECK_ERROR(err);
	       
		err = clGetEventProfilingInfo(evTransfer, CL_PROFILING_COMMAND_END,
                                     sizeof(cl_ulong), &endTime, NULL);
		CL_CHECK_ERROR(err);

        double totalTransfer = inTransferTime + endTime - startTime;
        totalTransfer /= 1.e9; // Convert to seconds
		
		
	
		// If answer is incorrect, stop test and do not report performance
        if (! mergesortCPU(buffer_in_data, buffer_out_data, size))
        {
			return;
        }
		
		char atts[1024];
        double avgTime = totalMergeSortTime / (double) iters;
        double num_of_elements = size / avgTime;
        sprintf(atts, "%d ", size);
        resultDB.AddResult("mergesort", testName, atts, "elements/s", num_of_elements);
		
	}
	
	// Clean up device memory
    err = clReleaseMemObject(device_in_data);
    CL_CHECK_ERROR(err);
	
    err = clReleaseMemObject(device_out_data);
    CL_CHECK_ERROR(err);

    // Clean up other host memory
    delete[] reference;

    err = clReleaseProgram(prog);
    CL_CHECK_ERROR(err);
	
    err = clReleaseKernel(mergesort);
    CL_CHECK_ERROR(err);
	
}

