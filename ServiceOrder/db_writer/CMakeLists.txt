cmake_minimum_required(VERSION 3.2)

project(hl_event_server C CXX)

SET (EXAMPLE_BINARY "db_writer")
set (REDISCPP_FLAGS "-DREDISCPP_HEADER_ONLY=ON")

find_package(OpenSSL)
find_package(Threads)
find_package(ZLIB)
find_package(Poco REQUIRED COMPONENTS Foundation Util Net XML JSON Crypto NetSSL Data)
find_package(CppKafka REQUIRED)


if(NOT ${Poco_FOUND})
    message(FATAL_ERROR "Poco C++ Libraries not found.")
endif()

include_directories(${Poco_INCLUDE_DIRS})
link_directories("/usr/local/lib")

add_executable(${EXAMPLE_BINARY} main.cpp 
                                 config/config.cpp
                                 database/database.cpp
                                 database/user.cpp)

target_include_directories(${EXAMPLE_BINARY} PRIVATE "${CMAKE_BINARY_DIR}")

target_compile_options(${EXAMPLE_BINARY} PRIVATE -Wall -Wextra -pedantic -Werror )

target_link_libraries(${EXAMPLE_BINARY} PRIVATE 
                             ${CMAKE_THREAD_LIBS_INIT} 
                             ${Poco_LIBRARIES}
                             ZLIB::ZLIB
                             "PocoDataMySQL"
                             "rdkafka++"
                             CppKafka::cppkafka)

set_target_properties(${EXAMPLE_BINARY} PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(${EXAMPLE_BINARY} PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)

