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

/** @file utility.cpp
*/
#include <stdexcept>

#include "utility.h"

const string 
    hostOption              = "host",
    md5Option               = "md5",
    scanOption              = "scan",
    firFilterOption         = "firfilter",
    mmOption                = "mm",
    nwOption                = "nw",
    ransacOption            = "ransac",
    mergesortOption         = "mergesort",
    allOption               = "all",
    benchmarksOption        = "benchmark",
    kerneldirOption         = "kerneldir",
    configOption            = "config",
    dumpxmlOption           = "dumpxml",
    dumpjsonOption          = "dumpjson",
    platformOption          = "platform",
    deviceOption            = "device",
    verboseOption           = "verbose",
    quietOption             = "quiet",
    kernelOption            = "kernel",
    md5KernelOption         = "md5kernel",
    scanKernelOption        = "scankernel",
    firFilterKernelOption   = "firfilterkernel",
    firFilterDataDir        = "inputdir",
    firFilterDataGroup      = "group",
    nwKernelOption          = "nwkernel",
    mmKernelOption          = "mmkernel",
    ransacKernelOption      = "ransackernel",
    mergesortKernelOption   = "mergesortkernel",
    ransacIfileOption       = "ifile",
    ransacModelOption       = "model",
    sizeOption              = "size",
    passesOption            = "passes",
    iterationsOption        = "iterations",
    intOption               = "specify int option",
    stringOption            = "specify string option",
    vectorStringOption      = "specify vector string option",
    booleanOption           = "specify boolean option",   
    md5DefaultKernel        = "md5.aocx",
    scanDefaultKernel       = "scan.aocx",
    firFilterDefaultKernel  = "firfilter.aocx",
    nwDefaultKernel         = "nw.aocx",
    mmDefaultKernel         = "mm.aocx",
    ransacDefaultKernel     = "ransac.aocx",
    ransacDefaultIfile      = "flowvector.csv",
    ransacDefaultModel      = "fv",
    mergesortDefaultKernel  = "mergesort.aocx";

/****************************************************************************
* <b>Method:</b> ListDevicesAndGetDevice()
*
* <b>Purpose:</b> Get the OpenCL device ID for the device with the specified index on
* the specified platform.
*
* @param platform platform index
* @param device device index
*
* @returns The OpenCL device ID if indeces are valid
*
* @note Function exits the program if specified platform or device
* are not found.
*
* @author Gabriel Marin
* @date August 21, 2009
****************************************************************************/
cl_device_id ListDevicesAndGetDevice(int platformIdx, int deviceIdx, bool output)
{
    cl_int err;

    // TODO remove duplication between this function and GetNumOclDevices.
    cl_uint nPlatforms = 0;
    err = clGetPlatformIDs(0, NULL, &nPlatforms);
    CL_CHECK_ERROR(err);

    if (nPlatforms <= 0)
    {
        cerr << "No OpenCL platforms found. Exiting." << endl;
        exit(0);
    }

    if (platformIdx<0 || platformIdx>=nPlatforms)  // platform ID out of range
    {
        cerr << "Platform index " << platformIdx << " is out of range. "
             << "Specify a platform index between 0 and "
             << nPlatforms-1 << endl;
        exit(-4);
    }

    cl_platform_id* platformIDs = new cl_platform_id[nPlatforms];
    err = clGetPlatformIDs(nPlatforms, platformIDs, NULL);
    CL_CHECK_ERROR(err);

    // query devices
    cl_uint nDevices = 0;
    err = clGetDeviceIDs(platformIDs[platformIdx],
                        CL_DEVICE_TYPE_ALL,
                        0,
                        NULL,
                        &nDevices );
    CL_CHECK_ERROR(err);
    cl_device_id* devIDs = new cl_device_id[nDevices];
    err = clGetDeviceIDs(platformIDs[platformIdx],
                        CL_DEVICE_TYPE_ALL,
                        nDevices,
                        devIDs,
                        NULL );
    CL_CHECK_ERROR(err);

    if (nDevices <= 0)
    {
        cerr << "No OpenCL devices found. Exiting." << endl;
        exit(0);
    }
    
    if (deviceIdx<0 || deviceIdx>=nDevices)  // platform ID out of range
    {
        cerr << "Device index " << deviceIdx << " is out of range. "
             << "Specify a device index between 0 and " << nDevices-1
             << endl;
        exit(-5);
    }

    cl_device_id retval = devIDs[deviceIdx];
    
    if (output)
    {
        size_t nBytesNeeded = 0;
        err = clGetDeviceInfo( retval,
                                CL_DEVICE_NAME,
                                0,
                                NULL,
                                &nBytesNeeded );
        CL_CHECK_ERROR(err);
        char* devName = new char[nBytesNeeded+1];
        err = clGetDeviceInfo( retval,
                                CL_DEVICE_NAME,
                                nBytesNeeded+1,
                                devName,
                                NULL );
        
        cout << "Chose device:"
             << " name='"<< devName <<"'"
             << " index="<<deviceIdx
             << " id="<<retval
             << endl;

        delete[] devName;
    }

    delete[] platformIDs;
    delete[] devIDs;

    return retval;
}

