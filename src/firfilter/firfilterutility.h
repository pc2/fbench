#ifndef FIR_UTILITY_H
#define FIR_UTILITY_H

#include <stdlib.h>
#include <stdio.h>
#include <string>

using namespace std;

#define FLOATING_POINT float
#define NON_CHANNELED

#define MAX_LINE_LENGTH 512
#define DBL_EPSILON_FPGA 0.00001
#define DBL_EPSILON_HOST 0.00001

#define FILES_PREFIX "data/"
#define INPUTS_FILE "-inputs.dat"
#define COEFFS_FILE "-coefficients.dat"
#define RESULTS_FILE "-results.dat"

typedef struct {
    bool valid;
    int size, group;
    FLOATING_POINT* elements;
} FileData;

typedef struct {
    FileData samples;
    FileData coefficients;
    FileData results;
} BenchmarkData;

double getDeviceEspsilon(int fpOps);
double getHostEspsilon(int fpOps);

bool readInteger(int &number, char* buffer, int buffSize, FILE* file);
bool readFloatingPoint(FLOATING_POINT &number, char* buffer, int buffSize, FILE* file);
bool readFile(std::string fileName, FileData &fileData);
bool readFiles(std::string dirName, int group, BenchmarkData &benchmarkData);

bool floatingPointEquals(FLOATING_POINT a, FLOATING_POINT b, double tolerance);

bool verifyResults(BenchmarkData& benchmarkData, FLOATING_POINT results[]);
bool verifyFilesData(BenchmarkData &benchmarkData);

#endif
