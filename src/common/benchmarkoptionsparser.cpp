/****************************************************************************
* @file benchmarkoptionsparser.cpp
* @class BenchmarkOptionsParser
*
* Purpose:
*
* @author Masood Raeisi Nafchi
* @date Feb 12, 2020
* 
*
* Modifications:
*****************************************************************************/

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "benchmarkoptionsparser.h"
#include "optionparser.h"
#include <string>
#include <vector>
#include <map>
using namespace std;

BenchmarkOptionsParser::BenchmarkOptionsParser()
{
	BOption *temp= new BOption;
	temp->benchmark = "host";
	temp->options = new OptionParser;
	temp->next=NULL;

	head=temp;
}
/****************************************************************************
* <b>Method:</b> BenchmarkOptionsParser::addBenchmark
*
* <b>Purpose:</b> Adds a new benchmark to the benchmark database.
*
* @param benchmark the benchmark name
* @param options options for the current benchmark
* @returns Void
*
* @author Masood Raeisi Nafchi
* @date Feb 04, 2020
*
* Modifications:
*
****************************************************************************/
void BenchmarkOptionsParser::addBenchmark(const string &benchmark)
{
	BOption *temp= new BOption;
	temp->benchmark = benchmark;
	temp->options = new OptionParser;
	temp->next=NULL;

	BOption *i=head;
	while(i->next!=NULL)
		i=i->next;
	i->next=temp;
}

void BenchmarkOptionsParser::addOption(const string &longName,
								OptionType type,
								const string &defaultValue,
								const string &helpText,
								char shortLetter)
{
	head->options->addOption(longName,type,defaultValue,helpText,shortLetter);
}

void BenchmarkOptionsParser::assignBenchmarkOption(const string &benchmark,
											const string &longName,
											const string &value)
{
	OptionMap::const_iterator iter = head->options->optionMap.find(longName);
	if (iter == head->options->optionMap.end())
	{
		cout << "option name \"" << longName << "\" not recognized.\n";
		return;
	}


	BOption *i=head->next;

	while(i!=NULL)
	{
		if(i->benchmark.compare(benchmark)==0)
		{
			Option opt;
			opt.longName = iter->second.longName;
			opt.type = iter->second.type;
			opt.defaultValue = iter->second.defaultValue;
			opt.value = value;
			opt.helpText = iter->second.helpText;
			opt.shortLetter = iter->second.shortLetter;

			if(i->options->optionMap.count(longName)>0)
				cout << "Internal error: used option long name '"<<longName<<"' twice.\n";
			i->options->optionMap[longName] = opt;

			return;
		}
		i=i->next;
	}
}

bool BenchmarkOptionsParser::parse(int argc, const char *const argv[])
{
	return head->options->parse(argc,argv);
}


bool BenchmarkOptionsParser::parseJSON(const string &fileName)
{
	using boost::property_tree::ptree;
	boost::property_tree::ptree pt;
	boost::property_tree::read_json(fileName, pt);

    for (ptree::const_iterator itBenchmark = pt.begin(); itBenchmark != pt.end(); ++itBenchmark)
    {
        boost::property_tree::ptree ptt=itBenchmark->second;
        for (ptree::const_iterator itOption = ptt.begin(); itOption != ptt.end(); ++itOption)
        {
        	this->assignBenchmarkOption(itBenchmark->first,itOption->first,itOption->second.get_value<std::string>());
        }
    }
	return 0;
}

long long BenchmarkOptionsParser::getOptionInt(const string &benchmark, const string &name) const
{
	long long retVal;

	OptionMap::const_iterator iter = head->options->optionMap.find(name);

	if (iter == head->options->optionMap.end())
	{
		cout << "getOptionInt: option name \"" << name << "\" not recognized.\n";
		return -9999;
	}

	stringstream ss(iter->second.value);
	ss >> retVal;

	if(iter->second.value != iter->second.defaultValue)
	{
		return retVal;
	}

	BOption *i=head->next;

	while(i!=NULL)
	{
		if(i->benchmark.compare(benchmark)==0)
		{
			iter = i->options->optionMap.find(name);
			if (iter == i->options->optionMap.end())
			{
				return retVal;
			}
			stringstream ss(iter->second.value);
			ss >> retVal;
			return retVal;
		}
		i=i->next;
	}

	return -9999;
}

float BenchmarkOptionsParser::getOptionFloat(const string &benchmark, const string &name) const
{
	float retVal;

	OptionMap::const_iterator iter = head->options->optionMap.find(name);

	if (iter == head->options->optionMap.end())
	{
		cout << "getOptionInt: option name \"" << name << "\" not recognized.\n";
		return -9999;
	}

	stringstream ss(iter->second.value);
	ss >> retVal;

	if(iter->second.value != iter->second.defaultValue)
	{
		return retVal;
	}

	BOption *i=head->next;

	while(i!=NULL)
	{
		if(i->benchmark.compare(benchmark)==0)
		{
			iter = i->options->optionMap.find(name);
			if (iter == i->options->optionMap.end())
			{
				return retVal;
			}
			stringstream ss(iter->second.value);
			ss >> retVal;
			return retVal;
		}
		i=i->next;
	}

	return -9999;
}

