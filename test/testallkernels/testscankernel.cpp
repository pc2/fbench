/*
    Author: Akhtar, Junaid
    E-mail:  junaida@mail.uni-paderborn.de
    Date:   2020/05/30
*/

#include <gtest/gtest.h>
#include <time.h>
#include "../../src/common/benchmarkoptionsparser.h"
#include "../../src/common/utility.h"
#include "../common/basetest.h"

using namespace std;
using ::testing::Values;
using ::testing::WithParamInterface;

extern BenchmarkOptions t_options;
extern cl_int t_clErr;
extern cl_command_queue t_queue;
extern cl_context t_ctx;
extern cl_device_id t_dev;

// scan specific Implementation from BaseFixtureTest
class ScanKernelsTestFixture : public BaseTestFixture
{
};

struct ScanTestItem
{
    int size;
};

// Parameterized Tests implementation of Unit Testing Scan with Test Fixtures

class ScanKernelsTestFixtureWithParam : public ScanKernelsTestFixture,
                                        public WithParamInterface<ScanTestItem>
{
};

// Value Parameterized Test with Test Fixture for executing Scan Unit Test
TEST_P(ScanKernelsTestFixtureWithParam, TestScan)
{

    auto param = GetParam();
    // Check if Device Initilization was Successful or not
    ASSERT_EQ(CL_SUCCESS, t_clErr);
    int errNum = 0;

    auto iter = t_options.appsToRun.find(scan);
    bool status = iter == t_options.appsToRun.end();
    if (status)
    {
        iter = t_options.appsToRun.find(all);
        ASSERT_TRUE(status == 1) << "Missing Benchmark Options";
    }

    ApplicationOptions appOptions = iter->second;

    cl_program fbenchProgram = createProgramFromBitstream(t_ctx, appOptions.bitstreamFile, t_dev);
    {
        cl_kernel scankernel = clCreateKernel(fbenchProgram, "scan", &errNum);
        ASSERT_FALSE(scankernel == 0);
        ASSERT_EQ(CL_SUCCESS, errNum);

        // To-do code start
        int size = param.size;

        // Convert to MB
        size = (size * 1024 * 1024) / sizeof(float);

        // Create input data on CPU
        unsigned int bytes = size * sizeof(float);
        float *reference = new float[size];

        // Allocate pinned host memory for input data (h_idata)
        cl_mem h_i = clCreateBuffer(t_ctx, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR,
                                    bytes, NULL, &errNum);
        ASSERT_EQ(CL_SUCCESS, errNum);
        float *h_idata = (float *)clEnqueueMapBuffer(t_queue, h_i, true,
                                                     CL_MAP_READ | CL_MAP_WRITE, 0, bytes, 0, NULL, NULL, &errNum);
        ASSERT_EQ(CL_SUCCESS, errNum);

        // Allocate pinned host memory for output data (h_odata)
        cl_mem h_o = clCreateBuffer(t_ctx, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR,
                                    bytes, NULL, &errNum);
        ASSERT_EQ(CL_SUCCESS, errNum);
        float *h_odata = (float *)clEnqueueMapBuffer(t_queue, h_o, true,
                                                     CL_MAP_READ | CL_MAP_WRITE, 0, bytes, 0, NULL, NULL, &errNum);
        ASSERT_EQ(CL_SUCCESS, errNum);

        // Initialize host memory
        for (int i = 0; i < size; i++)
        {
            h_idata[i] = i % 3; //Fill with some pattern
            h_odata[i] = -1;
        }

        // Allocate device memory for input array
        cl_mem d_idata = clCreateBuffer(t_ctx, CL_MEM_READ_WRITE, bytes, NULL, &errNum);
        ASSERT_EQ(CL_SUCCESS, errNum);

        // Allocate device memory for output array
        cl_mem d_odata = clCreateBuffer(t_ctx, CL_MEM_READ_WRITE, bytes, NULL, &errNum);
        ASSERT_EQ(CL_SUCCESS, errNum);

        // SINGLE WORK ITEM KERNELS
        // Number of local work items per group
        const size_t local_wsize = 1;
        // Number of global work items
        const size_t global_wsize = 1;

        // Set the kernel arguments
        errNum = clSetKernelArg(scankernel, 0, sizeof(cl_mem), (void *)&d_idata);
        ASSERT_EQ(CL_SUCCESS, errNum);
        errNum = clSetKernelArg(scankernel, 1, sizeof(cl_mem), (void *)&d_odata);
        ASSERT_EQ(CL_SUCCESS, errNum);
        errNum = clSetKernelArg(scankernel, 2, sizeof(cl_int), (void *)&size);
        ASSERT_EQ(CL_SUCCESS, errNum);

        cl_event evTransfer = NULL;

        errNum = clEnqueueWriteBuffer(t_queue, d_idata, true, 0, bytes, h_idata, 0,
                                      NULL, &evTransfer);
        ASSERT_EQ(CL_SUCCESS, errNum);

        errNum = clFinish(t_queue);
        ASSERT_EQ(CL_SUCCESS, errNum);

        errNum = clEnqueueNDRangeKernel(t_queue, scankernel, 1, NULL,
                                        &global_wsize, &local_wsize, 0, NULL, NULL);
        ASSERT_EQ(CL_SUCCESS, errNum);

        errNum = clFinish(t_queue);
        ASSERT_EQ(CL_SUCCESS, errNum);

        errNum = clEnqueueReadBuffer(t_queue, d_odata, true, 0, bytes, h_odata,
                                     0, NULL, &evTransfer);
        ASSERT_EQ(CL_SUCCESS, errNum);

        errNum = clFinish(t_queue);
        ASSERT_EQ(CL_SUCCESS, errNum);

        float last = 0.0f;
        for (unsigned int i = 0; i < size; ++i)
        {
            reference[i] = h_idata[i] + last;
            last = reference[i];
        }

        for (unsigned int i = 0; i < size; ++i)
        {
            ASSERT_FLOAT_EQ(reference[i], h_odata[i]);
        }

        // Clean up device memory
        errNum = clReleaseMemObject(d_idata);
        ASSERT_EQ(CL_SUCCESS, errNum);
        errNum = clReleaseMemObject(d_odata);
        ASSERT_EQ(CL_SUCCESS, errNum);

        // Clean up other host memory
        delete[] reference;

        // To-do code end

        errNum = clReleaseKernel(scankernel);
        ASSERT_EQ(CL_SUCCESS, errNum);
    }

    errNum = clReleaseProgram(fbenchProgram);
    ASSERT_EQ(CL_SUCCESS, errNum);

}; // ScanKernelsWithParameters

// In order to run value-parameterized tests, we need to instantiate them,
// or bind them to a list of values which will be used as test parameters.
// We can instantiate them in a different translation module, or even
// instantiate them several times.

// Here to pass test names after fetching from arguments

// Here, we instantiate our tests with a list of two PrimeTable object factory functions
INSTANTIATE_TEST_CASE_P(TestBaseInstantiation, ScanKernelsTestFixtureWithParam,
                        Values(
                            ScanTestItem{1},
                            ScanTestItem{2},
                            ScanTestItem{4},
                            ScanTestItem{8},
                            ScanTestItem{16},
                            ScanTestItem{32},
                            ScanTestItem{64}));
