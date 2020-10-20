/*

Copyright (c) 2011, UT-Battelle, LLC
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.
* Neither the name of Oak Ridge National Laboratory, nor UT-Battelle, LLC, nor
  the names of its contributors may be used to endorse or promote products
  derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

/** @file md5host.cpp
*/
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>

#include "../common/utility.h"
#include "../common/benchmarkoptions.h"

using namespace std;

/// leftrotate function definition
#define LEFTROTATE(x, c) (((x) << (c)) | ((x) >> (32 - (c))))

#define F(x,y,z) ((x & y) | ((~x) & z))
#define G(x,y,z) ((x & z) | ((~z) & y))
#define H(x,y,z) (x ^ y ^ z)
#define I(x,y,z) (y ^ (x | (~z)))

/// This version of the round shifts the interpretation of a,b,c,d by one
/// and must be called with v/x/y/z in a matching shuffle pattern.
/// Every four Rounds, a,b,c,d are back to their original interpretation,
/// though, so it all works out in the end (we have 64 rounds per block).
#define ROUND_INPLACE_VIA_SHIFT(w, r, k, v, x, y, z, func)       \
{                                                                \
    v += func(x,y,z) + w + k;                                    \
    v = x + LEFTROTATE(v, r);                                    \
}

/// This version ignores the mapping of a/b/c/d to v/x/y/z and simply
/// uses a temporary variable to keep the interpretation of a/b/c/d
/// consistent.  Whether this one or the previous one performs better
/// probably depends on the compiler....
#define ROUND_USING_TEMP_VARS(w, r, k, v, x, y, z, func)         \
{                                                                \
    a = a + func(b,c,d) + k + w;                                 \
    unsigned int temp = d;                                       \
    d = c;                                                       \
    c = b;                                                       \
    b = b + LEFTROTATE(a, r);                                    \
    a = temp;                                                    \
}

// Here, we pick which style of ROUND we use.
#define ROUND ROUND_USING_TEMP_VARS
//#define ROUND ROUND_INPLACE_VIA_SHIFT

