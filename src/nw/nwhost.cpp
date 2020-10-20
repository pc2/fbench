/*
 *
 * Copyright (c) 2008-2011 University of Virginia
 * Copyright (c) 2016 RIKEN
 * Copyright (c) 2016 Tokyo Institute of Technology
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted without royalty fees or other restrictions, provided that the following conditions are met:
 *
 *      > Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 *      > Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
 *      > Neither the name of the University of Virginia, the Dept. of Computer Science, RIKEN, Tokyo Institute of Technology, nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE UNIVERSITY OF VIRGINIA OR THE SOFTWARE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The code is based on rodinia_fpga project
 * - H. R. Zohouri, N. Maruyama, A. Smith, M. Matsuda, and S. Matsuoka. "Evaluating and Optimizing OpenCL Kernels for High Performance Computing with FPGAs," Proceedings of the ACM/IEEE International Conference for High Performance Computing, Networking, Storage and Analysis (SC'16), Nov 2016.
 *   
 * Ported to FBench by: Masood Raeisi Nafchi
 * Date: 2020/09/21
 * 
*/

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <fstream>
#include <vector>

#include "../common/utility.h"
#include "timer.h"
using namespace std;

#include "CL/cl_ext_intelfpga.h"

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

template <class T>
void runTest(const string& testName, cl_device_id dev, cl_context ctx,
        cl_command_queue queue, BenchmarkDatabase& resultDB, BenchmarkOptions& op,
        const string& compileFlags);

// ****************************************************************************
// Function: nwCPU
//
// Purpose:
//   Simple cpu nw routine to verify device results
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
inline int maximum(int, int, int);

