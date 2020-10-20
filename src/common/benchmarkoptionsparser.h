/****************************************************************************
* @file benchmarkoptionsparser.h
* @class BenchmarkOptionsParser
*
* Purpose:
*
* @author Masood Raeisi Nafchi
* @date Feb 12, 2020
*
* Modifications:
****************************************************************************/

#ifndef BENCHMARKOPTIONSPARSER_H_
#define BENCHMARKOPTIONSPARSER_H_

#include "optionparser.h"
#include <string>
#include <vector>
#include <map>

inline vector<string> SplitValues(const std::string &buff, char delim);

struct BOption
{
	string benchmark;
	OptionParser *options;
	BOption *next;
};

class BenchmarkOptionsParser
{
public:
	BenchmarkOptionsParser();

    void addBenchmark(const string &benchmark);
    void addOption(	const string &longName,
					OptionType type,
					const string &defaultValue,
					const string &helpText = "No help specified",
					char shortLetter = '\0');

    void assignBenchmarkOption(const string &benchmark,
							const string &longName,
							const string &value);

    bool parse(int argc, const char *const argv[]);
    bool parseJSON(const string &fileName);

    long long   getOptionInt(const string &benchmark, const string &name) const;
    float       getOptionFloat(const string &benchmark, const string &name) const;
    bool        getOptionBool(const string &benchmark, const string &name) const;
    string      getOptionString(const string &benchmark, const string &name) const;

    vector<long long>     getOptionVecInt(const string &benchmark, const string &name) const;
    vector<float>         getOptionVecFloat(const string &benchmark, const string &name) const;
    vector<string>        getOptionVecString(const string &benchmark, const string &name) const;


	void printHelp(const string &optionName) const;
    void usage() const;

    void printOptions();

protected:

private:
	BOption *head;



};

#endif /* BENCHMARKOPTIONSPARSER_H_ */
