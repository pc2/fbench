/** @file benchmarkoptions.h
*/
#ifndef BENCHMARKOPTIONS_H_
#define BENCHMARKOPTIONS_H_

#include <string>
#include <map>

using namespace std;

// Enumeration types for representing each benchmark application to exist 
// in the suite.
enum ApplicationType { all, md5Hash, scan, firFilter, mm, nw, ransac, mergesort /*....*/};

// A struct representing a Benchmark Application's options.
struct ApplicationOptions
{
    int size;
    int passes;
    int iterations;

    string kernelDir;
    string bitstreamFile;

    string dataDir;
    int dataGroup;
    
    // RANSAC specific
    string ifile;
    string model;
};

// A struct representing Benchmark suite options specified.
struct BenchmarkOptions
{    
    string dumpXml;
    string dumpJson;
    bool verbose;
    bool quiet;
    
    int platform;
    int device;
    
    string kernelDir;
    string configFile;
    map<ApplicationType, ApplicationOptions> appsToRun;
};

#endif /* BENCHMARKOPTIONS_H_ */
