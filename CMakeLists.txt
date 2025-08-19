cmake_minimum_required(VERSION 3.31)
project(sippy)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenSSL REQUIRED)

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
        src/sip/writer.cpp
        src/util/streams.h
        include/sip/requests.h
        src/sip/requests.cpp
        include/sip/responses.h
        src/sip/responses.cpp
        include/sip/auth.h
        src/sip/auth.cpp
        src/util/hash.h
        src/util/hash.cpp
        src/util/base64.h
        src/util/base64.cpp
        src/crypto/milenge.h
        src/crypto/milenge.cpp
        src/util/hex.h
        src/util/hex.cpp
        include/sip/transport.h
        include/sip/account.h
        src/sip/account.cpp
        src/sip/transport.cpp
        include/sip/session.h
        src/sip/session.cpp
        include/sdp/message.h
        include/sdp/types.h
        include/sdp/fields.h
        src/sdp/fields_read_write.cpp
        src/util/string_helper.h
        src/util/string_helper.cpp
        src/sdp/message.cpp
        src/sdp/writer.h
        src/sdp/writer.cpp
        src/sdp/reader.h
        src/sdp/reader.cpp
        src/sdp/types.cpp)
target_link_libraries(sippy PUBLIC looper PRIVATE OpenSSL::SSL OpenSSL::Crypto)
target_include_directories(sippy
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        ${CMAKE_CURRENT_SOURCE_DIR}/src) #PRIVATE

add_executable(client1 client1.cpp client.cpp)
target_link_libraries(client1 sippy looper)
add_executable(client2 client2.cpp client.cpp)
target_link_libraries(client2 sippy looper)
