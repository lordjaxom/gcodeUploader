cmake_minimum_required(VERSION 3.5)
project(gcodeUploader)

include(CMakeLocal.cmake)

#add_compile_options(-Wall -Wno-unused-variable -Wno-strict-aliasing -std=c++14 -pthread)

set(SOURCE_FILES
        main.cpp
        uploader_form.cpp
        uploader_form.hpp
        repetier_client.cpp
        repetier_client.hpp
        resource.rc
        repetier.cpp
        repetier.hpp
        repetier_definitions.hpp
        repetier_connection.cpp
        repetier_connection.hpp
        repetier_collator.cpp
        repetier_collator.hpp
        repetier_action.cpp
        repetier_action.hpp
        json.cpp
        json.hpp)

set(CMAKE_CXX_STANDARD 14)

find_package(Boost COMPONENTS ""  REQUIRED)

find_path(nana_INCLUDE_DIRS nana/config.hpp HINTS ${nana_ROOT_DIR}/include)
find_library(nana_LIBRARIES NAMES nana libnana HINTS ${nana_ROOT_DIR})

find_path(asio_INCLUDE_DIRS asio.hpp HINTS ${asio_ROOT_DIR}/include)
set(asio_DEFINITIONS ASIO_STANDALONE)

find_path(websocketpp_INCLUDE_DIRS websocketpp/client.hpp HINTS ${websocketpp_ROOT_DIR})
set(websocketpp_DEFINITIONS _WEBSOCKETPP_CPP11_THREAD_)

find_path(jsoncpp_INCLUDE_DIRS json/json.h HINTS ${JSONCPP_ROOT}/include)
find_library(jsoncpp_LIBRARIES NAMES jsoncpp libjsoncpp HINTS ${JSONCPP_ROOT})

message("json include: ${jsoncpp_INCLUDE_DIRS}")

add_executable(gcodeUploader ${SOURCE_FILES})
target_compile_definitions(gcodeUploader PRIVATE ${asio_DEFINITIONS} ${websocketpp_DEFINITIONS})
target_include_directories(gcodeUploader PRIVATE ${Boost_INCLUDE_DIRS} ${nana_INCLUDE_DIRS} ${asio_INCLUDE_DIRS} ${websocketpp_INCLUDE_DIRS} ${jsoncpp_INCLUDE_DIRS})
target_link_libraries(gcodeUploader ${Boost_LIBRARIES} ${nana_LIBRARIES} ${jsoncpp_LIBRARIES})
if(WIN32)
    target_link_libraries(gcodeUploader ws2_32 stdc++fs)
endif()