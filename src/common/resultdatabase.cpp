/*
 * Copyright (c) 2011, UT-Battelle, LLC
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *      > Redistributions of source code must retain the above copyright
 *        notice, this list of conditions and the following disclaimer.
 *      > Redistributions in binary form must reproduce the above copyright
 *        notice, this list of conditions and the following disclaimer in the
 *        documentation and/or other materials provided with the distribution.
 *      > Neither the name of Oak Ridge National Laboratory, nor UT-Battelle, LLC, nor
 *        the names of its contributors may be used to endorse or promote products
 *        derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
*/

/** @file resultdatabase.cpp
* @class ResultDatabase
*/
#include "resultdatabase.h"
#include "optionparser.h"

#include <cfloat>
#include <algorithm>
#include <cmath>

using namespace std;

bool Result::operator<(const Result &rhs) const
{
    if (test < rhs.test)
        return true;
    if (test > rhs.test)
        return false;
    if (atts < rhs.atts)
        return true;
    if (atts > rhs.atts)
        return false;
    return false; // less-operator returns false on equal
}

double Result::GetMin() const
{
    double r = FLT_MAX;
    for (int i=0; i<value.size(); i++)
    {
        r = min(r, value[i]);
    }
    return r;
}

double Result::GetMax() const
{
    double r = -FLT_MAX;
    for (int i=0; i<value.size(); i++)
    {
        r = max(r, value[i]);
    }
    return r;
}

double Result::GetMedian() const
{
    return GetPercentile(50);
}

double Result::GetPercentile(double q) const
{
    int n = value.size();
    if (n == 0)
        return FLT_MAX;
    if (n == 1)
        return value[0];

    if (q <= 0)
        return value[0];
    if (q >= 100)
        return value[n-1];

    double index = ((n + 1.) * q / 100.) - 1;

    vector<double> sorted = value;
    sort(sorted.begin(), sorted.end());

    if (n == 2)
        return (sorted[0] * (1 - q/100.)  +  sorted[1] * (q/100.));

    int index_lo = int(index);
    double frac = index - index_lo;
    if (frac == 0)
        return sorted[index_lo];

    double lo = sorted[index_lo];
    double hi = sorted[index_lo + 1];
    return lo + (hi-lo)*frac;
}

double Result::GetMean() const
{
    double r = 0;
    for (int i=0; i<value.size(); i++)
    {
        r += value[i];
    }
    return r / double(value.size());
}

double Result::GetStdDev() const
{
    double r = 0;
    double u = GetMean();
    if (u == FLT_MAX)
        return FLT_MAX;
    for (int i=0; i<value.size(); i++)
    {
        r += (value[i] - u) * (value[i] - u);
    }
    r = sqrt(r / value.size());
    return r;
}


void ResultDatabase::AddResults(const string &test,
                                const string &atts,
                                const string &unit,
                                const vector<double> &values)
{
    for (int i=0; i<values.size(); i++)
    {
        AddResult(test, atts, unit, values[i]);
    }
}

static string RemoveAllButLeadingSpaces(const string &a)
{
    string b;
    int n = a.length();
    int i = 0;
    while (i<n && a[i] == ' ')
    {
        b += a[i];
        ++i;
    }
    for (; i<n; i++)
    {
        if (a[i] != ' ' && a[i] != '\t')
            b += a[i];
    }
    return b;
}

void ResultDatabase::AddResult(const string &test_orig,
                               const string &atts_orig,
                               const string &unit_orig,
                               double value)
{
    string test = RemoveAllButLeadingSpaces(test_orig);
    string atts = RemoveAllButLeadingSpaces(atts_orig);
    string unit = RemoveAllButLeadingSpaces(unit_orig);
    int index;
    for (index = 0; index < results.size(); index++)
    {
        if (results[index].test == test &&
            results[index].atts == atts)
        {
            if (results[index].unit != unit)
                throw "Internal error: mixed units";

            break;
        }
    }

    if (index >= results.size())
    {
        Result r;
        r.test = test;
        r.atts = atts;
        r.unit = unit;
        results.push_back(r);
    }

    results[index].value.push_back(value);
}

