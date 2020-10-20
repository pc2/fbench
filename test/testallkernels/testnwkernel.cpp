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
#include "CL/cl_ext_intelfpga.h"

using namespace std;
using ::testing::Values;
using ::testing::WithParamInterface;

extern BenchmarkOptions t_options;
extern cl_int t_clErr;
extern cl_command_queue t_queue;
extern cl_context t_ctx;
extern cl_device_id t_dev;

#define AOCL_ALIGNMENT 64
inline static void* alignedMalloc(size_t size)
{
#ifdef _WIN32
	void *ptr = _aligned_malloc(size, AOCL_ALIGNMENT);
	if (ptr == NULL)
	{
		fprintf(stderr, "Aligned Malloc failed due to insufficient memory.\n");
		exit(-1);
	}
	return ptr;
#else
	void *ptr = NULL;
	if ( posix_memalign (&ptr, AOCL_ALIGNMENT, size) )
	{
		fprintf(stderr, "Aligned Malloc failed due to insufficient memory.\n");
		exit(-1);
	}
	return ptr;
#endif	
}

inline int maximum(int a, int b, int c)
{
	int k;
	if(a <= b)
		k = b;
	else
		k = a;

	if(k <= c)
		return(c);
	else
		return(k);
}

// scan specific Implementation from BaseFixtureTest
class NWKernelsTestFixture : public BaseTestFixture
{
};

struct NWTestItem
{
    int dim;
	vector<int> reference;
	vector<int> input_itemsets;
	vector<int> output_itemsets;
};

// Parameterized Tests implementation of Unit Testing Scan with Test Fixtures

class NWKernelsTestFixtureWithParam : public NWKernelsTestFixture,
                                        public WithParamInterface<NWTestItem>
{
};

