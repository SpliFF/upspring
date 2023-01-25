add_library(FLNativeFileChooser)

target_compile_definitions(FLNativeFileChooser PUBLIC FLTK2)

target_link_libraries (FLNativeFileChooser 
    fltk2
)

target_include_directories(FLNativeFileChooser 
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../fltk2"
)

target_link_directories(FLNativeFileChooser
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../fltk2/lib"
)

if(UNIX AND NOT APPLE) 
    target_sources(FLNativeFileChooser PRIVATE
        Fl_Native_File_Chooser.cxx
        Fl_Native_File_Chooser_FLTK.cxx
    )
elseif(APPLE)
    target_sources(FLNativeFileChooser PRIVATE
        Fl_Native_File_Chooser.cxx
        Fl_Native_File_Chooser_MAC.cxx
    )
elseif(WIN32)
    target_sources(FLNativeFileChooser PRIVATE
        Fl_Native_File_Chooser.cxx
        Fl_Native_File_Chooser_WIN32.cxx
    )
endif()