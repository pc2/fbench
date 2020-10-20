/*
    Author:         Akhtar, Junaid
    E-mail:         junaida@mail.uni-paderborn.de
    Date:           2020/03/30

    Modified by:    Jennifer Faj
    Date:           2020/06/22
*/

#include <gtest/gtest.h>
#include <time.h>
#include "../../src/common/benchmarkoptionsparser.h"
#include "../../src/common/utility.h"
#include "../common/basetest.h"

#include "../../src/ransac/ransacutility.h"

#define NUM_IDATA 2

using namespace std;
using ::testing::Values;
using ::testing::WithParamInterface;

extern BenchmarkOptions t_options;    
extern cl_int t_clErr;
extern cl_command_queue t_queue;
extern cl_context t_ctx;
extern cl_device_id t_dev;

// ransac specific Implementation from BaseFixtureTest
class RansacKernelsTestFixture : public BaseTestFixture
{

};

struct RansacTestItem
{
    flowvector fv_idata[NUM_IDATA];
    point p_idata[NUM_IDATA];
    int errorThreshold;
    float convergenceThreshold;
    int bestModelParamCount;
    int bestOutlierCount;
};

// Parameterized Tests implementation of Unit Testing ransac Kernel with Test Fixtures
class RansacKernelsTestFixtureWithParam : public RansacKernelsTestFixture,
                                          public WithParamInterface<RansacTestItem>
{
};