// Value Parameterized Test with Test Fixture for executing Scan Unit Test
TEST_P(NWKernelsTestFixtureWithParam, TestNW)
{

    auto param = GetParam();
    // Check if Device Initilization was Successful or not
    ASSERT_EQ(CL_SUCCESS, t_clErr);
    int err = 0;

    auto iter = t_options.appsToRun.find(nw);
    bool status = iter == t_options.appsToRun.end();
    if (status)
    {
        iter = t_options.appsToRun.find(all);
        ASSERT_TRUE(status == 1) << "Missing Benchmark Options";
    }

    ApplicationOptions appOptions = iter->second;

    cl_program fbenchProgram = createProgramFromBitstream(t_ctx, appOptions.bitstreamFile, t_dev);

    cl_kernel nwkernel = clCreateKernel(fbenchProgram, "nw", &err);
    ASSERT_FALSE(nwkernel == 0);
    ASSERT_EQ(CL_SUCCESS, err);

    // To-do code start
    int dim = param.dim;

    int max_rows = dim;
    int max_cols = dim;
    int penalty = 10;
//    int BSIZE = 16;
//    int PAR = 4;
    
    max_rows = max_rows + 1;
    max_cols = max_cols + 1;
    
    int num_rows = max_rows;
    int num_cols = max_cols - 1;
    
    int data_size = max_cols * max_rows;
    int ref_size = num_cols * num_rows;
    
    int *buffer_v = NULL;
    int *buffer_h = NULL;
    
    buffer_h = (int *)alignedMalloc(num_cols * sizeof(int));
    buffer_v = (int *)alignedMalloc(num_rows * sizeof(int));

	int *reference = (int *)alignedMalloc(ref_size * sizeof(int));
    std::copy(param.reference.begin(), param.reference.end(), reference);

	int *input_itemsets = (int *)alignedMalloc(data_size * sizeof(int));
    std::copy(param.input_itemsets.begin(), param.input_itemsets.end(), input_itemsets);

	int *output_itemsets = (int *)alignedMalloc(ref_size * sizeof(int));
    int *test_result = (int *)alignedMalloc(ref_size * sizeof(int));
    std::copy(param.output_itemsets.begin(), param.output_itemsets.end(), test_result);

    for(int i = 1; i < max_rows; i++)
    {
        input_itemsets[i * max_cols] = -i * penalty;
        buffer_v[i] = -i * penalty;
    }
    buffer_v[0] = 0;
    
    for(int j = 1; j < max_cols; j++)
    {
        input_itemsets[j] = -j * penalty;
        buffer_h[j - 1] = -j * penalty;
    }

    // Allocate host memory for reference data (reference)
    cl_mem reference_d = clCreateBuffer(t_ctx, CL_MEM_READ_ONLY | CL_CHANNEL_1_INTELFPGA, ref_size * sizeof(int), NULL, &err);
    ASSERT_EQ(CL_SUCCESS, err);

    // Allocate host memory for input data (input_itemsets)
    int device_buff_size = ref_size;
    cl_mem input_itemsets_d = clCreateBuffer(t_ctx, CL_MEM_READ_WRITE | CL_CHANNEL_2_INTELFPGA, device_buff_size * sizeof(int), NULL, &err);
    ASSERT_EQ(CL_SUCCESS, err);

    // Allocate
    cl_mem buffer_v_d = clCreateBuffer(t_ctx, CL_MEM_READ_ONLY | CL_CHANNEL_1_INTELFPGA, num_rows * sizeof(int), NULL, &err);
    ASSERT_EQ(CL_SUCCESS, err);

    err = clEnqueueWriteBuffer(t_queue, reference_d, 1, 0, ref_size * sizeof(int), reference, 0, 0, 0);
    ASSERT_EQ(CL_SUCCESS, err);

    err = clFinish(t_queue);
    ASSERT_EQ(CL_SUCCESS, err);

    err = clEnqueueWriteBuffer(t_queue, input_itemsets_d, 1, 0, num_cols * sizeof(int), buffer_h, 0, 0, 0);
    ASSERT_EQ(CL_SUCCESS, err);

    err = clFinish(t_queue);
    ASSERT_EQ(CL_SUCCESS, err);

    err = clEnqueueWriteBuffer(t_queue, buffer_v_d, 1, 0, num_rows * sizeof(int), buffer_v, 0, 0, 0);
    ASSERT_EQ(CL_SUCCESS, err);

    err = clFinish(t_queue);
    ASSERT_EQ(CL_SUCCESS, err);

    int worksize = max_cols - 1;
    cout<< "WG size of kernel = " << BSIZE << endl;
    cout<< "worksize = " << worksize << endl;
    int offset_r = 0, offset_c = 0;
    int block_width = worksize/BSIZE;

    // Set the kernel arguments
    int cols = num_cols - 1 + PAR;
    int exit_col = (cols % PAR == 0) ? cols : cols + PAR - (cols % PAR);
    int loop_exit = exit_col * (BSIZE / PAR);

    err = clSetKernelArg(nwkernel, 0, sizeof(void *), (void*) &reference_d);
    ASSERT_EQ(CL_SUCCESS, err);
    err = clSetKernelArg(nwkernel, 1, sizeof(void *), (void*) &input_itemsets_d);
    ASSERT_EQ(CL_SUCCESS, err);
    err = clSetKernelArg(nwkernel, 2, sizeof(void *), (void*) &buffer_v_d);
    ASSERT_EQ(CL_SUCCESS, err);
    err = clSetKernelArg(nwkernel, 3, sizeof(cl_int), (void*) &num_cols);
    ASSERT_EQ(CL_SUCCESS, err);
    err = clSetKernelArg(nwkernel, 4, sizeof(cl_int), (void*) &penalty);
    ASSERT_EQ(CL_SUCCESS, err);
    err = clSetKernelArg(nwkernel, 5, sizeof(cl_int), (void*) &loop_exit);
    ASSERT_EQ(CL_SUCCESS, err);
        
    int num_diags  = max_rows - 1;
    int comp_bsize = BSIZE - 1;
    int last_diag  = (num_diags % comp_bsize == 0) ? num_diags : num_diags + comp_bsize - (num_diags % comp_bsize);
    int num_blocks = last_diag / comp_bsize;
        
    for (int bx = 0; bx < num_blocks; bx++)
    {
        int block_offset = bx * comp_bsize;

        err = clSetKernelArg(nwkernel, 6, sizeof(cl_int), (void*) &block_offset);
        CL_CHECK_ERROR(err);
            
        err = clEnqueueTask(t_queue, nwkernel, 0, NULL, NULL);
        CL_CHECK_ERROR(err);

        err = clFinish(t_queue);
        CL_CHECK_ERROR(err);
    }
    
    err = clEnqueueReadBuffer(t_queue, input_itemsets_d, 1, 0, ref_size * sizeof(int), output_itemsets, 0, 0, 0);
    CL_CHECK_ERROR(err);

    err = clFinish(t_queue);
    CL_CHECK_ERROR(err);
    
	for (int i=0 ; i < max_rows - 2 ; ++i)
	{
		for (int j = 0 ; j < max_cols - 2 ; ++j)
		{
            ASSERT_FLOAT_EQ(output_itemsets[i * (max_cols - 1) + j], test_result[i * (max_cols - 2) + j]);
		}
	}

    // Clean up device memory
    err = clReleaseMemObject(reference_d);
    ASSERT_EQ(CL_SUCCESS, err);
    err = clReleaseMemObject(input_itemsets_d);
    ASSERT_EQ(CL_SUCCESS, err);

    // Clean up other host memory

    err = clReleaseProgram(fbenchProgram);
    ASSERT_EQ(CL_SUCCESS, err);
    err = clReleaseKernel(nwkernel);
    ASSERT_EQ(CL_SUCCESS, err);


};

