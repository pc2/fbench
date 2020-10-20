/*
    Author: Akhtar, Junaid
    E-mail:  junaida@mail.uni-paderborn.de
    Date:   2020/04/21
*/

#ifndef DEVICE_H_
#define DEVICE_H_

#include <gtest/gtest.h>
#include "../../src/common/benchmarkoptionsparser.h"
#include "../../src/common/utility.h"

using namespace std;
using ::testing::Environment;

extern BenchmarkOptions t_options;    
cl_int t_clErr;
cl_command_queue t_queue;
cl_context t_ctx;
cl_device_id t_dev;

class OptionsParserEnvironment : public Environment
{
public:

    // Start OpenCL Device
    void SetUp() override
    {
        t_dev = ListDevicesAndGetDevice(t_options.platform, t_options.device, t_options.verbose);
        t_ctx = clCreateContext(NULL, // properties
                              1,    // number of devices
                              &t_dev, // device
                              NULL, // notification function
                              NULL,
                              &t_clErr);
        t_queue = clCreateCommandQueue(t_ctx, t_dev, CL_QUEUE_PROFILING_ENABLE, &t_clErr);
        // cout << "Device Started\n" ;
    }

    // Close OpenCL Device
    void TearDown() override
    {
        // Do the clean-up work here that doesn't throw exceptions
        clReleaseCommandQueue(t_queue);
        clReleaseContext(t_ctx);
        // cout << "Device Closed\n";
    }
};
#endif

