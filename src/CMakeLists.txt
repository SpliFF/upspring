add_executable (Upspring)

set(CXX_STANDARD_REQUIRED ON)
set(CXX_STANDARD 20)
set(CXX_EXTENSIONS ON)

if (UNIX AND NOT APPLE)
    target_compile_definitions(Upspring PUBLIC HAVE_SCANDIR=1 HAVE_SCANDIR_POSIX=1)
elseif (WIN32)
    target_compile_definitions(Upspring PUBLIC _HAS_STD_BYTE=0 HAVE_SCANDIR=1)
endif ()

find_package(GLEW REQUIRED)
find_package(DevIL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Boost REQUIRED COMPONENTS regex)
find_package(OpenGL REQUIRED)
find_package(Lua REQUIRED)

add_subdirectory(swig)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../scripts DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries (Upspring fltk2)

target_link_libraries (Upspring 
    ZLIB::ZLIB
    DevIL::IL
    DevIL::ILU
    DevIL::ILUT
    GLEW::GLEW
    OpenGL::GL
    OpenGL::GLU
    lib3ds
)

target_link_libraries (Upspring
    ${Boost_LIBRARIES}
    ${LUA_LIBRARIES}
)

# TODO(jochumdev): I hate those manual directories here.
target_include_directories(Upspring 
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}" 
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../vendor/lib3ds/src"
)

target_sources(Upspring PRIVATE
    AnimTrackEditor.cpp
    Animation.cpp
    AnimationUI.cpp
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
    math/Mathlib.cpp
    nv_dds.cpp
    pch.cpp
)