/****************************************************************************
* <b>Function:</b> ResultDatabase::DumpDetailed()
* <b>Purpose:</b> Writes the full results, including all trials.
*
* @param out where to print
*
* @author Jeremy Meredith
* @date August 14, 2009
*
* <b>Modifications:</b>
*
* Jeremy Meredith, Wed Nov 10 14:25:17 EST 2010
* Renamed to DumpDetailed to make room for a DumpSummary.
*
* Jeremy Meredith, Thu Nov 11 11:39:57 EST 2010
* Added note about (*) missing value tag.
*
* Jeremy Meredith, Tue Nov 23 13:57:02 EST 2010
* Changed note about missing values to be worded a little better.
****************************************************************************/
void ResultDatabase::DumpDetailed(ostream &out)
{
    vector<Result> sorted(results);

    sort(sorted.begin(), sorted.end());

    int maxtrials = 1;
    for (int i=0; i<sorted.size(); i++)
    {
        if (sorted[i].value.size() > maxtrials)
            maxtrials = sorted[i].value.size();
    }

    // TODO: in big parallel runs, the "trials" are the procs
    // and we really don't want to print them all out....
    out << "test\t"
        << "atts\t"
        << "units\t"
        << "median\t"
        << "mean\t"
        << "stddev\t"
        << "min\t"
        << "max\t";
    for (int i=0; i<maxtrials; i++)
        out << "trial"<<i<<"\t";
    out << endl;

    for (int i=0; i<sorted.size(); i++)
    {
        Result &r = sorted[i];
        out << r.test << "\t";
        out << r.atts << "\t";
        out << r.unit << "\t";
        if (r.GetMedian() == FLT_MAX)
            out << "N/A\t";
        else
            out << r.GetMedian() << "\t";
        if (r.GetMean() == FLT_MAX)
            out << "N/A\t";
        else
            out << r.GetMean()   << "\t";
        if (r.GetStdDev() == FLT_MAX)
            out << "N/A\t";
        else
            out << r.GetStdDev() << "\t";
        if (r.GetMin() == FLT_MAX)
            out << "N/A\t";
        else
            out << r.GetMin()    << "\t";
        if (r.GetMax() == FLT_MAX)
            out << "N/A\t";
        else
            out << r.GetMax()    << "\t";
        for (int j=0; j<r.value.size(); j++)
        {
            if (r.value[j] == FLT_MAX)
                out << "N/A\t";
            else
                out << r.value[j] << "\t";
        }

        out << endl;
    }
    out << endl
        << "Note: Any results marked with (*) had missing values." << endl
        << "      This can occur on systems with a mixture of" << endl
        << "      device types or architectural capabilities." << endl;
}


/****************************************************************************
* <b>Function:</b> ResultDatabase::DumpSummary()
*
* <b>Purpose:</b> Writes the summary results (min/max/stddev/med/mean), but not
* every individual trial.
*
* @param out where to print
*
* @author Jeremy Meredith
* @date November 10, 2010
*
* <b>Modifications:</b> Jeremy Meredith, Thu Nov 11 11:39:57 EST 2010.
* Added note about (*) missing value tag.
*****************************************************************************/
void ResultDatabase::DumpSummary(ostream &out)
{
    vector<Result> sorted(results);

    sort(sorted.begin(), sorted.end());

    // TODO: in big parallel runs, the "trials" are the procs
    // and we really don't want to print them all out....
    out << "test\t"
        << "atts\t"
        << "units\t"
        << "median\t"
        << "mean\t"
        << "stddev\t"
        << "min\t"
        << "max\t";
    out << endl;

    for (int i=0; i<sorted.size(); i++)
    {
        Result &r = sorted[i];
        out << r.test << "\t";
        out << r.atts << "\t";
        out << r.unit << "\t";
        if (r.GetMedian() == FLT_MAX)
            out << "N/A\t";
        else
            out << r.GetMedian() << "\t";
        if (r.GetMean() == FLT_MAX)
            out << "N/A\t";
        else
            out << r.GetMean()   << "\t";
        if (r.GetStdDev() == FLT_MAX)
            out << "N/A\t";
        else
            out << r.GetStdDev() << "\t";
        if (r.GetMin() == FLT_MAX)
            out << "N/A\t";
        else
            out << r.GetMin()    << "\t";
        if (r.GetMax() == FLT_MAX)
            out << "N/A\t";
        else
            out << r.GetMax()    << "\t";

        out << endl;
    }
    out << endl
        << "Note: results marked with (*) had missing values such as" << endl
        << "might occur with a mixture of architectural capabilities." << endl;
}

