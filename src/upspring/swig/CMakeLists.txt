set(UseSWIG_MODULE_VERSION 2)
find_package(SWIG 4.0 REQUIRED)
INCLUDE(${SWIG_USE_FILE})

set_property(SOURCE list.i PROPERTY CPLUSPLUS ON)
set_property(SOURCE upspring.i PROPERTY CPLUSPLUS ON)

swig_add_library(luaUpspring 
    TYPE OBJECT
    LANGUAGE lua 
    SOURCES list.i upspring.i
)
target_include_directories(luaUpspring
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/.."
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../fltk2"
)
set_target_properties(luaUpspring PROPERTIES 
    SWIG_USE_TARGET_INCLUDE_DIRECTORIES ON
)
target_link_libraries(Upspring luaUpspring)