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

/** @file optionparser.cpp
*/
#include <string>
#include <vector>
#include <map>
#include "optionparser.h"
#include <sstream>
#include <fstream>
#include <iomanip>

using namespace std;

void Option::print() {

   cout << "Long Name: " << longName << endl;
   cout << "Short Name: " << shortLetter << endl;
   cout << "Default Value: " << defaultValue << endl;
   cout << "Actual Value: " << value << endl;
   cout << "Type: " << type << endl;
   cout << "helpText: " << helpText << endl;

}

OptionParser::OptionParser()
  : helpRequested( false )
{
    addOption("help", OPT_BOOL, "", "print this usage", 'h');
}

void OptionParser::addOption(const string &longName,
                             OptionType type,
                             const string &defaultValue,
                             const string &helpText,
                             char shortLetter)
{
  Option opt;
  opt.longName = longName;
  opt.type = type;
  opt.defaultValue = defaultValue;
  opt.value = defaultValue;
  opt.helpText = helpText;
  opt.shortLetter = shortLetter;
  if (optionMap.count(longName)>0)
      cout << "Internal error: used option long name '"<<longName<<"' twice.\n";
  optionMap[longName] = opt;
  if (shortLetter != '\0')
  {
      if (shortLetterMap.count(shortLetter)>0)
          cout << "Internal error: used option short letter '"
               << shortLetter<<"' twice (for '"
               << shortLetterMap[opt.shortLetter]
               << "' and '"
               << longName <<"')\n";
      shortLetterMap[opt.shortLetter] = opt.longName;
  }
}

bool OptionParser::parse(int argc, const char *const argv[])
{
    vector<string> args;
    for (int i=1; i<argc; i++)
        args.push_back(argv[i]);
    return parse(args);
}


/** <b>Modifications:</b>
*
*   Jeremy Meredith, Thu Nov  4 14:42:18 EDT 2010.
*   Don't print out usage here; count on the caller to do that.
*   The main reason is we parse the options in parallel and
*   don't want every task to print an error or help text.
*/
bool OptionParser::parse(const vector<string> &args) {

   for (int i=0; i<args.size(); i++) {

      //parse arguments
      string temp = args[i];
      if (temp[0] != '-')
      {
         cout << "failure, no leading - in option: " << temp << "\n";
         cout << "Ignoring remaining options" << endl;
         return false;
      }
      else if (temp[0] == '-' && temp[1] == '-') //Long Name argument
      {
         string longName = temp.substr(2);
         if (optionMap.find(longName) == optionMap.end()) {
            cout << "Option not recognized: " << temp << endl;
            cout << "Ignoring remaining options" << endl;
            return false;
         }
         if (optionMap[longName].type == OPT_BOOL) {
            //Option is bool and is flagged true
            optionMap[longName].value = "true";
         } else {
            if (i+1 >= args.size()) {
               cout << "failure, option: " << temp << " with no value\n";
               cout << "Ignoring remaining options" << endl;
               return false;
            } else {
               optionMap[longName].value = args[i+1];
               i++;
            }
         }
      }
      else  //Short name argument
      {
          int nopts = temp.length()-1;
          for (int p=0; p<nopts; p++)
          {
              char shortLetter = temp[p+1];
              if (shortLetterMap.find(shortLetter) == shortLetterMap.end()) {
                  cout << "Option: " << temp << " not recognized.\n";
                  cout << "Ignoring remaining options" << endl;
                  return false;
              }
              string longName = shortLetterMap[shortLetter];

              if (optionMap[longName].type == OPT_BOOL) {
                  //Option is bool and is flagged true
                  optionMap[longName].value = "true";
              } else {
                  if (i+1 >= args.size() || p < nopts-1)
                  {
                      //usage();
                      cout << "failure, option: -" << shortLetter << " with no value\n";
                      cout << "Ignoring remaining options" << endl;
                      return false;
                  }
                  else
                  {
                      optionMap[longName].value = args[i+1];
                      i++;
                  }
              }
          }
      }
   }

   if (getOptionBool("help"))
   {
       helpRequested = true;
       return false;
   }

   return true;
}

void OptionParser::print() const {

   vector<string> printed;

    OptionMap::const_iterator i = optionMap.begin();
   cout << "Printing Options" << endl;
   while(i != optionMap.end()) {
      Option o = i->second;
      bool skip = false;
      for (int j=0; j<printed.size(); j++) {
         if (printed[j] == o.longName) skip = true;
      }
      if (!skip) {
        printed.push_back(o.longName);
        o.print();
        i++;
        cout << "---------------------" << endl;
      } else { i++; }
   }

}

bool OptionParser::getOptionBool(const string &name) const {

   int retVal;

    OptionMap::const_iterator iter = optionMap.find( name );
   if (iter == optionMap.end()) {
     cout << "getOptionBool: option name \"" << name << "\" not recognized.\n";
     return false;
   }

   return (iter->second.value == "true");
}

void OptionParser::printHelp(const string &optionName) const {

    OptionMap::const_iterator iter = optionMap.find( optionName );
   if (iter == optionMap.end()) {
     cout << "printHelp: option name \"" << optionName << "\" not recognized.\n";
   } else {
      cout << iter->second.helpText;
   }
}

void OptionParser::usage() const {


   string type;
   cout << "Usage: benchmark ";
    OptionMap::const_iterator j = optionMap.begin();

   Option jo = j->second;
   cout << "[--" << jo.longName << " ";

   if (jo.type == OPT_INT || jo.type == OPT_FLOAT)
      type = "number";
   else if (jo.type == OPT_BOOL)
      type = "";
   else if (jo.type == OPT_STRING)
      type = "value";
   else if (jo.type == OPT_VECFLOAT || jo.type == OPT_VECINT)
      type = "n1,n2,...";
   else if (jo.type == OPT_VECSTRING)
      type = "value1,value2,...";
   cout << type << "]" << endl;

   while (++j !=optionMap.end()) {
      jo = j->second;
      cout << "                 [--" << jo.longName << " ";

      if (jo.type == OPT_INT || jo.type == OPT_FLOAT)
          type = "number";
      else if (jo.type == OPT_BOOL)
          type = "";
      else if (jo.type == OPT_STRING)
         type = "value";
      else if (jo.type == OPT_VECFLOAT || jo.type == OPT_VECINT)
          type = "n1,n2,...";
      else if (jo.type == OPT_VECSTRING)
          type = "value1,value2,...";

      cout << type << "]" << endl;
   }

   cout << endl;
   cout << "Available Options: " << endl;
    OptionMap::const_iterator i = optionMap.begin();
   while(i != optionMap.end()) {
      Option o = i->second;
      cout << "    ";
      if (o.shortLetter)
          cout << "-" << o.shortLetter << ", ";
      else
          cout << "    ";
      cout << setiosflags(ios::left) << setw(25)
           << "--" + o.longName + "    " << o.helpText << endl;

      i++;
   }

}
