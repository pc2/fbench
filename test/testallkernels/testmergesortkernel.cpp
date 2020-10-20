/*
    Author: Akhtar, Junaid
    E-mail:  junaida@mail.uni-paderborn.de
    Date:   2020/05/30
	
	Modifier: Farjana Jalil
	Date: 2020/08/30
*/

#include <gtest/gtest.h>
#include <time.h>
#include "../../src/common/benchmarkoptionsparser.h"
#include "../../src/common/utility.h"
#include "../common/basetest.h"

#include <iostream>
#include <algorithm>

using namespace std;
using ::testing::Values;
using ::testing::WithParamInterface;

extern BenchmarkOptions t_options;
extern cl_int t_clErr;
extern cl_command_queue t_queue;
extern cl_context t_ctx;
extern cl_device_id t_dev;

// mergesort specific Implementation from BaseFixtureTest
class MergeSortKernelsTestFixture : public BaseTestFixture
{
};

struct MergeSortTestItem
{
    int size;
};

// Parameterized Tests implementation of Unit Testing mergesort kernel with Test Fixtures

class MergeSortKernelsTestFixtureWithParam : public MergeSortKernelsTestFixture,
                                        public WithParamInterface<MergeSortTestItem>
{
};

// Value Parameterized Test with Test Fixture for executing mergesort Unit Test
TEST_P(MergeSortKernelsTestFixtureWithParam, TestMergeSort)
{

    // Check if Device Initilization was Successful or not
    ASSERT_EQ(CL_SUCCESS, t_clErr);
    int errNum = 0;

    auto iter = t_options.appsToRun.find(mergesort);
    bool status = iter == t_options.appsToRun.end();
    if (status)
    {
        iter = t_options.appsToRun.find(all);
        ASSERT_TRUE(status == 1) << "Missing Benchmark Options";
    }

    ApplicationOptions appOptions = iter->second;

    cl_program fbenchProgram = createProgramFromBitstream(t_ctx, appOptions.bitstreamFile, t_dev);
    {
        cl_kernel mergesortkernel = clCreateKernel(fbenchProgram, "mergesort", &errNum);
        ASSERT_FALSE(mergesortkernel == 0);
        ASSERT_EQ(CL_SUCCESS, errNum);

		int size = 64;

        // Create input data on CPU
        unsigned int bytes = size * sizeof(int);
		
        int *reference = new int[size];

        // Allocate pinned host memory for input data (buffer_in_data)
        cl_mem buffer_in = clCreateBuffer(t_ctx, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR,
                                    bytes, NULL, &errNum);
        ASSERT_EQ(CL_SUCCESS, errNum);
		
        int *buffer_in_data = (int *)clEnqueueMapBuffer(t_queue, buffer_in, true,
                                                     CL_MAP_READ | CL_MAP_WRITE, 0, bytes, 0, NULL, NULL, &errNum);
        ASSERT_EQ(CL_SUCCESS, errNum);

        // Allocate pinned host memory for output data (buffer_out_data)
        cl_mem buffer_out = clCreateBuffer(t_ctx, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR,
                                    bytes, NULL, &errNum);
        ASSERT_EQ(CL_SUCCESS, errNum);
		
        int *buffer_out_data = (int *)clEnqueueMapBuffer(t_queue, buffer_out, true,
                                                     CL_MAP_READ | CL_MAP_WRITE, 0, bytes, 0, NULL, NULL, &errNum);
        ASSERT_EQ(CL_SUCCESS, errNum);


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
        cl_mem device_in_data = clCreateBuffer(t_ctx, CL_MEM_READ_WRITE, bytes, NULL, &errNum);
        ASSERT_EQ(CL_SUCCESS, errNum);

        // Allocate device memory for output array
        cl_mem device_out_data = clCreateBuffer(t_ctx, CL_MEM_READ_WRITE, bytes, NULL, &errNum);
        ASSERT_EQ(CL_SUCCESS, errNum);

		// Set the kernel arguments
        errNum = clSetKernelArg(mergesortkernel, 0, sizeof(cl_mem), (void *)&device_in_data);
        ASSERT_EQ(CL_SUCCESS, errNum);
		
        errNum = clSetKernelArg(mergesortkernel, 1, sizeof(cl_mem), (void *)&device_out_data);
        ASSERT_EQ(CL_SUCCESS, errNum);
		
        errNum = clSetKernelArg(mergesortkernel, 2, sizeof(cl_int), (void *)&size);
        ASSERT_EQ(CL_SUCCESS, errNum);

        cl_event evTransfer = NULL;

        errNum = clEnqueueWriteBuffer(t_queue, device_in_data, true, 0, bytes, buffer_in_data, 0,
                                      NULL, &evTransfer);
        ASSERT_EQ(CL_SUCCESS, errNum);

        errNum = clFinish(t_queue);
        ASSERT_EQ(CL_SUCCESS, errNum);
		
		errNum = clEnqueueTask(t_queue, mergesortkernel, 0, NULL, NULL);

		ASSERT_EQ(CL_SUCCESS, errNum);

        errNum = clFinish(t_queue);
        ASSERT_EQ(CL_SUCCESS, errNum);

        errNum = clEnqueueReadBuffer(t_queue, device_out_data, true, 0, bytes, buffer_out_data,
                                     0, NULL, &evTransfer);
        ASSERT_EQ(CL_SUCCESS, errNum);

        errNum = clFinish(t_queue);
        ASSERT_EQ(CL_SUCCESS, errNum);
		

		for (int i = 0; i < size; i++)
        {
            reference[i] = buffer_in_data[i];
        }
		
		sort(reference, reference+size);
		
		for (int i = 0; i < size; i++)
        {
            ASSERT_EQ(reference[i], buffer_out_data[i]);
        }
		

        // Clean up device memory
        errNum = clReleaseMemObject(device_in_data);
        ASSERT_EQ(CL_SUCCESS, errNum);
		
        errNum = clReleaseMemObject(device_out_data);
        ASSERT_EQ(CL_SUCCESS, errNum);

        // Clean up other host memory
        delete[] reference;

        errNum = clReleaseKernel(mergesortkernel);
        ASSERT_EQ(CL_SUCCESS, errNum);
    }

    errNum = clReleaseProgram(fbenchProgram);
    ASSERT_EQ(CL_SUCCESS, errNum);

};

// In order to run value-parameterized tests, we need to instantiate them,
// or bind them to a list of values which will be used as test parameters.
// We can instantiate them in a different translation module, or even
// instantiate them several times.

// Here to pass test names after fetching from arguments

// Here, we instantiate our tests with a list of two PrimeTable object factory functions
INSTANTIATE_TEST_CASE_P(TestBaseInstantiation, MergeSortKernelsTestFixtureWithParam,
                        Values( MergeSortTestItem{64} ));
