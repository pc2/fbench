/****************************************************************************
* @file mainhost.h
*
* @brief <b>Purpose:</b>
*
* @author
* @date 
*****************************************************************************/
#include <iostream>
#include <stdlib.h>

#include "common/support.h"
#include "common/utility.h"
#include "common/benchmarkoptions.h"
#include "common/benchmarkdatabase.h"

void addBenchmarkFunctions(BenchmarkOptions &options,
                    std::vector<BenchFunction> &benchFunctions);

void runBenchmarks(BenchmarkOptions &options, BenchmarkDatabase &benchDb,
                    std::vector<BenchFunction> benchFunctions);


void benchmarkMd5(cl_device_id dev,
                    cl_context ctx,
                    cl_command_queue queue,
                    BenchmarkDatabase &resultDB,
                    BenchmarkOptions  &op);

void benchmarkScan(cl_device_id dev,
                    cl_context ctx,
                    cl_command_queue queue,
                    BenchmarkDatabase &resultDB,
                    BenchmarkOptions  &op);

void benchmarkFirFilter(cl_device_id dev,
                    cl_context ctx,
                    cl_command_queue queue,
                    BenchmarkDatabase &resultDB,
                    BenchmarkOptions  &op);

void benchmarkNW(cl_device_id dev,
                    cl_context ctx,
                    cl_command_queue queue,
                    BenchmarkDatabase &resultDB,
                    BenchmarkOptions  &op);

void benchmarkRansac(cl_device_id dev,
                    cl_context ctx,
                    cl_command_queue queue,
                    BenchmarkDatabase &resultDB,
                    BenchmarkOptions  &op);
					
void benchmarkMergeSort(cl_device_id dev,
                    cl_context ctx,
                    cl_command_queue queue,
                    BenchmarkDatabase &resultDB,
                    BenchmarkOptions  &op);

void benchmarkMM(cl_device_id dev,
                    cl_context ctx,
                    cl_command_queue queue,
                    BenchmarkDatabase &resultDB,
                    BenchmarkOptions  &op);

int main(int argc, char *argv[]);
