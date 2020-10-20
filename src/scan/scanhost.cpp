#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <fstream>
#include <vector>

#include "../common/utility.h"
#include "timer.h"
using namespace std;

template <class T>
void runTest(const string& testName, cl_device_id dev, cl_context ctx,
        cl_command_queue queue, BenchmarkDatabase& resultDB, BenchmarkOptions& op,
        const string& compileFlags);

inline size_t
getMaxWorkGroupSize (cl_context &ctx, cl_kernel &ker)
{
    int err;
    // Find the maximum work group size
    size_t retSize = 0;
    size_t maxGroupSize = 0;
    // we must find the device asociated with this context first
    cl_device_id devid;   // we create contexts with a single device only
    err = clGetContextInfo (ctx, CL_CONTEXT_DEVICES, sizeof(devid), &devid, &retSize);
    CL_CHECK_ERROR(err);
    if (retSize < sizeof(devid))  // we did not get any device, pass 0 to the function
       devid = 0;
    err = clGetKernelWorkGroupInfo (ker, devid, CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t),
                &maxGroupSize, &retSize);
    CL_CHECK_ERROR(err);
    return (maxGroupSize);
}


// ****************************************************************************
// Function: scanCPU
//
// Purpose:
//   Simple cpu scan routine to verify device results
//
// Arguments:
//   data : the input data
//   reference : space for the cpu solution
//   dev_result : result from the device
//   size :
//
// Returns:  nothing, prints relevant info to stdout
//
// Programmer: Kyle Spafford
// Creation: August 13, 2009
//
// Modifications:
//
// ****************************************************************************
template <class T>
bool scanCPU(T *data, T* reference, T* dev_result, const size_t size)
{

    bool passed = true;
    T last = 0.0f;

    for (unsigned int i = 0; i < size; ++i)
    {
        reference[i] = data[i] + last;
        last = reference[i];
    }
    for (unsigned int i = 0; i < size; ++i)
    {
        if (reference[i] != dev_result[i])
        {
#ifdef VERBOSE_OUTPUT
            cout << "Mismatch at i: " << i << " ref: " << reference[i]
                 << " dev: " << dev_result[i] << endl;
#endif
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
//   Executes the scan (parallel prefix sum) benchmark
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
extern const char *cl_source_scan;

void
benchmarkScan(cl_device_id dev,
                    cl_context ctx,
                    cl_command_queue queue,
                    BenchmarkDatabase &resultDB,
                    BenchmarkOptions  &op)
{
    // Always run single precision test
    // OpenCL doesn't support templated kernels, so we have to use macros
    string spMacros = "-DSINGLE_PRECISION";
    runTest<float>("scan", dev, ctx, queue, resultDB, op, spMacros);

    // If double precision is supported, run the DP test
/*
    if (checkExtension(dev, "cl_khr_fp64"))
    {
        cout << "DP Supported\n";
        string dpMacros = "-DK_DOUBLE_PRECISION ";
        runTest<double>
        ("Scan-DP", dev, ctx, queue, resultDB, op, dpMacros);
    }
*/
}

template <class T>
void runTest(const string& testName, cl_device_id dev, cl_context ctx,
        cl_command_queue queue, BenchmarkDatabase& resultDB, 
        BenchmarkOptions& options, const string& compileFlags)
{
    auto iter = options.appsToRun.find(scan);

    if (iter == options.appsToRun.end())
    {   
        cerr << "ERROR: Could not find benchmark options";
        return; 
    }

    ApplicationOptions appOptions = iter->second;

    int err = 0;

	cl_program prog =  createProgramFromBitstream(ctx, appOptions.bitstreamFile, dev);

    // Extract out the kernel
    cl_kernel scan = clCreateKernel(prog, "scan", &err);
    CL_CHECK_ERROR(err);


    // Problem Sizes
    int probSizes[4] = { 1, 8, 32, 64 };
    int size = probSizes[appOptions.size-1];

    // Convert to MB
    size = (size * 1024 * 1024) / sizeof(T);

    // Create input data on CPU
    unsigned int bytes = size * sizeof(T);
    T* reference = new T[size];

    // Allocate pinned host memory for input data (h_idata)
    cl_mem h_i = clCreateBuffer(ctx, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR,
            bytes, NULL, &err);
    CL_CHECK_ERROR(err);
    T* h_idata = (T*)clEnqueueMapBuffer(queue, h_i, true,
            CL_MAP_READ|CL_MAP_WRITE, 0, bytes, 0, NULL, NULL, &err);
    CL_CHECK_ERROR(err);

    // Allocate pinned host memory for output data (h_odata)
    cl_mem h_o = clCreateBuffer(ctx, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR,
            bytes, NULL, &err);
    CL_CHECK_ERROR(err);
    T* h_odata = (T*)clEnqueueMapBuffer(queue, h_o, true,
            CL_MAP_READ|CL_MAP_WRITE, 0, bytes, 0, NULL, NULL, &err);
    CL_CHECK_ERROR(err);

    // Initialize host memory
    cout << "Initializing host memory." << endl;
    for (int i = 0; i < size; i++)
    {
        h_idata[i] = i % 3; //Fill with some pattern
        h_odata[i] = -1;
    }

    // Allocate device memory for input array
    cl_mem d_idata = clCreateBuffer(ctx, CL_MEM_READ_WRITE, bytes, NULL, &err);
    CL_CHECK_ERROR(err);

    // Allocate device memory for output array
    cl_mem d_odata = clCreateBuffer(ctx, CL_MEM_READ_WRITE, bytes, NULL, &err);
    CL_CHECK_ERROR(err);

    // SINGLE WORK ITEM KERNELS
    // Number of local work items per group
    const size_t local_wsize  = 1;
    // Number of global work items
    const size_t global_wsize = 1;

    // Set the kernel arguments
    err = clSetKernelArg(scan, 0, sizeof(cl_mem), (void*)&d_idata);
    CL_CHECK_ERROR(err);
    err = clSetKernelArg(scan, 1, sizeof(cl_mem), (void*)&d_odata);
    CL_CHECK_ERROR(err);
    err = clSetKernelArg(scan, 2, sizeof(cl_int), (void*)&size);
    CL_CHECK_ERROR(err);

    // Copy data to GPU
    cout << "Copying input data to device." << endl;
    cl_event evTransfer = NULL;

    err = clEnqueueWriteBuffer(queue, d_idata, true, 0, bytes, h_idata, 0,
            NULL, &evTransfer);
    CL_CHECK_ERROR(err);

    err = clFinish(queue);
    CL_CHECK_ERROR(err);

//    evTransfer.FillTimingInfo();
	cl_ulong    submitTime;
	cl_ulong    endTime;
		
	err = clGetEventProfilingInfo(evTransfer, CL_PROFILING_COMMAND_SUBMIT,
                                     sizeof(cl_ulong), &submitTime, NULL);
	CL_CHECK_ERROR(err);
	       
	err = clGetEventProfilingInfo(evTransfer, CL_PROFILING_COMMAND_END,
                                     sizeof(cl_ulong), &endTime, NULL);
	CL_CHECK_ERROR(err);


    double inTransferTime = endTime - submitTime;

    // Repeat the test multiplie times to get a good measurement
    int passes = appOptions.passes;
    int iters  = appOptions.iterations;

    cout << "Running benchmark with size " << size << endl;
    for (int k = 0; k < passes; k++)
    {
        int th = Timer::Start();
        for (int j = 0; j < iters; j++)
        {

            err = clEnqueueNDRangeKernel(queue, scan, 1, NULL,
                        &global_wsize, &local_wsize, 0, NULL, NULL);
            CL_CHECK_ERROR(err);

        }
        err = clFinish(queue);
        CL_CHECK_ERROR(err);
        double totalScanTime = Timer::Stop(th, "total scan time");

        err = clEnqueueReadBuffer(queue, d_odata, true, 0, bytes, h_odata,
                0, NULL, &evTransfer);
        CL_CHECK_ERROR(err);

        err = clFinish(queue);
        CL_CHECK_ERROR(err);

//        evTransfer.FillTimingInfo();
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
        if (! scanCPU(h_idata, reference, h_odata, size))
        {
            return;
        }

        char atts[1024];
        double avgTime = totalScanTime / (double) iters;
        double gbs = (double) (size * sizeof(T)) / (1000. * 1000. * 1000.);
        sprintf(atts, "%ditems", size);
        resultDB.AddResult("scan", testName, atts, "GB/s", gbs / (avgTime));
    }

    // Clean up device memory
    err = clReleaseMemObject(d_idata);
    CL_CHECK_ERROR(err);
    err = clReleaseMemObject(d_odata);
    CL_CHECK_ERROR(err);

    // Clean up other host memory
    delete[] reference;

    err = clReleaseProgram(prog);
    CL_CHECK_ERROR(err);
    err = clReleaseKernel(scan);
    CL_CHECK_ERROR(err);
}

