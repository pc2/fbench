/*
    Author: Akhtar, Junaid
    E-mail:  junaida@mail.uni-paderborn.de
    Date:   2020/04/21
*/

#ifndef BASETEST_H_
#define BASETEST_H_

#include <gtest/gtest.h>

using namespace std;
using ::testing::Test;

// Generic Implementation of BaseFixtureTest for Resuability
class BaseTestFixture : public Test
{
protected:
    // The UTC time (in seconds) when the test starts
    time_t start_time_;

    // Record the start time of Unit Test Execution
    void SetUp() override
    {
        start_time_ = time(nullptr);
    }

    // TearDown() is invoked immediately after a test finishes.  Here we check if the test was too slow.
    void TearDown() override
    {
        // Gets the time when the test finishes
        const time_t end_time = time(nullptr);

        // Asserts that the test took no more than ~5 seconds.
        EXPECT_TRUE(end_time - start_time_ <= 5) << "The test took too long.";
    }
};

#endif
