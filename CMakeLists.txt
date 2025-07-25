cmake_minimum_required(VERSION 3.31)
project(sippy)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(looper)

add_library(sippy STATIC
        include/sip/types.h
        include/sip/message.h
        include/sip/headers.h
        include/sip/bodies.h

        src/util/meta.h
        src/serialization/reader.h
        src/serialization/matchers.h
        src/sip/types_storage.h
        src/sip/reader.h
        src/sip/writer.h

        src/serialization/reader.cpp
        src/sip/types.cpp
        src/sip/message.cpp
        src/sip/headers_read_write.cpp
        src/sip/bodies_read_write.cpp
        src/sip/types_storage.cpp
        src/sip/reader.cpp
        src/sip/writer.cpp)
target_link_libraries(sippy looper)
target_include_directories(sippy
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src)
