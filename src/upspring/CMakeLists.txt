add_executable (Upspring)
add_dependencies(Upspring BuildFltk2)

find_package(X11 COMPONENTS Xft Xinerama Xcursor Xi Xext)
if(X11_FOUND)
        message(STATUS "X11 found")
else()
        message(STATUS "X11 NOT FOUND") 
endif(X11_FOUND)

find_package(GLEW REQUIRED)
find_package(DevIL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Boost REQUIRED COMPONENTS regex)
find_package(OpenGL REQUIRED)
find_package(Lua REQUIRED)

add_subdirectory(swig)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../../data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../../scripts DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries (Upspring 
    ${X11_LIBRARIES}
    ${X11_Xft_LIB}
    ${X11_Xinerama_LIB}
    ${X11_Xcursor_LIB}
    ${X11_Xi_LIB}

    fltk2
    fltk2_gl
    fltk2_images
)

target_link_libraries (Upspring 
    ZLIB::ZLIB
    DevIL::IL
    DevIL::ILU
    DevIL::ILUT
    GLEW::GLEW
    OpenGL::GL
    OpenGL::GLU
    3ds
)

target_link_libraries (Upspring
    ${Boost_LIBRARIES}
    ${LUA_LIBRARIES}
)

target_include_directories(Upspring 
    PRIVATE "${X11_INCLUDE_DIRS}"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}" 
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../fltk2" 
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../lib3ds/src"
)

target_link_directories(Upspring
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../fltk2/lib"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../lib3ds/lib"
)

target_sources(Upspring PRIVATE
    AnimTrackEditor.cpp
    Animation.cpp
    AnimationUI.cpp
    BackupManager.cpp
    BackupViewerUI.cpp
    CfgParser.cpp
    CurvedSurface.cpp
    DebugTrace.cpp
    Editor.cpp
    EditorUI.cpp
    FileDialog.cpp
    FileIO/3DO.cpp
    FileIO/3DS.cpp
    FileIO/MD5.cpp
    FileIO/OBJ.cpp
    FileIO/S3O.cpp
    FileSearch.cpp
    IK.cpp
    IK_UI.cpp
    Image.cpp
    MdlObject.cpp
    Model.cpp
    ModelDrawer.cpp
    ObjectView.cpp
    PolyMesh.cpp
    RotatorUI.cpp
    TexBuilderUI.cpp
    TexGroupUI.cpp
    Texture.cpp
    TextureBrowser.cpp
    Timeline.cpp
    Tools.cpp
    UVMappingUI.cpp
    Util.cpp
    VertexBuffer.cpp
    View.cpp
    ZipFile.cpp
    creg/Serializer.cpp
    creg/VarTypes.cpp
    creg/creg.cpp
    math/Mathlib.cpp
    nv_dds.cpp
    pch.cpp
)