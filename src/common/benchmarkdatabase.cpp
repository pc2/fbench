/*****************************************************************************
* @file benchmarkdatabase.cpp
* @class BenchmarkDatabase
*
* <b>Purpose:</b> Track results for multiple benchmarks
* Print statistics of raw results. Generate JSON, XML, CSV 
* file for results.
*
* <b>Modifications:</b>
*
* @author Masood Raeisi Nafchi
* @date Feb 04, 2020
******************************************************************************/
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "benchmarkdatabase.h"
#include "resultdatabase.h"
#include "optionparser.h"
#include "benchmarkoptions.h"


/*****************************************************************************
* <b>Function:</b> BenchmarkDatabase::AddBenchmark()
*
* <b>Purpose:</b> Adds a new benchmark to the benchmark database.
*
* @param benchmark the benchmark name
* @param options options for the current benchmark
* @return Nothing
*
* <b>Modifications:</b>
*
* @author Masood Raeisi Nafchi
* @date Feb 04, 2020
*******************************************************************************/
void BenchmarkDatabase::AddBenchmark(const string &benchmark,BenchmarkOptions &options)
{
	BenchmarkResult *temp= new BenchmarkResult;
	temp->benchmark = benchmark;
	temp->options = options;
	temp->resultdb = new ResultDatabase;
	temp->next=NULL;

	if(head==NULL)
		head=temp;
	else
	{
		BenchmarkResult *i=head;
		while(i->next!=NULL)
			i=i->next;

		i->next=temp;
	}
}

/*****************************************************************************
* <b>Function:</b> BenchmarkDatabase::AddResult()
*
* <b>Purpose:</b>
*
* @param benchmark benchmark name
* @param test test name
* @param atts
* @param unit measurement unit
* @param value test value
*
* <b>Modifications:</b>
*
* @author Masood Raeisi Nafchi
* @date Feb 04, 2020
*****************************************************************************/
void BenchmarkDatabase::AddResult(const string &benchmark,
								const string &test,
								const string &atts,
								const string &unit,
								double value)
{
	BenchmarkResult *i=head;

	while(i!=NULL)
	{
		if(i->benchmark.compare(benchmark)==0)
		{
			i->resultdb->AddResult(test,atts,unit,value);
			return;
		}
		i=i->next;
	}
}

/*****************************************************************************
* <b>Function:</b> BenchmarkDatabase::AddResults()
*
* <b>Purpose:</b>
*
* @param benchmark benchmark name
* @param test test name
* @param atts
* @param unit measurement unit
* @param value test value
*
* <b>Modifications:</b>
*
* @author Masood Raeisi Nafchi
* @date Feb 04, 2020
*****************************************************************************/
void BenchmarkDatabase::AddResults(const string &benchmark,
								const string &test,
								const string &atts,
								const string &unit,
								const vector<double> &values)
{
	BenchmarkResult *i=head;

	while(i!=NULL)
	{
		if(i->benchmark.compare(benchmark)==0)
		{
			i->resultdb->AddResults(test,atts,unit,values);
			return;
		}
		i=i->next;
	}
}

void BenchmarkDatabase::DumpResults()
{
	cout<<"printing resluts to ";
	
	if (head->options.dumpJson.length() != 0)
	{
		cout<<head->options.dumpJson<<endl;
		DumpJSON(head->options.dumpJson);
	} 
	else if (head->options.dumpXml.length() != 0)
	{
		cout<<head->options.dumpXml<<endl;
		DumpXML(head->options.dumpXml);
	} 
	else
	{
		cout<<"output screen"<<endl;
		DumpDetailed(std::cout);
	}
}

/*****************************************************************************
* <b>Function:</b> BenchmarkDatabase::DumpDetailed()
*
* <b>Purpose:</b> Writes the summary results (min/max/stddev/med/mean) to desired output stream
*
* @param out where to print
*
* <b>Modifications:</b>
*
* @author Masood Raeisi Nafchi
* @date Feb 04, 2020
*****************************************************************************/
void BenchmarkDatabase::DumpDetailed(ostream &out)
{
	BenchmarkResult *i=head;
	while(i!=NULL)
	{
		out<<i->benchmark<<"\t";
		//*i->options.DumpDetailed(out);
		i->resultdb->DumpDetailed(out);

		i=i->next;
	}
}

