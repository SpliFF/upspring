CMAKE_MINIMUM_REQUIRED(VERSION 3.16.0)

if (POLICY CMP0072)
  cmake_policy(SET CMP0072 NEW)
  set(OpenGL_GL_PREFERENCE "GLVND")
endif()

set(CXX_STANDARD_REQUIRED ON)
set(CXX_STANDARD 20)
set(CXX_EXTENSIONS ON)

if ((CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU") AND NOT CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
  set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-Wno-unused-parameter -Wno-unused-label -fno-strict-aliasing -Wno-deprecated -Wall -DUSE_IK -O2 -fpermissive -Wint-to-pointer-cast")
  set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-Wdelete-non-virtual-dtor -Wno-unused-parameter -Wno-unused-label -fno-strict-aliasing -Wno-deprecated -Wall -DUSE_IK -O2 -fpermissive -Wint-to-pointer-cast -D_HAS_STD_BYTE=0")

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

elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
  # using Intel C++
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        # clang-cl
    else ()
        # using Visual Studio C++
    endif ()
endif()

project(Upspring LANGUAGES CXX C)

add_subdirectory(vendor)
add_subdirectory(src)