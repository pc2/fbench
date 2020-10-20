/**
* @file mmhost.cpp 
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <gmp.h>

#include "../common/utility.h"
#include "../common/benchmarkoptions.h"

using namespace std;


double multiplyWithFPGA(cl_context ctx,
                        cl_command_queue queue,
                        cl_program prog,
                        cl_uint a[],
                        cl_uint b[],
                        cl_uint n[],
                        cl_uint n_[],
                        cl_uint *c,
                        cl_uint m_size)
{
    int err;

    //
    // find the kernel
    //
    cl_kernel mmkernel = clCreateKernel(prog, "Montgomery", &err);
    CL_CHECK_ERROR(err);

    //
    // allocate device memory for input buffers.
    // 
    cl_mem d_a = clCreateBuffer(ctx, CL_MEM_READ_ONLY,
                                          sizeof(cl_uint)*m_size, NULL, &err);
    CL_CHECK_ERROR(err);
    cl_mem d_b = clCreateBuffer(ctx, CL_MEM_READ_ONLY,
                                          sizeof(cl_uint)*m_size, NULL, &err);
    CL_CHECK_ERROR(err);
    cl_mem d_n = clCreateBuffer(ctx, CL_MEM_READ_ONLY,
                                          sizeof(cl_uint)*m_size, NULL, &err);
    CL_CHECK_ERROR(err);
    cl_mem d_n_ = clCreateBuffer(ctx, CL_MEM_READ_ONLY,
                                          sizeof(cl_uint)*m_size, NULL, &err);
    CL_CHECK_ERROR(err);

    //
    // allocate partial buffers for tl and m
    //

    cl_mem d_tl = clCreateBuffer(ctx, CL_MEM_READ_WRITE,
                                          sizeof(cl_uint)*(m_size*2), NULL, &err);
    CL_CHECK_ERROR(err);
    cl_mem d_m = clCreateBuffer(ctx, CL_MEM_READ_WRITE,
                                          sizeof(cl_uint)*(m_size*2), NULL, &err);
    CL_CHECK_ERROR(err);

    //
    // allocate output buffer for c
    //

    cl_mem d_c = clCreateBuffer(ctx, CL_MEM_READ_WRITE,
                                          sizeof(cl_uint)*m_size, NULL, &err);
    CL_CHECK_ERROR(err);

    //
    // write input buffers to the device memory.
    //
    err = clEnqueueWriteBuffer(queue, d_a, true, 0,
                               sizeof(cl_uint)*m_size, a,
                               0, NULL, NULL);
    CL_CHECK_ERROR(err);
    err = clEnqueueWriteBuffer(queue, d_b, true, 0,
                               sizeof(cl_uint)*m_size, b,
                               0, NULL, NULL);
    CL_CHECK_ERROR(err);
    err = clEnqueueWriteBuffer(queue, d_n, true, 0,
                               sizeof(cl_uint)*m_size, n,
                               0, NULL, NULL);
    CL_CHECK_ERROR(err);
    err = clEnqueueWriteBuffer(queue, d_n_, true, 0,
                               sizeof(cl_uint)*m_size, n_,
                               0, NULL, NULL);
    CL_CHECK_ERROR(err);

    err = clFinish(queue);
    CL_CHECK_ERROR(err);

    //
    // set arguments for the kernel
    //
    
    err = clSetKernelArg(mmkernel, 0, sizeof(cl_mem), (void*)&d_a);
    CL_CHECK_ERROR(err);
    err = clSetKernelArg(mmkernel, 1, sizeof(cl_mem), (void*)&d_b);
    CL_CHECK_ERROR(err);    
    err = clSetKernelArg(mmkernel, 2, sizeof(cl_mem), (void*)&d_n);
    CL_CHECK_ERROR(err);
    err = clSetKernelArg(mmkernel, 3, sizeof(cl_mem), (void*)&d_n_);
    CL_CHECK_ERROR(err);
    err = clSetKernelArg(mmkernel, 4, sizeof(cl_mem), (void*)&d_tl);
    CL_CHECK_ERROR(err);
    err = clSetKernelArg(mmkernel, 5, sizeof(cl_mem), (void*)&d_m);
    CL_CHECK_ERROR(err);
    err = clSetKernelArg(mmkernel, 6, sizeof(cl_mem), (void*)&d_c);
    CL_CHECK_ERROR(err);
    // err = clSetKernelArg(mmkernel, 7, sizeof(cl_uint), (void*)&m_size);
    // CL_CHECK_ERROR(err);

    //
    // calculate work thread shape
    //
    

    //
    // run the kernel
    //
    double nanosec = 0;
    cl_event event = NULL;

    //err = clEnqueueTask(queue, mmkernel, 0, NULL,
    //                            &event);
    
    // Number of local work items per group
    const size_t local_wsize  = m_size;
    // Number of global work items
    const size_t global_wsize = m_size;

    err = clEnqueueNDRangeKernel(queue, mmkernel, 1, NULL,
                                &global_wsize, &local_wsize, 0, NULL, NULL);
    CL_CHECK_ERROR(err);

    err = clFinish(queue);
    CL_CHECK_ERROR (err);

    //
    // get the timing/rate info
    //
    cl_ulong submitTime;
    cl_ulong endTime;

    //err = clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_SUBMIT,
    //                                sizeof(cl_ulong), &submitTime, NULL);
    CL_CHECK_ERROR(err);
    
    //err = clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END,
      //                              sizeof(cl_ulong), &endTime, NULL);
    CL_CHECK_ERROR(err);

    nanosec = endTime - submitTime;
    
    //
    // read the output buffer
    //
    CL_CHECK_ERROR(err);
    err = clEnqueueReadBuffer(queue, d_c, true, 0,
                              sizeof(cl_uint)*m_size, &c,
                              0, NULL, NULL);
    CL_CHECK_ERROR(err);

    err = clFinish(queue);
    CL_CHECK_ERROR(err);

    //
    // free device memory
    //
    err = clReleaseMemObject(d_a);
    CL_CHECK_ERROR(err);
    
    err = clReleaseMemObject(d_b);
    CL_CHECK_ERROR(err);

    err = clReleaseMemObject(d_n);
    CL_CHECK_ERROR(err);

    err = clReleaseMemObject(d_n_);
    CL_CHECK_ERROR(err);

    err = clReleaseMemObject(d_tl);
    CL_CHECK_ERROR(err);

    err = clReleaseMemObject(d_m);
    CL_CHECK_ERROR(err);

    err = clReleaseMemObject(d_c);
    CL_CHECK_ERROR(err);

    // What metric? operations/second

    //
    // return the runtime in seconds
    //

    return nanosec / 1.e9;

    err = clReleaseKernel(mmkernel);
    CL_CHECK_ERROR(err);
}



// ****************************************************************************
// Function: RunBenchmark
//
// Purpose:
//   Executes the Montgomery Multiplication benchmark
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
// Programmer: Akhtar, Junaid
// Creation:   July 03, 2020
//
// ****************************************************************************

void
benchmarkMM(cl_device_id dev,
                    cl_context ctx,
                    cl_command_queue queue,
                    BenchmarkDatabase &resultDB,
                    BenchmarkOptions  &options)
{

    auto iter = options.appsToRun.find(mm);
    if (iter == options.appsToRun.end())
    {   
        iter = options.appsToRun.find(all);
        cerr << "ERROR: Could not find montgomery multipication benchmark options";
        return; 
    }

    ApplicationOptions appOptions = iter->second;

    // TODO: Input settings to the benchmark check

    if (options.verbose)
        cout << "Creating program from the montgomery multipication bitstream." << endl;

    cl_program program = createProgramFromBitstream(ctx,
                                                    appOptions.bitstreamFile, 
                                                    dev);
    
    if (appOptions.size < 1 || appOptions.size > 6)
    {
        cerr << "ERROR: Invalid size parameter\n";
        return;
    }

    //
    // determine multipication size
    //

    const unsigned int integer_sizes[]  = {256, 512, 1024, 2048, 4096, 8192};
    const unsigned int polynomials[6][2] = {{255,82}, {511,216}, {1014,385}, {2044,45}, {4074,595}, {8145,728}};

    unsigned int bitWidth = integer_sizes[appOptions.size-1];

    //
    // select Random A and B
    //
    mpz_t a, b, a_, b_;
    gmp_randstate_t state;
    mpz_inits(a, b, a_, b_, NULL);
    gmp_randinit_mt(state);
    mpz_urandomb(a, state, bitWidth);
    mpz_urandomb(b, state, bitWidth);

    //
    // calculate N from irreducible polynomials
    //
    mpz_t  n, n_prime, temp_x, temp_y;

    mpz_init2(n, bitWidth);
    mpz_inits(n_prime, temp_x, temp_y, NULL);
    mpz_ui_pow_ui(temp_x, 2, polynomials[appOptions.size-1][0]);
    mpz_ui_pow_ui(temp_y, 2, polynomials[appOptions.size-1][1]);
    mpz_add(n, temp_x, temp_y);
    mpz_add_ui(n, n, 1);

    // find number of limbs of N
    cl_uint size = bitWidth / 32;

    // gmp_printf("%Zd\n", n);
    
    //
    // select random R where R > N and coprime with N
    //
    mpz_t r, r_inverse, gcd;
    mpz_inits(r, r_inverse, gcd, NULL);
    // while((mpz_cmp_ui(gcd,1) != 0) && (mpz_cmp(r,n) < 1)){
        // mpz_urandomb(r, state, (bitWidth));
        mpz_ui_pow_ui(r,2,(size+1)*32);
        mpz_gcd(gcd, r, n);
    // }
    // gmp_printf("GCD: %Zd\n", gcd);

    //
    // calculate R inverse
    //
    mpz_invert(r_inverse, r, n);

    //
    // calculate N' - R.R^-1 = N.N' + 1
    //
    mpz_mul(n_prime, r, r_inverse);
    mpz_sub_ui(n_prime, n_prime, 1);
    mpz_divexact(n_prime, n_prime, n);


    //
    // transform A and B to Montgomery space
    //
    mpz_t temp_a, temp_b;
    mpz_inits(temp_a, temp_b, NULL);
    mpz_mul(temp_a, a, r);
    mpz_mod(a_, temp_a, n);

    mpz_mul(temp_b, b, r);
    mpz_mod(b_, temp_b, n);

    /*
    cout << "Actual Size: \n" << size << endl;
    cout << "Size A: \n" << a_->_mp_size << endl;
    cout << "Size B: \n" << b_->_mp_size << endl;
    cout << "Size N: \n" << n->_mp_size << endl;
    cout << "Size N': \n" << n_prime->_mp_size << endl;
    */
    
    //
    // Split A,B,N,N' into limbs of 32bit integer
    //
    cl_uint _a[size], _b[size], _n[size], _n_prime[size], _c[size];
    int j=0;
    for(int i=0; i<size/2; i++){

        _a[j] = (cl_uint)((a_->_mp_d[i] & 0xFFFFFFFF00000000LL) >> 32);
        _a[j+1] = (cl_uint)(a_->_mp_d[i] & 0xFFFFFFFFLL);

        _b[j] = (cl_uint)((b_->_mp_d[i] & 0xFFFFFFFF00000000LL) >> 32);
        _b[j+1] = (cl_uint)(b_->_mp_d[i] & 0xFFFFFFFFLL);

        _n[j] = (cl_uint)((n->_mp_d[i] & 0xFFFFFFFF00000000LL) >> 32);
        _n[j+1] = (cl_uint)(n->_mp_d[i] & 0xFFFFFFFFLL);

        _n_prime[j] = (cl_uint)((n_prime->_mp_d[i] & 0xFFFFFFFF00000000LL) >> 32);
        _n_prime[j+1] = (cl_uint)(n_prime->_mp_d[i] & 0xFFFFFFFFLL);

        j=+2;
    }

    //
    // calculate A * B (mod) N on CPU - School Book Method
    //
    mpz_t m, c, c_;
    mpz_inits(m, c, NULL);
    mpz_mul(m, a, b);
    mpz_mod(c, m, n);

    //
    // calculate A * B (mod) N on FPGA
    //
    multiplyWithFPGA(ctx, queue, program,_a,_b,_n,_n_prime,_c,size);    

    //
    // transform C from montgomery space
    //
    mpz_t m_, c_fpga;
    mpz_inits(m_, c_, c_fpga, NULL);
    mpz_urandomb(c_, state, bitWidth);
    
    j = 0;
    for(int i=0; i<size/2; i++){
        c_->_mp_d[i] = ((unsigned long)_c[j]) << 32 | _c[j+1];
        j=+2;
    }

    mpz_mul(m_, c_, r_inverse);
    mpz_mod(c_fpga, m_, n);

    gmp_printf("FPGA: %Zd\n", c_fpga);
    gmp_printf("CPU: %Zd\n", c);


    //
    // Clear memeory
    //
    mpz_clear(a);
    mpz_clear(b);
    mpz_clear(a_);
    mpz_clear(b_);
    mpz_clear(c);
    mpz_clear(c_);
    mpz_clear(c_fpga);
    mpz_clear(n);
    mpz_clear(n_prime);
    mpz_clear(m);
    mpz_clear(m_);
    mpz_clear(r);
    mpz_clear(r_inverse);
    gmp_randclear(state);
    mpz_clear(gcd);
    mpz_clear(temp_x);
    mpz_clear(temp_y);
    mpz_clear(temp_a);
    mpz_clear(temp_b);
    
    clReleaseProgram(program);

}