/****************************************************************************
* <b>Method:</b> GetNumOclDevices()
*
* <b>Purpose:</b> Gets the number of available OpenCL devices in the specified
* platform.
*
* @param platform platform index
*
* @returns The number of ocl devices
*
* @author Kyle Spafford
* @date August 21, 2009
****************************************************************************/
int GetNumOclDevices(int platformIndex)
{
    cl_int err;
    
    cl_uint nPlatforms = 0;
    err = clGetPlatformIDs(0, NULL, &nPlatforms);   // determine number of platforms available
    CL_CHECK_ERROR(err);

    if (nPlatforms <= 0)
    {
        cerr << "No OpenCL platforms found. Exiting." << endl;
        exit(-1);
    }

    if (platformIndex<0 || platformIndex>=nPlatforms)  // platform index out of range
    {

        cerr << "Platform index " << platformIndex << " is out of range. "
             << "Specify a platform index between 0 and "
             << nPlatforms-1 << endl;
        exit(-4);
    }

    cl_platform_id* platformIDs = new cl_platform_id[nPlatforms];
    err = clGetPlatformIDs(nPlatforms, platformIDs, NULL);
    CL_CHECK_ERROR(err);

    // query devices for the indicated platform
    cl_uint nDevices = 0;
    err = clGetDeviceIDs( platformIDs[platformIndex],
                            CL_DEVICE_TYPE_ALL,
                            0,
                            NULL,
                            &nDevices );
    CL_CHECK_ERROR(err);
    cl_device_id* devIDs = new cl_device_id[nDevices];
    err = clGetDeviceIDs( platformIDs[platformIndex],
                            CL_DEVICE_TYPE_ALL,
                            nDevices,
                            devIDs,
                            NULL );
    CL_CHECK_ERROR(err);

    delete[] platformIDs;
    delete[] devIDs;

    return (int)nDevices;
}

/****************************************************************************
* <b>Function:</b> createProgramFromBitstream()
*
* <b>Purpose:</b> This function creates a cl_program from the bitstream/binary specified 
* and return it. 
*
* @param ctx OpenCL context.
* @param bitStreamFile Name of the bitstream file.
* @param dev device Id of the accelerator.
*
* @returns cl_program created from the bitstream.
*
* @author Abdul Rehman
* @date Feburuary 04, 2020
*****************************************************************************/
cl_program createProgramFromBitstream(cl_context ctx,
                    std::string bitStreamFile, cl_device_id dev)
{
    int err = 0;

	// Preparing the OpenCL binary file
	char kernelf[bitStreamFile.size() + 1];
	strcpy(kernelf, bitStreamFile.c_str());

	size_t lengths[1];
	unsigned char* binaries[1] ={NULL};
	cl_int status[1];
	cl_int error;
	const char options[] = "";

	FILE *fp = fopen(kernelf, "rb");
	
    if (fp == NULL)
    {
        std::cerr<< "Could not open file: " + bitStreamFile << std::endl;        
        exit(-1);
    }
    
    fseek(fp,0,SEEK_END);
	lengths[0] = ftell(fp);
	binaries[0] = (unsigned char*)malloc(sizeof(unsigned char)*lengths[0]);
	rewind(fp);
	fread(binaries[0],lengths[0],1,fp);
	fclose(fp);

	//Initializing the OpenCL program
	cl_program prog = clCreateProgramWithBinary(ctx,
                                                1,
                                                &dev,
                                                lengths,
                                                (const unsigned char **)binaries,
                                                status,
                                                &error);
										
    CL_CHECK_ERROR(err);

    // Before proceeding, make sure the kernel code compiles and
    // all kernels are valid.

    // TODO: Also take the compile flags in the arguments.
    err = clBuildProgram(prog, 1, &dev, NULL, NULL, NULL);

    if (err != CL_SUCCESS)
    {
        char log[5000];
        size_t retsize = 0;
        
        err = clGetProgramBuildInfo(prog, dev, CL_PROGRAM_BUILD_LOG, 5000
                * sizeof(char), log, &retsize);

        cout << "Build error." << endl;
        cout << "Retsize: " << retsize << endl;
        cout << "Log: " << log << endl;
    }

    CL_CHECK_ERROR(err);

    return prog;
}

