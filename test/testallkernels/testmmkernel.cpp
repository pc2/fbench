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

// MontgomerMultiplication specific Implementation from BaseFixtureTest
class MontgomerMultiplicationKernelsTestFixture : public BaseTestFixture
{
};

struct MontgomerMultiplicationTestItem
{
    int size;
};

// Parameterized Tests implementation of Unit Testing ScMontgomerMultiplicationan with Test Fixtures

class MontgomerMultiplicationKernelsTestFixtureWithParam : public MontgomerMultiplicationKernelsTestFixture,
                                        public WithParamInterface<MontgomerMultiplicationTestItem>
{
};

// Value Parameterized Test with Test Fixture for executing MontgomerMultiplication Unit Test
TEST_P(MontgomerMultiplicationKernelsTestFixtureWithParam, TestMontgomerMultiplication)
{

    auto param = GetParam();
    // Check if Device Initilization was Successful or not
    ASSERT_EQ(CL_SUCCESS, t_clErr);
    int errNum = 0;

    auto iter = t_options.appsToRun.find(mm);
    bool status = iter == t_options.appsToRun.end();
    if (status)
    {
        iter = t_options.appsToRun.find(all);
        ASSERT_TRUE(status == 1) << "Missing Benchmark Options";
    }

    ApplicationOptions appOptions = iter->second;

    cl_program fbenchProgram = createProgramFromBitstream(t_ctx, appOptions.bitstreamFile, t_dev);
    {
        cl_kernel mmkernel = clCreateKernel(fbenchProgram, "mm", &errNum);
        ASSERT_FALSE(mmkernel == 0);
        ASSERT_EQ(CL_SUCCESS, errNum);

        // To-do code start
        int size = param.size;

        errNum = clReleaseKernel(mmkernel);
        ASSERT_EQ(CL_SUCCESS, errNum);
    }

    errNum = clReleaseProgram(fbenchProgram);
    ASSERT_EQ(CL_SUCCESS, errNum);

}; // MontgomerMultiplicationKernelsWithParameters

// In order to run value-parameterized tests, we need to instantiate them,
// or bind them to a list of values which will be used as test parameters.
// We can instantiate them in a different translation module, or even
// instantiate them several times.

// Here to pass test names after fetching from arguments

// Here, we instantiate our tests with a list of two PrimeTable object factory functions
INSTANTIATE_TEST_CASE_P(TestBaseInstantiation, MontgomerMultiplicationKernelsTestFixtureWithParam,
                        Values(
                            MontgomerMultiplicationTestItem{8},
                            MontgomerMultiplicationTestItem{16},
                            MontgomerMultiplicationTestItem{32},
                            MontgomerMultiplicationTestItem{64}));