/// @note This really only allows a length up to 7 bytes, not 8, because
/// we need to start the padding in the first byte following the message,
/// and we only have two words to work with here....
/// It also assumes words[] has all zero bits except the chars of interest.
inline void md5_2words(unsigned int *words, unsigned int len,
                       unsigned int *digest)
{
    // For any block but the first one, these should be passed in, not
    // initialized, but we are assuming we only operate on a single block.
    unsigned int h0 = 0x67452301;
    unsigned int h1 = 0xefcdab89;
    unsigned int h2 = 0x98badcfe;
    unsigned int h3 = 0x10325476;

    unsigned int a = h0;
    unsigned int b = h1;
    unsigned int c = h2;
    unsigned int d = h3;

    unsigned int WL = len * 8;
    unsigned int W0 = words[0];
    unsigned int W1 = words[1];

    switch (len)
    {
      case 0: W0 |= 0x00000080; break;
      case 1: W0 |= 0x00008000; break;
      case 2: W0 |= 0x00800000; break;
      case 3: W0 |= 0x80000000; break;
      case 4: W1 |= 0x00000080; break;
      case 5: W1 |= 0x00008000; break;
      case 6: W1 |= 0x00800000; break;
      case 7: W1 |= 0x80000000; break;
      default: printf("ERROR, ONLY SUPPORT UP TO 7 BYTES IN THIS FUNC\n"); break;
    }

    /// @param word data, per-round shift amt, constant, 4 vars, function macro
    ROUND(W0,   7, 0xd76aa478, a, b, c, d, F);
    ROUND(W1,  12, 0xe8c7b756, d, a, b, c, F);
    ROUND(0,   17, 0x242070db, c, d, a, b, F);
    ROUND(0,   22, 0xc1bdceee, b, c, d, a, F);
    ROUND(0,    7, 0xf57c0faf, a, b, c, d, F);
    ROUND(0,   12, 0x4787c62a, d, a, b, c, F);
    ROUND(0,   17, 0xa8304613, c, d, a, b, F);
    ROUND(0,   22, 0xfd469501, b, c, d, a, F);
    ROUND(0,    7, 0x698098d8, a, b, c, d, F);
    ROUND(0,   12, 0x8b44f7af, d, a, b, c, F);
    ROUND(0,   17, 0xffff5bb1, c, d, a, b, F);
    ROUND(0,   22, 0x895cd7be, b, c, d, a, F);
    ROUND(0,    7, 0x6b901122, a, b, c, d, F);
    ROUND(0,   12, 0xfd987193, d, a, b, c, F);
    ROUND(WL,  17, 0xa679438e, c, d, a, b, F);
    ROUND(0,   22, 0x49b40821, b, c, d, a, F);

    ROUND(W1,   5, 0xf61e2562, a, b, c, d, G);
    ROUND(0,    9, 0xc040b340, d, a, b, c, G);
    ROUND(0,   14, 0x265e5a51, c, d, a, b, G);
    ROUND(W0,  20, 0xe9b6c7aa, b, c, d, a, G);
    ROUND(0,    5, 0xd62f105d, a, b, c, d, G);
    ROUND(0,    9, 0x02441453, d, a, b, c, G);
    ROUND(0,   14, 0xd8a1e681, c, d, a, b, G);
    ROUND(0,   20, 0xe7d3fbc8, b, c, d, a, G);
    ROUND(0,    5, 0x21e1cde6, a, b, c, d, G);
    ROUND(WL,   9, 0xc33707d6, d, a, b, c, G);
    ROUND(0,   14, 0xf4d50d87, c, d, a, b, G);
    ROUND(0,   20, 0x455a14ed, b, c, d, a, G);
    ROUND(0,    5, 0xa9e3e905, a, b, c, d, G);
    ROUND(0,    9, 0xfcefa3f8, d, a, b, c, G);
    ROUND(0,   14, 0x676f02d9, c, d, a, b, G);
    ROUND(0,   20, 0x8d2a4c8a, b, c, d, a, G);

    ROUND(0,    4, 0xfffa3942, a, b, c, d, H);
    ROUND(0,   11, 0x8771f681, d, a, b, c, H);
    ROUND(0,   16, 0x6d9d6122, c, d, a, b, H);
    ROUND(WL,  23, 0xfde5380c, b, c, d, a, H);
    ROUND(W1,   4, 0xa4beea44, a, b, c, d, H);
    ROUND(0,   11, 0x4bdecfa9, d, a, b, c, H);
    ROUND(0,   16, 0xf6bb4b60, c, d, a, b, H);
    ROUND(0,   23, 0xbebfbc70, b, c, d, a, H);
    ROUND(0,    4, 0x289b7ec6, a, b, c, d, H);
    ROUND(W0,  11, 0xeaa127fa, d, a, b, c, H);
    ROUND(0,   16, 0xd4ef3085, c, d, a, b, H);
    ROUND(0,   23, 0x04881d05, b, c, d, a, H);
    ROUND(0,    4, 0xd9d4d039, a, b, c, d, H);
    ROUND(0,   11, 0xe6db99e5, d, a, b, c, H);
    ROUND(0,   16, 0x1fa27cf8, c, d, a, b, H);
    ROUND(0,   23, 0xc4ac5665, b, c, d, a, H);

    ROUND(W0,   6, 0xf4292244, a, b, c, d, I);
    ROUND(0,   10, 0x432aff97, d, a, b, c, I);
    ROUND(WL,  15, 0xab9423a7, c, d, a, b, I);
    ROUND(0,   21, 0xfc93a039, b, c, d, a, I);
    ROUND(0,    6, 0x655b59c3, a, b, c, d, I);
    ROUND(0,   10, 0x8f0ccc92, d, a, b, c, I);
    ROUND(0,   15, 0xffeff47d, c, d, a, b, I);
    ROUND(W1,  21, 0x85845dd1, b, c, d, a, I);
    ROUND(0,    6, 0x6fa87e4f, a, b, c, d, I);
    ROUND(0,   10, 0xfe2ce6e0, d, a, b, c, I);
    ROUND(0,   15, 0xa3014314, c, d, a, b, I);
    ROUND(0,   21, 0x4e0811a1, b, c, d, a, I);
    ROUND(0,    6, 0xf7537e82, a, b, c, d, I);
    ROUND(0,   10, 0xbd3af235, d, a, b, c, I);
    ROUND(0,   15, 0x2ad7d2bb, c, d, a, b, I);
    ROUND(0,   21, 0xeb86d391, b, c, d, a, I);

    h0 += a;
    h1 += b;
    h2 += c;
    h3 += d;

    // Write the final result out
    digest[0] = h0;
    digest[1] = h1;
    digest[2] = h2;
    digest[3] = h3;
}

