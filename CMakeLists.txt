CMAKE_MINIMUM_REQUIRED( VERSION 3.25 )

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -g -DUSE_IK -O2   -Wno-unused-parameter -Wno-unused-label -fno-strict-aliasing -Wno-deprecated -Wall -g -DUSE_IK -O2")

IF (UPSPRING_ASAN)
  MESSAGE(STATUS "AddressSanitizer enabled")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")
ENDIF()

IF (UPSPRING_UBSAN)
  MESSAGE(STATUS "Undefined Behavior sanitizer enabled")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined,shift,shift-exponent,integer-divide-by-zero,unreachable,vla-bound,null,return,signed-integer-overflow,bounds,float-divide-by-zero,float-cast-overflow,nonnull-attribute,returns-nonnull-attribute,bool,enum,vptr,pointer-overflow,builtin -fno-sanitize-recover=all")
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=undefined,shift,shift-exponent,integer-divide-by-zero,unreachable,vla-bound,null,return,signed-integer-overflow,bounds,float-divide-by-zero,float-cast-overflow,nonnull-attribute,returns-nonnull-attribute,bool,enum,vptr,pointer-overflow,builtin -fno-sanitize-recover=all")
ENDIF()

PROJECT ( Upspring LANGUAGES CXX )

add_subdirectory(src)