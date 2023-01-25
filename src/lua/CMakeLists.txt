add_custom_target(BuildLua
    COMMAND make linux
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Orginal lua makefile target"
)

add_library(lua STATIC IMPORTED)
target_include_directories(lua INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)