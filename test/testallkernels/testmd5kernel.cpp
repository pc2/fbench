/*
    Author: Akhtar, Junaid
    E-mail:  junaida@mail.uni-paderborn.de
    Date:   2020/03/30
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

// Md5 specific Implementation from BaseFixtureTest
class Md5KernelsTestFixture : public BaseTestFixture
{

protected:

    // ****************************************************************************
    // Function:  AsHex
    //
    // Purpose:
    ///   For a given key string, return the raw hex string for its bytes.
    //
    // Arguments:
    //   vals       key string
    //   len        length of key string
    //
    // Programmer:  Jeremy Meredith
    // Creation:    July 23, 2014
    //
    // Modifications:
    // ****************************************************************************
    std::string AsHex(unsigned char *vals, int len)
    {
        ostringstream out;
        char tmp[256];
        for (int i = 0; i < len; ++i)
        {
            sprintf(tmp, "%2.2X", vals[i]);
            out << tmp;
        }
        return out.str();
    }
};

struct Md5TestItem
{
    int keyspace;
    int byteLength;
    int valsPerByte;
    int index;
    string key;
    string digest;
    unsigned int searchDigest[4];
};


// Parameterized Tests implementation of Unit Testing MD5 Kernel with Test Fixtures
class Md5KernelsTestFixtureWithParam : public Md5KernelsTestFixture,
                                          public WithParamInterface<Md5TestItem>
{
};

// Value Parameterized Test with Test Fixture for executing md5 Unit Test
TEST_P(Md5KernelsTestFixtureWithParam, TestMd5)
{

    auto param = GetParam();

    // Check if Device Initilization was Successful or not
    ASSERT_EQ(CL_SUCCESS, t_clErr);
    int errNum = 0;

    auto iter = t_options.appsToRun.find(md5Hash);
    bool status = iter == t_options.appsToRun.end();
    if (status)
    {
        iter = t_options.appsToRun.find(all);
        ASSERT_TRUE(status == 1) << "Missing Benchmark Options";
    }

    ApplicationOptions appOptions = iter->second;

    cl_program fbenchProgram = createProgramFromBitstream(t_ctx, appOptions.bitstreamFile, t_dev);
    {
        cl_kernel md5kernel = clCreateKernel(fbenchProgram, "FindKeyWithDigest_Kernel", &errNum);
        ASSERT_FALSE(md5kernel == 0);
        ASSERT_EQ(CL_SUCCESS, errNum);

        // Allocate output buffers Values per Byte
        cl_mem d_foundIndex = clCreateBuffer(t_ctx, CL_MEM_READ_WRITE,
                                             sizeof(int) * 1, NULL, &errNum);
        ASSERT_EQ(CL_SUCCESS, errNum);

        cl_mem d_foundKey = clCreateBuffer(t_ctx, CL_MEM_READ_WRITE,
                                           8, NULL, &errNum);
        ASSERT_EQ(CL_SUCCESS, errNum);

        cl_mem d_foundDigest = clCreateBuffer(t_ctx, CL_MEM_READ_WRITE,
                                              sizeof(unsigned int) * 4, NULL, &errNum);
        ASSERT_EQ(CL_SUCCESS, errNum);

        int foundIndex = -1;
        unsigned char foundKey[8] = {0, 0, 0, 0, 0, 0, 0, 0};
        unsigned int foundDigest[4] = {0, 0, 0, 0};
        
        
        const int keyspace = param.keyspace;
        const int byteLength = param.byteLength;
        const int valsPerByte = param.valsPerByte;

        ASSERT_LE(byteLength, 7);

        const int index = param.index;
        string key = param.key;
        string digest = param.digest;

        // unsigned int searchDigest[] = param.searchDigest;

        // Initialize output buffers to return found result
        errNum = clEnqueueWriteBuffer(t_queue, d_foundIndex, true, 0,
                                      sizeof(int) * 1, &foundIndex,
                                      0, NULL, NULL);
        ASSERT_EQ(CL_SUCCESS, errNum);

        errNum = clEnqueueWriteBuffer(t_queue, d_foundKey, true, 0,
                                      8, foundKey,
                                      0, NULL, NULL);
        ASSERT_EQ(CL_SUCCESS, errNum);

        errNum = clEnqueueWriteBuffer(t_queue, d_foundDigest, true, 0,
                                      sizeof(int) * 4, foundDigest,
                                      0, NULL, NULL);
        ASSERT_EQ(CL_SUCCESS, errNum);

        // Set arguments for the kernel
        errNum = clSetKernelArg(md5kernel, 0, sizeof(unsigned int), (void *)&param.searchDigest[0]);
        ASSERT_EQ(CL_SUCCESS, errNum);
        errNum = clSetKernelArg(md5kernel, 1, sizeof(unsigned int), (void *)&param.searchDigest[1]);
        ASSERT_EQ(CL_SUCCESS, errNum);
        errNum = clSetKernelArg(md5kernel, 2, sizeof(unsigned int), (void *)&param.searchDigest[2]);
        ASSERT_EQ(CL_SUCCESS, errNum);
        errNum = clSetKernelArg(md5kernel, 3, sizeof(unsigned int), (void *)&param.searchDigest[3]);
        ASSERT_EQ(CL_SUCCESS, errNum);
        errNum = clSetKernelArg(md5kernel, 4, sizeof(int), (void *)&keyspace);
        ASSERT_EQ(CL_SUCCESS, errNum);
        errNum = clSetKernelArg(md5kernel, 5, sizeof(int), (void *)&byteLength);
        ASSERT_EQ(CL_SUCCESS, errNum);
        errNum = clSetKernelArg(md5kernel, 6, sizeof(int), (void *)&valsPerByte);
        ASSERT_EQ(CL_SUCCESS, errNum);
        errNum = clSetKernelArg(md5kernel, 7, sizeof(cl_mem), (void *)&d_foundIndex);
        ASSERT_EQ(CL_SUCCESS, errNum);
        errNum = clSetKernelArg(md5kernel, 8, sizeof(cl_mem), (void *)&d_foundKey);
        ASSERT_EQ(CL_SUCCESS, errNum);
        errNum = clSetKernelArg(md5kernel, 9, sizeof(cl_mem), (void *)&d_foundDigest);
        ASSERT_EQ(CL_SUCCESS, errNum);
        errNum = clFinish(t_queue);
        ASSERT_EQ(CL_SUCCESS, errNum);

        // Run the kernel
        cl_event event = NULL;

        errNum = clEnqueueTask(t_queue, md5kernel, 0, NULL,
                               &event);
        ASSERT_EQ(CL_SUCCESS, errNum);

        errNum = clFinish(t_queue);
        ASSERT_EQ(CL_SUCCESS, errNum);

        // Read the found Key and Digest
        errNum = clEnqueueReadBuffer(t_queue, d_foundIndex, true, 0,
                                     sizeof(int) * 1, &foundIndex,
                                     0, NULL, NULL);
        ASSERT_EQ(CL_SUCCESS, errNum);

        errNum = clEnqueueReadBuffer(t_queue, d_foundKey, true, 0,
                                     8, foundKey,
                                     0, NULL, NULL);
        ASSERT_EQ(CL_SUCCESS, errNum);

        errNum = clEnqueueReadBuffer(t_queue, d_foundDigest, true, 0,
                                     sizeof(int) * 4, foundDigest,
                                     0, NULL, NULL);
        ASSERT_EQ(CL_SUCCESS, errNum);

        errNum = clFinish(t_queue);
        ASSERT_EQ(CL_SUCCESS, errNum);

        // Check if found keyIndex matched
        ASSERT_EQ(foundIndex, index);

        // Check if found key matched
        string s_foundKey = AsHex(foundKey, 8);
        ASSERT_EQ(s_foundKey, key);

        // Check if found digest matched
        string s_foundDigest = AsHex((unsigned char *)foundDigest, 16);
        ASSERT_EQ(s_foundDigest, digest);

        // Free Memory
        errNum = clReleaseMemObject(d_foundIndex);
        ASSERT_EQ(CL_SUCCESS, errNum);
        errNum = clReleaseMemObject(d_foundKey);
        ASSERT_EQ(CL_SUCCESS, errNum);
        errNum = clReleaseMemObject(d_foundDigest);
        ASSERT_EQ(CL_SUCCESS, errNum);

        errNum = clReleaseKernel(md5kernel);
        ASSERT_EQ(CL_SUCCESS, errNum);
    }

    clReleaseProgram(fbenchProgram);

}; // md5KernelsWithParameters

// In order to run value-parameterized tests, we need to instantiate them,
// or bind them to a list of values which will be used as test parameters.
// We can instantiate them in a different translation module, or even
// instantiate them several times.

// Here to pass test names after fetching from arguments

// Here, we instantiate our tests with a list of two PrimeTable object factory functions
INSTANTIATE_TEST_CASE_P(TestBaseInstantiation, Md5KernelsTestFixtureWithParam,
                        Values(
                            Md5TestItem{27000, 3, 30, 22835, "050B190000000000", "BF48946A2D32A86AEE90556424184DA7", {1788102847, 1789407789, 1683329262, 2806847524}},
                            Md5TestItem{160000, 4, 20, 13167, "07120C0100000000", "ED1E790D32F2E72CD8638D6D5D03DDD1", {226041581, 753398322, 1837982680, 3520922461}},
                            Md5TestItem{1000000, 6, 10, 337540, "0004050703030000", "E3E0631BF124CD15DB417DC8962054DC", {459530467, 365765873, 3363652059, 3696500886}},
                            Md5TestItem{279936, 7, 6, 223060, "0400040004040400", "C37913433E85F59B8B9993AF66436251", {1125349827, 2616558910, 2945685899, 1365394278}},
                            Md5TestItem{59049, 5, 9, 3011, "0501010400000000", "830D8A4F12048448759511B01184BE16", {1334447491, 1216611346, 2953942389, 381584401}},
                            Md5TestItem{2025, 2, 45, 412, "0709000000000000", "8374F96CC135BEE7C02D129FBA5D6DFB", {1828287619, 3888002497, 2668768704, 4218248634}}
                        ));