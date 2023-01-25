set(UseSWIG_MODULE_VERSION 2)
find_package(SWIG 4.0 REQUIRED)
INCLUDE(${SWIG_USE_FILE})

set(UseSWIG_MODULE_VERSION 2)
set_property(SOURCE upspring.i PROPERTY CPLUSPLUS ON)
# set_property(SOURCE upspring.i PROPERTY SWIG_MODULE_NAME "upspring")

swig_add_library(upspring 
    TYPE OBJECT
    OUTFILE_DIR "../"
    LANGUAGE lua
    SOURCES upspring.i
)
target_include_directories(upspring
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/.."
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../fltk2"
)
set_target_properties(upspring PROPERTIES 
    PREFIX ""
    SWIG_USE_TARGET_INCLUDE_DIRECTORIES ON
)
target_link_libraries(Upspring upspring)