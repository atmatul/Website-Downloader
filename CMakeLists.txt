cmake_minimum_required(VERSION 3.6)
project(website_downloader)

set(CMAKE_CXX_STANDARD 11)

include(FindPkgConfig)

pkg_check_modules(LIBMYSQLCLIENT REQUIRED mysqlclient)

foreach(FLAG ${LIBMYSQLCLIENT_CFLAGS_OTHER})
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FLAG}")
endforeach()

link_directories(${LIBMYSQLCLIENT_LIBRARY_DIRS})

set(SOURCE_FILES main.c
        lib/slre/slre.h
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
        lib/config.h)

add_executable(fetcher ${SOURCE_FILES})
target_include_directories(fetcher PRIVATE ${LIBMYSQLCLIENT_INCLUDE_DIRS})
target_link_libraries(fetcher PRIVATE ${LIBMYSQLCLIENT_LIBRARIES})
