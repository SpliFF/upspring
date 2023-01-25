CMAKE_MINIMUM_REQUIRED( VERSION 3.25 )

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -g -DUSE_IK -O2   -Wno-unused-parameter -Wno-unused-label -fno-strict-aliasing -Wno-deprecated -Wall -g -DUSE_IK -O2")

PROJECT ( Upspring LANGUAGES CXX )

add_subdirectory(src)