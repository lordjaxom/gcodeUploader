cmake_minimum_required(VERSION 3.5)
project(gcodeUploader)

set(CMAKE_CXX_STANDARD 14)
if(WIN32)
    set(CMAKE_CXX_FLAGS "-Wa,-mbig-obj")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mwindows")
endif()

find_package(Boost 1.66.0 COMPONENTS system coroutine REQUIRED)
set(Boost_DEFINITIONS BOOST_COROUTINES_NO_DEPRECATION_WARNING)

message("Looking for json in ${JSON_ROOT}")
find_path(json_INCLUDE_DIRS nlohmann/json.hpp HINTS "${JSON_ROOT}/include")

message("Looking for lib3dprnet in ${LIB3DPRNET_ROOT}")
find_path(lib3dprnet_INCLUDE_DIRS core/config.hpp HINTS "${LIB3DPRNET_ROOT}")
find_library(lib3dprnet_LIBRARIES 3dprnet HINTS "${LIB3DPRNET_ROOT}/cmake-build-release" "${LIB3DPRNET_ROOT}/cmake-build-debug")

find_package(wxWidgets COMPONENTS core base REQUIRED)
set(wxWidgets_DEFINITIONS ${wxWidgets_DEFINITIONS} -DwxUSE_UNICODE=0)

add_executable(gcodeTool
        utf8.cpp
        utf8.hpp
        wx_generated.cpp
        wx_generated.h
        wx_resource.rc
        wx_app.cpp
        wx_app.hpp
        wx_uploadframe.cpp
        wx_uploadframe.hpp
        wx_clientptr.hpp)
target_compile_definitions(gcodeTool PRIVATE WIN32_LEAN_AND_MEAN ${wxWidgets_DEFINITIONS} ${Boost_DEFINITIONS})
target_include_directories(gcodeTool PRIVATE ${wxWidgets_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS} ${json_INCLUDE_DIRS} ${lib3dprnet_INCLUDE_DIRS})
target_link_libraries(gcodeTool ${wxWidgets_LIBRARIES} ${Boost_LIBRARIES} ${lib3dprnet_LIBRARIES} stdc++fs)
if (WIN32)
    target_link_libraries(gcodeTool ws2_32 version shlwapi setupapi)

#    add_custom_command(TARGET gcodeTool POST_BUILD
#            COMMAND ${CMAKE_COMMAND} -E copy ${wxWidgets_ROOT_DIR}/lib/gcc_dll/wxbase310_gcc_custom.dll $<TARGET_FILE_DIR:gcodeTool>)
#    add_custom_command(TARGET gcodeTool POST_BUILD
#            COMMAND ${CMAKE_COMMAND} -E copy ${wxWidgets_ROOT_DIR}/lib/gcc_dll/wxmsw310_core_gcc_custom.dll $<TARGET_FILE_DIR:gcodeTool>)
endif()