cmake_minimum_required(VERSION 3.6)
project(website_downloader)

set(CMAKE_CXX_STANDARD 11)

include(FindPkgConfig)

pkg_check_modules(LIBMYSQLCLIENT REQUIRED mysqlclient)

foreach(FLAG ${LIBMYSQLCLIENT_CFLAGS_OTHER})
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FLAG}")
endforeach()

# OpenSSL
find_package(PkgConfig REQUIRED)
pkg_search_module(OPENSSL REQUIRED openssl)

if( OPENSSL_FOUND )
    include_directories(${OPENSSL_INCLUDE_DIRS})
    message(STATUS "Using OpenSSL ${OPENSSL_VERSION}")
endif()

# Threads
find_package (Threads)

set(SOURCE_FILES lib/slre/slre.h
        lib/slre/slre.c
        lib/extractor.h
        lib/includes.h
        lib/downloader.h
        lib/file_saver.h
        lib/util.h
        lib/database.h
        lib/ini/ini.h
        lib/ini/ini.c
        lib/config.h
        lib/mthread.h)

add_executable(fetcher main.c ${SOURCE_FILES})
target_include_directories(fetcher PRIVATE ${LIBMYSQLCLIENT_INCLUDE_DIRS})
target_include_directories(fetcher PRIVATE ${OPENSSL_INCLUDE_DIR})

target_link_libraries(fetcher PRIVATE ${LIBMYSQLCLIENT_LIBRARIES} ${OPENSSL_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

add_executable(server server.c ${SOURCE_FILES})
target_include_directories(server PRIVATE ${LIBMYSQLCLIENT_INCLUDE_DIRS})
target_link_libraries(server PRIVATE ${LIBMYSQLCLIENT_LIBRARIES})
