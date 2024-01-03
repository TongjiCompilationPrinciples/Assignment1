cmake_minimum_required(VERSION 3.27)
project(compiling)

set(CMAKE_CXX_STANDARD 17)

add_executable(compiling main.cpp
        Help/Ana.h
        Help/Ana.cpp
        Help/Deceleration.h
        Help/GrammerAna.cpp
        Help/GrammerAna.h
)