/*****************************************************************************
* <b>Function:</b> BenchmarkDatabase::DumpCSV()
*
* <b>Purpose:</b> Writes the summary results (min/max/stddev/med/mean), to CSV file
*
* @param fileName file to be written to
*
* <b>Modifications:</b>
*
* @author Masood Raeisi Nafchi
* @date Feb 04, 2020
*****************************************************************************/
void BenchmarkDatabase::DumpCSV(string fileName)
{

}

/*****************************************************************************
* <b>Function:</b> BenchmarkDatabase::DumpJSON()
*
* <b>Purpose:</b> Writes the summary results (min/max/stddev/med/mean), to JSON file
*
* @param out file to be written to
*
* <b>Modifications:</b>
*
* @author Masood Raeisi Nafchi
* @date Feb 04, 2020
*****************************************************************************/
void BenchmarkDatabase::DumpJSON(string fileName)
{
	boost::property_tree::ptree btree;

	BenchmarkResult *i=head;

	while(i!=NULL)
	{
		vector<Result> sorted(i->resultdb->results);
		sort(sorted.begin(), sorted.end());

		auto& x = btree.put_child(i->benchmark, {});

		for (int j=0; j<sorted.size(); j++)
		{
			Result &r = sorted[j];

			x.add(r.test+".atts", r.atts);
			x.add(r.test+".units", r.unit);

	        if (r.GetMedian() == FLT_MAX)
	        	x.add(r.test+".Median","N/A");
	        else
	        	x.add(r.test+".Median",r.GetMedian());
	        if (r.GetMean() == FLT_MAX)
	        	x.add(r.test+".Mean","N/A");
	        else
	        	x.add(r.test+".Mean",r.GetMean());
	        if (r.GetStdDev() == FLT_MAX)
	        	x.add(r.test+".StdDev","N/A");
	        else
	        	x.add(r.test+".StdDev",r.GetStdDev());
	        if (r.GetMin() == FLT_MAX)
	        	x.add(r.test+".Min","N/A");
	        else
	        	x.add(r.test+".Min",r.GetMin());
	        if (r.GetMax() == FLT_MAX)
	        	x.add(r.test+".Max","N/A");
	        else
	        	x.add(r.test+".max",r.GetMax());
	    }

		i=i->next;
	}

	boost::property_tree::write_json(fileName, btree);
}

/*****************************************************************************
* <b>Function:</b> BenchmarkDatabase::DumpXML()
*
* <b>Purpose:</b> Writes the summary results (min/max/stddev/med/mean), to XML file
*
* @param out file to be written to
*
* <b>Modifications:</b>
*
* @author Masood Raeisi Nafchi
* @date Feb 04, 2020
*****************************************************************************/
void BenchmarkDatabase::DumpXML(string fileName)
{
	boost::property_tree::ptree btree;

	BenchmarkResult *i=head;

	while(i!=NULL)
	{
		vector<Result> sorted(i->resultdb->results);
		sort(sorted.begin(), sorted.end());

		auto& x = btree.put_child(i->benchmark, {});

		for (int j=0; j<sorted.size(); j++)
		{
			Result &r = sorted[j];

			x.add(r.test+".atts", r.atts);
			x.add(r.test+".units", r.unit);

	        if (r.GetMedian() == FLT_MAX)
	        	x.add(r.test+".Median","N/A");
	        else
	        	x.add(r.test+".Median",r.GetMedian());
	        if (r.GetMean() == FLT_MAX)
	        	x.add(r.test+".Mean","N/A");
	        else
	        	x.add(r.test+".Mean",r.GetMean());
	        if (r.GetStdDev() == FLT_MAX)
	        	x.add(r.test+".StdDev","N/A");
	        else
	        	x.add(r.test+".StdDev",r.GetStdDev());
	        if (r.GetMin() == FLT_MAX)
	        	x.add(r.test+".Min","N/A");
	        else
	        	x.add(r.test+".Min",r.GetMin());
	        if (r.GetMax() == FLT_MAX)
	        	x.add(r.test+".Max","N/A");
	        else
	        	x.add(r.test+".max",r.GetMax());
	    }

		i=i->next;
	}

	boost::property_tree::write_xml(fileName, btree);
}

