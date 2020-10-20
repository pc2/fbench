/** @file firfilterhost.cpp */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "../common/utility.h"
#include "../common/benchmarkoptions.h"

#include "firfilterutility.h"

using namespace std;

double filterSamplesFPGAChanneled(cl_device_id dev,
                             cl_context ctx,
                             cl_command_queue queue,
                             cl_program prog,
                             BenchmarkData &benchmarkData,
                             FLOATING_POINT* results)
{
    FLOATING_POINT* samples = benchmarkData.samples.elements;
    FLOATING_POINT* coefficients = benchmarkData.coefficients.elements;
    int numSamples = benchmarkData.samples.size;
    int numCoefficients = benchmarkData.coefficients.size;
    
    cl_int clErr;
    cl_command_queue queue1 = clCreateCommandQueue( ctx,
                                                    dev,
                                                    CL_QUEUE_PROFILING_ENABLE,
                                                    &clErr );

    cl_command_queue queue2 = clCreateCommandQueue( ctx,
                                                    dev,
                                                    CL_QUEUE_PROFILING_ENABLE,
                                                    &clErr );
    CL_CHECK_ERROR(clErr);    

    cl_command_queue queue3 = clCreateCommandQueue( ctx,
                                                    dev,
                                                    CL_QUEUE_PROFILING_ENABLE,
                                                    &clErr );
    CL_CHECK_ERROR(clErr);    

    int err;

    //
    // find the kernels
    //
    cl_kernel readKernel = clCreateKernel(prog, "convolve_read", &err);
    CL_CHECK_ERROR(err);

    cl_kernel performKernel = clCreateKernel(prog, "convolve_perform", &err);
    CL_CHECK_ERROR(err);
    
    cl_kernel writeKernel = clCreateKernel(prog, "convolve_write", &err);
    CL_CHECK_ERROR(err);

    //
    // allocate device memory for input buffers.
    // 
    cl_mem d_samples = clCreateBuffer(ctx, CL_MEM_READ_ONLY,
                                          sizeof(FLOATING_POINT)*numSamples, NULL, &err);
    CL_CHECK_ERROR(err);

    cl_mem d_coefficients = clCreateBuffer(ctx, CL_MEM_READ_ONLY,
                                          sizeof(FLOATING_POINT)*numCoefficients, NULL, &err);
    CL_CHECK_ERROR(err);

    //
    // allocate device memory for output buffer.
    // 
    cl_mem d_results = clCreateBuffer(ctx, CL_MEM_WRITE_ONLY,
                                          sizeof(FLOATING_POINT)*numSamples, NULL, &err);
    CL_CHECK_ERROR(err);

    //
    // write input buffers to the device memory.
    //
    err = clEnqueueWriteBuffer(queue1, d_samples, true, 0,
                               sizeof(FLOATING_POINT)*numSamples, samples,
                               0, NULL, NULL);
    CL_CHECK_ERROR(err);

    err = clEnqueueWriteBuffer(queue2, d_coefficients, true, 0,
                            sizeof(FLOATING_POINT)*numCoefficients, coefficients,
                            0, NULL, NULL);
    CL_CHECK_ERROR(err);

    err = clFinish(queue1);
    CL_CHECK_ERROR(err);

    err = clFinish(queue2);
    CL_CHECK_ERROR(err);

    //
    // set arguments for the kernel
    //
    err = clSetKernelArg(readKernel, 0, sizeof(cl_mem), (void*)&d_samples);
    CL_CHECK_ERROR(err);
    err = clSetKernelArg(readKernel, 1, sizeof(int), (void*)&numSamples);
    CL_CHECK_ERROR(err);
    
    err = clSetKernelArg(performKernel, 0, sizeof(cl_mem), (void*)&d_coefficients);
    CL_CHECK_ERROR(err);
    err = clSetKernelArg(performKernel, 1, sizeof(int), (void*)&numSamples);
    CL_CHECK_ERROR(err);

    err = clSetKernelArg(writeKernel, 0, sizeof(cl_mem), (void*)&d_results);
    CL_CHECK_ERROR(err);  
    err = clSetKernelArg(writeKernel, 1, sizeof(int), (void*)&numSamples);
    CL_CHECK_ERROR(err);

    //
    // run the kernel
    //
    double nanosec = 0;
    cl_event event1 = NULL;
    err = clEnqueueTask(queue1, readKernel, 0, NULL, &event1);
    CL_CHECK_ERROR(err);    

    cl_event event2 = NULL;
    err = clEnqueueTask(queue2, performKernel, 0, NULL, &event2);
    CL_CHECK_ERROR(err);

    cl_event event3 = NULL;
    err = clEnqueueTask(queue3, writeKernel, 0, NULL, &event3);
    CL_CHECK_ERROR(err);

    err = clFinish(queue1);
    CL_CHECK_ERROR (err);

    err = clFinish(queue2);
    CL_CHECK_ERROR (err);

    err = clFinish(queue3);
    CL_CHECK_ERROR (err);

    //
    // get the timing/rate info
    //
    cl_ulong submitTime;
    cl_ulong endTime;

    err = clGetEventProfilingInfo(event2, CL_PROFILING_COMMAND_SUBMIT,
                                    sizeof(cl_ulong), &submitTime, NULL);
    CL_CHECK_ERROR(err);
    
    err = clGetEventProfilingInfo(event2, CL_PROFILING_COMMAND_END,
                                    sizeof(cl_ulong), &endTime, NULL);
    CL_CHECK_ERROR(err);

    nanosec = endTime - submitTime;
    
    double rate = double(numSamples) / double(nanosec);
    // cout << "rate = " << rate << " GSamples/sec" << endl;

    //
    // read the samples response result
    //
    err = clEnqueueReadBuffer(queue3, d_results, true, 0,
                              sizeof(FLOATING_POINT)*(numSamples), results,
                              0, NULL, NULL);
    CL_CHECK_ERROR(err);

    err = clFinish(queue3);
    CL_CHECK_ERROR(err);

    //
    // free device memory
    //
    err = clReleaseMemObject(d_results);
    CL_CHECK_ERROR(err);

    clReleaseCommandQueue(queue1);
    clReleaseCommandQueue(queue2);
    clReleaseCommandQueue(queue3);

    //
    // return the runtime in seconds
    //
    return nanosec / 1.e9;
}

