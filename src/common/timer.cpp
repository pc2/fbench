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

/** @file timer.cpp
* @class Timer
*/
#include "timer.h"
#include <stdio.h>
#include <algorithm>
#include <sys/time.h>

using std::cerr;
using std::endl;
using std::max;

// ----------------------------------------------------------------------------

Timer *Timer::instance = NULL;

// ----------------------------------------------------------------------------
static double
DiffTime(const struct TIMEINFO &startTime, const struct TIMEINFO &endTime)
{
    double seconds = double(endTime.tv_sec - startTime.tv_sec) +
                     double(endTime.tv_usec - startTime.tv_usec) / 1000000.;

    return seconds;
}

static void
GetCurrentTimeInfo(struct TIMEINFO &timeInfo)
{
    gettimeofday(&timeInfo, 0);

}



/****************************************************************************
* Constructor: Timer::Timer()
*
* @author Jeremy Meredith
* @date August 9, 2004
*****************************************************************************/
Timer::Timer()
{
    // Initialize some timer methods and reserve some space.
    startTimes.reserve(1000);
    timeLengths.reserve(1000);
    descriptions.reserve(1000);
    currentActiveTimers = 0;
}

/****************************************************************************
* Destructor: Timer::~Timer()
*
* @author Jeremy Meredith
* @date August 9, 2004
****************************************************************************/
Timer::~Timer()
{
    // nothing to do
}

/****************************************************************************
* <b>Method:</b> Timer::Instance()
*
* <b>Purpose:</b> Return the timer singleton.
*
* @author Jeremy Meredith
* @date August 9, 2004
****************************************************************************/
Timer *Timer::Instance()
{
    if (!instance)
    {
        instance = new Timer;
    }
    return instance;
}

/****************************************************************************
* <b>Method:</b> Timer::Start()
*
* <b>Purpose:</b> Start a timer, and return a handle.
*
* @param Void
*
* @author Jeremy Meredith
* @date August  9, 2004
****************************************************************************/
int Timer::Start()
{
    return Instance()->real_Start();
}

/****************************************************************************
* <b>Method:</b> Timer::Stop()
*
* </b>Purpose:</b> Stop a timer and add its length to our list.
*
* @param handle a timer handle returned by Timer::Start
* @param desription a description for the event timed
*
* @author Jeremy Meredith
* @date August 9, 2004
****************************************************************************/
double Timer::Stop(int handle, const std::string &description)
{
    return Instance()->real_Stop(handle, description);
}

/****************************************************************************
* <b>Method:</b> Timer::Insert()
*
* <b>Purpose:</b> Add a user-generated (e.g. calculated) timing to the list.
*
* @param desription a description for the event timed
* @param value the runtime to insert
*
* @author Jeremy Meredith
* @date October 22, 2007
****************************************************************************/
void Timer::Insert(const std::string &description, double value)
{
    Instance()->real_Insert(description, value);
}

/****************************************************************************
* <b>Method:</b> Timer::Dump()
*
* <b>Purpose:<b> Add timings to on ostream.
*
* @param out the stream to print to.
*
* @author Jeremy Meredith
* @date August  9, 2004
****************************************************************************/
void Timer::Dump(std::ostream &out)
{
    return Instance()->real_Dump(out);
}

/****************************************************************************
* <b>Method:</b> Timer::real_Start()
*
* <b>Purpose:</b> The true start routine.
*
* @param Void
*
* @author Jeremy Meredith
* @date August  9, 2004
****************************************************************************/
int Timer::real_Start()
{
    int handle = startTimes.size();
    currentActiveTimers++;

    struct TIMEINFO t;
    GetCurrentTimeInfo(t);
    startTimes.push_back(t);

    return handle;
}

/****************************************************************************
* <b>Method:</b> Timer::real_Stop()
*
* <b>Purpose:</b> The true stop routine.
*
* @param handle a timer handle returned by Timer::Start
* @param desription a description for the event timed
*
* @author Jeremy Meredith
* @date August 9, 2004
****************************************************************************/

double Timer::real_Stop(int handle, const std::string &description)
{
    if ((unsigned int)handle > startTimes.size())
    {
        cerr << "Invalid timer handle '"<<handle<<"'\n";
        exit(1);
    }

    struct TIMEINFO t;
    GetCurrentTimeInfo(t);
    double length = DiffTime(startTimes[handle], t);
    timeLengths.push_back(length);

    char str[2048];
    sprintf(str, "%*s%s", currentActiveTimers*3, " ", description.c_str());
    descriptions.push_back(str);

    currentActiveTimers--;
    return length;
}

/****************************************************************************
* <b>Method:</b> Timer::real_Insert
*
* <b>Purpose:</b> The true insert routine
*
* @param desription a description for the event timed
* @param value the run time to insert
*
* @author Jeremy Meredith
* @date October 22, 2007
****************************************************************************/
void Timer::real_Insert(const std::string &description, double value)
{
#if 0 // can disable inserting just to make sure it isn't broken
    cerr << description << " " << value << endl;
#else
    timeLengths.push_back(value);

    char str[2048];
    sprintf(str, "%*s[%s]",
            (currentActiveTimers+1)*3, " ", description.c_str());
    descriptions.push_back(str);
#endif
}

/****************************************************************************
*<b>Method:</b> Timer::real_Dump()
*
* <b>Purpose:</b> The true dump routine
*
* @param out the stream to print to.
*
* @author Jeremy Meredith
* @date August  9, 2004
****************************************************************************/
void Timer::real_Dump(std::ostream &out)
{
    size_t maxlen = 0;
    for (unsigned int i=0; i<descriptions.size(); i++)
        maxlen = max(maxlen, descriptions[i].length());

    out << "\nTimings\n-------\n";
    for (unsigned int i=0; i<descriptions.size(); i++)
    {
        char desc[10000];
        sprintf(desc, "%-*s", (int)maxlen, descriptions[i].c_str());
        out << desc << " took " << timeLengths[i] << endl;
    }
}


