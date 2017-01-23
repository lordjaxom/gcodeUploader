cmake_minimum_required(VERSION 3.5)
project(gcodeUploader)

include(CMakeLocal.cmake)

set(SOURCE_FILES
        main.cpp
        json.cpp
        json.hpp
        uploader_form.cpp
        uploader_form.hpp
        repetier.cpp
        repetier.hpp
        repetier_action.hpp
        repetier_client.cpp
        repetier_client.hpp
        repetier_definitions.cpp
        repetier_definitions.hpp
        repetier_upload.cpp
        repetier_upload.hpp
        resource.rc)

set(CMAKE_CXX_STANDARD 14)

find_package(Boost 1.60.0 COMPONENTS ""  REQUIRED)

find_path(nana_INCLUDE_DIRS nana/config.hpp HINTS ${NANA_ROOT}/include)
find_library(nana_LIBRARIES NAMES nana libnana HINTS ${NANA_ROOT})

find_path(asio_INCLUDE_DIRS asio.hpp HINTS ${ASIO_ROOT}/include)
set(asio_DEFINITIONS ASIO_STANDALONE)

find_path(websocketpp_INCLUDE_DIRS websocketpp/client.hpp HINTS ${WEBSOCKETPP_ROOT})
set(websocketpp_DEFINITIONS _WEBSOCKETPP_CPP11_THREAD_)

find_path(jsoncpp_INCLUDE_DIRS json/json.h HINTS ${JSONCPP_ROOT}/include)
find_library(jsoncpp_LIBRARIES NAMES jsoncpp libjsoncpp HINTS ${JSONCPP_ROOT}/src/lib_json)

if(MINGW)
    add_compile_options(-Wa,-mbig-obj)
endif()

add_executable(gcodeUploader ${SOURCE_FILES})
target_compile_definitions(gcodeUploader PRIVATE ${asio_DEFINITIONS} ${websocketpp_DEFINITIONS})
target_include_directories(gcodeUploader PRIVATE ${Boost_INCLUDE_DIRS} ${nana_INCLUDE_DIRS} ${asio_INCLUDE_DIRS} ${websocketpp_INCLUDE_DIRS} ${jsoncpp_INCLUDE_DIRS} ${CPPNETLIB_INCLUDE_DIRS})
target_link_libraries(gcodeUploader ${Boost_LIBRARIES} ${nana_LIBRARIES} ${jsoncpp_LIBRARIES} ${CPPNETLIB_LIBRARIES})
if(WIN32)
    target_link_libraries(gcodeUploader ws2_32 stdc++fs)
endif()