// Value Parameterized Test with Test Fixture for executing ransac Unit Test
TEST_P(RansacKernelsTestFixtureWithParam, TestRansac)
{

    auto param = GetParam();

    // Check if Device Initilization was Successful or not
    ASSERT_EQ(CL_SUCCESS, t_clErr);
    int errNum = 0;

    auto iter = t_options.appsToRun.find(ransac);
    bool status = iter == t_options.appsToRun.end();
    if (status)
    {
        iter = t_options.appsToRun.find(all);
        ASSERT_TRUE(status == 1) << "Missing Benchmark Options";
    }

    ApplicationOptions appOptions = iter->second;

    cl_program fbenchProgram = createProgramFromBitstream(t_ctx, appOptions.bitstreamFile, t_dev);
    {
        cl_command_queue t_queue_in = clCreateCommandQueue(t_ctx, t_dev, CL_QUEUE_PROFILING_ENABLE, &errNum);
        ASSERT_EQ(CL_SUCCESS, errNum);

        cl_command_queue t_queue_out = clCreateCommandQueue(t_ctx, t_dev, CL_QUEUE_PROFILING_ENABLE, &errNum);
        ASSERT_EQ(CL_SUCCESS, errNum);

        cl_kernel datakernel = clCreateKernel(fbenchProgram, "ransac_data_handler", &errNum);
        ASSERT_FALSE(datakernel == 0);
        ASSERT_EQ(CL_SUCCESS, errNum);

        cl_kernel modelkernel = clCreateKernel(fbenchProgram, "ransac_model_gen", &errNum);
        ASSERT_FALSE(modelkernel == 0);
        ASSERT_EQ(CL_SUCCESS, errNum);

        cl_kernel outkernel = clCreateKernel(fbenchProgram, "ransac_model_count", &errNum);
        ASSERT_FALSE(outkernel == 0);
        ASSERT_EQ(CL_SUCCESS, errNum);

        int n_idata = 4;
        int n_iterations = 1;
        int errorThreshold = param.errorThreshold;
        float convergenceThreshold = param.convergenceThreshold;
        int bestOutliers = n_idata;
        int bestModelParams = 0; 

        // use a single vector pair for unit test
        int *randNumbers;
        posix_memalign(reinterpret_cast<void**>(&randNumbers), 64, 2 * n_iterations * sizeof(int));
        randNumbers[0] = 0;
        randNumbers[1] = 1;

        flowvector *fv_idata;
        posix_memalign(reinterpret_cast<void**>(&fv_idata), 64, n_idata * sizeof(flowvector));
        fv_idata[0] = param.fv_idata[0];
        fv_idata[1] = param.fv_idata[1];
        fv_idata[2] = param.fv_idata[0];
        fv_idata[3] = param.fv_idata[1];

        point *p_idata;
        posix_memalign(reinterpret_cast<void**>(&p_idata), 64, n_idata * sizeof(point));
        p_idata[0] = param.p_idata[0];
        p_idata[1] = param.p_idata[1];
        p_idata[2] = param.p_idata[0];
        p_idata[3] = param.p_idata[1];

        size_t n;
        if(appOptions.model == "fv"){
            n = n_idata * sizeof(flowvector);
        }else if(appOptions.model == "p"){
            n = n_idata * sizeof(point);
        }

        //
        // create device buffers
        //
        cl_mem d_idata = clCreateBuffer(t_ctx, CL_MEM_READ_ONLY, n, NULL, &errNum);
        ASSERT_EQ(CL_SUCCESS, errNum);
        cl_mem d_randNumbers = clCreateBuffer(t_ctx, CL_MEM_READ_ONLY, 2 * n_iterations * sizeof(int), NULL, &errNum);
        ASSERT_EQ(CL_SUCCESS, errNum);

        //
        // output buffers
        // 
        cl_mem n_bestModelParams = clCreateBuffer(t_ctx, CL_MEM_READ_WRITE, sizeof(int)*1, NULL, &errNum);
        ASSERT_EQ(CL_SUCCESS, errNum);
        cl_mem n_bestOutliers = clCreateBuffer(t_ctx, CL_MEM_READ_WRITE, sizeof(int)*1, NULL, &errNum);
        ASSERT_EQ(CL_SUCCESS, errNum);

        //
        // enqueue data
        //

        if(appOptions.model == "fv"){
            errNum = clEnqueueWriteBuffer(t_queue_in, d_idata, true, 0, n, fv_idata, 0, NULL, NULL);
            ASSERT_EQ(CL_SUCCESS, errNum);
        }else if(appOptions.model == "p"){
            errNum = clEnqueueWriteBuffer(t_queue_in, d_idata, true, 0, n, p_idata, 0, NULL, NULL);
            ASSERT_EQ(CL_SUCCESS, errNum);
        }

        errNum = clFinish(t_queue_in);
        ASSERT_EQ(CL_SUCCESS, errNum);

        errNum = clEnqueueWriteBuffer(t_queue, d_randNumbers, true, 0, 2 * n_iterations * sizeof(int), randNumbers, 0, NULL, NULL);
        ASSERT_EQ(CL_SUCCESS, errNum);

        errNum = clFinish(t_queue);
        ASSERT_EQ(CL_SUCCESS, errNum);

        errNum = clEnqueueWriteBuffer(t_queue_out, n_bestModelParams, true, 0, 1 * sizeof(int), &bestModelParams, 0, NULL, NULL);
        ASSERT_EQ(CL_SUCCESS, errNum);
        errNum = clEnqueueWriteBuffer(t_queue_out, n_bestOutliers, true, 0, 1 * sizeof(int), &bestOutliers, 0, NULL, NULL);
        ASSERT_EQ(CL_SUCCESS, errNum);

        errNum = clFinish(t_queue_out);
        ASSERT_EQ(CL_SUCCESS, errNum);

        //
        // set kernel arguments
        //
        errNum = clSetKernelArg(datakernel, 0, sizeof(cl_mem), (void*)&d_idata);
        ASSERT_EQ(CL_SUCCESS, errNum);
        errNum = clSetKernelArg(datakernel, 1, sizeof(int), (void*)&errorThreshold);
        ASSERT_EQ(CL_SUCCESS, errNum);
        errNum = clSetKernelArg(datakernel, 2, sizeof(int), (void*)&n_idata);
        ASSERT_EQ(CL_SUCCESS, errNum);
        errNum = clSetKernelArg(datakernel, 3, sizeof(int), (void*)&n_iterations);
        ASSERT_EQ(CL_SUCCESS, errNum);

        errNum = clSetKernelArg(modelkernel, 0, sizeof(cl_mem), (void*)&d_randNumbers);
        ASSERT_EQ(CL_SUCCESS, errNum);
        errNum = clSetKernelArg(modelkernel, 1, sizeof(int), (void*)&n_idata);
        ASSERT_EQ(CL_SUCCESS, errNum);
        errNum = clSetKernelArg(modelkernel, 2, sizeof(int), (void*)&n_iterations);
        ASSERT_EQ(CL_SUCCESS, errNum);

        errNum = clSetKernelArg(outkernel, 0, sizeof(int), (void*)&n_idata);
        ASSERT_EQ(CL_SUCCESS, errNum);
        errNum = clSetKernelArg(outkernel, 1, sizeof(int), (void*)&n_iterations);
        ASSERT_EQ(CL_SUCCESS, errNum);
        errNum = clSetKernelArg(outkernel, 2, sizeof(float), (void*)&convergenceThreshold);
        ASSERT_EQ(CL_SUCCESS, errNum);
        errNum = clSetKernelArg(outkernel, 3, sizeof(cl_mem), (void*)&n_bestModelParams);
        ASSERT_EQ(CL_SUCCESS, errNum);
        errNum = clSetKernelArg(outkernel, 4, sizeof(cl_mem), (void*)&n_bestOutliers);
        ASSERT_EQ(CL_SUCCESS, errNum);

        // Run the kernel
        cl_event event = NULL;

        errNum = clEnqueueTask(t_queue_in, datakernel, 0, NULL, &event);
        ASSERT_EQ(CL_SUCCESS, errNum);
        errNum = clEnqueueTask(t_queue, modelkernel, 0, NULL, &event);
        ASSERT_EQ(CL_SUCCESS, errNum);
        errNum = clEnqueueTask(t_queue_out, outkernel, 0, NULL, &event);
        ASSERT_EQ(CL_SUCCESS, errNum);

        errNum = clFinish(t_queue_in);
        ASSERT_EQ(CL_SUCCESS, errNum);
        errNum = clFinish(t_queue);
        ASSERT_EQ(CL_SUCCESS, errNum);
        errNum = clFinish(t_queue_out);
        ASSERT_EQ(CL_SUCCESS, errNum);

        //
        // read buffers
        //
        errNum = clEnqueueReadBuffer(t_queue_out, n_bestModelParams, true, 0, 1 * sizeof(int), 
                                &bestModelParams, 0, NULL, NULL);
        ASSERT_EQ(CL_SUCCESS, errNum);

        errNum = clEnqueueReadBuffer(t_queue_out, n_bestOutliers, true, 0, 1 * sizeof(int), 
                                &bestOutliers, 0, NULL, NULL);
        ASSERT_EQ(CL_SUCCESS, errNum);

        errNum = clFinish(t_queue_out);
        ASSERT_EQ(CL_SUCCESS, errNum);

        /*----- CHECK FOR FUNCTIONAL CORRECTNESS -----*/

        int bestModelParamCount = param.bestModelParamCount;
        int bestOutlierCount = param.bestOutlierCount;

        // Check if best model parameters were found 
        // by checking the respective iteration index
        // better: also check the actual parameters here..
        // though this would require changing the kernel interface
        ASSERT_EQ(bestModelParams, bestModelParamCount);

        // Also check if the number of outliers is correct
        ASSERT_EQ(bestOutliers, bestOutlierCount);

        /*--------------------------------------------*/

        // Free Memory
        errNum = clReleaseMemObject(d_idata);
        ASSERT_EQ(CL_SUCCESS, errNum);
        errNum = clReleaseMemObject(d_randNumbers);
        ASSERT_EQ(CL_SUCCESS, errNum);
        errNum = clReleaseMemObject(n_bestModelParams);
        ASSERT_EQ(CL_SUCCESS, errNum);
        errNum = clReleaseMemObject(n_bestOutliers);
        ASSERT_EQ(CL_SUCCESS, errNum);

        errNum = clReleaseKernel(datakernel);
        ASSERT_EQ(CL_SUCCESS, errNum);
        errNum = clReleaseKernel(modelkernel);
        ASSERT_EQ(CL_SUCCESS, errNum);
        errNum = clReleaseKernel(outkernel);
        ASSERT_EQ(CL_SUCCESS, errNum);
    }

    clReleaseProgram(fbenchProgram);

}; 

// In order to run value-parameterized tests, we need to instantiate them,
// or bind them to a list of values which will be used as test parameters.
// We can instantiate them in a different translation module, or even
// instantiate them several times.

// Here to pass test names after fetching from arguments

// Here, we instantiate our tests with a list of two PrimeTable object factory functions
INSTANTIATE_TEST_CASE_P(TestBaseInstantiation, RansacKernelsTestFixtureWithParam,
                        Values(
                            RansacTestItem{{{.x=300, .y=20, .vx=330, .vy=40},{.x=100, .y=10, .vx=110, .vy=20}},
                                            {{.x=98, .y=23589},{.x=97, .y=23588}},
                                            3, 0.75, 1, 0}, // fitting model
                            RansacTestItem{{{.x=300, .y=20, .vx=300, .vy=20},{.x=300, .y=20, .vx=300, .vy=20}}, 
                                            {{.x=98, .y=23589},{.x=98, .y=23588}},
                                            3, 0.75, 0, 4}  // division by 0
                        ));