bool BenchmarkOptionsParser::getOptionBool(const string &benchmark, const string &name) const
{
	bool retVal;

	OptionMap::const_iterator iter = head->options->optionMap.find(name);

	if (iter == head->options->optionMap.end())
	{
		cout << "getOptionInt: option name \"" << name << "\" not recognized.\n";
		return false;
	}

	stringstream ss(iter->second.value);
	ss >> retVal;

	if(iter->second.value != iter->second.defaultValue)
	{
		return retVal;
	}

	BOption *i=head->next;

	while(i!=NULL)
	{
		if(i->benchmark.compare(benchmark)==0)
		{
			iter = i->options->optionMap.find(name);
			if (iter == i->options->optionMap.end())
			{
				return retVal;
			}
			stringstream ss(iter->second.value);
			ss >> retVal;
			return retVal;
		}
		i=i->next;
	}

	return false;
}

string BenchmarkOptionsParser::getOptionString(const string &benchmark, const string &name) const
{
	string retVal;

	OptionMap::const_iterator iter = head->options->optionMap.find(name);

	if (iter == head->options->optionMap.end())
	{
		cout << "getOptionInt: option name \"" << name << "\" not recognized.\n";
		return "ERROR - Option not recognized";
	}

	stringstream ss(iter->second.value);
	ss >> retVal;

	if(iter->second.value != iter->second.defaultValue)
	{
		return retVal;
	}

	BOption *i=head;

	while(i!=NULL)
	{
		if(i->benchmark.compare(benchmark)==0)
		{
			iter = i->options->optionMap.find(name);
			if (iter == i->options->optionMap.end())
			{
				return retVal;
			}
			stringstream ss(iter->second.value);
			ss >> retVal;
			return retVal;
		}
		i=i->next;
	}

	return "ERROR - Option not recognized";
}

vector<long long> BenchmarkOptionsParser::getOptionVecInt(const string &benchmark, const string &name) const
{
	vector<long long> retVal;
	return retVal;
}

vector<float> BenchmarkOptionsParser::getOptionVecFloat(const string &benchmark, const string &name) const
{
	vector<float> retVal;
	return retVal;
}

vector<string> BenchmarkOptionsParser::getOptionVecString(const string &benchmark, const string &name) const
{
	vector<string> retVal;
	string stringvalue = getOptionString(benchmark, name);
	
	string delimiter = ",";
	size_t pos = 0;
	string token;

	while ((pos = stringvalue.find(delimiter)) != string::npos) {
		
		token = stringvalue.substr(0, pos); 
		retVal.push_back(token);
		stringvalue.erase(0, pos + delimiter.length());

	}

	retVal.push_back(stringvalue);
	return retVal;
}

void BenchmarkOptionsParser::printHelp(const string &optionName) const
{
	head->options->printHelp(optionName);
}

void BenchmarkOptionsParser::usage() const
{
	head->options->usage();
}

void BenchmarkOptionsParser::printOptions()
{
	BOption *i=head;

	while(i!=NULL)
	{
		OptionMap::const_iterator j = i->options->optionMap.begin();

		cout<<i->benchmark<<endl;

		cout<<"longName"<<"\t";
		cout<<"shortLetter"<<"\t";
		cout<<"type"<<"\t";
		cout<<"value"<<"\t";
		cout<<"defaultValue"<<"\t";
		cout<<"helpText"<<"\n";

		while(j != i->options->optionMap.end())
		{
			Option jo = j->second;
			cout<<jo.longName<<"\t";
			cout<<jo.shortLetter<<"\t";
			cout<<jo.type<<"\t";
			cout<<jo.value<<"\t";
			cout<<jo.defaultValue<<"\t";
			cout<<jo.helpText<<"\n";

			j++;
		}
		cout<<endl;
		i=i->next;
	}
}

/****************************************************************************
* <b>Method:</b> SplitValues()
*
* <b>Purpose:</b> Various generic utility routines having to do with string and number
* manipulation.
*
* @author Jeremy Meredith
* @date September 18, 2009
*
* <b>Modified:</b>
*    Jan 2010, rothpc
*
*    Jeremy Meredith, Tue Oct 9 17:25:25 EDT 2012
*    Round is c99, not Windows-friendly.  Assuming we are using
*    positive values, replaced it with an equivalent of int(x+.5).
*
****************************************************************************/
inline vector<string> SplitValues(const std::string &buff, char delim)
{
    vector<std::string> output;
    std::string tmp="";
    for (size_t i=0; i<buff.length(); i++)
    {
       if (buff[i] == delim)
       {
          if (!tmp.empty())
             output.push_back(tmp);
          tmp = "";
       }
       else
       {
          tmp += buff[i];
       }
    }
    if (!tmp.empty())
       output.push_back(tmp);

    return output;
}
