cmake_minimum_required (VERSION 3.16)
project (tty2rpi-cpp)

set(SOURCES
    main.cpp
)

if (CMAKE_SYSTEM_PROCESSOR MATCHES "^arm")
    add_compile_definitions(PI_BUILD)
endif ()

add_executable(tty2rpi-cpp ${SOURCES} ${EXTRA_LIBS})

find_package(Freetype REQUIRED)
target_link_libraries(tty2rpi-cpp ${FREETYPE_LIBRARIES})
target_include_directories(tty2rpi-cpp PRIVATE ${FREETYPE_INCLUDE_DIRS})
set(EXTRA_LIBS freetype)