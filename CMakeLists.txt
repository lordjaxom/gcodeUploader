cmake_minimum_required(VERSION 3.5)
project(gcodeUploader)

cmake_host_system_information(RESULT HOSTNAME QUERY HOSTNAME)
include("CMakeLocal.${HOSTNAME}.cmake")

find_package(Boost 1.66.0 COMPONENTS system coroutine REQUIRED)
set(Boost_DEFINITIONS BOOST_COROUTINES_NO_DEPRECATION_WARNING)

find_path(json_INCLUDE_DIRS nlohmann/json.hpp HINTS "${JSON_ROOT}/include")

find_path(lib3dprnet_INCLUDE_DIRS core/config.hpp HINTS "${LIB3DPRNET_ROOT}")
if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    set(lib3dprnet_LIB_DIR "${LIB3DPRNET_ROOT}/cmake-build-debug${LIB3DPRNET_BUILD_SUFFIX}")
elseif("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    set(lib3dprnet_LIB_DIR "${LIB3DPRNET_ROOT}/cmake-build-release${LIB3DPRNET_BUILD_SUFFIX}")
else()
    message(FATAL_ERROR "Unknown build type ${CMAKE_BUILD_TYPE}")
endif()
find_library(lib3dprnet_LIBRARIES 3dprnet HINTS "${lib3dprnet_LIB_DIR}")

find_package(wxWidgets COMPONENTS core base REQUIRED)
set(wxWidgets_DEFINITIONS ${wxWidgets_DEFINITIONS} -DwxUSE_UNICODE=0)

set(CMAKE_CXX_STANDARD 14)
if(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wa,-mbig-obj -m64")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mwindows -m64")
endif()

add_executable(gcodeTool
        wx_generated.cpp
        wx_generated.h
        wx_resource.rc
        wx_app.cpp
        wx_uploadframe.cpp
        wx_uploadframe.hpp
        wx_clientptr.hpp)
target_compile_definitions(gcodeTool PRIVATE WIN32_LEAN_AND_MEAN ${wxWidgets_DEFINITIONS} ${Boost_DEFINITIONS})
target_include_directories(gcodeTool PRIVATE ${wxWidgets_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS} ${json_INCLUDE_DIRS} ${lib3dprnet_INCLUDE_DIRS})
target_link_libraries(gcodeTool ${wxWidgets_LIBRARIES} ${Boost_LIBRARIES} ${lib3dprnet_LIBRARIES} stdc++fs)
if (WIN32)
    target_link_libraries(gcodeTool ws2_32 version shlwapi setupapi)

    file(GLOB LIBS "${CMAKE_CURRENT_LIST_DIR}/lib/*.dll")
    foreach(LIB "${LIBS}")
        add_custom_command(TARGET gcodeTool POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy ${LIB} $<TARGET_FILE_DIR:gcodeTool>)
    endforeach()
    add_custom_command(TARGET gcodeTool POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${lib3dprnet_LIB_DIR}/lib3dprnet.dll $<TARGET_FILE_DIR:gcodeTool>)
endif()