/****************************************************************************
* <b>Function:</b> createOptionParser()
*
* <b>Purpose:</b> This function will create an instance of the OptionParser and set its settings.
*
* @param Void
*
* @returns The instance of OptionParser made ready.
*
* @author Abdul Rehman
* @date Feburuary 04, 2020
****************************************************************************/
BenchmarkOptionsParser createOptionParser()
{	
BenchmarkOptionsParser bopts;

    bopts.addBenchmark(md5Option);
    bopts.addBenchmark(scanOption);
    bopts.addBenchmark(firFilterOption);
    bopts.addBenchmark(mmOption);
    bopts.addBenchmark(nwOption);
    bopts.addBenchmark(ransacOption);
    bopts.addBenchmark(mergesortOption);
    bopts.addBenchmark(allOption);

    bopts.addOption(benchmarksOption, OPT_VECSTRING, allOption, vectorStringOption, 'b');
    bopts.addOption(kerneldirOption, OPT_STRING, "", stringOption);
    bopts.addOption(configOption, OPT_STRING, "", stringOption, 'c');
    bopts.addOption(dumpxmlOption, OPT_STRING, "", stringOption, 'x');
    bopts.addOption(dumpjsonOption, OPT_STRING, "", stringOption, 'j');
    bopts.addOption(platformOption, OPT_INT, "-1", intOption, 'p');
    bopts.addOption(deviceOption, OPT_INT, "-1", intOption, 'd');
    bopts.addOption(verboseOption, OPT_BOOL, "false", booleanOption, 'v');
    bopts.addOption(quietOption, OPT_BOOL, "false", booleanOption, 'q');
    bopts.addOption(kernelOption, OPT_STRING, "", stringOption, 'k');
    bopts.addOption(md5KernelOption, OPT_STRING, md5DefaultKernel, stringOption, 'y');
    bopts.addOption(scanKernelOption, OPT_STRING, scanDefaultKernel, stringOption, 'z');
    bopts.addOption(firFilterKernelOption, OPT_STRING, firFilterDefaultKernel, stringOption, 'm');
    bopts.addOption(mmKernelOption, OPT_STRING, mmDefaultKernel);
    bopts.addOption(nwKernelOption, OPT_STRING, nwDefaultKernel, stringOption);
    bopts.addOption(ransacKernelOption, OPT_STRING, ransacDefaultKernel, stringOption, 'r');
    bopts.addOption(mergesortKernelOption, OPT_STRING, mergesortDefaultKernel);
    bopts.addOption(sizeOption, OPT_INT, "1", intOption, 's');
    bopts.addOption(passesOption, OPT_INT, "10", intOption, 'n');
    bopts.addOption(iterationsOption, OPT_INT, "256", intOption, 'i');

    bopts.addOption(firFilterDataDir, OPT_STRING, "data/", stringOption);
    bopts.addOption(firFilterDataGroup, OPT_INT, "1", stringOption);

    // RANSAC specific options
    bopts.addOption(ransacIfileOption, OPT_STRING, ransacDefaultIfile, stringOption);
    bopts.addOption(ransacModelOption, OPT_STRING, ransacDefaultModel, stringOption);

    return bopts;
}

/****************************************************************************
* <b>Function:</b> validateOptions()
*
* <b>Purpose:</b> This function will perform misc. validation checks for the options parsed
* in the case of failure it will print to the console and exit the program.
*  
* @param options Parsed program arguments.
*
* @returns Nothing
*
* @author Abdul Rehman
* @date Feburuary 04, 2020
****************************************************************************/
void validateOptions(BenchmarkOptions options)
{	

}

ApplicationType getTypeAgainstName(string name)
{    
    if (name.compare(md5Option) == 0)
    {
        return md5Hash;
    } 
    else if (name.compare(scanOption) == 0)
    {
        return scan;
    }
    else if (name.compare(firFilterOption) == 0)
    {
        return firFilter;
    }
    else if (name.compare(mmOption) == 0)
    {
        return mm;
    }
    else if (name.compare(nwOption) == 0)
    {
        return nw;
    }
    else if (name.compare(ransacOption) == 0)
    {
        return ransac;
    }
	else if (name.compare(mergesortOption) == 0)
    {
        return mergesort;
    }
    else if (name.compare(allOption) == 0)
    {
        return all;
    }
}

