add_executable (Upspring)

find_package(GLEW REQUIRED)
find_package(DevIL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Boost REQUIRED COMPONENTS regex)
find_package(OpenGL REQUIRED)
find_package(Lua REQUIRED)

# find_package(FLTK REQUIRED)
# if (FLTK_FOUND)
#     message(STATUS "FLTK found")
# endif ()

# if (UNIX)
#     set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -idirafter /usr/include")
#     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -idirafter /usr/include")
# endif ()

add_subdirectory(swig)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../../data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../../scripts DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

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
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../fltk2" 
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../lib3ds/src"
)

target_link_directories(Upspring
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../fltk2/lib"
    PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/../lib3ds/lib"
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