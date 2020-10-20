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


#ifndef TIMER_H
#define TIMER_H

#include <vector>
#include <string>
#include <iostream>

#include <time.h>
#include <sys/timeb.h>

// decide which timer type we are supposed to use
#define TIMEINFO timeval



/****************************************************************************
* @file timer.h
* @class Timer
*
* <b>Purpose:</b> Encapsulated a set of hierarchical timers. Starting a timer
* returns a handle to a timer. Pass this handle, and a description, into the
* timer Stop routine. Timers can nest and output will be displayed in a tree format.
* Externally, Timer represents time in units of seconds.
*
* @author Jeremy Meredith
* @date August 6, 2004
****************************************************************************/
class Timer
{
  public:
    static Timer *Instance();

    static int    Start();

    // Returns time since start of corresponding timer (determined by handle),
    // in seconds.
    static double Stop(int handle, const std::string &descr);
    static void   Insert(const std::string &descr, double value);

    static void   Dump(std::ostream&);

  private:

    int    real_Start();
    double real_Stop(int, const std::string &);
    void   real_Insert(const std::string &descr, double value);
    void   real_Dump(std::ostream&);

    Timer();
    ~Timer();

    static Timer *instance;

    std::vector<TIMEINFO>    startTimes;
    std::vector<double>      timeLengths;
    std::vector<std::string> descriptions;
    int                      currentActiveTimers;
};

#endif
