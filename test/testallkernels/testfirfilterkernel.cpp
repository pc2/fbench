/*
    Author: Akhtar, Junaid
    E-mail:  junaida@mail.uni-paderborn.de
    Date:   2020/06/03
*/

#include <gtest/gtest.h>
#include <time.h>
#include "../../src/common/benchmarkoptionsparser.h"
#include "../../src/common/utility.h"

#include "../../src/firfilter/firfilterutility.h"

#include "../common/basetest.h"

using namespace std;
using ::testing::Values;
using ::testing::WithParamInterface;

extern BenchmarkOptions t_options;    
extern cl_int t_clErr;
extern cl_command_queue t_queue;
extern cl_context t_ctx;
extern cl_device_id t_dev;

// firfilter specific Implementation from BaseFixtureTest
class FirFilterKernelsTestFixture : public BaseTestFixture
{

};

// Parameterized Tests implementation of Unit Testing firfilter with Test Fixtures
class FirFilterKernelsTestFixtureWithParam : public FirFilterKernelsTestFixture,
                                          public WithParamInterface<const char *>
{
};

// Test Fixture for executing firfilter Unit Test
TEST_F(FirFilterKernelsTestFixture, DISABLED_TestFirFilter)
{
    // Check if Device Initilization was Successful or not
    ASSERT_EQ(CL_SUCCESS, t_clErr);
    int err = 0;

    auto iter = t_options.appsToRun.find(firFilter);
    bool status = iter == t_options.appsToRun.end();
    if (status)
    {   
        iter = t_options.appsToRun.find(all);
        ASSERT_TRUE(status == 1) << "Missing Benchmark Options";
    }

    ApplicationOptions appOptions = iter->second; 

    cl_program program = createProgramFromBitstream(t_ctx,
                                                appOptions.bitstreamFile, 
                                                t_dev); 
    
    cl_kernel filterkernel = clCreateKernel(program, "convolve", &err);
    ASSERT_FALSE(filterkernel == 0);
    ASSERT_EQ(CL_SUCCESS, err);

    // Input data lengths.
    int numSamples = 100;
    int numCoefficients = 100;

    // allocate device memory for input buffers.
    cl_mem d_samples = clCreateBuffer(t_ctx, CL_MEM_READ_ONLY,
                                          sizeof(double)*numSamples, NULL, &err);
    ASSERT_EQ(CL_SUCCESS, err);

    cl_mem d_coefficients = clCreateBuffer(t_ctx, CL_MEM_READ_ONLY,
                                          sizeof(double)*numCoefficients, NULL, &err);
    ASSERT_EQ(CL_SUCCESS, err);

    // allocate device memory for output buffer.
    cl_mem d_results = clCreateBuffer(t_ctx, CL_MEM_WRITE_ONLY,
                                          sizeof(double)*numSamples, NULL, &err);
    ASSERT_EQ(CL_SUCCESS, err);


    double* samples = new double[numSamples];
    double* coefficients = new double[numCoefficients];
    double* results = new double[numSamples];

    // Input data all equals to 1.

    for (int i=0; i<numSamples; i++)
        samples[i] = 1; 

    for (int i=0; i<numCoefficients; i++)
        coefficients[i] = 1; 

    // write input buffers to the device memory.
    err = clEnqueueWriteBuffer(t_queue, d_samples, true, 0,
                               sizeof(double)*numSamples, samples,
                               0, NULL, NULL);
    ASSERT_EQ(CL_SUCCESS, err);

    err = clEnqueueWriteBuffer(t_queue, d_coefficients, true, 0,
                            sizeof(double)*numCoefficients, coefficients,
                            0, NULL, NULL);
    ASSERT_EQ(CL_SUCCESS, err);

    err = clFinish(t_queue);
    ASSERT_EQ(CL_SUCCESS, err);

    // set arguments for the kernel
    err = clSetKernelArg(filterkernel, 0, sizeof(cl_mem), (void*)&d_samples);
    ASSERT_EQ(CL_SUCCESS, err);
    err = clSetKernelArg(filterkernel, 1, sizeof(cl_mem), (void*)&d_coefficients);
    ASSERT_EQ(CL_SUCCESS, err);
    err = clSetKernelArg(filterkernel, 2, sizeof(cl_mem), (void*)&d_results);
    ASSERT_EQ(CL_SUCCESS, err);  
    err = clSetKernelArg(filterkernel, 3, sizeof(int), (void*)&numSamples);
    ASSERT_EQ(CL_SUCCESS, err);
    err = clSetKernelArg(filterkernel, 4, sizeof(int), (void*)&numCoefficients);
    ASSERT_EQ(CL_SUCCESS, err);
    

    // run the kernel
    cl_event event = NULL;
    err = clEnqueueTask(t_queue, filterkernel, 0, NULL, &event);
    ASSERT_EQ(CL_SUCCESS, err);

    err = clFinish(t_queue);
    ASSERT_EQ(CL_SUCCESS, err);
    
    // read the samples response result
    err = clEnqueueReadBuffer(t_queue, d_results, true, 0,
                              sizeof(double)*(numSamples), results,
                              0, NULL, NULL);
    ASSERT_EQ(CL_SUCCESS, err);

    err = clFinish(t_queue);
    ASSERT_EQ(CL_SUCCESS, err);

    double tolerance = getDeviceEspsilon(numCoefficients);

    for (int i=0; i<numSamples; i++)
    {   
        // All the unit input samples and coefficients will
        // produce a sequential filter result with the 
        // increments of 1, starting from 1. 
        ASSERT_TRUE(floatingPointEquals(results[i], i+1.0, tolerance));
    }

    // free device memory
    err = clReleaseMemObject(d_samples);
    ASSERT_EQ(CL_SUCCESS, err);

    err = clReleaseMemObject(d_coefficients);
    ASSERT_EQ(CL_SUCCESS, err);

    err = clReleaseMemObject(d_results);
    ASSERT_EQ(CL_SUCCESS, err);

    delete samples;
    delete coefficients;
    delete results;

}; // Testfirfilter
