/** @file ransachost.cpp
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include <CL/cl_ext_intelfpga.h>

#include "../common/utility.h"
#include "../common/benchmarkoptions.h"
#include "ransacutility.h"
using namespace std;

/****************************************************************************
* Function: runRansac()
*
* Purpose: This function will be used, if the kernel operates on local memory 
*
* @param dev the opencl device id to use for the benchmark
* @param ctx the opencl context to use for the benchmark
* @param queue the opencl command queue to issue commands to
* @param prog the opencl program containing the kernel
* @param iters number of iterations (random samples to take)
* @param ifile file containing the input data
* @param errorThreshold error threshold for the specified model
* @param convergenceThreshold convergence threshold for the specified model
* 
* @returns double runtime in seconds
*
* @author Jennifer Faj
* @date June 10, 2020
*
*****************************************************************************/

template <class T>
double runRansac(cl_device_id dev,
                 cl_context ctx,
                 cl_command_queue queue,
                 cl_program prog,
                 int iters,
                 string ifile,
                 int errorThreshold,
                 float convergenceThreshold)
{
    int err;

    string inputDataFile = "../../src/ransac/data/" + ifile;

    cl_command_queue queue_in = clCreateCommandQueue(ctx, dev, CL_QUEUE_PROFILING_ENABLE, &err);
    CL_CHECK_ERROR(err);

    cl_command_queue queue_out = clCreateCommandQueue(ctx, dev, CL_QUEUE_PROFILING_ENABLE, &err);
    CL_CHECK_ERROR(err);

    //
    // find the kernels
    //
    cl_kernel datakernel = clCreateKernel(prog, "ransac_data_handler", &err);
    CL_CHECK_ERROR(err);

    cl_kernel modelkernel = clCreateKernel(prog, "ransac_model_gen", &err);
    CL_CHECK_ERROR(err);

    cl_kernel outkernel = clCreateKernel(prog, "ransac_model_count", &err);
    CL_CHECK_ERROR(err);

    //
    // read input data
    //
    int n_idata = readInputSize(inputDataFile);
    T *idata;
    posix_memalign(reinterpret_cast<void**>(&idata), 64, n_idata * sizeof(T));
    readInputData(idata, inputDataFile);

    // 
    // other parameters
    //
    int n_iterations = iters;
    int *randNumbers;
    posix_memalign(reinterpret_cast<void**>(&randNumbers), 64, 2 * n_iterations * sizeof(int));
    genRandNumbers(randNumbers, n_iterations, n_idata);

    int bestOutliers = n_idata;
    int bestModelParams = -1; 

    //
    // create device buffers
    //
    cl_mem d_idata = clCreateBuffer(ctx, CL_MEM_READ_ONLY, n_idata * sizeof(T), NULL, &err);
    CL_CHECK_ERROR(err);
    cl_mem d_randNumbers = clCreateBuffer(ctx, CL_MEM_READ_ONLY, 2 * n_iterations * sizeof(int), NULL, &err);
    CL_CHECK_ERROR(err);

    //
    // output buffers
    // 
    cl_mem n_bestModelParams = clCreateBuffer(ctx, CL_MEM_READ_WRITE, sizeof(int)*1, NULL, &err);
    CL_CHECK_ERROR(err);
    cl_mem n_bestOutliers = clCreateBuffer(ctx, CL_MEM_READ_WRITE, sizeof(int)*1, NULL, &err);
    CL_CHECK_ERROR(err);

    //
    // enqueue data
    //
    err = clEnqueueWriteBuffer(queue_in, d_idata, true, 0, n_idata * sizeof(T), idata, 0, NULL, NULL);
    CL_CHECK_ERROR(err);

    err = clFinish(queue_in);
    CL_CHECK_ERROR(err);

    err = clEnqueueWriteBuffer(queue, d_randNumbers, true, 0, 2 * n_iterations * sizeof(int), randNumbers, 0, NULL, NULL);
    CL_CHECK_ERROR(err);

    err = clFinish(queue);
    CL_CHECK_ERROR(err);

    err = clEnqueueWriteBuffer(queue_out, n_bestModelParams, true, 0, 1 * sizeof(int), &bestModelParams, 0, NULL, NULL);
    CL_CHECK_ERROR(err);
    err = clEnqueueWriteBuffer(queue_out, n_bestOutliers, true, 0, 1 * sizeof(int), &bestOutliers, 0, NULL, NULL);
    CL_CHECK_ERROR(err);

    err = clFinish(queue_out);
    CL_CHECK_ERROR(err);
    

    //
    // set kernel arguments
    //
    err = clSetKernelArg(datakernel, 0, sizeof(cl_mem), (void*)&d_idata);
    CL_CHECK_ERROR(err);
    err = clSetKernelArg(datakernel, 1, sizeof(int), (void*)&errorThreshold);
    CL_CHECK_ERROR(err);
    err = clSetKernelArg(datakernel, 2, sizeof(int), (void*)&n_idata);
    CL_CHECK_ERROR(err);
    err = clSetKernelArg(datakernel, 3, sizeof(int), (void*)&n_iterations);
    CL_CHECK_ERROR(err);

    err = clSetKernelArg(modelkernel, 0, sizeof(cl_mem), (void*)&d_randNumbers);
    CL_CHECK_ERROR(err);
    err = clSetKernelArg(modelkernel, 1, sizeof(int), (void*)&n_idata);
    CL_CHECK_ERROR(err);
    err = clSetKernelArg(modelkernel, 2, sizeof(int), (void*)&n_iterations);
    CL_CHECK_ERROR(err);

    err = clSetKernelArg(outkernel, 0, sizeof(int), (void*)&n_idata);
    CL_CHECK_ERROR(err);
    err = clSetKernelArg(outkernel, 1, sizeof(int), (void*)&n_iterations);
    CL_CHECK_ERROR(err);
    err = clSetKernelArg(outkernel, 2, sizeof(float), (void*)&convergenceThreshold);
    CL_CHECK_ERROR(err);
    err = clSetKernelArg(outkernel, 3, sizeof(cl_mem), (void*)&n_bestModelParams);
    CL_CHECK_ERROR(err);
    err = clSetKernelArg(outkernel, 4, sizeof(cl_mem), (void*)&n_bestOutliers);
    CL_CHECK_ERROR(err);

    //
    // run the kernel
    //
    double nanosec = 0;
    cl_event event = NULL;

    err = clEnqueueTask(queue_in, datakernel, 0, NULL, &event);
    CL_CHECK_ERROR(err);
    // clGetProfileDataDeviceIntelFPGA(dev, prog, true, true, NULL, NULL, NULL, NULL, &err);
    // CL_CHECK_ERROR(err);
    err = clEnqueueTask(queue, modelkernel, 0, NULL, &event);
    CL_CHECK_ERROR(err);
    // clGetProfileDataDeviceIntelFPGA(dev, prog, true, true, NULL, NULL, NULL, NULL, &err);
    // CL_CHECK_ERROR(err);
    err = clEnqueueTask(queue_out, outkernel, 0, NULL, &event);
    CL_CHECK_ERROR(err);

    err = clFinish(queue_in);
    CL_CHECK_ERROR(err);
    err = clFinish(queue);
    CL_CHECK_ERROR(err);
    err = clFinish(queue_out);
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

    //
    // read buffers
    //
    err = clEnqueueReadBuffer(queue_out, n_bestModelParams, true, 0, 1 * sizeof(int), 
                                &bestModelParams, 0, NULL, NULL);
    CL_CHECK_ERROR(err);
    err = clEnqueueReadBuffer(queue_out, n_bestOutliers, true, 0, 1 * sizeof(int), 
                                &bestOutliers, 0, NULL, NULL);
    CL_CHECK_ERROR(err);

    
    err = clFinish(queue_out);
    CL_CHECK_ERROR (err);

    //
    // verify
    //
    verify(idata, n_idata, randNumbers, iters, errorThreshold,
        convergenceThreshold, bestModelParams, bestOutliers);

    //
    // free device memory
    //
    err = clReleaseMemObject(d_idata);
    CL_CHECK_ERROR(err);
    err = clReleaseMemObject(d_randNumbers);
    CL_CHECK_ERROR(err);
    err = clReleaseMemObject(n_bestModelParams);
    CL_CHECK_ERROR(err);
    err = clReleaseMemObject(n_bestOutliers);
    CL_CHECK_ERROR(err);

    // What metric?

    //
    // return the runtime in seconds
    //
    return nanosec / 1.e9;
}

