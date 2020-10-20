##
#  Author: Jennifer Faj
#  eMail: jfaj@mail.uni-paderborn.de
#  Date:  2020/02/18
#

# Adding boost (dependency of Benchmark Database & Option Module) 
# Version number can be given after "Boost"
find_package(Boost) 
include_directories(${Boost_INCLUDE_DIRS})

# Add Utilities
add_library(utility utility.cpp)
target_include_directories(utility PUBLIC ../common)

# Add Timer library
add_library(timer timer.cpp)
target_include_directories(timer PUBLIC ../common)

# Add result DB library (Depending on Boost)
add_library(benchmarkdatabase benchmarkdatabase.cpp)
target_include_directories(benchmarkdatabase PUBLIC ../common)
target_sources(benchmarkdatabase PRIVATE
               resultdatabase.cpp)
target_link_libraries(benchmarkdatabase PRIVATE ${Boost_LIBRARIES})

# Add option parser library
add_library(benchmarkoptionsparser benchmarkoptionsparser.cpp)
target_include_directories(benchmarkoptionsparser PUBLIC ../common)
target_sources(benchmarkoptionsparser PRIVATE
               optionparser.cpp)
target_link_libraries(benchmarkoptionsparser PRIVATE ${Boost_LIBRARIES}) 