/****************************************************************************
* Function: FindKeyspaceSize()
*
* Purpose: Multiply out the byteLength by valsPerByte to find the total size 
* of the key space, with error checking.
*
* @param byteLength number of bytes in a key
* @param valsPerByte number of values each byte can take on
*
* @author Jeremy Meredith
* @date July 23, 2014
*
* Modifications:
****************************************************************************/
int FindKeyspaceSize(int byteLength, int valsPerByte)
{
    int keyspace = 1;
    for (int i=0; i<byteLength; ++i)
    {
        if (keyspace >= 0x7fffffff / valsPerByte)
        {
            /// error, we're about to overflow a signed int
            return -1;
        }
        keyspace *= valsPerByte;
    }
    return keyspace;
}

/****************************************************************************
* Function: IndexToKey()
*
* Purpose: For a given index in the keyspace, find the actual key string which 
* is at that index.
*
* @param index index in key space
* @param byteLength number of bytes in a key
* @param valsPerByte number of values each byte can take on
* @param vals output key string
*
* @author Jeremy Meredith
* @date July 23, 2014
*
* Modifications:
****************************************************************************/
void IndexToKey(unsigned int index, int byteLength, int valsPerByte,
                unsigned char vals[8])
{
    // Loop pointlessly unrolled to avoid CUDA compiler complaints
    // about unaligned accesses (!?) on older compute capabilities.
    vals[0] = index % valsPerByte;
    index /= valsPerByte;

    vals[1] = index % valsPerByte;
    index /= valsPerByte;

    vals[2] = index % valsPerByte;
    index /= valsPerByte;

    vals[3] = index % valsPerByte;
    index /= valsPerByte;

    vals[4] = index % valsPerByte;
    index /= valsPerByte;

    vals[5] = index % valsPerByte;
    index /= valsPerByte;

    vals[6] = index % valsPerByte;
    index /= valsPerByte;

    vals[7] = index % valsPerByte;
    index /= valsPerByte;
}


/****************************************************************************
* Function: AsHex()
*
* Purpose: For a given key string, return the raw hex string for its bytes.
*
* @param vals key string
* @param len length of key string
*
* @author Jeremy Meredith
* @date July 23, 2014
*
* Modifications:
****************************************************************************/
std::string AsHex(unsigned char *vals, int len)
{
    ostringstream out;
    char tmp[256];
    for (int i=0; i<len; ++i)
    {
        sprintf(tmp, "%2.2X", vals[i]);
        out << tmp;
    }
    return out.str();
}