void ResultDatabase::DumpOutliers(ostream &out)
{
    // get only the mean results
    vector<Result> means;
    for (int i=0; i<results.size(); i++)
    {
        Result &r = results[i];
        if (r.test.length() > 6 &&
            r.test.substr(r.test.length()-6) == "(mean)")
        {
            means.push_back(r);
        }
    }

    // sort them
    sort(means.begin(), means.end());

    // get the max trials (in this case processors)
    int maxtrials = 1;
    for (int i=0; i<means.size(); i++)
    {
        if (means[i].value.size() > maxtrials)
            maxtrials = means[i].value.size();
    }

    out << "\nDetecting outliers based on per-process mean values.\n";

    // List of IQR thresholds to test.  Please put these in
    // increasing order so we can avoid reporting outliers twice.
    int nOutlierThresholds = 2;
    const char *outlierHeaders[] = {
        "Mild outliers (>1.5 IQR from 1st/3rd quartile)",
        "Extreme outliers (>3.0 IQR from 1st/3rd quartile)"
    };
    double outlierThresholds[] = {
        1.5,
        3.0
    };

    // for each threshold category, print any values
    // which are more than that many stddevs from the
    // all-processor-mean
    for (int pass=0; pass < nOutlierThresholds; pass++)
    {
        out << "\n" << outlierHeaders[pass]<< ":\n";
        bool foundAny = false;

        for (int i=0; i<means.size(); i++)
        {
            Result &r = means[i];
            double allProcsQ1 = r.GetPercentile(25);
            double allProcsQ3 = r.GetPercentile(75);
            double allProcsIQR = allProcsQ3 - allProcsQ1;
            double thresholdIQR = outlierThresholds[pass];
            double nextThresholdIQR = (pass < nOutlierThresholds-1) ?
                                                outlierThresholds[pass+1] : 0;
            for (int j=0; j<r.value.size(); j++)
            {
                double v = r.value[j];
                if (v < allProcsQ1 - thresholdIQR * allProcsIQR)
                {
                    // if they pass the next, more strict threshold,
                    // don't report it in this pass
                    if (pass == nOutlierThresholds-1 ||
                        v >= allProcsQ1 - nextThresholdIQR * allProcsIQR)
                    {
                        foundAny = true;
                        out << r.test << " " << r.atts << " "
                            << "Processor "<<j<<" had a mean value of "
                            << v << " " << r.unit <<", which is less than "
                            << "Q1 - IQR*" << thresholdIQR << ".\n";
                    }
                }
                else if (v > allProcsQ3 + thresholdIQR * allProcsIQR)
                {
                    // if they pass the next, more strict threshold,
                    // don't report it in this pass
                    if (pass == nOutlierThresholds-1 ||
                        v <= allProcsQ3 + nextThresholdIQR * allProcsIQR)
                    {
                        foundAny = true;
                        out << r.test << " " << r.atts << " "
                            << "Processor "<<j<<" had a mean value of "
                            << v << " " << r.unit <<", which is more than "
                            << "Q3 + IQR*" << thresholdIQR << ".\n";
                    }
                }
            }
        }
        // If we didn't find any, let them know that explicitly.
        if (!foundAny)
        {
            out << "None.\n";
        }
    }
}

/****************************************************************************
* <b>Function:</b> ResultDatabase::ClearAllResults()
*
* <b>Purpose:</b> Clears all existing results from the ResultDatabase; used 
* for multiple passes of the same test or multiple tests.  
*
* @param Void 
*
* @returns Void
*
* @author Jeffrey Young
* @date September 10th, 2014
*
* <b>Modifications:</b>
*****************************************************************************/

