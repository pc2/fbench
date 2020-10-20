/*
    Author: Akhtar, Junaid
    E-mail:  junaida@mail.uni-paderborn.de
    Date:   2020/03/16
*/

#include <gtest/gtest.h>
#include "common/device.h"

BenchmarkOptions t_options;

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    try
    {
        t_options = parseBenchmarkOptions(argc, argv);
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        // Test Fails
    }

    catch (...)
    {
        std::cerr << "unrecognized exception caught" << std::endl;
        // Test Fails
    }
    ::testing::AddGlobalTestEnvironment(new OptionsParserEnvironment);
    return RUN_ALL_TESTS();
}