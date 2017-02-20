cmake_minimum_required(VERSION 3.5)
project(gcodeUploader)

include(CMakeLocal.cmake)

set(LIB_SOURCE_FILES
        json.cpp
        json.hpp
        repetier.cpp
        repetier.hpp
        repetier_action.hpp
        repetier_client.cpp
        repetier_client.hpp
        repetier_definitions.cpp
        repetier_definitions.hpp
        printer_service.cpp
        printer_service.hpp
        utility.hpp
        std_optional.hpp
        std_filesystem.hpp)

set(GCU_SOURCE_FILES
        main.cpp
        commandline.cpp
        commandline.hpp
        uploader_form.cpp
        uploader_form.hpp
        resource.rc)

set(GCT_SOURCE_FILES
        wx_generated.cpp
        wx_generated.h
        wx_resource.rc
        wx_app.cpp
        wx_app.hpp wx_uploadframe.cpp wx_uploadframe.hpp)

set(CMAKE_CXX_STANDARD 14)

find_package(Boost 1.60.0 COMPONENTS ""  REQUIRED)
find_package(wxWidgets COMPONENTS core base REQUIRED)

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

add_library(gcodeLib OBJECT ${LIB_SOURCE_FILES})
target_compile_definitions(gcodeLib PRIVATE ${asio_DEFINITIONS} ${websocketpp_DEFINITIONS})
target_include_directories(gcodeLib PRIVATE ${Boost_INCLUDE_DIRS} ${asio_INCLUDE_DIRS} ${websocketpp_INCLUDE_DIRS} ${jsoncpp_INCLUDE_DIRS})

add_executable(gcodeUploader $<TARGET_OBJECTS:gcodeLib> ${GCU_SOURCE_FILES})
target_compile_definitions(gcodeUploader PRIVATE ${asio_DEFINITIONS} ${websocketpp_DEFINITIONS})
target_include_directories(gcodeUploader PRIVATE ${Boost_INCLUDE_DIRS} ${nana_INCLUDE_DIRS} ${asio_INCLUDE_DIRS} ${websocketpp_INCLUDE_DIRS} ${jsoncpp_INCLUDE_DIRS})
target_link_libraries(gcodeUploader ${Boost_LIBRARIES} ${nana_LIBRARIES} ${jsoncpp_LIBRARIES})
if(WIN32)
    target_link_libraries(gcodeUploader ws2_32 stdc++fs)
endif()

add_custom_command(TARGET gcodeUploader POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CURL_ROOT} $<TARGET_FILE_DIR:gcodeUploader>)

add_executable(gcodeTool $<TARGET_OBJECTS:gcodeLib> ${SOURCE_FILES} ${GCT_SOURCE_FILES})
target_compile_definitions(gcodeTool PRIVATE WIN32_LEAN_AND_MEAN ${asio_DEFINITIONS} ${websocketpp_DEFINITIONS})
target_include_directories(gcodeTool PRIVATE ${Boost_INCLUDE_DIRS} ${wxWidgets_INCLUDE_DIRS} ${asio_INCLUDE_DIRS} ${websocketpp_INCLUDE_DIRS} ${jsoncpp_INCLUDE_DIRS})
target_link_libraries(gcodeTool ${Boost_LIBRARIES} ${wxWidgets_LIBRARIES} ${jsoncpp_LIBRARIES})
if (WIN32)
    target_link_libraries(gcodeTool ws2_32 stdc++fs version shlwapi setupapi)
endif()