void ResultDatabase::ClearAllResults()
{
	results.clear();
}

/****************************************************************************
* <b>Function:</b> ResultDatabase::DumpCsv()
*
* <b>Purpose:</b> Writes either detailed or summary results (min/max/stddev/med/mean), 
* but not every individual trial. 
*
* @param out file to print CSV results
*
* @returns Void
*
* @author Jeffrey Young
* @date August 28th, 2014
*
* <b>Modifications:</b>
*****************************************************************************/

void ResultDatabase::DumpCsv(string fileName)
{
    bool emptyFile;
    vector<Result> sorted(results);

    sort(sorted.begin(), sorted.end());

    //Check to see if the file is empty - if so, add the headers
    emptyFile = this->IsFileEmpty(fileName);

    //Open file and append by default
    ofstream out;
    out.open(fileName.c_str(), std::ofstream::out | std::ofstream::app);

    //Add headers only for empty files
    if(emptyFile)
    {
    // TODO: in big parallel runs, the "trials" are the procs
    // and we really don't want to print them all out....
    out << "test, "
        << "atts, "
        << "units, "
        << "median, "
        << "mean, "
        << "stddev, "
        << "min, "
        << "max, ";
    out << endl;
    }

    for (int i=0; i<sorted.size(); i++)
    {
        Result &r = sorted[i];
        out << r.test << ", ";
        out << r.atts << ", ";
        out << r.unit << ", ";
        if (r.GetMedian() == FLT_MAX)
            out << "N/A, ";
        else
            out << r.GetMedian() << ", ";
        if (r.GetMean() == FLT_MAX)
            out << "N/A, ";
        else
            out << r.GetMean()   << ", ";
        if (r.GetStdDev() == FLT_MAX)
            out << "N/A, ";
        else
            out << r.GetStdDev() << ", ";
        if (r.GetMin() == FLT_MAX)
            out << "N/A, ";
        else
            out << r.GetMin()    << ", ";
        if (r.GetMax() == FLT_MAX)
            out << "N/A, ";
        else
            out << r.GetMax()    << ", ";

        out << endl;
    }
    out << endl;

    out.close();
}

/****************************************************************************
* <b>Function:</b> ResultDatabase::IsFileEmpty()
*
* <b>Purpose:</b> Returns whether a file is empty - used as a helper for CSV printing.
*
* @param file  The input file to check for emptiness
*
* @author Jeffrey Young
* @date August 28th, 2014
*
* <b>Modifications:</b>
*****************************************************************************/

bool ResultDatabase::IsFileEmpty(string fileName)
{
      bool fileEmpty;

      ifstream file(fileName.c_str());

      //If the file doesn't exist it is by definition empty
      if(!file.good())
      {
        return true;
      }
      else
      {
        fileEmpty = (bool)(file.peek() == ifstream::traits_type::eof());
        file.close();

	return fileEmpty;
      }

      //Otherwise, return false
        return false;
}

/****************************************************************************
* <b>Function:</b> ResultDatabase::GetResultsForTest()
*
* <b>Purpose:<b> Returns a vector of results for just one test name.
*
* @param test the name of the test results to search for
*
* @author Jeremy Meredith
* @date December 3, 2010
*
* <b>Modifications:</b>
*****************************************************************************/
vector<Result>
ResultDatabase::GetResultsForTest(const string &test)
{
    // get only the given test results
    vector<Result> retval;
    for (int i=0; i<results.size(); i++)
    {
        Result &r = results[i];
        if (r.test == test)
            retval.push_back(r);
    }
    return retval;
}

/****************************************************************************
* <b>Function:</b> ResultDatabase::GetResults()
*
* <b>Purpose:<b> Returns all the results.
*
* @param Void
*
* @author Jeremy Meredith
* @date December 3, 2010
*
* <b>Modifications:</b>
****************************************************************************/
const vector<Result> &
ResultDatabase::GetResults() const
{
    return results;
}