string getKernelNameOption(string appName, bool rootCall)
{
    if (rootCall)
    {
        return kernelOption;
    }
    else if (appName.compare(md5Option) == 0)
    {
        return md5KernelOption;
    }
    else if (appName.compare(scanOption) == 0)
    {
        return scanKernelOption;
    }
    else if (appName.compare(firFilterOption) == 0)
    {
        return firFilterKernelOption;
    }
    else if (appName.compare(nwOption) == 0)
    {
        return nwKernelOption;
    }
    else if (appName.compare(ransacOption) == 0)
    {
        return ransacKernelOption;
	}
	else if (appName.compare(mergesortOption) == 0)
    {
        return mergesortKernelOption;
    }
    else if (appName.compare(mmOption) == 0)
    {
        return mmKernelOption;
    }
}

void addApplications(BenchmarkOptionsParser &parser, 
                        BenchmarkOptions& benchOptions, 
                        std::vector<std::string> applications, bool rootCall)
{
    for (string appName : applications) {
        
        ApplicationType appType = getTypeAgainstName(appName);

        if (appType == all)
        {
            if (applications.size() != 1)
            {
                std::cerr<< "Benchmark option 'all' should be specified"; 
                std::cerr<< " as lone option." << std::endl;
                exit(1);
            }

            std::vector<std::string> allApplications;
            allApplications.push_back(md5Option);
            allApplications.push_back(scanOption);
            allApplications.push_back(firFilterOption);
            allApplications.push_back(mmOption);
            allApplications.push_back(nwOption);
            allApplications.push_back(ransacOption);
			allApplications.push_back(mergesortOption);
            addApplications(parser, benchOptions, allApplications, false);
        }
        else
        {   
            // Determining between the recursive call vs the root call.
            string appNameInConfig = rootCall ? appName : allOption;
            
            ApplicationOptions appOptions =
            {
                .size = parser.getOptionInt(appNameInConfig, sizeOption),
                .passes = parser.getOptionInt(appNameInConfig, passesOption),
                .iterations = parser.getOptionInt(appNameInConfig, iterationsOption),
                .kernelDir = parser.getOptionString(appNameInConfig, kerneldirOption),
                .bitstreamFile = parser.getOptionString(appNameInConfig, 
                    getKernelNameOption(appName, rootCall)),
                .dataDir = parser.getOptionString(appNameInConfig, firFilterDataDir),
		.dataGroup = parser.getOptionInt(appNameInConfig, firFilterDataGroup),
		.ifile = parser.getOptionString(appNameInConfig, ransacIfileOption), // ransac specific
                .model = parser.getOptionString(appNameInConfig, ransacModelOption)  // ransac specific
            };

            benchOptions.appsToRun[appType] = appOptions;
        }
    }
}

BenchmarkOptions parseBenchmarkOptions(int argc, char *argv[])
{   
    BenchmarkOptionsParser parser = createOptionParser();

    if (!parser.parse(argc, argv)) 
    {   
        std::cerr<< "Can not parse arguments." << std::endl;
        exit(1);
    }

    bool cmdArgsOnly = true;

    if (parser.getOptionString(hostOption, configOption) != "")
    {
        if (!parser.parseJSON(parser.getOptionString(hostOption, configOption))) 
        {   
            std::cerr<< "Can not parse configuration file." << std::endl;
            exit(1);
        }

        cmdArgsOnly = false;
    }

    BenchmarkOptions benchOptions = 
    {
        .dumpXml = parser.getOptionString(hostOption, dumpxmlOption),
        .dumpJson = parser.getOptionString(hostOption, dumpjsonOption),
        .verbose = parser.getOptionBool(hostOption, verboseOption),
        .quiet = parser.getOptionBool(hostOption, quietOption),
        .platform = parser.getOptionInt(hostOption, platformOption),
        .device = parser.getOptionInt(hostOption, deviceOption),
        .kernelDir = parser.getOptionString(hostOption, kerneldirOption),
        .configFile = parser.getOptionString(hostOption, configOption)
    };

    auto appNames =  parser.getOptionVecString(hostOption, benchmarksOption);

    addApplications(parser, benchOptions, appNames, !cmdArgsOnly);

    validateOptions(benchOptions);

    return benchOptions;
}

BenchmarkDatabase createResultDatabase(BenchmarkOptions options)
{
    BenchmarkDatabase benchDb;
    benchDb.AddBenchmark(md5Option, options);
    benchDb.AddBenchmark(scanOption, options);
    benchDb.AddBenchmark(firFilterOption, options);
    benchDb.AddBenchmark(mmOption, options);
    benchDb.AddBenchmark(nwOption, options);
    benchDb.AddBenchmark(ransacOption, options);
    benchDb.AddBenchmark(mergesortOption, options);
    return benchDb;
}