/****************************************************************************
* Function: runRansacGlobal()
*
* Purpose: This function will be used, if the kernel operates on global memory 
*
* @param dev the opencl device id to use for the benchmark
* @param ctx the opencl context to use for the benchmark
* @param queue the opencl command queue to issue commands to
* @param prog the opencl program containing the kernel
* @param iters number of iterations (random samples to take)
* @param ifile file containing the input data
* @param errorThreshold error threshold for the specified model
* @param convergenceThreshold convergence threshold for the specified model
* 
* @returns double runtime in seconds
*
* @author Jennifer Faj
* @date June 10, 2020
*
*****************************************************************************/

template <class T>
double runRansacGlobal(cl_device_id dev,
                 cl_context ctx,
                 cl_command_queue queue,
                 cl_program prog,
                 int iters,
                 string ifile,
                 int errorThreshold,
                 float convergenceThreshold)
{
    int err;

    string inputDataFile = "../../src/ransac/data/" + ifile;

    cl_command_queue queue_in = clCreateCommandQueue(ctx, dev, CL_QUEUE_PROFILING_ENABLE, &err);
    CL_CHECK_ERROR(err);

    cl_command_queue queue_out = clCreateCommandQueue(ctx, dev, CL_QUEUE_PROFILING_ENABLE, &err);
    CL_CHECK_ERROR(err);

    //
    // find the kernels
    //
    cl_kernel datakernel = clCreateKernel(prog, "ransac_data_handler", &err);
    CL_CHECK_ERROR(err);

    cl_kernel modelkernel = clCreateKernel(prog, "ransac_model_gen", &err);
    CL_CHECK_ERROR(err);

    cl_kernel outkernel = clCreateKernel(prog, "ransac_model_count", &err);
    CL_CHECK_ERROR(err);

    //
    // read input data
    //
    int n_idata = readInputSize(inputDataFile);
    T *idata;
    posix_memalign(reinterpret_cast<void**>(&idata), 64, n_idata * sizeof(T));
    readInputData(idata, inputDataFile);

    // 
    // other parameters
    //
    int n_iterations = iters;
    int *randNumbers;
    posix_memalign(reinterpret_cast<void**>(&randNumbers), 64, 2 * n_iterations * sizeof(int));
    genRandNumbers(randNumbers, n_iterations, n_idata);

    int bestOutliers = n_idata;
    int bestModelParams = -1; 

    //
    // create device buffers
    //
    cl_mem d_idata = clCreateBuffer(ctx, CL_MEM_READ_ONLY | CL_CHANNEL_1_INTELFPGA, n_idata * sizeof(T), NULL, &err);
    CL_CHECK_ERROR(err);
    cl_mem d_idata_repl = clCreateBuffer(ctx, CL_MEM_READ_ONLY | CL_CHANNEL_2_INTELFPGA, n_idata * sizeof(T), NULL, &err);
    CL_CHECK_ERROR(err);
    cl_mem d_randNumbers = clCreateBuffer(ctx, CL_MEM_READ_ONLY, 2 * n_iterations * sizeof(int), NULL, &err);
    CL_CHECK_ERROR(err);

    //
    // output buffers
    // 
    cl_mem n_bestModelParams = clCreateBuffer(ctx, CL_MEM_READ_WRITE, sizeof(int)*1, NULL, &err);
    CL_CHECK_ERROR(err);
    cl_mem n_bestOutliers = clCreateBuffer(ctx, CL_MEM_READ_WRITE, sizeof(int)*1, NULL, &err);
    CL_CHECK_ERROR(err);

    //
    // enqueue data
    //
    err = clEnqueueWriteBuffer(queue_in, d_idata, true, 0, n_idata * sizeof(T), idata, 0, NULL, NULL);
    CL_CHECK_ERROR(err);

    err = clFinish(queue_in);
    CL_CHECK_ERROR(err);

    err = clEnqueueWriteBuffer(queue, d_idata_repl, true, 0, n_idata * sizeof(T), idata, 0, NULL, NULL);
    CL_CHECK_ERROR(err);

    err = clEnqueueWriteBuffer(queue, d_randNumbers, true, 0, 2 * n_iterations * sizeof(int), randNumbers, 0, NULL, NULL);
    CL_CHECK_ERROR(err);

    err = clFinish(queue);
    CL_CHECK_ERROR(err);

    err = clEnqueueWriteBuffer(queue_out, n_bestModelParams, true, 0, 1 * sizeof(int), &bestModelParams, 0, NULL, NULL);
    CL_CHECK_ERROR(err);
    err = clEnqueueWriteBuffer(queue_out, n_bestOutliers, true, 0, 1 * sizeof(int), &bestOutliers, 0, NULL, NULL);
    CL_CHECK_ERROR(err);

    err = clFinish(queue_out);
    CL_CHECK_ERROR(err);
    

    //
    // set kernel arguments
    //
    err = clSetKernelArg(datakernel, 0, sizeof(cl_mem), (void*)&d_idata);
    CL_CHECK_ERROR(err);
    err = clSetKernelArg(datakernel, 1, sizeof(int), (void*)&errorThreshold);
    CL_CHECK_ERROR(err);
    err = clSetKernelArg(datakernel, 2, sizeof(int), (void*)&n_idata);
    CL_CHECK_ERROR(err);
    err = clSetKernelArg(datakernel, 3, sizeof(int), (void*)&n_iterations);
    CL_CHECK_ERROR(err);

    err = clSetKernelArg(modelkernel, 0, sizeof(cl_mem), (void*)&d_idata_repl);
    CL_CHECK_ERROR(err);
    err = clSetKernelArg(modelkernel, 1, sizeof(cl_mem), (void*)&d_randNumbers);
    CL_CHECK_ERROR(err);
    err = clSetKernelArg(modelkernel, 2, sizeof(int), (void*)&n_idata);
    CL_CHECK_ERROR(err);
    err = clSetKernelArg(modelkernel, 3, sizeof(int), (void*)&n_iterations);
    CL_CHECK_ERROR(err);

    err = clSetKernelArg(outkernel, 0, sizeof(int), (void*)&n_idata);
    CL_CHECK_ERROR(err);
    err = clSetKernelArg(outkernel, 1, sizeof(int), (void*)&n_iterations);
    CL_CHECK_ERROR(err);
    err = clSetKernelArg(outkernel, 2, sizeof(float), (void*)&convergenceThreshold);
    CL_CHECK_ERROR(err);
    err = clSetKernelArg(outkernel, 3, sizeof(cl_mem), (void*)&n_bestModelParams);
    CL_CHECK_ERROR(err);
    err = clSetKernelArg(outkernel, 4, sizeof(cl_mem), (void*)&n_bestOutliers);
    CL_CHECK_ERROR(err);

    //
    // run the kernel
    //
    double nanosec = 0;
    cl_event event = NULL;

    // Uncomment the commented lines below to obtain profiling information from the autorun kernels
    // when synthesizing with profiling enabled
    err = clEnqueueTask(queue_in, datakernel, 0, NULL, &event);
    CL_CHECK_ERROR(err);
    // clGetProfileDataDeviceIntelFPGA(dev, prog, true, true, NULL, NULL, NULL, NULL, &err);
    // CL_CHECK_ERROR(err);
    err = clEnqueueTask(queue, modelkernel, 0, NULL, &event);
    CL_CHECK_ERROR(err);
    // clGetProfileDataDeviceIntelFPGA(dev, prog, true, true, NULL, NULL, NULL, NULL, &err);
    // CL_CHECK_ERROR(err);
    err = clEnqueueTask(queue_out, outkernel, 0, NULL, &event);
    CL_CHECK_ERROR(err);

    err = clFinish(queue_in);
    CL_CHECK_ERROR(err);
    err = clFinish(queue);
    CL_CHECK_ERROR(err);
    err = clFinish(queue_out);
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

    //
    // read buffers
    //
    err = clEnqueueReadBuffer(queue_out, n_bestModelParams, true, 0, 1 * sizeof(int), 
                                &bestModelParams, 0, NULL, NULL);
    CL_CHECK_ERROR(err);
    err = clEnqueueReadBuffer(queue_out, n_bestOutliers, true, 0, 1 * sizeof(int), 
                                &bestOutliers, 0, NULL, NULL);
    CL_CHECK_ERROR(err);

    
    err = clFinish(queue_out);
    CL_CHECK_ERROR (err);

    //
    // verify
    //
    verify(idata, n_idata, randNumbers, iters, errorThreshold,
        convergenceThreshold, bestModelParams, bestOutliers);

    //
    // free device memory
    //
    err = clReleaseMemObject(d_idata);
    CL_CHECK_ERROR(err);
    err = clReleaseMemObject(d_randNumbers);
    CL_CHECK_ERROR(err);
    err = clReleaseMemObject(n_bestModelParams);
    CL_CHECK_ERROR(err);
    err = clReleaseMemObject(n_bestOutliers);
    CL_CHECK_ERROR(err);

    // What metric?

    //
    // return the runtime in seconds
    //
    return nanosec / 1.e9;
}