INSTANTIATE_TEST_CASE_P(TestBaseInstantiation, NWKernelsTestFixtureWithParam,
                        Values(
                            NWTestItem{16,
                                {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-3,-2,0,-2,0,-3,-3,0,-3,0,1,-1,8,0,-3,0,2,-4,-2,-4,-2,-1,2,-2,4,-2,-3,-4,-3,-2,-1,-2,2,-4,-2,-4,-2,-1,2,-2,4,-2,-3,-4,-3,-2,-1,-2,-3,0,0,0,0,-3,-3,0,-3,0,6,1,1,0,-3,0,-3,-2,0,-2,2,-4,-3,2,-3,2,0,2,0,0,-4,2,-1,-3,-3,-3,-3,9,-1,-3,-1,-3,-3,-3,-3,-3,9,-3,-4,6,-2,6,-2,-3,-4,-2,-4,-2,0,-1,-2,-2,-3,-2,-3,-2,0,-2,0,-3,-3,0,-3,0,1,-1,8,0,-3,0,-3,-2,5,-2,1,-3,-3,1,-2,1,0,-2,0,5,-3,1,-1,-3,-3,-3,-3,9,-1,-3,-1,-3,-3,-3,-3,-3,9,-3,-3,-2,0,-2,2,-4,-3,2,-3,2,0,2,0,0,-4,2,-3,0,0,0,0,-3,-3,0,-3,0,6,1,1,0,-3,0,-3,0,0,0,0,-3,-3,0,-3,0,6,1,1,0,-3,0,-4,6,-2,6,-2,-3,-4,-2,-4,-2,0,-1,-2,-2,-3,-2,-3,-2,0,-2,2,-4,-3,2,-3,2,0,2,0,0,-4,2,-3,-2,5,-2,1,-3,-3,1,-2,1,0,-2,0,5,-3,1},
                                {0,-10,-20,-30,-40,-50,-60,-70,-80,-90,-100,-110,-120,-130,-140,-150,-10,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-30,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-40,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-50,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-70,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-80,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-90,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-100,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-110,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-120,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-130,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-140,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-150,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                                {-10,-20,-30,-40,-50,-60,-70,-80,-90,-100,-110,-120,-130,-140,-150,-3,-12,-20,-30,-40,-50,-60,-70,-80,-90,-99,-109,-112,-122,-132,-8,-7,-14,-24,-32,-41,-48,-58,-66,-76,-86,-96,-106,-114,-123,-18,-12,-9,-18,-26,-33,-39,-49,-54,-64,-74,-84,-94,-104,-114,-28,-18,-12,-9,-18,-28,-36,-39,-49,-54,-58,-68,-78,-88,-98,-38,-28,-18,-14,-7,-17,-27,-34,-42,-47,-54,-56,-66,-76,-86,-48,-38,-28,-21,-17,2,-8,-18,-28,-38,-48,-57,-59,-69,-67,-58,-42,-38,-22,-23,-8,-2,-10,-20,-30,-38,-48,-58,-61,-71,-68,-52,-42,-32,-22,-18,-11,-2,-12,-20,-29,-39,-40,-50,-60,-78,-62,-47,-42,-31,-25,-21,-10,-4,-11,-20,-30,-39,-35,-45,-88,-72,-57,-50,-41,-22,-26,-20,-11,-7,-14,-23,-33,-42,-26,-98,-82,-67,-59,-48,-32,-25,-24,-21,-9,-7,-12,-22,-32,-36,-108,-92,-77,-67,-58,-42,-35,-25,-27,-19,-3,-6,-11,-21,-31,-118,-102,-87,-77,-67,-52,-45,-35,-28,-27,-13,-2,-5,-11,-21,-128,-112,-97,-81,-77,-62,-55,-45,-38,-30,-23,-12,-4,-7,-14}
                                }));
