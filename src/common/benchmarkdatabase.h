/****************************************************************************
* @file benchmarkdatabase.h
* @class BenchmarkDatabase
*
* <b>Purpose:</b> Track results for multiple benchmarks.
* Print statistics of raw results.
* Generate JSON, XML, CSV file for results.
*
* <b>Modifications:</b>
*
* @author Masood Raeisi Nafchi
* @date Feb 04, 2020
*****************************************************************************/

#ifndef BENCHMARKDATABASE_H_
#define BENCHMARKDATABASE_H_

#include "resultdatabase.h"
#include "benchmarkoptions.h"
#include "optionparser.h"
#include <vector>

using namespace std;

struct BenchmarkResult
{
       string benchmark;
       BenchmarkOptions options;
       ResultDatabase *resultdb;
       BenchmarkResult *next;
};

class BenchmarkDatabase
{
public:
       BenchmarkDatabase(){head=NULL;};

       void AddBenchmark(const string &benchmark, BenchmarkOptions &options);
       void AddResult(const string &benchmark,
                                       const string &test,
                                       const string &atts,
                                       const string &unit,
                                       double value);

       void AddResults(const string &benchmark,
                                       const string &test,
                                       const string &atts,
                                       const string &unit,
                                       const vector<double> &values);

       void DumpResults();
       void DumpDetailed(ostream &out);
       void DumpCSV(string fileName);
       void DumpJSON(string fileName);
       void DumpXML(string fileName);

protected:

private:
       BenchmarkResult *head;

};

#endif /* BENCHMARKDATABASE_H_ */