cmake_minimum_required(VERSION 3.5)
project(gcodeUploader)

include(CMakeLocal.cmake)

add_compile_options(-Wall -Wno-unused-variable -Wno-strict-aliasing -std=c++11)

set(SOURCE_FILES
        main.cpp
        uploader_form.cpp
        uploader_form.hpp
        repetier_client.cpp
        repetier_client.hpp
        resource.rc)

find_package(Boost COMPONENTS system thread  REQUIRED)

find_path(nana_INCLUDE_DIRS nana/config.hpp HINTS ${nana_ROOT_DIR}/include)
find_library(nana_LIBRARIES NAMES nana libnana HINTS ${nana_ROOT_DIR})

find_path(asio_INCLUDE_DIRS asio.hpp HINTS ${asio_ROOT_DIR}/include)
set(asio_DEFINITIONS ASIO_STANDALONE)

find_path(websocketpp_INCLUDE_DIRS websocketpp/client.hpp HINTS ${websocketpp_ROOT_DIR})

add_executable(gcodeUploader ${SOURCE_FILES})
target_compile_definitions(gcodeUploader PRIVATE ${asio_DEFINITIONS})
target_include_directories(gcodeUploader PRIVATE ${Boost_INCLUDE_DIRS} ${nana_INCLUDE_DIRS} ${asio_INCLUDE_DIRS} ${websocketpp_INCLUDE_DIRS})
target_link_libraries(gcodeUploader ${Boost_LIBRARIES} ${nana_LIBRARIES})
if(WIN32)
    target_link_libraries(gcodeUploader ws2_32)
endif()