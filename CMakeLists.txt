cmake_minimum_required(VERSION 3.6)
project(website_downloader)

set(CMAKE_CXX_STANDARD 11)

include(FindPkgConfig)

pkg_check_modules(LIBMYSQLCLIENT REQUIRED mysqlclient)

foreach(FLAG ${LIBMYSQLCLIENT_CFLAGS_OTHER})
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FLAG}")
endforeach()

FIND_PACKAGE(OPENSSL)

include_directories(${OPENSSL_INCLUDE_DIR})
link_directories(${LIBMYSQLCLIENT_LIBRARY_DIRS})

set(SOURCE_FILES lib/slre/slre.h
        lib/slre/slre.c
        lib/extractor.h
        lib/includes.h
        lib/downloader.h
        lib/file_saver.h
        lib/list.h
        lib/util.h
        lib/database.h
        lib/ini/ini.h
        lib/ini/ini.c
        lib/config.h
        lib/mthread.h)

add_executable(fetcher main.c ${SOURCE_FILES})
target_include_directories(fetcher PRIVATE ${LIBMYSQLCLIENT_INCLUDE_DIRS})
target_include_directories(fetcher PRIVATE ${OPENSSL_INCLUDE_DIR})

target_link_libraries(fetcher PRIVATE ${LIBMYSQLCLIENT_LIBRARIES} ${OPENSSL_LIBRARIES})

add_executable(server server.c ${SOURCE_FILES})
target_include_directories(server PRIVATE ${LIBMYSQLCLIENT_INCLUDE_DIRS})
target_link_libraries(server PRIVATE ${LIBMYSQLCLIENT_LIBRARIES})
