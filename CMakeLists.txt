# Copyright (c) 2014-2017, Ruslan Baratov (forexample)
# Copyright (c) 2017, David Hirvonen 
# All rights reserved.

cmake_minimum_required(VERSION 3.0)

### Hunter snapshot that will be used ###
include("cmake/HunterGate.cmake")
HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.19.111.tar.gz"
    SHA1 "683bdd1a56711abd24907973665f7f58f792ce5f"
    LOCAL
)

project(eos-dlib-test)

hunter_add_package(dlib)
find_package(dlib CONFIG REQUIRED)

hunter_add_package(eos)
find_package(eos CONFIG REQUIRED)

hunter_add_package(glm)
find_package(glm CONFIG REQUIRED)

hunter_add_package(OpenCV)
find_package(OpenCV CONFIG REQUIRED)

hunter_add_package(cxxopts)
find_package(cxxopts CONFIG REQUIRED)

add_executable(eos-dlib-test eos-dlib-test.cpp)
target_link_libraries(eos-dlib-test PUBLIC dlib::dlib eos::eos glm cxxopts::cxxopts ${OpenCV_LIBS})

install(TARGETS eos-dlib-test DESTINATION bin)
