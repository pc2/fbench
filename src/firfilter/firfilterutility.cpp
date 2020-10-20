#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <iostream>
#include <fstream>

#include "firfilterutility.h"

using namespace std;

double getDeviceEspsilon(int fpOps)
{
    // Correct tolerance for each sample is also dependant on 
    // the number of floating point operation performed 
    // for each sample. This is an effort towards it.
    return fpOps * 20 * DBL_EPSILON_FPGA;
}

double getHostEspsilon(int fpOps)
{
    // Correct tolerance for each sample is also dependant on 
    // the number of floating point operation performed 
    // for each sample. This is an effort towards it.
    // This however does take the EPSILON of the CPU into 
    // account.
    return fpOps * 10 * DBL_EPSILON_HOST;
}

bool readInteger(int &number, char* buffer, int buffSize, FILE* file)
{
    if (!fgets(buffer, buffSize, file)) return false;
    if (isnan(number = atoi(buffer))) return false; 
    return true;
}

bool readFloatingPoint(FLOATING_POINT &number, char* buffer, int buffSize, FILE* file)
{
    if (!fgets(buffer, buffSize, file)) return false;
    if (isnan(number = atof(buffer))) return false; 
    return true;
}

bool readFile(string fileName, FileData &fileData)
{
    fileData.valid = false;
    
    FILE *file;
    file = fopen(fileName.c_str(), "rb");
    
    if (file == NULL) 
    { 
        cout << "Failed opening: " << fileName << " for reading" << endl; 
        return false;
    }
    
    char file_line[MAX_LINE_LENGTH];
    memset(&file_line, 0, MAX_LINE_LENGTH);

    if (!readInteger(fileData.group, file_line, MAX_LINE_LENGTH, file)) 
    {
        cout << "Failed reading the group of the file: " << fileName.c_str() << endl; 
        return false;        
    }
    
    memset(&file_line, 0, MAX_LINE_LENGTH);

    if (!readInteger(fileData.size, file_line, MAX_LINE_LENGTH, file))
    {
        cout << "Failed reading the number of elements in the file: " << fileName << endl;   
        return false;        
    }
    
    posix_memalign(reinterpret_cast<void**>(&fileData.elements),
        64, fileData.size * sizeof(FLOATING_POINT));
    
    for (int i=0; i<fileData.size; i++) 
    {
        memset(&file_line, 0, MAX_LINE_LENGTH);

        if (!readFloatingPoint(fileData.elements[i], file_line, MAX_LINE_LENGTH, file))
        {
            cout << "Failed reading the element(s) in the file: " << fileName << endl;
            return false;  
        }
    }

    fileData.valid = true;
    return true;
}

bool readFiles(std::string dirName, int group, BenchmarkData &benchmarkData)
{    
    if (!readFile(dirName+to_string(group)+INPUTS_FILE, benchmarkData.samples)) 
        return false;

    if (!readFile(dirName+to_string(group)+COEFFS_FILE, benchmarkData.coefficients)) 
        return false;

    if (!readFile(dirName+to_string(group)+RESULTS_FILE, benchmarkData.results)) 
        return false;

    return true;
}

bool floatingPointEquals(FLOATING_POINT a, FLOATING_POINT b, double tolerance)
{
    return std::abs(a - b) < tolerance;
}

bool verifyFilesData(BenchmarkData &benchmarkData)
{
    if (benchmarkData.samples.size != benchmarkData.results.size)
    {
        cout << "ERROR: mismatch between the samples and results count." << endl;
        return false;
    }
    if (benchmarkData.samples.group != benchmarkData.results.group ||
        benchmarkData.samples.group != benchmarkData.coefficients.group)
    {
        cout << "ERROR: mismatch between the group id of the data files." << endl;
        return false;
    }

    // ofstream myfile (FILES_PREFIX+to_string(1)+"-temp-file-verf-results.dat");
    // if (myfile.is_open())
    // {
    //     myfile.clear();
    // }
    // else cout << "Unable to open temp result file";

    double tolerance = getHostEspsilon(benchmarkData.coefficients.size);

    for (int i=0; i<benchmarkData.samples.size; i++)
    {
        FLOATING_POINT sum = 0.0;

        for (int j=0, k=i; j<benchmarkData.coefficients.size && k >= 0; j++,k--)
        {
            sum += benchmarkData.samples.elements[k] 
                    * benchmarkData.coefficients.elements[j];
        }

        // myfile << sum << "\n" ;

        if (!floatingPointEquals(benchmarkData.results.elements[i], sum, tolerance))
        {   
            cout << "Error occured when verifying the input files." << endl;
            cout << "Result mismatch at index: " << i << endl;
            cout << "The cacluated value: " << sum
                << ", the precalculated value: " << benchmarkData.results.elements[i] << endl ;
            cout << "Comparision tolerance :" << tolerance << endl;
            
            return false;
        }
    }    

    // myfile.close();

    return true;
}

bool verifyResults(BenchmarkData& benchmarkData, FLOATING_POINT results[])
{   
    //ofstream myfile (FILES_PREFIX+to_string(1)+"-temp-results.dat");
    //if (myfile.is_open())
    //{
    //    myfile.clear();
    //    myfile << "This is a line.\n";
    //    myfile << "This is another line.\n";
    //    for(int count = 0; count < benchmarkData.results.size; count ++){
    //        myfile << results[count] << "\n" ;
    //    }
    //    myfile.close();
    //}
    //else cout << "Unable to open temp result file";
    
    double tolerance = getDeviceEspsilon(benchmarkData.coefficients.size);
    for (int i=0; i<benchmarkData.results.size; i++)
    {
        if (!floatingPointEquals(
                benchmarkData.results.elements[i], 
                results[i], tolerance))
        {
            cout << "Result mismatch at index: " << i << endl;
            cout << "The cacluated value: " << results[i]
                 << ", the precalculated value: " << benchmarkData.results.elements[i] << endl ;
            cout << "Comparision tolerance :" << tolerance << endl;
            return false;
        }
    }
    return true;
}