double filterSamplesFPGANonChanneled(cl_device_id dev,
                             cl_context ctx,
                             cl_command_queue queue,
                             cl_program prog,
                             BenchmarkData &benchmarkData,
                             FLOATING_POINT* results)
{
    FLOATING_POINT* samples = benchmarkData.samples.elements;
    FLOATING_POINT* coefficients = benchmarkData.coefficients.elements;
    int numSamples = benchmarkData.samples.size;
    int numCoefficients = benchmarkData.coefficients.size;
    
    int err;

    //
    // find the kernel
    //
    cl_kernel filterkernel = clCreateKernel(prog, "convolve", &err);
    CL_CHECK_ERROR(err);

    //
    // allocate device memory for input buffers.
    // 
    cl_mem d_samples = clCreateBuffer(ctx, CL_MEM_READ_ONLY,
                                          sizeof(FLOATING_POINT)*numSamples, NULL, &err);
    CL_CHECK_ERROR(err);

    cl_mem d_coefficients = clCreateBuffer(ctx, CL_MEM_READ_ONLY,
                                          sizeof(FLOATING_POINT)*numCoefficients, NULL, &err);
    CL_CHECK_ERROR(err);

    //
    // allocate device memory for output buffer.
    // 
    cl_mem d_results = clCreateBuffer(ctx, CL_MEM_WRITE_ONLY,
                                          sizeof(FLOATING_POINT)*numSamples, NULL, &err);
    CL_CHECK_ERROR(err);

    //
    // write input buffers to the device memory.
    //
    err = clEnqueueWriteBuffer(queue, d_samples, true, 0,
                               sizeof(FLOATING_POINT)*numSamples, samples,
                               0, NULL, NULL);
    CL_CHECK_ERROR(err);

    err = clEnqueueWriteBuffer(queue, d_coefficients, true, 0,
                            sizeof(FLOATING_POINT)*numCoefficients, coefficients,
                            0, NULL, NULL);
    CL_CHECK_ERROR(err);

    err = clFinish(queue);
    CL_CHECK_ERROR(err);

    //
    // set arguments for the kernel
    //
    err = clSetKernelArg(filterkernel, 0, sizeof(cl_mem), (void*)&d_samples);
    CL_CHECK_ERROR(err);
    err = clSetKernelArg(filterkernel, 1, sizeof(cl_mem), (void*)&d_coefficients);
    CL_CHECK_ERROR(err);
    err = clSetKernelArg(filterkernel, 2, sizeof(cl_mem), (void*)&d_results);
    CL_CHECK_ERROR(err);  
    err = clSetKernelArg(filterkernel, 3, sizeof(int), (void*)&numSamples);
    CL_CHECK_ERROR(err);
    err = clSetKernelArg(filterkernel, 4, sizeof(int), (void*)&numCoefficients);
    CL_CHECK_ERROR(err);
    
    //
    // run the kernel
    //
    double nanosec = 0;
    cl_event event = NULL;
    err = clEnqueueTask(queue, filterkernel, 0, NULL, &event);
    CL_CHECK_ERROR(err);

    err = clFinish(queue);
    CL_CHECK_ERROR (err);

    //
    // get the timing/rate info
    //
    cl_ulong submitTime;
    cl_ulong endTime;

    err = clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_SUBMIT,
                                    sizeof(cl_ulong), &submitTime, NULL);
    CL_CHECK_ERROR(err);
    
    err = clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END,
                                    sizeof(cl_ulong), &endTime, NULL);
    CL_CHECK_ERROR(err);

    nanosec = endTime - submitTime;
    
    double rate = double(numSamples) / double(nanosec);
    // cout << "rate = " << rate << " GSamples/sec" << endl;

    //
    // read the samples response result
    //
    err = clEnqueueReadBuffer(queue, d_results, true, 0,
                              sizeof(FLOATING_POINT)*(numSamples), results,
                              0, NULL, NULL);
    CL_CHECK_ERROR(err);

    err = clFinish(queue);
    CL_CHECK_ERROR(err);

    //
    // free device memory
    //
    err = clReleaseMemObject(d_results);
    CL_CHECK_ERROR(err);

    //
    // return the runtime in seconds
    //
    return nanosec / 1.e9;
}

