cmake_minimum_required(VERSION 3.6)
project(website_downloader)

set(CMAKE_CXX_STANDARD 11)
set(SOURCE_FILES main.c
        lib/slre/slre.h
        lib/slre/slre.c
        lib/extractor.h
        lib/includes.h
        lib/downloader.h
        lib/file_saver.h)

add_executable(fetcher ${SOURCE_FILES})