bool nwCPU(int *reference, int *input_itemsets, int *output_itemsets, int max_rows, int max_cols, int penalty)
{
    bool passed = true;

	for (int i = 1; i < max_rows - 1; ++i)
	{
		for (int j = 1; j < max_cols - 1; ++j)
		{
			int index = i * max_cols + j;
            int ref_offset = i * (max_cols-1) + (j-1);
			input_itemsets[index] = maximum(input_itemsets[index - 1 - max_rows] + reference[ref_offset],
									 input_itemsets[index - 1]       - penalty,
									 input_itemsets[index - max_rows]     - penalty);
 		}
	}

	for (int i=0 ; i < max_rows - 2 ; ++i)
	{
		for (int j = 0, jt = 1 ; j < max_cols - 2 && jt < max_cols - 1 ; ++j,++jt)
		{
            if (output_itemsets[i * (max_cols - 1) + j] != input_itemsets[i * (max_cols) + jt])
            {
                passed = false;
            }
		}
	}
        
    cout << "Test ";
    if (passed)
        cout << "Passed" << endl;
    else
        cout << "Failed" << endl;
    return passed;
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

// ****************************************************************************
// Function: RunBenchmark
//
// Purpose:
//   Executes the nw (Needleman-Wunsch) benchmark
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
// Programmer: Masood Raeisi Nafchi
// Creation: September 25, 2020
//
// ****************************************************************************
extern const char *cl_source_nw;

int blosum62[24][24] = {
{ 4, -1, -2, -2,  0, -1, -1,  0, -2, -1, -1, -1, -1, -2, -1,  1,  0, -3, -2,  0, -2, -1,  0, -4},
{-1,  5,  0, -2, -3,  1,  0, -2,  0, -3, -2,  2, -1, -3, -2, -1, -1, -3, -2, -3, -1,  0, -1, -4},
{-2,  0,  6,  1, -3,  0,  0,  0,  1, -3, -3,  0, -2, -3, -2,  1,  0, -4, -2, -3,  3,  0, -1, -4},
{-2, -2,  1,  6, -3,  0,  2, -1, -1, -3, -4, -1, -3, -3, -1,  0, -1, -4, -3, -3,  4,  1, -1, -4},
{ 0, -3, -3, -3,  9, -3, -4, -3, -3, -1, -1, -3, -1, -2, -3, -1, -1, -2, -2, -1, -3, -3, -2, -4},
{-1,  1,  0,  0, -3,  5,  2, -2,  0, -3, -2,  1,  0, -3, -1,  0, -1, -2, -1, -2,  0,  3, -1, -4},
{-1,  0,  0,  2, -4,  2,  5, -2,  0, -3, -3,  1, -2, -3, -1,  0, -1, -3, -2, -2,  1,  4, -1, -4},
{ 0, -2,  0, -1, -3, -2, -2,  6, -2, -4, -4, -2, -3, -3, -2,  0, -2, -2, -3, -3, -1, -2, -1, -4},
{-2,  0,  1, -1, -3,  0,  0, -2,  8, -3, -3, -1, -2, -1, -2, -1, -2, -2,  2, -3,  0,  0, -1, -4},
{-1, -3, -3, -3, -1, -3, -3, -4, -3,  4,  2, -3,  1,  0, -3, -2, -1, -3, -1,  3, -3, -3, -1, -4},
{-1, -2, -3, -4, -1, -2, -3, -4, -3,  2,  4, -2,  2,  0, -3, -2, -1, -2, -1,  1, -4, -3, -1, -4},
{-1,  2,  0, -1, -3,  1,  1, -2, -1, -3, -2,  5, -1, -3, -1,  0, -1, -3, -2, -2,  0,  1, -1, -4},
{-1, -1, -2, -3, -1,  0, -2, -3, -2,  1,  2, -1,  5,  0, -2, -1, -1, -1, -1,  1, -3, -1, -1, -4},
{-2, -3, -3, -3, -2, -3, -3, -3, -1,  0,  0, -3,  0,  6, -4, -2, -2,  1,  3, -1, -3, -3, -1, -4},
{-1, -2, -2, -1, -3, -1, -1, -2, -2, -3, -3, -1, -2, -4,  7, -1, -1, -4, -3, -2, -2, -1, -2, -4},
{ 1, -1,  1,  0, -1,  0,  0,  0, -1, -2, -2,  0, -1, -2, -1,  4,  1, -3, -2, -2,  0,  0,  0, -4},
{ 0, -1,  0, -1, -1, -1, -1, -2, -2, -1, -1, -1, -1, -2, -1,  1,  5, -2, -2,  0, -1, -1,  0, -4},
{-3, -3, -4, -4, -2, -2, -3, -2, -2, -3, -2, -3, -1,  1, -4, -3, -2, 11,  2, -3, -4, -3, -2, -4},
{-2, -2, -2, -3, -2, -1, -2, -3,  2, -1, -1, -2, -1,  3, -3, -2, -2,  2,  7, -1, -3, -2, -1, -4},
{ 0, -3, -3, -3, -1, -2, -2, -3, -3,  3,  1, -2,  1, -1, -2, -2,  0, -3, -1,  4, -3, -2, -1, -4},
{-2, -1,  3,  4, -3,  0,  1, -1,  0, -3, -4,  0, -3, -3, -2,  0, -1, -4, -3, -3,  4,  1, -1, -4},
{-1,  0,  0,  1, -3,  3,  4, -2,  0, -3, -3,  1, -1, -3, -1,  0, -1, -3, -2, -2,  1,  4, -1, -4},
{ 0, -1, -1, -1, -2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -2,  0,  0, -2, -1, -1, -1, -1, -1, -4},
{-4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4,  1}
};

void
benchmarkNW(cl_device_id dev,
                    cl_context ctx,
                    cl_command_queue queue,
                    BenchmarkDatabase &resultDB,
                    BenchmarkOptions &options)
{
    cout << "Needleman-Wunsch Benchmark running ..."<<endl;
    auto iter = options.appsToRun.find(nw);

    if (iter == options.appsToRun.end())
    {   
        cerr << "ERROR: Could not find benchmark options";
        return; 
    }

    ApplicationOptions appOptions = iter->second;

    int err = 0;

    // Problem Sizes
    int probSizes[7] = { 1, 2, 4, 8, 16, 32, 64 };
    int size = probSizes[appOptions.size-1];

    // Convert to MB
    size = size * 1024;
    int dim = size;

    int max_rows = dim;
    int max_cols = dim;
    int penalty = 10;
    
    max_rows = max_rows + 1;
    max_cols = max_cols + 1;
    
    int num_rows = max_rows;
    int num_cols = max_cols - 1;
    
    int data_size = max_cols * max_rows;
    int ref_size = num_cols * num_rows;
    
	int *reference = (int *)alignedMalloc(ref_size * sizeof(int));
	int *input_itemsets = (int *)alignedMalloc(data_size * sizeof(int));
	int *output_itemsets = (int *)alignedMalloc(ref_size * sizeof(int));
    
    int *buffer_v = NULL;
    int *buffer_h = NULL;
    
    buffer_h = (int *)alignedMalloc(num_cols * sizeof(int));
    buffer_v = (int *)alignedMalloc(num_rows * sizeof(int));
    
    //initialization
    srand(7);
    
	for (int i = 0; i < max_cols; i++)
	{
		for (int j = 0; j < max_rows; j++)
		{
			input_itemsets[i * max_rows + j] = 0;
		}
	}

	for(int i = 1; i < max_rows; i++)
	{    //initialize the first column
		input_itemsets[i * max_cols] = rand() % 10 + 1;
	}
	
	for(int j = 1; j < max_cols; j++)
	{    //initialize the first row
		input_itemsets[j] = rand() % 10 + 1;
	}
	
	for (int i = 1; i < max_rows; i++)
	{
		for (int j = 1; j < max_cols; j++)
		{
            int ref_offset = i * num_cols + (j-1);
            reference[ref_offset] = blosum62[input_itemsets[i * max_cols]][input_itemsets[j]];
		}
	}

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
    
    int nworkitems, workgroupsize = 0;
    nworkitems = BSIZE;
    
    if(nworkitems < 1 || workgroupsize < 0)
    {
        cout<<"ERROR: invalid or missing <num_work_items>\n";
        return; 
    }
    
    size_t local_work[3] = { (size_t)((workgroupsize>0)?workgroupsize:1), 1, 1 };
    size_t global_work[3] = { (size_t)nworkitems, 1, 1 };
    
	cl_program prog =  createProgramFromBitstream(ctx, appOptions.bitstreamFile, dev);

    // Extract the kernel
    cl_kernel nwkernel = clCreateKernel(prog, "nw", &err);
    CL_CHECK_ERROR(err);

    // Allocate host memory for reference data (reference)
    cl_mem reference_d = clCreateBuffer(ctx, CL_MEM_READ_ONLY | CL_CHANNEL_1_INTELFPGA, ref_size * sizeof(int), NULL, &err);
    CL_CHECK_ERROR(err);
    
    // Allocate host memory for input data (input_itemsets)
    int device_buff_size = ref_size;
    cl_mem input_itemsets_d = clCreateBuffer(ctx, CL_MEM_READ_WRITE | CL_CHANNEL_2_INTELFPGA, device_buff_size * sizeof(int), NULL, &err);
    CL_CHECK_ERROR(err);
    
    // Allocate
    cl_mem buffer_v_d = clCreateBuffer(ctx, CL_MEM_READ_ONLY | CL_CHANNEL_1_INTELFPGA, num_rows * sizeof(int), NULL, &err);
    CL_CHECK_ERROR(err);
    
    // write buffers
    err = clEnqueueWriteBuffer(queue, reference_d, 1, 0, ref_size * sizeof(int), reference, 0, 0, 0);
    CL_CHECK_ERROR(err);

    err = clFinish(queue);
    CL_CHECK_ERROR(err);

    err = clEnqueueWriteBuffer(queue, input_itemsets_d, 1, 0, num_cols * sizeof(int), buffer_h, 0, 0, 0);
    CL_CHECK_ERROR(err);

    err = clFinish(queue);
    CL_CHECK_ERROR(err);

    err = clEnqueueWriteBuffer(queue, buffer_v_d, 1, 0, num_rows * sizeof(int), buffer_v, 0, 0, 0);
    CL_CHECK_ERROR(err);

    err = clFinish(queue);
    CL_CHECK_ERROR(err);
    
    int worksize = max_cols - 1;
    cout<< "BSIZE = " << BSIZE << endl;
    cout<< "PAR = " << PAR << endl;
    cout<< "WG size of kernel = " << BSIZE << endl;
    cout<< "worksize = " << worksize << endl;
    int offset_r = 0, offset_c = 0;
    int block_width = worksize/BSIZE;
    
    // Set the kernel arguments
    int cols = num_cols - 1 + PAR;
    int exit_col = (cols % PAR == 0) ? cols : cols + PAR - (cols % PAR);
    int loop_exit = exit_col * (BSIZE / PAR);

    err = clSetKernelArg(nwkernel, 0, sizeof(void *), (void*) &reference_d);
    CL_CHECK_ERROR(err);
    err = clSetKernelArg(nwkernel, 1, sizeof(void *), (void*) &input_itemsets_d);
    CL_CHECK_ERROR(err);
    err = clSetKernelArg(nwkernel, 2, sizeof(void *), (void*) &buffer_v_d);
    CL_CHECK_ERROR(err);
    err = clSetKernelArg(nwkernel, 3, sizeof(cl_int), (void*) &num_cols);
    CL_CHECK_ERROR(err);
    err = clSetKernelArg(nwkernel, 4, sizeof(cl_int), (void*) &penalty);
    CL_CHECK_ERROR(err);
    err = clSetKernelArg(nwkernel, 5, sizeof(cl_int), (void*) &loop_exit);
    CL_CHECK_ERROR(err);
    
    int passes = appOptions.passes;

    for (int k = 0; k < passes; k++)
    {

        int th = Timer::Start();
        
        int num_diags  = max_rows - 1;
        int comp_bsize = BSIZE - 1;
        int last_diag  = (num_diags % comp_bsize == 0) ? num_diags : num_diags + comp_bsize - (num_diags % comp_bsize);
        int num_blocks = last_diag / comp_bsize;
        
        for (int bx = 0; bx < num_blocks; bx++)
        {
            int block_offset = bx * comp_bsize;

            err = clSetKernelArg(nwkernel, 6, sizeof(cl_int), (void*) &block_offset);
            CL_CHECK_ERROR(err);
            
            err = clEnqueueTask(queue, nwkernel, 0, NULL, NULL);
            CL_CHECK_ERROR(err);

            err = clFinish(queue);
            CL_CHECK_ERROR(err);
        }
    
        double totalNWTime = Timer::Stop(th, "total NW time");

        err = clEnqueueReadBuffer(queue, input_itemsets_d, 1, 0, ref_size * sizeof(int), output_itemsets, 0, 0, 0);
        CL_CHECK_ERROR(err);

        err = clFinish(queue);
        CL_CHECK_ERROR(err);
    
        // If answer is incorrect, stop test and do not report performance
        if (! nwCPU(reference, input_itemsets, output_itemsets, max_rows, max_cols , penalty))
        {
            return;
        }
        
        char atts[1024];
        sprintf(atts, "%d Elements", (dim-1) * (dim-1));
        double GigaElement = ( (double(dim-1) * double(dim-1)) / ((1000.) * (1000.) * (1000.)) );
        
        resultDB.AddResult("nw", "Needleman-Wunsch", atts, "GigaElement/s", GigaElement / totalNWTime);

    }
    // Clean up device memory
    err = clReleaseMemObject(reference_d);
    CL_CHECK_ERROR(err);
    err = clReleaseMemObject(input_itemsets_d);
    CL_CHECK_ERROR(err);

    // Clean up other host memory
    delete[] reference;

    err = clReleaseProgram(prog);
    CL_CHECK_ERROR(err);
    err = clReleaseKernel(nwkernel);
    CL_CHECK_ERROR(err);
}

