cmake_minimum_required (VERSION 3.16)
project (tty2rpi-cpp)

set(CMAKE_CXX_STANDARD 17)

set(SOURCES
    main.cpp
    FontManager.cpp
    Image.cpp
    Socket.cpp
    GameDatabase.cpp
    StringManip.cpp
)

if (CMAKE_SYSTEM_PROCESSOR MATCHES "^arm")
    add_compile_definitions(PI_BUILD)
endif ()

add_executable(tty2rpi-cpp ${SOURCES})

find_package(Freetype REQUIRED)
target_link_libraries(tty2rpi-cpp ${FREETYPE_LIBRARIES})
target_include_directories(tty2rpi-cpp PRIVATE ${FREETYPE_INCLUDE_DIRS})

set(SOCKPP_BUILD_SHARED OFF)
set(SOCKPP_BUILD_STATIC ON)
add_subdirectory(sockpp)

add_dependencies(tty2rpi-cpp sockpp-static)

target_link_libraries(tty2rpi-cpp sockpp-static pthread)
target_include_directories(tty2rpi-cpp PRIVATE
                           ${CMAKE_CURRENT_SOURCE_DIR}/sockpp/include
                           ${CMAKE_CURRENT_BINARY_DIR}/sockpp/generated/include)