/****************************************************************************
* Function: benchmarkRansac()
*
* Purpose: Execute the ransac (Random Sample Consensus Benchmark)
*
* @param dev the opencl device id to use for the benchmark
* @param ctx the opencl context to use for the benchmark
* @param queue the opencl command queue to issue commands to
* @param resultDB results from the benchmark are stored in this db
* @param options the options parser / parameter database
* 
* @returns nothing
*
* @author Jennifer Faj
* @date June 10, 2020
*
*****************************************************************************/

void benchmarkRansac(cl_device_id dev,
                     cl_context ctx,
                     cl_command_queue queue,
                     BenchmarkDatabase & resultDB,
                     BenchmarkOptions &options)
{
    auto iter = options.appsToRun.find(ransac);
    if (iter == options.appsToRun.end())
    {   
        iter = options.appsToRun.find(all);
        cerr << "ERROR: Could not find ransac benchmark options";
        return; 
    }

    ApplicationOptions appOptions = iter->second;
    int iters  = appOptions.iterations;
    string model = appOptions.model;
    string ifile = appOptions.ifile;

    double t;

    if (options.verbose)
        cout << "Creating program from ransac bitstream." << endl;

    cl_program prog = createProgramFromBitstream(ctx,
                                                 appOptions.bitstreamFile, 
                                                 dev);

    for (int pass = 0 ; pass < appOptions.passes; ++pass) 
    {
        string inputDataFile = "../../src/ransac/data/" + ifile;
        int numElements = readInputSize(inputDataFile);
        double bytePerIter = 0.0;
        if (!options.quiet) cout << "Pass: " << pass << endl;
        if (model == "fv"){
            int errorThreshold = 3;                  // as benchmark option?
            float convergenceThreshold = 0.75;       // as benchmark option?
            t = runRansac<flowvector>(dev, ctx, queue, prog, iters, ifile, errorThreshold, convergenceThreshold);
            bytePerIter = double(sizeof(flowvector)) * double(numElements);
        } else if (model == "fvg"){
            int errorThreshold = 3;                  // as benchmark option?
            float convergenceThreshold = 0.75;       // as benchmark option?
            t = runRansacGlobal<flowvector>(dev, ctx, queue, prog, iters, ifile, errorThreshold, convergenceThreshold);
            bytePerIter = double(sizeof(flowvector)) * double(numElements);
        } else if (model == "p") {
            int errorThreshold = 50;                 // as benchmark option?
            float convergenceThreshold = 0.75;       // as benchmark option?
            t = runRansac<point>(dev, ctx, queue, prog, iters, ifile, errorThreshold, convergenceThreshold);
            bytePerIter = double(sizeof(point)) * double(numElements);
        } else {
            cout << "Unknown Model " << model << endl;
            break;
        }

        
        double itersPerSec = double(iters) / double(t);
        double gbPerSec = (itersPerSec * bytePerIter) / 1.e9;
        char atts[1024];
        sprintf(atts, "%diters", iters);
        resultDB.AddResult("ransac", "ransac", atts, "GB/s", gbPerSec);
    }


}