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

/** @file optionparser.h
*/
#ifndef OPTION_PARSER_H
#define OPTION_PARSER_H

#include <iostream>
#include <string>
#include <vector>
#include <map>

using namespace std;

enum OptionType {OPT_FLOAT, OPT_INT, OPT_STRING, OPT_BOOL,
                 OPT_VECFLOAT, OPT_VECINT, OPT_VECSTRING};

/****************************************************************************
* @class Option
*
* <b>Purpose:</b> Encapsulation of a single option, to be used by an option parser.
*
* @author Kyle Spafford
* @date August 4, 2009
*****************************************************************************/
class Option {

  public:

   string longName;
   char   shortLetter;
   string defaultValue;
   string value;
   OptionType type;
   string helpText;

   void print();
};

typedef std::map<std::string, Option> OptionMap;
/****************************************************************************
* @class OptionParser
*
* <b>Purpose:</b> Class used to specify and parse command-line options to programs.
*
* @author Kyle Spafford
* @date August 4, 2009
*****************************************************************************/
class OptionParser
{
  private:

    OptionMap optionMap;
    map<char, string>   shortLetterMap;

    bool helpRequested;

  public:
    friend class BenchmarkOptionsParser;

    OptionParser();
    void addOption(const string &longName,
                   OptionType type,
                   const string &defaultValue,
                   const string &helpText = "No help specified",
                   char shortLetter = '\0');

    void print() const;

    //Returns false on failure, true on success
    bool parse(int argc, const char *const argv[]);
    bool parse(const vector<string> &args);

    //Accessors for options
    bool        getOptionBool(const string &name) const;

    vector<long long>     getOptionVecInt(const string &name) const;
    vector<float>         getOptionVecFloat(const string &name) const;
    vector<string>        getOptionVecString(const string &name) const;

    void printHelp(const string &optionName) const;
    void usage() const;

    bool HelpRequested( void ) const    { return helpRequested; }
};

#endif