/****************************************************************************
* Function: findKeyWithDigestFPGA()
*
* Purpose: On the FPGA, search the key space to find a key with the given digest.
*
* @param ctx the opencl context to use for the benchmark
* @param queue the opencl command queue to issue commands to
* @param prog the opencl program containing the kernel
* @param searchDigest the digest to search for
* @param byteLength number of bytes in a key
* @param valsPerByte number of values each byte can take on
* @param foundIndex output - the index of the found key (if found)
* @param foundKey output - the string of the found key (if found)
* @param foundDigest output - the digest of the found key (if found)
*
* @author Jeremy Meredith
* @date July 23, 2014
*
* Modifiers: Abdul Rehman
*****************************************************************************/
double findKeyWithDigestFPGA(cl_context ctx,
                             cl_command_queue queue,
                             cl_program prog,
                             const unsigned int searchDigest[4],
                             const int byteLength,
                             const int valsPerByte,
                             int *foundIndex,
                             unsigned char foundKey[8],
                             unsigned int foundDigest[4])
{
    int err;
    int keyspace = FindKeyspaceSize(byteLength, valsPerByte);

    //
    // find the kernel
    //
    cl_kernel md5kernel = clCreateKernel(prog, "FindKeyWithDigest_Kernel", &err);
    CL_CHECK_ERROR(err);

    //
    // allocate output buffers
    // valsPerByte
    cl_mem d_foundIndex = clCreateBuffer(ctx, CL_MEM_READ_WRITE,
                                         sizeof(int)*1, NULL, &err);
    CL_CHECK_ERROR(err);

    cl_mem d_foundKey = clCreateBuffer(ctx, CL_MEM_READ_WRITE,
                                       8, NULL, &err);
    CL_CHECK_ERROR(err);

    cl_mem d_foundDigest = clCreateBuffer(ctx, CL_MEM_READ_WRITE,
                                          sizeof(unsigned int)*4, NULL, &err);
    CL_CHECK_ERROR(err);

    //
    // initialize output buffers to show no found result
    //
    err = clEnqueueWriteBuffer(queue, d_foundIndex, true, 0,
                               sizeof(int)*1, foundIndex,
                               0, NULL, NULL);
    CL_CHECK_ERROR(err);
    err = clEnqueueWriteBuffer(queue, d_foundKey, true, 0,
                               8, foundKey,
                               0, NULL, NULL);
    CL_CHECK_ERROR(err);
    err = clEnqueueWriteBuffer(queue, d_foundDigest, true, 0,
                               sizeof(int)*4, foundDigest,
                               0, NULL, NULL);
    CL_CHECK_ERROR(err);

    err = clFinish(queue);
    CL_CHECK_ERROR(err);

    //
    // set arguments for the kernel
    //
    err = clSetKernelArg(md5kernel, 0, sizeof(unsigned int), (void*)&searchDigest[0]);
    CL_CHECK_ERROR(err);
    err = clSetKernelArg(md5kernel, 1, sizeof(unsigned int), (void*)&searchDigest[1]);
    CL_CHECK_ERROR(err);
    err = clSetKernelArg(md5kernel, 2, sizeof(unsigned int), (void*)&searchDigest[2]);
    CL_CHECK_ERROR(err);
    err = clSetKernelArg(md5kernel, 3, sizeof(unsigned int), (void*)&searchDigest[3]);
    CL_CHECK_ERROR(err);
    err = clSetKernelArg(md5kernel, 4, sizeof(int), (void*)&keyspace);
    CL_CHECK_ERROR(err);
    err = clSetKernelArg(md5kernel, 5, sizeof(int), (void*)&byteLength);
    CL_CHECK_ERROR(err);
    err = clSetKernelArg(md5kernel, 6, sizeof(int), (void*)&valsPerByte);
    CL_CHECK_ERROR(err);
    err = clSetKernelArg(md5kernel, 7, sizeof(cl_mem), (void*)&d_foundIndex);
    CL_CHECK_ERROR(err);
    err = clSetKernelArg(md5kernel, 8, sizeof(cl_mem), (void*)&d_foundKey);
    CL_CHECK_ERROR(err);
    err = clSetKernelArg(md5kernel, 9, sizeof(cl_mem), (void*)&d_foundDigest);
    CL_CHECK_ERROR(err);

    //
    // calculate work thread shape
    //
    size_t nthreads = 256;
    size_t nblocks  = ceil((double(keyspace) / double(valsPerByte)) / double(nthreads));
    size_t globalsize = nblocks * nthreads;

    //
    // run the kernel
    //
    double nanosec = 0;
    cl_event event = NULL;

    err = clEnqueueTask(queue, md5kernel, 0, NULL,
                                 &event);
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
    
    double rate = double(keyspace) / double(nanosec);
    // cout << "rate = " << rate << " GHash/sec" << endl;


    //
    // read the (presumably) found key
    //
    err = clEnqueueReadBuffer(queue, d_foundIndex, true, 0,
                              sizeof(int)*1, foundIndex,
                              0, NULL, NULL);
    CL_CHECK_ERROR(err);
    err = clEnqueueReadBuffer(queue, d_foundKey, true, 0,
                              8, foundKey,
                              0, NULL, NULL);
    CL_CHECK_ERROR(err);
    err = clEnqueueReadBuffer(queue, d_foundDigest, true, 0,
                              sizeof(int)*4, foundDigest,
                              0, NULL, NULL);
    CL_CHECK_ERROR(err);

    err = clFinish(queue);
    CL_CHECK_ERROR(err);

    //
    // free device memory
    //
    err = clReleaseMemObject(d_foundIndex);
    CL_CHECK_ERROR(err);
    err = clReleaseMemObject(d_foundKey);
    CL_CHECK_ERROR(err);
    err = clReleaseMemObject(d_foundDigest);
    CL_CHECK_ERROR(err);

    //
    // return the runtime in seconds
    //
    return nanosec / 1.e9;
}

