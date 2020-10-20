/** @file mainhost.cpp
*/
#include <stdlib.h>

#include "mainhost.h"

/*******************************************************************************
* <b>Function:</b> main()
* 
* <b>Purpose:</b> Entry function for Main/level 0 host to all the applications in the
* benchmark suite.
*
* @param argc: arguments count
* @param *argv[]: actual arguments
* @returns Integer exit code
*
* @author Abdul Rehman
* @date Feburuary 04, 2020
*****************************************************************************/

int main(int argc, char *argv[])
{
    int exitCode = 0;

    try
    {
        BenchmarkOptions benchOptions = parseBenchmarkOptions(argc, argv);

        std::vector<BenchFunction> benchFunctions;
        addBenchmarkFunctions(benchOptions, benchFunctions);        

        BenchmarkDatabase benchDb = createResultDatabase(benchOptions);
        runBenchmarks(benchOptions, benchDb, benchFunctions);

        benchDb.DumpResults();
    }
	catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        exitCode = 1;
    }
    catch (...)
    {
        std::cerr << "unrecognized exception caught" << std::endl;
        exitCode = 1;
    }

    return exitCode;
}

/****************************************************************************
* <b>Function:</b> addBenchmarkFunctions()
*
* <b>Purpose:</b> This function will add the benchmark functions' pointers into a vector 
* for execution in the suite, depending upon the options.
*
* @param options: Parsed program arguments.
* @param benchFunctions: Vector of function pointers to the benchmark functions.
* @returns Nothing
*
* @author Abdul Rehman
* @date Feburuary 04, 2020
******************************************************************************/
void addBenchmarkFunctions(BenchmarkOptions &options,
                    std::vector<BenchFunction> &benchFunctions)
{
    for (std::pair<ApplicationType, ApplicationOptions> pair 
        : options.appsToRun) {
        switch(pair.first)
        {
            case md5Hash:   benchFunctions.push_back(benchmarkMd5); break;
            case scan:      benchFunctions.push_back(benchmarkScan); break;
            case firFilter: benchFunctions.push_back(benchmarkFirFilter); break;
            case mm:        benchFunctions.push_back(benchmarkMM); break;
            case nw:        benchFunctions.push_back(benchmarkNW); break;
            case ransac:    benchFunctions.push_back(benchmarkRansac); break;
			case mergesort: benchFunctions.push_back(benchmarkMergeSort); break;
            default:        break;
        }
    }
}

/****************************************************************************
* <b>Function:</b> runBenchmarks()
*
* <b>Purpose:</b> This function will make the device, contex and command queue ready 
* and executed all the benchmark functions passsed to it.
* 
* @param options: Parsed program arguments.
* @param benchFunctions: Vector of function pointers to the benchmark functions.
* @returns Nothing
*
* @author Abdul Rehman
* @date Feburuary 04, 2020
******************************************************************************/
void runBenchmarks(BenchmarkOptions &options, BenchmarkDatabase &benchDb,
        std::vector<BenchFunction> benchFunctions)
{
    cl_device_id devId = ListDevicesAndGetDevice( options.platform,
                                                    options.device,
                                                    options.verbose);
    cl_int clErr;
    cl_context ctx = clCreateContext( NULL,     // properties
                                        1,      // number of devices
                                        &devId, // device
                                        NULL,   // notification function
                                        NULL,
                                        &clErr );
    
    CL_CHECK_ERROR(clErr);

    /** @note By default each benchmark function will be provided with one 
    * command queue, but if the need be, they can create more...
    * but be sure to release them after use. */
    cl_command_queue queue = clCreateCommandQueue( ctx,
                                                    devId,
                                                    CL_QUEUE_PROFILING_ENABLE,
                                                    &clErr );

    CL_CHECK_ERROR(clErr);    

    for (BenchFunction benchFunction : benchFunctions) {
        benchFunction(devId, ctx, queue, benchDb, options);
    }

    clReleaseCommandQueue(queue);
    clReleaseContext(ctx);
}
