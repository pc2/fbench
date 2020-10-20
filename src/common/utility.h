#ifndef UTILITY_H
#define UTILITY_H

#include <CL/opencl.h>

#include "support.h"
#include "benchmarkoptions.h"
#include "benchmarkoptionsparser.h"
#include "benchmarkdatabase.h"

/****************************************************************************
* @file utility.h
*
* <b>Purpose:</b> Various OpenCL-related and option parser related support routines.
*
* @author Abdul Rehman Tareen
****************************************************************************/

// Benchmark function pointer type.
typedef void (*BenchFunction)(cl_device_id,
                                cl_context,
                                cl_command_queue,
                                BenchmarkDatabase&,
                                BenchmarkOptions&);

// Get device for a specified platform and device.
cl_device_id ListDevicesAndGetDevice(int platform, int device, bool output=true);

// This function will create a cl_program from the bitstream/binary specified 
//  and return it. 
cl_program createProgramFromBitstream(cl_context ctx,
                    std::string bitStreamFile, cl_device_id dev);

// This function will parse the arguments to the program as benchmark options.
BenchmarkOptions parseBenchmarkOptions(int argc, char *argv[]);

// This function will create an instance of the BenchmarkDatabase and set its
// settings.
BenchmarkDatabase createResultDatabase(BenchmarkOptions options);

#endif