/****************************************************************************
* Function: benchmarkMd5()
*
* Purpose: Executes the MD5 Hash benchmark
*
* @param dev the opencl device id to use for the benchmark
* @param ctx the opencl context to use for the benchmark
* @param queue the opencl command queue to issue commands to
* @param resultDB results from the benchmark are stored in this db
* @param op the options parser / parameter database
*
* @returns Nothing
*
* @author Jeremy Meredith
* @date July 23, 2014
*
* Modifiers: Abdul Rehman
****************************************************************************/
void benchmarkMd5(cl_device_id dev,
                    cl_context ctx,
                    cl_command_queue queue,
                    BenchmarkDatabase &resultDB,
                    BenchmarkOptions  &options)
{
    auto iter = options.appsToRun.find(md5Hash);
    if (iter == options.appsToRun.end())
    {   
        iter = options.appsToRun.find(all);
        cerr << "ERROR: Could not find benchmark options";
        return; 
    }

    ApplicationOptions appOptions = iter->second;

    // TODO: After refactoring of the options to object based model 
    // we will use something i.e.: op.size.

    if (appOptions.size < 1 || appOptions.size > 4)
    {
        cerr << "ERROR: Invalid size parameter\n";
        return;
    }

    if (options.verbose)
        cout << "Creating program from md5 bitstream." << endl;

    // TODO: Use the hardcoded value from the options.
    cl_program program = createProgramFromBitstream(ctx,
                                                    appOptions.bitstreamFile, 
                                                    dev);
    //
    // Determine the shape/size of key space.
    //
    const int sizes_byteLength[]  = { 7,  5,  6,  5};
    const int sizes_valsPerByte[] = {10, 30, 25, 70};

    const int byteLength = sizes_byteLength[appOptions.size-1];
    const int valsPerByte = sizes_valsPerByte[appOptions.size-1];

    char atts[1024];
    sprintf(atts, "%dx%d", byteLength, valsPerByte);

    if (options.verbose)
    {
        cout << "Searching keys of length " << byteLength << " bytes "
             << "and " << valsPerByte << " values per byte" << endl;
    }       

    const int keyspace = FindKeyspaceSize(byteLength, valsPerByte);

    if (keyspace < 0)
    {
        cerr << "Error: more than 2^31 bits of entropy is unsupported.\n";
        return;
    }

    if (byteLength > 7)
    {
        cerr << "Error: more than 7 byte key length is unsupported.\n";
        return;
    }

    if (options.verbose)
    {
        cout << "|keyspace| = " << keyspace << " ("<<int(keyspace/1e6)<<"M)" << endl;
    }    

    //
    // Choose a random key from the keyspace, and calculate its hash.
    //
    srandom(time(NULL));

    for (int pass = 0 ; pass < appOptions.passes ; ++pass)
    {
        int randomIndex = random() % keyspace;;
        unsigned char randomKey[8] = {0,0,0,0, 0,0,0,0};
        unsigned int randomDigest[4];

        IndexToKey(randomIndex, byteLength, valsPerByte, randomKey);
        md5_2words((unsigned int*)randomKey, byteLength, randomDigest);

        if (options.verbose)
        {
            cout << endl;
            cout << "--- pass " << pass << " ---" << endl;
            cout << "Looking for random key:" << endl;
            cout << " randomIndex = " << randomIndex << endl;
            cout << " randomKey   = 0x" << AsHex(randomKey, 8/*byteLength*/) << endl;
            cout << " randomDigest= " << AsHex((unsigned char*)randomDigest, 16) << endl;
        }

        //
        // Use the FPGA to brute force search the keyspace for this key.
        //
        unsigned int foundDigest[4] = {0,0,0,0};
        int foundIndex = -1;
        unsigned char foundKey[8] = {0,0,0,0, 0,0,0,0};

        // in seconds.
        double t = findKeyWithDigestFPGA(ctx, queue, program,
                                        randomDigest, byteLength, valsPerByte,
                                        &foundIndex, foundKey, foundDigest);
        
        //
        // Calculate the rate and add it to the results.
        //
        double rate = (double(keyspace) / double(t)) / 1.e9;
        if (options.verbose)
        {
            cout << "time = " << t << " sec, rate = " << rate << " GHash/sec\n";
        }

        //
        // Double check everything matches (index, key, hash).
        //
        if (foundIndex < 0)
        {
            cerr << "\nERROR: could not find a match.\n";
            rate = FLT_MAX;
        }
        else if (foundIndex != randomIndex)
        {
            cerr << "\nERROR: mismatch in key index found.\n";
            rate = FLT_MAX;
        }
        else if (foundKey[0] != randomKey[0] ||
            foundKey[1] != randomKey[1] ||
            foundKey[2] != randomKey[2] ||
            foundKey[3] != randomKey[3] ||
            foundKey[4] != randomKey[4] ||
            foundKey[5] != randomKey[5] ||
            foundKey[6] != randomKey[6] ||
            foundKey[7] != randomKey[7])
        {
            cerr << "\nERROR: mismatch in key value found.\n";
            rate = FLT_MAX;
        }
        else if (foundDigest[0] != randomDigest[0] ||
            foundDigest[1] != randomDigest[1] ||
            foundDigest[2] != randomDigest[2] ||
            foundDigest[3] != randomDigest[3])
        {
            cerr << "\nERROR: mismatch in digest of key.\n";
            rate = FLT_MAX;
        }
        else
        {
            if (options.verbose)
                cout << endl << "Successfully found match (index, key, hash):" << endl;
        }

        //
        // Add the calculated performancethe results.
        //
        resultDB.AddResult("md5", "md5", atts, "GHash/s", rate);

        if (options.verbose)
        {
            cout << " foundIndex  = " << foundIndex << endl;
            cout << " foundKey    = 0x" << AsHex(foundKey, 8/*byteLength*/) << endl;
            cout << " foundDigest = " << AsHex((unsigned char*)foundDigest, 16) << endl;
            cout << endl;
        }
    }

    return;
}