/****************************************************************************
* Function: filterSamplesFPGA()
*
* Purpose: On the FPGA, apply the FIR filter in coefficients to the input samples 
*          and populate the results by convolution.
*
* @param ctx the opencl context to use for the benchmark
* @param queue the opencl command queue to issue commands to
* @param prog the opencl program containing the kernel
* @param benchmarkData the input benchmark data to convolve.
* @param result output -the resulted convolved samples.
* @returns Void
*
* @author Abdul Rehman Tareen
* @date May 14, 2020
*
*****************************************************************************/
double filterSamplesFPGA(cl_device_id dev, 
                             cl_context ctx,
                             cl_command_queue queue,
                             cl_program prog,
                             BenchmarkData benchmarkData,
                             FLOATING_POINT* results)
{
#if defined(CHANNELED) && defined(NON_CHANNELED) 
    printf("Please specifiy non conflicting channel version to be used\n");
    exit(1);
#elif defined(CHANNELED)
    return filterSamplesFPGAChanneled(dev, ctx, queue, prog, benchmarkData, results);
#elif defined(NON_CHANNELED) 
    return filterSamplesFPGANonChanneled(dev, ctx, queue, prog, benchmarkData, results);
#else
    printf("Please specifiy if the channeled version kernel be used or not");
    exit(1);
#endif
}

/****************************************************************************
* Function: benchmarkFirFilter()
*
* Purpose: Executes the FIR Filter benchmark test(s).
*
* @param dev the opencl device id to use for the benchmark
* @param ctx the opencl context to use for the benchmark
* @param queue the opencl command queue to issue commands to
* @param resultDB results from the benchmark are stored in this db
* @param op the options parser / parameter database
*
* @returns Nothing
*
* @author Abdul Rehman Tareen
* @date May 14, 2020
*
****************************************************************************/
void benchmarkFirFilter(cl_device_id dev,
                    cl_context ctx,
                    cl_command_queue queue,
                    BenchmarkDatabase &resultDB,
                    BenchmarkOptions  &options)
{
    auto iter = options.appsToRun.find(firFilter);
    if (iter == options.appsToRun.end())
    {   
        iter = options.appsToRun.find(all);
        cerr << "ERROR: Could not find fir filter benchmark options";
        return; 
    }

    ApplicationOptions appOptions = iter->second;

    // TODO: Input settings to the benchmark check

    if (options.verbose)
        cout << "Creating program from the fir filter bitstream." << endl;

    cl_program program = createProgramFromBitstream(ctx,
                                                    appOptions.bitstreamFile, 
                                                    dev);

    BenchmarkData benchmarkData;

    if (!readFiles(appOptions.dataDir, appOptions.dataGroup, benchmarkData)) return;
    
    if (!options.quiet) cout << "Data files read." << endl;
    
    if (options.verbose) cout << "Verifying files' data.";
    
    if (!verifyFilesData(benchmarkData)) return;   
    
    if (!options.quiet) cout << "Input data verified." << endl;

    if (options.verbose)
    {
        cout << "Number of samples/results: " << benchmarkData.samples.size << endl;
        cout << "Number of coefficients: " << benchmarkData.coefficients.size << endl;
    }

    FLOATING_POINT *results = new FLOATING_POINT[benchmarkData.samples.size];

    for (int pass = 0 ; pass < appOptions.passes; ++pass) 
    {
        if (!options.quiet) cout << "Pass: " << pass << endl;

        // in seconds.
        double t = filterSamplesFPGA(dev, ctx, queue, program, benchmarkData, results);
        
        // Calculate the rate and add it to the results.
        double rate = (double(benchmarkData.samples.size) / double(t)) / 1.e9;
        
        if (options.verbose)
            cout << "time = " << t << " sec, rate = " << rate << " GSamples/sec\n";

        // Verify the computed results by the file results.
        if (!verifyResults(benchmarkData, results))
        {
            cout << "Could not verify the computed result." << endl;
        } 
        else if (!options.quiet)
        {
           cout << "Successfully verified the computed results." << endl;
        }

        if (options.verbose)
        {   
            cout << "Sample (floating point double) equality tolerance: " 
                << benchmarkData.coefficients.size * 20 * DBL_EPSILON_FPGA << endl; 
        }

        char atts[1024];
        sprintf(atts, "%d,%d", benchmarkData.samples.size,
                benchmarkData.coefficients.size);

        // Add the calculated performance to the results.
        resultDB.AddResult("firfilter", "firfilter", atts, "GSample/s", rate);
    }

    delete results;
    delete benchmarkData.samples.elements;
    delete benchmarkData.coefficients.elements;
    delete benchmarkData.results.elements;

    return;
}
