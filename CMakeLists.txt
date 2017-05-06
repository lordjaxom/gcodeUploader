cmake_minimum_required(VERSION 3.5)
project(gcodeUploader)

include(CMakeLocal.cmake)

set(LIB_SOURCE_FILES
        repetier.cpp
        repetier.hpp
        repetier_action.hpp
        repetier_client.cpp
        repetier_client.hpp
        repetier_definitions.cpp
        repetier_definitions.hpp
        printer_service.cpp
        printer_service.hpp
        std/optional.hpp
        std/filesystem.hpp
        conversion.hpp
        http.cpp
        http.hpp
        string.cpp
        string.hpp
        std/variant.hpp)

set(GCT_SOURCE_FILES
        utf8.cpp
        utf8.hpp
        wx_generated.cpp
        wx_generated.h
        wx_resource.rc
        wx_app.cpp
        wx_app.hpp
        wx_uploadframe.cpp
        wx_uploadframe.hpp
        wx_clientptr.hpp
        wx_explorerframe.cpp
        wx_explorerframe.hpp)

set(CMAKE_CXX_STANDARD 14)

find_package(Boost 1.60.0 COMPONENTS ""  REQUIRED)
find_package(wxWidgets COMPONENTS core base REQUIRED)
set(wxWidgets_DEFINITIONS ${wxWidgets_DEFINITIONS} -DwxUSE_UNICODE=0)

find_path(asio_INCLUDE_DIRS asio.hpp HINTS ${ASIO_ROOT}/include)
set(asio_DEFINITIONS ASIO_STANDALONE)

find_path(websocketpp_INCLUDE_DIRS websocketpp/client.hpp HINTS ${WEBSOCKETPP_ROOT})
set(websocketpp_DEFINITIONS _WEBSOCKETPP_CPP11_THREAD_)

find_path(json_INCLUDE_DIRS json.hpp HINTS ${JSON_ROOT}/src)
find_path(variant_INCLUDE_DIRS mpark/variant.hpp HINTS ${VARIANT_ROOT}/include)

if(MINGW)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wa,-mbig-obj")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mwindows")
endif()

add_library(gcodeLib OBJECT ${LIB_SOURCE_FILES})
target_compile_definitions(gcodeLib PRIVATE ${asio_DEFINITIONS} ${websocketpp_DEFINITIONS})
target_include_directories(gcodeLib PRIVATE ${Boost_INCLUDE_DIRS} ${asio_INCLUDE_DIRS} ${websocketpp_INCLUDE_DIRS} ${json_INCLUDE_DIRS} ${variant_INCLUDE_DIRS})

add_executable(gcodeTool $<TARGET_OBJECTS:gcodeLib> ${SOURCE_FILES} ${GCT_SOURCE_FILES})
target_compile_definitions(gcodeTool PRIVATE WIN32_LEAN_AND_MEAN ${wxWidgets_DEFINITIONS} ${asio_DEFINITIONS} ${websocketpp_DEFINITIONS})
target_include_directories(gcodeTool PRIVATE ${Boost_INCLUDE_DIRS} ${wxWidgets_INCLUDE_DIRS} ${asio_INCLUDE_DIRS} ${websocketpp_INCLUDE_DIRS} ${json_INCLUDE_DIRS} ${variant_INCLUDE_DIRS})
target_link_libraries(gcodeTool ${Boost_LIBRARIES} ${wxWidgets_LIBRARIES})
if (WIN32)
    target_link_libraries(gcodeTool ws2_32 stdc++fs version shlwapi setupapi)

    add_custom_command(TARGET gcodeTool POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${wxWidgets_ROOT_DIR}/lib/gcc_dll/wxbase310_gcc_custom.dll $<TARGET_FILE_DIR:gcodeTool>)
    add_custom_command(TARGET gcodeTool POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${wxWidgets_ROOT_DIR}/lib/gcc_dll/wxmsw310_core_gcc_custom.dll $<TARGET_FILE_DIR:gcodeTool>)

endif()